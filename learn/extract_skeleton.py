import json
from mathutils import Vector, Euler, Matrix
from math import radians, ceil
import numpy as np

class BVH_Node:
    __slots__ = (
        'name',  # bvh joint name
        'parent',  # BVH_Node type or None for no parent
        'children',  # a list of children of this type.
        'rest_head_world',  # worldspace rest location for the head of this node
        'rest_head_local',  # localspace rest location for the head of this node
        'rest_tail_world',  # worldspace rest location for the tail of this node
        'rest_tail_local',  # worldspace rest location for the tail of this node
        'channels',  # list of 6 ints, -1 for an unused channel, otherwise an index for the BVH motion data lines, loc triple then rot triple
        'rot_order',  # a triple of indices as to the order rotation is applied. [0,1,2] is x/y/z - [None, None, None] if no rotation.
        'rot_order_str',  # same as above but a string 'XYZ' format.
        'anim_data',  # a list one tuple's one for each frame. (locx, locy, locz, rotx, roty, rotz), euler rotation ALWAYS stored xyz order, even when native used.
        'has_loc',  # Convenience function, bool, same as (channels[0]!=-1 or channels[1]!=-1 or channels[2]!=-1)
        'has_rot',  # Convenience function, bool, same as (channels[3]!=-1 or channels[4]!=-1 or channels[5]!=-1)
        'index',  # index from the file, not strictly needed but nice to maintain order
        'temp',  # use this for whatever you want
        )

    _eul_order_lookup = {
        (None, None, None): 'XYZ',  # XXX Dummy one, no rotation anyway!
        (0, 1, 2): 'XYZ',
        (0, 2, 1): 'XZY',
        (1, 0, 2): 'YXZ',
        (1, 2, 0): 'YZX',
        (2, 0, 1): 'ZXY',
        (2, 1, 0): 'ZYX',
        }

    def __init__(self, name, rest_head_world, rest_head_local, parent, channels, rot_order, index):
        self.name = name
        self.rest_head_world = rest_head_world
        self.rest_head_local = rest_head_local
        self.rest_tail_world = None
        self.rest_tail_local = None
        self.parent = parent
        self.channels = channels
        self.rot_order = tuple(rot_order)
        self.rot_order_str = BVH_Node._eul_order_lookup[self.rot_order]
        self.index = index

        # convenience functions
        self.has_loc = channels[0] != -1 or channels[1] != -1 or channels[2] != -1
        self.has_rot = channels[3] != -1 or channels[4] != -1 or channels[5] != -1

        self.children = []

        # list of 6 length tuples: (lx,ly,lz, rx,ry,rz)
        # even if the channels aren't used they will just be zero
        #
        self.anim_data = [(0, 0, 0, 0, 0, 0)]

    def __repr__(self):
        return ("BVH name: '%s', rest_loc:(%.3f,%.3f,%.3f), rest_tail:(%.3f,%.3f,%.3f)" %
                (self.name,
                 self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z,
                 self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z))

def sorted_nodes(bvh_nodes):
    bvh_nodes_list = list(bvh_nodes.values())
    bvh_nodes_list.sort(key=lambda bvh_node: bvh_node.index)
    return bvh_nodes_list

def read_bvh(context, file_path, rotate_mode='XYZ', global_scale=1.0):
    # File loading stuff
    # Open the file for importing
    file = open(file_path, 'rU')

    # Seperate into a list of lists, each line a list of words.
    file_lines = file.readlines()
    # Non standard carrage returns?
    if len(file_lines) == 1:
        file_lines = file_lines[0].split('\r')

    # Split by whitespace.
    file_lines = [ll for ll in [l.split() for l in file_lines] if ll]

    # Create hierarchy as empties
    if file_lines[0][0].lower() == 'hierarchy':
        #print 'Importing the BVH Hierarchy for:', file_path
        pass
    else:
        raise Exception("This is not a BVH file")

    bvh_nodes = {None: None}
    bvh_nodes_serial = [None]
    bvh_frame_count = None
    bvh_frame_time = None

    channelIndex = -1

    lineIdx = 0  # An index for the file.
    while lineIdx < len(file_lines) - 1:
        #...
        if file_lines[lineIdx][0].lower() == 'root' or file_lines[lineIdx][0].lower() == 'joint':

            # Join spaces into 1 word with underscores joining it.
            if len(file_lines[lineIdx]) > 2:
                file_lines[lineIdx][1] = '_'.join(file_lines[lineIdx][1:])
                file_lines[lineIdx] = file_lines[lineIdx][:2]

            # MAY NEED TO SUPPORT MULTIPLE ROOTS HERE! Still unsure weather multiple roots are possible?

            # Make sure the names are unique - Object names will match joint names exactly and both will be unique.
            name = file_lines[lineIdx][1]

            #print '%snode: %s, parent: %s' % (len(bvh_nodes_serial) * '  ', name,  bvh_nodes_serial[-1])

            lineIdx += 2  # Increment to the next line (Offset)
            rest_head_local = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale
            lineIdx += 1  # Increment to the next line (Channels)

            # newChannel[Xposition, Yposition, Zposition, Xrotation, Yrotation, Zrotation]
            # newChannel references indices to the motiondata,
            # if not assigned then -1 refers to the last value that will be added on loading at a value of zero, this is appended
            # We'll add a zero value onto the end of the MotionDATA so this always refers to a value.
            my_channel = [-1, -1, -1, -1, -1, -1]
            my_rot_order = [None, None, None]
            rot_count = 0
            for channel in file_lines[lineIdx][2:]:
                channel = channel.lower()
                channelIndex += 1  # So the index points to the right channel
                if channel == 'xposition':
                    my_channel[0] = channelIndex
                elif channel == 'yposition':
                    my_channel[1] = channelIndex
                elif channel == 'zposition':
                    my_channel[2] = channelIndex

                elif channel == 'xrotation':
                    my_channel[3] = channelIndex
                    my_rot_order[rot_count] = 0
                    rot_count += 1
                elif channel == 'yrotation':
                    my_channel[4] = channelIndex
                    my_rot_order[rot_count] = 1
                    rot_count += 1
                elif channel == 'zrotation':
                    my_channel[5] = channelIndex
                    my_rot_order[rot_count] = 2
                    rot_count += 1

            channels = file_lines[lineIdx][2:]

            my_parent = bvh_nodes_serial[-1]  # account for none

            # Apply the parents offset accumulatively
            if my_parent is None:
                rest_head_world = Vector(rest_head_local)
            else:
                rest_head_world = my_parent.rest_head_world + rest_head_local

            bvh_node = bvh_nodes[name] = BVH_Node(name, rest_head_world, rest_head_local, my_parent, my_channel, my_rot_order, len(bvh_nodes) - 1)

            # If we have another child then we can call ourselves a parent, else
            bvh_nodes_serial.append(bvh_node)

        # Account for an end node
        if file_lines[lineIdx][0].lower() == 'end' and file_lines[lineIdx][1].lower() == 'site':  # There is sometimes a name after 'End Site' but we will ignore it.
            lineIdx += 2  # Increment to the next line (Offset)
            rest_tail = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale

            bvh_nodes_serial[-1].rest_tail_world = bvh_nodes_serial[-1].rest_head_world + rest_tail
            bvh_nodes_serial[-1].rest_tail_local = bvh_nodes_serial[-1].rest_head_local + rest_tail

            # Just so we can remove the Parents in a uniform way - End has kids
            # so this is a placeholder
            bvh_nodes_serial.append(None)

        if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0] == '}':  # == ['}']
            bvh_nodes_serial.pop()  # Remove the last item

        # End of the hierarchy. Begin the animation section of the file with
        # the following header.
        #  MOTION
        #  Frames: n
        #  Frame Time: dt
        if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0].lower() == 'motion':
            lineIdx += 1  # Read frame count.
            if (len(file_lines[lineIdx]) == 2 and
                file_lines[lineIdx][0].lower() == 'frames:'):

                bvh_frame_count = int(file_lines[lineIdx][1])

            lineIdx += 1  # Read frame rate.
            if (len(file_lines[lineIdx]) == 3 and
                file_lines[lineIdx][0].lower() == 'frame' and
                file_lines[lineIdx][1].lower() == 'time:'):

                bvh_frame_time = float(file_lines[lineIdx][2])

            lineIdx += 1  # Set the cursor to the first frame

            break

        lineIdx += 1

    # Remove the None value used for easy parent reference
    del bvh_nodes[None]
    # Dont use anymore
    del bvh_nodes_serial

    # importing world with any order but nicer to maintain order
    # second life expects it, which isn't to spec.
    bvh_nodes_list = sorted_nodes(bvh_nodes)

    while lineIdx < len(file_lines):
        line = file_lines[lineIdx]
        for bvh_node in bvh_nodes_list:
            #for bvh_node in bvh_nodes_serial:
            lx = ly = lz = rx = ry = rz = 0.0
            channels = bvh_node.channels
            anim_data = bvh_node.anim_data
            if channels[0] != -1:
                lx = global_scale * float(line[channels[0]])

            if channels[1] != -1:
                ly = global_scale * float(line[channels[1]])

            if channels[2] != -1:
                lz = global_scale * float(line[channels[2]])

            if channels[3] != -1 or channels[4] != -1 or channels[5] != -1:

                rx = radians(float(line[channels[3]]))
                ry = radians(float(line[channels[4]]))
                rz = radians(float(line[channels[5]]))

            # Done importing motion data #
            anim_data.append((lx, ly, lz, rx, ry, rz))
        lineIdx += 1

    # Assign children
    for bvh_node in bvh_nodes_list:
        bvh_node_parent = bvh_node.parent
        if bvh_node_parent:
            bvh_node_parent.children.append(bvh_node)

    # Now set the tip of each bvh_node
    for bvh_node in bvh_nodes_list:

        if not bvh_node.rest_tail_world:
            if len(bvh_node.children) == 0:
                # could just fail here, but rare BVH files have childless nodes
                bvh_node.rest_tail_world = Vector(bvh_node.rest_head_world)
                bvh_node.rest_tail_local = Vector(bvh_node.rest_head_local)
            elif len(bvh_node.children) == 1:
                bvh_node.rest_tail_world = Vector(bvh_node.children[0].rest_head_world)
                bvh_node.rest_tail_local = bvh_node.rest_head_local + bvh_node.children[0].rest_head_local
            else:
                # allow this, see above
                #if not bvh_node.children:
                #	raise Exception("bvh node has no end and no children. bad file")

                # Removed temp for now
                rest_tail_world = Vector((0.0, 0.0, 0.0))
                rest_tail_local = Vector((0.0, 0.0, 0.0))
                for bvh_node_child in bvh_node.children:
                    rest_tail_world += bvh_node_child.rest_head_world
                    rest_tail_local += bvh_node_child.rest_head_local

                bvh_node.rest_tail_world = rest_tail_world * (1.0 / len(bvh_node.children))
                bvh_node.rest_tail_local = rest_tail_local * (1.0 / len(bvh_node.children))

        # Make sure tail isn't the same location as the head.
        if (bvh_node.rest_tail_local - bvh_node.rest_head_local).length <= 0.001 * global_scale:
            print("\tzero length node found:", bvh_node.name)
            bvh_node.rest_tail_local.y = bvh_node.rest_tail_local.y + global_scale / 10
            bvh_node.rest_tail_world.y = bvh_node.rest_tail_world.y + global_scale / 10

    return bvh_nodes, bvh_frame_time, bvh_frame_count

bvh_nodes, bvh_frame_time, bvh_frame_count = read_bvh(None, 'bvh/walk00.bvh')

print(bvh_nodes)

# fp = open('test.json', 'w')
# json.dump(bvh_nodes, fp)

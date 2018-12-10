from os import path
from sys import argv
from bvh_reader import BVHReader


def nodes(fp, node, indent):

    if node.name in ['RightHandThumb1', 'RightInHandIndex', 'RightInHandRing', 'RightInHandPinky',
                     'LeftHandThumb1', 'LeftInHandIndex', 'LeftInHandRing', 'LeftInHandPinky']:
        return

    indent_str = '\t' * indent

    if 'InHandMiddle' in node.name:
        fp.write('%sEnd Site\n' % indent_str)
        fp.write('%s{\n' % indent_str)
        fp.write('%s\tOFFSET %.6f %.6f %.6f\n' %
                 (indent_str, *node.offset))
        fp.write('%s}\n' % indent_str)
        return

    if indent:
        fp.write('%sJOINT %s\n' % (indent_str, node.name))
    else:
        fp.write('%sROOT %s\n' % (indent_str, node.name))

    fp.write('%s{\n' % indent_str)
    fp.write('%s\tOFFSET %.6f %.6f %.6f\n' % (indent_str, *node.offset))

    if indent:
        fp.write('%s\tCHANNELS 3 %s %s %s\n' %
                 (indent_str, *node.channels))
    else:
        fp.write('%s\tCHANNELS 6 %s %s %s %s %s %s\n' % (
            indent_str, *node.channels))

    for child in node.children:
        if child.name != 'End Site':
            nodes(fp, child, indent+1)
        else:
            fp.write('%s\tEnd Site\n' % indent_str)
            fp.write('%s\t{\n' % indent_str)
            fp.write('%s\t\tOFFSET %.6f %.6f %.6f\n' %
                     (indent_str, *child.offset))
            fp.write('%s\t}\n' % indent_str)

    fp.write('%s}\n' % indent_str)


if __name__ == "__main__":

    filename = argv[1]

    if path.exists(filename) is False:
        raise FileNotFoundError()

    root, ext = path.splitext(filename)

    if ext != ".bvh":
        raise Exception()

    br = BVHReader(filename)
    br.read()

    with open('%s_rmfinger%s' % (root, ext), 'w') as ftarget:

        ftarget.write('HIERARCHY\n')

        nodes(ftarget, br._root, 0)

        ftarget.write('MOTION\n')
        ftarget.write('Frames: %d\n' % len(br.motions))
        ftarget.write('Frame Time: 0.041667\n')

        delindices = list(range(17*3+3, 36*3+3))
        delindices.extend(list(range(40*3+3, 59*3+3)))

        for frame in br.motions:

            arr = [x for i, x in enumerate(frame) if i not in delindices]

            ftarget.write(' '.join(map(str, arr)) + '\n')

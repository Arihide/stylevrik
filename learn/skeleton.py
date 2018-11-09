from mathutils import Vector, Matrix, Euler
from math import radians


class Joint():
    def __init__(self):
        # self.basis = Matrix.Identity(3)
        # self.translation = Vector()
        self.transform = Matrix.Identity(4)
        self.global_transform = Matrix.Identity(4)
        self.parent = None
        self.children = []
        self.name = None

    @property
    def translation(self):
        return self.transform.translation

    @translation.setter
    def translation(self, value):
        self.transform.translation = value

    def update_global_transform(self, parent_global=Matrix.Identity(3)):

        if(self.parent == None):
            self.global_transform = self.transform
        else:
            # self.global_transform = self.transform * self.parent.global_transform
            self.global_transform = self.parent.global_transform * self.transform

        for child in self.children:
            child.update_global_transform()


class Skeleton():
    def __init__(self, root):
        self.root = root
        self.joints = []

    def apply_frame(self, frame):
        for i, joint in enumerate(self.joints):
            translation = joint.transform.to_translation()
            joint.transform = Euler((
                radians(frame[i*3+1]),
                radians(frame[i*3+0]),
                radians(frame[i*3+2])
            ), 'ZXY').to_matrix().to_4x4()
            joint.transform.translation = translation


# BVHReader
class BVHReader:
    """Read BioVision Hierarchical (BVH) files.
"""

    def __init__(self, filename):

        self.filename = filename
        # A list of unprocessed tokens (strings)
        self.tokenlist = []
        # The current line number
        self.linenr = 0

        # Root node
        self._root = None
        self._nodestack = []

        self.skeleton = None

        self.motions = []
        self.angvels = []

        # Total number of channels
        self._numchannels = 0

    def onHierarchy(self, root):
        pass

    def onMotion(self, frames, dt):
        pass
        # print(frames)

    def onFrame(self, values):
        self.motions.append(values)
        # print(values)

    # read
    def read(self):
        """Read the entire file.
"""
        self.fhandle = open(self.filename)

        self.readHierarchy()
        self.onHierarchy(self._root)
        self.readMotion()

    # readMotion
    def readMotion(self):
        """Read the motion samples.
"""
        # No more tokens (i.e. end of file)? Then just return
        try:
            tok = self.token()
        except StopIteration:
            return

        if tok != "MOTION":
            raise SyntaxError(
                "Syntax error in line %d: 'MOTION' expected, got '%s' instead" % (self.linenr, tok))

        # Read the number of frames
        tok = self.token()
        if tok != "Frames:":
            raise SyntaxError(
                "Syntax error in line %d: 'Frames:' expected, got '%s' instead" % (self.linenr, tok))

        frames = self.intToken()

        # Read the frame time
        tok = self.token()
        if tok != "Frame":
            raise SyntaxError(
                "Syntax error in line %d: 'Frame Time:' expected, got '%s' instead" % (self.linenr, tok))
        tok = self.token()
        if tok != "Time:":
            raise SyntaxError(
                "Syntax error in line %d: 'Frame Time:' expected, got 'Frame %s' instead" % (self.linenr, tok))

        dt = self.floatToken()

        self.onMotion(frames, dt)

        # Read the channel values
        for i in range(frames):
            s = self.readLine()
            a = s.split()
            if len(a) != self._numchannels:
                raise SyntaxError("Syntax error in line %d: %d float values expected, got %d instead" % (
                    self.linenr, self._numchannels, len(a)))
            values = list(map(lambda x: float(x), a))
            self.onFrame(values)

    # readHierarchy

    def readHierarchy(self):
        """Read the skeleton hierarchy.
"""
        tok = self.token()
        if tok != "HIERARCHY":
            raise SyntaxError(
                "Syntax error in line %d: 'HIERARCHY' expected, got '%s' instead" % (self.linenr, tok))

        tok = self.token()
        if tok != "ROOT":
            raise SyntaxError(
                "Syntax error in line %d: 'ROOT' expected, got '%s' instead" % (self.linenr, tok))

        # self._root = Node(root=True)
        self._root = Joint()
        self.skeleton = Skeleton(self._root)
        self.skeleton.joints.append(self._root)

        self._nodestack.append(self._root)
        self.readNode()

    # readNode
    def readNode(self):
        """Read the data for a node.
"""

        # Read the node name (or the word 'Site' if it was a 'End Site'
        # node)
        name = self.token()

        if name == "Site":
            self.token()
            self.token()
            self.floatToken()
            self.floatToken()
            self.floatToken()
            self.token()
            return

        self._nodestack[-1].name = name

        tok = self.token()
        if tok != "{":
            raise SyntaxError(
                "Syntax error in line %d: '{' expected, got '%s' instead" % (self.linenr, tok))

        while 1:
            tok = self.token()
            if tok == "OFFSET":
                x = self.floatToken()
                y = self.floatToken()
                z = self.floatToken()

                self._nodestack[-1].translation = (x, y, z)
            elif tok == "CHANNELS":
                n = self.intToken()
                channels = []
                for i in range(n):
                    tok = self.token()
                    if tok not in ["Xposition", "Yposition", "Zposition",
                                   "Xrotation", "Yrotation", "Zrotation"]:
                        raise SyntaxError(
                            "Syntax error in line %d: Invalid channel name: '%s'" % (self.linenr, tok))
                    channels.append(tok)
                self._numchannels += len(channels)
                self._nodestack[-1].channels = channels
            elif tok == "JOINT":
                node = Joint()
                self.skeleton.joints.append(node)

                self._nodestack[-1].children.append(node)
                node.parent = self._nodestack[-1]

                self._nodestack.append(node)
                self.readNode()
            elif tok == "End":
                # node = Joint()
                # self.skeleton.joints.append(node)
                # self._nodestack[-1].children.append(node)
                # self._nodestack.append(node)
                self.readNode()
            elif tok == "}":
                # if self._nodestack[-1].isEndSite():
                #     self._nodestack[-1].name = "End Site"
                self._nodestack.pop()
                break
            else:
                raise SyntaxError(
                    "Syntax error in line %d: Unknown keyword '%s'" % (self.linenr, tok))

    # intToken

    def intToken(self):
        """Return the next token which must be an int.
"""

        tok = self.token()
        try:
            return int(tok)
        except ValueError:
            raise SyntaxError(
                "Syntax error in line %d: Integer expected, got '%s' instead" % (self.linenr, tok))

    # floatToken
    def floatToken(self):
        """Return the next token which must be a float.
"""

        tok = self.token()
        try:
            return float(tok)
        except ValueError:
            raise SyntaxError(
                "Syntax error in line %d: Float expected, got '%s' instead" % (self.linenr, tok))

    # token
    def token(self):
        """Return the next token."""

        # Are there still some tokens left? then just return the next one
        if self.tokenlist != []:
            tok = self.tokenlist[0]
            self.tokenlist = self.tokenlist[1:]
            return tok

        # Read a new line
        s = self.readLine()
        self.createTokens(s)
        return self.token()

    # readLine
    def readLine(self):
        """Return the next line.
Empty lines are skipped. If the end of the file has been
reached, a StopIteration exception is thrown. The return
value is the next line containing data (this will never be an
empty string).
"""
        # Discard any remaining tokens
        self.tokenlist = []

        # Read the next line
        while 1:
            s = self.fhandle.readline()
            self.linenr += 1
            if s == "":
                raise StopIteration
            return s

    # createTokens
    def createTokens(self, s):
        """Populate the token list from the content of s.
"""
        s = s.strip()
        a = s.split()
        self.tokenlist = a

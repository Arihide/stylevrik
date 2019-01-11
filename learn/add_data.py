import numpy as np
from sys import argv
from os import path
from bvh_reader import BVHReader

if __name__ == "__main__":

    filename = argv[1]

    br = BVHReader(filename)
    br.read()

    Y = np.asarray(br.motions)

    N = 100

    noise = np.random.normal(0, .005, (N, *Y.shape))

    noised_Y = (Y + noise).reshape(N * Y.shape[0], Y.shape[1])

    root, ext = path.splitext(filename)

    with open('%s_added%s' % (root, ext), 'w') as ftarget:

        for line in noised_Y:
            ftarget.write(' '.join(map(str, line)))
            ftarget.write('\n')

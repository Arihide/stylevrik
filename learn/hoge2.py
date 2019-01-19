import numpy as np
import sys
from bvh_reader import BVHReader

if __name__ == '__main__':

    N = 60
    # N = 130

    # Scene_5_rmfinger_reduced.bvh 
    extract_indices = np.hstack((
        np.linspace(957, 2763, N).astype(np.int),
        np.linspace(3502, 4899, N).astype(np.int)
    ))

    # shagamu_Char00.bvh
    extract_indices = np.hstack((
        np.linspace(543, 731, N).astype(np.int),
        np.linspace(1176, 1334, N).astype(np.int),
        np.linspace(2826, 3017, N).astype(np.int),
        np.linspace(3717, 3900, N).astype(np.int)
    ))

    try:
        sys.argv[1]
    except:
        sys.exit()

    br = BVHReader(sys.argv[1])
    br.read()

    Y = np.asarray(br.motions)

    with open('%s_reduced.bvh' % sys.argv[1][:-4], 'w') as ftarget:

        ftarget.write('Frames: %d\n' % extract_indices.size)
        ftarget.write('Frame Time: %f\n' % 0.01)

        for row in Y[extract_indices]:
            ftarget.write(' '.join(map(str, row)))
            ftarget.write('\n')

    print(Y[extract_indices])
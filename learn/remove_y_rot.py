import sys
import numpy as np 
from bvh_reader import BVHReader

if __name__ == "__main__":
    br = BVHReader(sys.argv[1])
    br.read()

    Y = np.asarray(br.motions)

    Y[:, 3].fill(0)
    # Y[:, 5].fill(0)

    with open('%s_removeY.bvh' % sys.argv[1][:-4], 'w') as fout:

        br.fhandle.seek(0)

        bvh = br.fhandle.read()

        bvh = bvh[:bvh.find('Frame Time')]

        fout.write(bvh)
        fout.write('Frame Time: 0.01\n')

        for row in Y:
            fout.write(' '.join(map(str, row)))
            fout.write('\n')
import sys
from bvh_reader import BVHReader
import numpy as np

if __name__ == "__main__":
    
    br = BVHReader(sys.argv[1])
    br.read()

    Y = np.asarray(br.motions)
    Y[:, 0] = 0
    Y[:, 2] = 0

    with open('%s_vertial.bvh' % sys.argv[1][:-4], 'w') as fout:

        br.fhandle.seek(0)

        bvh = br.fhandle.read()

        bvh = bvh[:bvh.find('Frame Time')]

        fout.write(bvh)
        fout.write('Frame Time: 0.01\n')

        for row in Y:
            fout.write(' '.join(map(str, row)))
            fout.write('\n')

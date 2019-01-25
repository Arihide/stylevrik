import numpy as np
import sys
from bvh_reader import BVHReader

if __name__ == '__main__':

    N = 60
    # N = 130

    # Scene_5_rmfinger_reduced.bvh 
    extract_indices = np.hstack((
        np.linspace(123, 391, N).astype(np.int),
        np.linspace(615, 802, N).astype(np.int),
    ))

    try:
        sys.argv[1]
    except:
        sys.exit()

    br = BVHReader(sys.argv[1])
    br.read()

    br.fhandle.seek(0)
    s = br.fhandle.read()
    
    try:
        assert s.find("MOTION") != -1
    except:
        print(s)
        raise '-s'

    s = s[:s.find("MOTION")]

    Y = np.asarray(br.motions)

    with open('%s_reduced.bvh' % sys.argv[1][:-4], 'w') as ftarget:

        ftarget.write(s)
        ftarget.write('MOTION\n')
        ftarget.write('Frames: %d\n' % extract_indices.size)
        ftarget.write('Frame Time: %f\n' % 0.01)

        for row in Y[extract_indices]:
            ftarget.write(' '.join(map(str, row)))
            ftarget.write('\n')

    print(Y[extract_indices])
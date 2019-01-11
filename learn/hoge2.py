import numpy as np
from bvh_reader import BVHReader

if __name__ == '__main__':

    # extract_indices = [
    #     957, 967, 1146, 2763
    # ]
    
    extract_indices = np.hstack((
        np.linspace(957, 2763, 100).astype(np.int),
        np.linspace(3502, 4899, 100).astype(np.int)
    ))

    br = BVHReader('bvh/Scene_5_Char00_rmfinger.bvh')
    br.read()
    
    Y = np.asarray(br.motions)

    with open('bvh/Scene_5_rmfinger_reduced.bvh', 'w') as ftarget:

        ftarget.write('Frames: %d\n' % extract_indices.size)
        ftarget.write('Frame Time: %f\n' % 0.01)

        for row in Y[extract_indices]:
            ftarget.write(' '.join(map(str, row)))
            ftarget.write('\n')

    print(Y[extract_indices])
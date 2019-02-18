import matplotlib.pyplot as plt
from bvh_reader import BVHReader
import numpy as np
import mathfunc

if __name__ == "__main__":
    br = BVHReader('bvh/Scene_5_rmfinger_reduced.bvh')
    # br = BVHReader('bvh/walk00_rmfinger.bvh')
    br.read()

    # RightShoulder
    index = 13

    # LeftArm
    index = 19

    Y = np.asarray(mathfunc.eulers_to_expmap(br.motions))[:, [index*3, index*3+1, index*3+2]]

    # print(np.asarray(br.motions)[46:56, [13*3 + 0, 13*3 + 1, 13*3 + 2]])
    print(Y[46:56] / np.sqrt(np.sum(Y[46:56]**2, axis=1))[:, None])
    print(np.sum(Y[46:56]**2, axis=1))


    xx = []
    yy = []
    zz = []

    for i in range(0, Y.shape[0]):
        xx.append(Y[i, 0])
        yy.append(Y[i, 1])
        zz.append(Y[i, 2])
        # yy.append(np.sqrt(Y[i, 0]**2 + Y[i, 1]**2 + Y[i, 2]**2))

    plt.subplot(1, 1, 1)
    plt.plot(np.arange(len(xx)), xx)
    plt.plot(np.arange(len(yy)), yy)
    plt.plot(np.arange(len(zz)), zz)
    plt.show()
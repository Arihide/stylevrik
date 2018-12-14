import numpy as np
import GPy

from scaled_gplvm import ScaledGPLVM
from bvh_reader import BVHReader

if __name__ == "__main__":
    br = BVHReader('bvh/walk00_rmfinger.bvh')
    br.read()

    Y = np.asarray(br.motions)
    kernel = GPy.kern.RBF(3)

    m = ScaledGPLVM(Y-Y.mean(0), 3, kernel=kernel)

    print(m.checkgrad())
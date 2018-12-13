import numpy as np
import GPy

from scaled_gplvm import ScaledGPLVM
from bvh_reader import BVHReader

if __name__ == "__main__":
    br = BVHReader('bvh/walk00_rmfinger.bvh')
    br.read()

    Y = np.asarray(br.motions)
    kernel = GPy.kern.RBF(2)

    m = ScaledGPLVM(Y, 2, kernel=kernel)
    # m.unlink_parameter(m.likelihood)
    # m.unlink_parameter(m.kern)

    print(m.checkgrad())
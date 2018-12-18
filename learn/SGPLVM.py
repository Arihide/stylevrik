import numpy as np
import pylab
from PCA_EM import PCA_EM
import kernels
import GP
from scipy import optimize

from bvh_reader import BVHReader

from mathfunc import eulers_to_expmap

class SGPLVM:
    """ TODO: this should inherrit a GP, not contain an instance of it..."""

    def __init__(self, Y, dim):
        self.Xdim = dim
        self.N, self.Ydim = Y.shape
        self.Ymean = Y.mean(0)
        self.W = np.ones(self.Ydim)
        self.orgY = Y.copy()

        """Use PCA to initalise the problem. Uses EM version in this case..."""
        myPCA_EM = PCA_EM(Y, dim)
        myPCA_EM.learn(100)
        X = np.array(myPCA_EM.m_Z)

        # choose particular kernel here if so desired.
        self.GP = GP.GP(X, Y-self.Ymean, kernel=kernels.RBF(1., 1.))

    def learn(self, niters):
        for i in range(niters):
            self.optimise_GP_kernel()
            self.optimise_latents()
            # self.optimize_scale()
            self.GP.Y = (self.orgY - self.Ymean)
            self.GP.update()
            self.W = np.sqrt(self.N / np.sum(self.GP.A * self.GP.Y, axis=0))

    def optimise_GP_kernel(self):
        """optimisation of the GP's kernel parameters"""
        self.GP.Y = (self.orgY - self.Ymean) * self.W
        self.GP.update()
        self.GP.find_kernel_params()
        print(self.GP.marginal(), 0.5*np.sum(np.square(self.GP.X)))

    def lls(self, s):
        self.GP.Y = (self.orgY - self.Ymean) * s
        self.GP.update()
        return -self.GP.marginal() + self.N * np.sum(np.log(s))

    def lls_grad(self, s):
        self.GP.Y = (self.orgY - self.Ymean)
        self.GP.update()
        return s * (self.GP.A * self.GP.Y).sum(0) + self.N / s

    def optimize_scale(self):
        wopt = optimize.fmin_cg(
            self.lls, self.W, fprime=self.lls_grad, disp=True)
        self.W = wopt.copy()

    def ll(self, xx, i):
        """The log likelihood function - used when changing the ith latent variable to xx"""
        self.GP.X[i] = xx
        self.GP.update()
        return -self.GP.marginal() + 0.5*np.sum(np.square(xx))

    def ll_grad(self, xx, i):
        """the gradient of the likelihood function for us in optimisation"""
        self.GP.X[i] = xx
        self.GP.update()
        self.GP.update_grad()
        matrix_grads = [self.GP.kernel.gradients_wrt_data(
            self.GP.X, i, jj) for jj in range(self.GP.Xdim)]
        grads = [-0.5*np.trace(np.dot(self.GP.alphalphK, e))
                 for e in matrix_grads]
        return np.array(grads) + xx

    def optimise_latents(self):
        """Direct optimisation of the latents variables."""
        xtemp = np.zeros(self.GP.X.shape)
        self.GP.Y = (self.orgY - self.Ymean) * self.W
        for i, yy in enumerate(self.GP.Y):
            original_x = self.GP.X[i].copy()
            # xopt = optimize.fmin(self.ll,self.GP.X[i],disp=True,args=(i,))
            xopt = optimize.fmin_cg(
                self.ll, self.GP.X[i], fprime=self.ll_grad, disp=True, args=(i,))
            self.GP.X[i] = original_x
            xtemp[i] = xopt
        self.GP.X = xtemp.copy()


if __name__ == "__main__":
    # Y[10] = np.zeros(5)
    # Y = np.random.normal(0, 0.5, (30,5))

    br = BVHReader('bvh/walk00_rmfinger.bvh')
    br.read()
    Y = np.asarray(br.motions)
    Y = np.asarray(eulers_to_expmap(Y))
    Y += np.random.normal(1, 0.05, Y.shape)
    # Y = Y[:, 3:]
    # Y = Y[::3]

    model = SGPLVM(Y, 2)

    # print(optimize.check_grad(model.ll, model.ll_grad, np.random.rand(2), (1,)))
    # print(optimize.check_grad(model.lls, model.lls_grad, np.random.rand(model.GP.Ydim)))

    model.learn(100)

    def save_model(model, output_filename):
        import json
        output_dict = {}

        output_dict["X"] = model.GP.X.tolist()
        output_dict["Y"] = model.GP.Y.tolist()
        output_dict["kernel"] = {
            "variance": model.GP.kernel.alpha,
            "lengthscale": 1 / np.sqrt(model.GP.kernel.gamma)
        }
        output_dict["likelihood"] = {
            "variance": 1/model.GP.beta,
        }
        output_dict["W"] = model.W.tolist()
        output_dict["mean"] = model.Ymean.tolist()

        with open(output_filename + ".json", "w") as outfile:
            json.dump(output_dict, outfile, indent=2)

    print(Y.std(0))

    save_model(model, 'test')

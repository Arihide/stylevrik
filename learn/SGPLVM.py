import numpy as np
import pylab
from PCA_EM import PCA_EM
import kernels
import GP
from scipy import optimize

from bvh_reader import BVHReader

from mathfunc import eulers_to_expmap

def lik(x_star, y_star, sgplvm):

    gp = sgplvm.GP

    x_star = x_star.reshape(1, x_star.size)
    y_star = y_star.reshape(1, y_star.size)

    k_x_star_x = gp.kernel(x_star, gp.X)
    k_x_star_x_star = gp.kernel(x_star, x_star)

    means = np.dot(k_x_star_x, gp.A)
    means += sgplvm.Ymean

    v = np.linalg.solve(gp.L, k_x_star_x.T)
    variance = k_x_star_x_star - np.dot(v.T, v) + 1./gp.beta

    pyx = np.sum(((y_star - means) * sgplvm.W)**2) / variance

    return (pyx + (gp.Ydim * np.log(variance)) + np.sum(x_star**2))[0, 0] / 2.


def lik_grad(x_star, y_star, sgplvm):

    gp = sgplvm.GP

    x_star = x_star.reshape(1, x_star.size)
    y_star = y_star.reshape(1, y_star.size)

    k_x_star_x = gp.kernel(x_star, gp.X)
    k_x_star_x_star = gp.kernel(x_star, x_star)

    means = np.dot(k_x_star_x, gp.A)
    means += sgplvm.Ymean

    v = np.linalg.solve(gp.L, k_x_star_x.T)
    variance = k_x_star_x_star - np.dot(v.T, v) + 1./gp.beta

    y_grad = ((y_star - means) * sgplvm.W) / variance

    pyx = np.sum(((y_star - means) * sgplvm.W)**2) / variance

    dk_star_dx = np.zeros((gp.N, gp.Xdim))
    for ix in range(gp.N):
        dk_star_dx[ix] = (gp.X[ix] - x_star) * k_x_star_x[0, ix] * gp.kernel.gamma

    gp.update_grad()

    dfdx = np.dot(gp.A.T, dk_star_dx)
    dsigmadx = -2. * np.dot(k_x_star_x, np.dot(gp.Kinv, dk_star_dx))

    return (-np.dot(y_grad, dfdx) + dsigmadx * (gp.Ydim - pyx) / (2. * variance[0,0]) + x_star).flatten()
    # return y_grad.flatten()


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
            # self.GP.Y = (self.orgY - self.Ymean) * self.W
            self.optimise_GP_kernel()
            # self.optimise_latents2()
            # self.optimize_scale()
            # self.GP.Y = (self.orgY - self.Ymean)
            # self.GP.update()
            # self.W = np.sqrt(self.N / np.sum(self.orgY *
            #                                  np.dot(self.GP.Kinv, self.orgY), axis=0))

    def optimise_GP_kernel(self):
        """optimisation of the GP's kernel parameters"""
        self.GP.update()
        self.GP.find_kernel_params()
        # print(self.GP.marginal(), 0.5*np.sum(np.square(self.GP.X)))

    def lls(self, s):
        self.GP.update()
        return -self.GP.marginal() + self.N * np.sum(np.log(s))

    def lls_grad(self, s):
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

    def optimise_latents2(self):

        xtemp = np.zeros(self.GP.X.shape)
        for i, yy in enumerate(self.GP.Y):
            original_x = self.GP.X[i].copy()
            xopt = optimize.fmin_cg(lik, self.GP.X[i], fprime=lik_grad, disp=False, args=(yy, self))
            self.GP.X[i] = original_x
            xtemp[i] = xopt
        self.GP.X = xtemp.copy()

if __name__ == "__main__":
    # Y[10] = np.zeros(5)
    # Y = np.random.normal(0, 0.5, (30, 5))

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

    print(model.GP.X)
    print(model.GP.Y)

    model.GP.find_kernel_params()

    # model.learn(10)

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

    # print(Y.std(0))

    save_model(model, 'test')

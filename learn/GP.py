# -*- coding: utf-8 -*-
# Copyright 2009 James Hensman
# Licensed under the Gnu General Public license, see COPYING
import numpy as np
import pylab
from scipy.optimize import fmin, fmin_ncg, fmin_cg, check_grad, fmin_l_bfgs_b
from scipy import linalg
from sys import stdout
import kernels

from bvh_reader import BVHReader
from mathfunc import eulers_to_expmap

class GP:
    def __init__(self, X, Y, kernel=None, parameter_priors=None):
        """ a simple GP with optimisation of the Hyper parameters via the marginal likelihood approach.
        There is a Univariate Gaussian Prior on the Hyper parameters (the kernel parameters and the noise parameter).
        SCG is used to optimise the parameters (MAP estimate)"""
        self.N = Y.shape[0]
        self.setX(X)
        self.setY(Y)

        if kernel == None:
            self.kernel = kernels.RBF_full(-1, -np.ones(self.Xdim))
        else:
            self.kernel = kernel
        if parameter_priors == None:
            self.parameter_prior_widths = np.ones(self.kernel.nparams+1)
        else:
            assert parameter_priors.size == (self.kernel.nparams+1)
            self.parameter_prior_widths = np.array(parameter_priors).flatten()
        self.beta = 1.
        self.update()
        # constant in the marginal. precompute for convenience.
        self.n2ln2pi = 0.5*self.Ydim*self.N*np.log(2*np.pi)

    def setX(self, newX):
        self.X = newX.copy()
        N, self.Xdim = newX.shape
        assert N == self.N, "bad shape"
        # normalise...
        self.xmean = self.X.mean(0)
        self.xstd = self.X.std(0)
        self.X -= self.xmean
        self.X /= self.xstd

    def setY(self, newY):
        self.Y = newY.copy()
        N, self.Ydim = newY.shape
        assert N == self.N, "bad shape"
        # normalise...
        # self.ystd = self.Y.std(0)
        # self.Y /= self.ystd

    def hyper_prior(self):
        """return the log of the current hyper paramters under their prior"""
        # return np.dot(self.parameter_prior_widths, self.get_params())
        return np.dot(self.parameter_prior_widths, np.log(np.r_[self.kernel.alpha, self.kernel.gamma, self.beta]))

    def hyper_prior_grad(self):
        """return the gradient of the (log of the) hyper prior for the current parameters"""
        return self.parameter_prior_widths / np.r_[self.kernel.alpha, self.kernel.gamma, self.beta]

    def get_params(self):
        """return the parameters of this GP: that is the kernel parameters and the beta value"""
        return np.hstack((self.kernel.get_params(), np.log(self.beta)))

    def set_params(self, params):
        """ set the kernel parameters and the noise parameter beta"""
        self.beta = params[-1]
        self.kernel.set_params(params[:-1])

    def ll(self, params):
        """  A cost function to optimise for setting the kernel parameters. Uses current parameter values if none are passed """
        if params is not None:
            self.set_params(params)
        try:
            self.update()
        except:
            return np.inf
        return -self.marginal() + self.hyper_prior()

    def ll_grad(self, params):
        """ the gradient of the ll function, for use with conjugate gradient optimisation. uses current values of parameters if none are passed """
        if params is not None:
            self.set_params(params)
        try:
            self.update()
        except:
            return np.ones(params.shape)*np.NaN
        self.update_grad()
        matrix_grads = [e for e in self.kernel.gradients(self.X)]
        # # noise gradient matrix

        grads = [-0.5 * np.sum(self.alphalphK * e) for e in matrix_grads]
        grads.append(0.5 * np.trace(self.alphalphK) / self.beta**2)

        return np.array(grads) + self.hyper_prior_grad()

    def find_kernel_params(self, iters=1000):
        """Optimise the marginal likelihood. work with the log of beta - fmin works better that way.  """
        # new_params = fmin(self.ll,np.hstack((self.kernel.get_params(), np.log(self.beta))),maxiter=iters)
        new_params = fmin_cg(self.ll, np.hstack(
            (self.kernel.get_params(), self.beta)), fprime=self.ll_grad, maxiter=iters)
        final_ll = self.ll(new_params)  # sets variables - required!

    def update(self):
        """do the Cholesky decomposition as required to make predictions and calculate the marginal likelihood"""
        self.K = self.kernel(self.X, self.X)
        self.K += np.eye(self.K.shape[0])/self.beta
        self.L = np.linalg.cholesky(self.K)
        self.A = linalg.cho_solve((self.L, 1), self.Y)

    def update_grad(self):
        """do the matrix manipulation required in order to calculate gradients"""
        self.Kinv = np.linalg.solve(
            self.L.T, np.linalg.solve(self.L, np.eye(self.L.shape[0])))
        self.alphalphK = np.dot(self.A, self.A.T)-self.Ydim*self.Kinv

    def marginal(self):
        """The Marginal Likelihood. Useful for optimising Kernel parameters"""
        return -self.Ydim*np.sum(np.log(np.diag(self.L))) - 0.5*np.trace(np.dot(self.Y.T, self.A)) - self.n2ln2pi

    def predict(self, x_star):
        """Make a prediction upon new data points"""
        x_star = (np.asarray(x_star)-self.xmean)/self.xstd

        # Kernel matrix k(X_*,X)
        k_x_star_x = self.kernel(x_star, self.X)
        k_x_star_x_star = self.kernel(x_star, x_star)

        # find the means and covs of the projection...
        # means = np.dot(np.dot(k_x_star_x, self.K_inv), self.Y)
        means = np.dot(k_x_star_x, self.A)
        # means *= self.ystd

        v = np.linalg.solve(self.L, k_x_star_x.T)
        # covs = np.diag( k_x_star_x_star - np.dot(np.dot(k_x_star_x,self.K_inv),k_x_star_x.T)).reshape(x_star.shape[0],1) + self.beta
        variances = (np.diag(k_x_star_x_star - np.dot(v.T, v)).reshape(
            x_star.shape[0], 1) + 1./self.beta)  # * self.ystd.reshape(1, self.Ydim)
        return means, variances


if __name__ == '__main__':
    # generate data:
    Ndata = 50
    X = np.linspace(-3, 3, Ndata).reshape(Ndata, 1)
    Y = np.hstack((np.sin(X) + np.random.normal(0, 10, X.shape)/20, np.random.normal(0, 1, X.shape), np.cos(X))) 
    # Y = np.random.standard_normal(X.shape)
    Y_mean = Y.mean(0)

    # create GP object
    # ,kernels.linear(-1,-1))
    myGP = GP(X, (Y-Y_mean), kernel=kernels.RBF(1., 1.))

    # stuff for plotting
    xx = np.linspace(-4, 4, 200).reshape(200, 1)

    def plot():
        pylab.figure()
        pylab.plot(X, Y, 'r.')
        yy, cc = myGP.predict(xx)
        yy += Y_mean
        pylab.plot(xx, yy, scaley=False)
        pylab.plot(xx, yy + 2*np.sqrt(cc), 'k--', scaley=False)
        pylab.plot(xx, yy - 2*np.sqrt(cc), 'k--', scaley=False)

    plot()

    N, D = Y.shape
    W = np.ones(D)

    def lls(s, gp):
        orgY = gp.Y.copy()
        gp.Y = s * (Y-Y_mean)
        gp.update()
        result = -gp.marginal() - N * np.sum(np.log(s))
        gp.Y = orgY
        return  result

    def lls_grad(s, gp):
        orgY = gp.Y.copy()
        gp.Y = s * (Y-Y_mean)
        gp.update()
        grad = (gp.A * gp.Y).sum(0) / s - N / s
        gp.Y = orgY
        return grad

    print(check_grad(myGP.ll, myGP.ll_grad, np.r_[2.,2.,1.,]))
    print(check_grad(lls, lls_grad, np.r_[2., 2., 1.], myGP))

    for _ in range(10):
        myGP.Y = W * (Y - Y_mean)
        myGP.find_kernel_params()
        newW = fmin_l_bfgs_b(lls, W, fprime=lls_grad, args=(myGP,), bounds=[(0, None) for e in W])[0]
        W = newW

    print(W)

    plot()

    pylab.show()

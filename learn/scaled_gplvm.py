import numpy as np

from GPy.core import GP, Param, Mapping, Param
from GPy.likelihoods import Gaussian
from GPy.util.initialization import initialize_latent
from GPy.mappings import Linear
from GPy.util import diag
from GPy.util.linalg import pdinv, dpotrs, tdot


class Scale(Mapping):

    def __init__(self, input_dim, name='scalemap'):
        super(Scale, self).__init__(
            input_dim=input_dim, output_dim=input_dim, name=name)
        self.S = Param('S', np.random.randn(self.input_dim))
        # self.link_parameter(self.S)

    def f(self, X):
        return X * self.S

    def update_gradients(self, dL_dF, X):
        # そのままかければよいはず
        self.S.gradient = X * dL_dF

    def gradients_X(self, dL_dF, X):
        return dL_dF * self.S

    def to_dict(self):
        """
        Convert the object into a json serializable dictionary.

        Note: It uses the private method _save_to_input_dict of the parent.

        :return dict: json serializable dictionary containing the needed information to instantiate the object
        """

        input_dict = super(Linear, self)._save_to_input_dict()
        input_dict["class"] = "GPy.mappings.Scale"
        input_dict["S"] = self.S.values.tolist()
        return input_dict

    @staticmethod
    def _build_from_input_dict(mapping_class, input_dict):
        import copy


class ScaledGPLVM(GP):

    def __init__(self, Y, input_dim=2, kernel=None):
        X, fracs = initialize_latent('PCA', input_dim, Y)
        # self.mapping = Scale(Y.shape[1])
        self.S = Param('S', np.ones(Y.shape[1]))

        likelihood = Gaussian()

        super(ScaledGPLVM, self).__init__(
            X, Y, kernel, likelihood, name="ScaledGPLVM")

        self.Y_raw = Y

        self.X = Param('latent_mean', X)
        self.link_parameter(self.X, index=0)
        self.link_parameter(self.S)

    def log_likelihood(self):
        return super(ScaledGPLVM, self).log_likelihood() +\
            self.Y.shape[0] * np.log(self.S).sum()-\
            0.5 * np.sum(self.X**2)
        #  - np.log(self.kern.variance) + np.log(self.kern.lengthscale) + np.log(self.likelihood.variance)\

    def parameters_changed(self):
        self.Y_normalized = self.S * self.Y

        # A = (2 * self.S * self.Y).T.reshape(self.Y.shape[1], self.Y.shape[0], 1)
        # B = self.Y.T.reshape(self.Y.shape[1], 1, self.Y.shape[0])

        # grad = np.sum(self.grad_dict['dL_dK'] * np.matmul(A, B), axis=(1,2)) / 66

        YYT_factor = self.Y

        Ky = self.kern.K(self.X).copy()
        diag.add(Ky, self.likelihood.gaussian_variance(self.Y_metadata)+1e-8)

        Wi, LW, LWi, W_logdet = pdinv(Ky)

        alpha, _ = dpotrs(LW, YYT_factor, lower=1)

        # grad = -1. * np.diag(np.matmul((self.S * alpha).T,
        #                                YYT_factor)) + self.Y.shape[0] / self.S

        grad = -1. * self.S * (alpha * YYT_factor).sum(0) + self.Y.shape[0] / self.S

        # print((alpha).shape)
        # print((alpha * YYT_factor).shape)

        # grad = -self.S * np.diag(np.dot(np.dot(self.Y.T, np.linalg.inv(self.kern.K(self.X))), self.Y))

        self.S.gradient = grad

        super(ScaledGPLVM, self).parameters_changed()

        # self.likelihood.variance.gradient += 1 / self.likelihood.variance
        # self.kern.variance.gradient -= 1 / self.kern.variance
        # self.kern.lengthscale.gradient += 1 / self.kern.lengthscale

        self.X.gradient = self.kern.gradients_X(
            self.grad_dict['dL_dK'], self.X, None) - self.X

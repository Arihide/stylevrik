import numpy as np

from GPy.core import GP, Param, Mapping, Param
from GPy.likelihoods import Gaussian
from GPy.util.initialization import initialize_latent
from GPy.mappings import Linear


class Scale(Mapping):

    def __init__(self, input_dim, name='scalemap'):
        super(Scale, self).__init__(
            input_dim=input_dim, output_dim=input_dim, name=name)
        self.S = Param('S', np.random.randn(self.input_dim))
        self.link_parameter(self.S)

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
        self.mapping = Scale(Y.shape[0])

        likelihood = Gaussian()

        super(ScaledGPLVM, self).__init__(X, Y, kernel, likelihood, name="ScaledGPLVM")

        self.X = Param('latent_mean', X)
        self.link_parameter(self.X, index=0)

    def parameters_changed(self):
        # self.Y = self.mapping.f(self.Y)
        super(ScaledGPLVM, self).parameters_changed()
        self.X.gradient = self.kern.gradients_X(
            self.grad_dict['dL_dK'], self.X, None)
        # self.mapping.update_gradients()

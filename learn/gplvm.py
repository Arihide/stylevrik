import numpy as np

from GPy.core import GP, Param, Mapping, Param
from GPy.likelihoods import Gaussian
from GPy.util.initialization import initialize_latent

class GPLVM(GP):
    """
    Gaussian Process Latent Variable Model


    """
    def __init__(self, Y, input_dim, init='PCA', X=None, kernel=None, name="gplvm"):

        """
        :param Y: observed data
        :type Y: np.ndarray
        :param input_dim: latent dimensionality
        :type input_dim: int
        :param init: initialisation method for the latent space
        :type init: 'PCA'|'random'
        """
        if X is None:
            X, fracs = initialize_latent(init, input_dim, Y)
        else:
            fracs = np.ones(input_dim)
        if kernel is None:
            kernel = kern.RBF(input_dim, lengthscale=fracs, ARD=input_dim > 1) + kern.Bias(input_dim, np.exp(-2))

        likelihood = Gaussian()

        super(GPLVM, self).__init__(X, Y, kernel, likelihood, name='GPLVM')

        self.X = Param('latent_mean', X)
        self.link_parameter(self.X, index=0)

    def parameters_changed(self):
        super(GPLVM, self).parameters_changed()
        self.X.gradient = self.kern.gradients_X(self.grad_dict['dL_dK'], self.X, None)
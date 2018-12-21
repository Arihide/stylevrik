import numpy as np
from scipy import optimize, linalg
from SGPLVM import SGPLVM

Y = np.random.rand(5, 10)
model = SGPLVM(Y, 2)

# print(model.GP.alphalphK)
# print(np.dot(np.dot(model.GP.Kinv, model.GP.Y), np.dot(
#     model.GP.Y.T, model.GP.Kinv)) - model.GP.Ydim * model.GP.Kinv)

# print(model.GP.marginal())
# print(0.5 * model.GP.Ydim * np.log(np.linalg.det(model.GP.K)) + 0.5 * np.sum(model.GP.Y *
#                                                                              np.dot(model.GP.Kinv, model.GP.Y)) + 0.5 * model.GP.Ydim * model.GP.N * np.log(2*np.pi))

import matplotlib
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


def kernel(alpha, X1, X2):
    N1, D1 = X1.shape
    N2, D2 = X2.shape
    return alpha[0] * np.exp(-0.5 * alpha[1] * ((X1.reshape(N1, 1, D1) - X2.reshape(1, N2, D2))**2).sum(-1))


def kernel_grad(alpha, X1, X2):
    N1, D1 = X1.shape
    N2, D2 = X2.shape
    diff = ((X1.reshape(N1, 1, D1) - X2.reshape(1, N2, D2)) ** 2).sum(-1)
    return [alpha[0] * np.exp(-0.5 * alpha[1] * diff), -0.5 * diff * alpha[1] * (alpha[0] * np.exp(-0.5 * alpha[1] * diff))]


def lgp(params, X, Y):
    params = np.exp(params)
    N, Ydim = Y.shape
    K = kernel(params, X, X) + np.eye(N) / params[2]
    L = linalg.cholesky(K)
    Kinv = np.linalg.inv(K)
    return Ydim * np.log(np.diag(L)).sum() + 0.5 * np.sum(Y * np.dot(Kinv, Y)) + 0.5 * np.sum(np.square(np.log(params)))


def lgp_grad(params, X, Y):
    params = np.exp(params)
    N, Ydim = Y.shape
    K = kernel(params, X, X) + np.eye(N) / params[2]
    Kinv = np.linalg.inv(K)
    alphalphK = np.dot(np.dot(Kinv, Y), np.dot(Y.T, Kinv)) - Ydim * Kinv
    grads = kernel_grad(params, X, X)
    return np.r_[-0.5 * np.sum(alphalphK * grads[0]) + np.log(params[0]),
                 -0.5 * np.sum(alphalphK * grads[1]) + np.log(params[1]),
                 0.5 * np.trace(alphalphK / (params[2])) + np.log(params[2])
                 ]


def predict(x_star, X, Y, params):
    """Make a prediction upon new data points"""
    # x_star = (np.asarray(x_star)-self.xmean)/self.xstd
    # x_star = np.asarray(x_star).reshape(1, x_star.size)

    params = np.exp(params)

    k_x_star_x = kernel(params, x_star, X)
    k_x_star_x_star = kernel(params, x_star, x_star)
    K = kernel(params, X, X)
    L = linalg.cholesky(K)
    A = linalg.cho_solve((L, True), Y)

    means = np.dot(k_x_star_x, A)
    # means *= self.ystd

    v = np.linalg.solve(L, k_x_star_x.T)
    # covs = np.diag( k_x_star_x_star - np.dot(np.dot(k_x_star_x,self.K_inv),k_x_star_x.T)).reshape(x_star.shape[0],1) + self.beta
    variances = (np.diag(k_x_star_x_star - np.dot(v.T, v)).reshape(
        x_star.shape[0], 1) + 1./params[2])  # * self.ystd.reshape(1, self.Ydim)
    return means, variances


def lik(x_star, y_star, X, Y, params):

    Ydim = Y.shape[1]

    # x_star = x_star.reshape(1, x_star.size)
    # y_star = y_star.reshape(1, y_star.size)

    k_x_star_x = kernel(params[:-1], x_star, X)
    k_x_star_x_star = kernel(params[:-1], x_star, x_star)
    K = kernel(params, X, X)

    L = linalg.cholesky(K)
    A = linalg.cho_solve((L, True), Y)

    means = np.dot(k_x_star_x, A)
    # means += sgplvm.Ymean

    v = np.linalg.solve(L, k_x_star_x.T)
    variance = k_x_star_x_star - np.dot(v.T, v) + 1./params[0]

    print(x_star.shape)
    print(K.shape)
    print(k_x_star_x.shape)
    print(k_x_star_x_star.shape)

    pyx = np.sum(((y_star - means))**2) / variance

    return (pyx + (Ydim * np.log(variance)) + np.sum(x_star**2))[0, 0] / 2.


N = 30
X = np.linspace(-3, 3, N).reshape(N, 1)
Y = np.sin(X)
print(optimize.check_grad(lgp, lgp_grad, np.r_[-10., 3., 30], X, Y))
params = optimize.fmin_cg(
    lgp, np.r_[0, 0, 0], fprime=lgp_grad, args=(X, Y), maxiter=1000)
print(params)
# mean, var = predict(X, X, Y, params)

# print(mean)
# plt.plot(X, Y, 'o')
# plt.plot(X.flatten(), mean.flatten())
# plt.show()

# XX = np.linspace(-3, 3, N).reshape(100, 1)
# YY = np.linspace(-1, 1, N).reshape(100, 1)
# ZZ = lik(XX, YY, X, Y, params)
# print(ZZ)

# fig = plt.figure()
# ax = fig.add_subplot(111, projection='3d')
# surf = ax.plot_surface(XX, YY, ZZ)
# fig.show()

# plt.show()

# print(optimize.check_grad(model.GP.ll, model.GP.ll_grad, np.r_[1., 1., 1.]))

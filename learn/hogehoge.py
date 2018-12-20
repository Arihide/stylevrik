import numpy as np
from scipy import optimize
from SGPLVM import SGPLVM

Y = np.random.rand(5, 10)
model = SGPLVM(Y, 2)

# model.GP.update()
# model.GP.update_grad()

# print(model.GP.alphalphK)
# print(np.dot(np.dot(model.GP.Kinv, model.GP.Y), np.dot(
#     model.GP.Y.T, model.GP.Kinv)) - model.GP.Ydim * model.GP.Kinv)

# print(model.GP.marginal())
# print(0.5 * model.GP.Ydim * np.log(np.linalg.det(model.GP.K)) + 0.5 * np.sum(model.GP.Y *
#                                                                              np.dot(model.GP.Kinv, model.GP.Y)) + 0.5 * model.GP.Ydim * model.GP.N * np.log(2*np.pi))

X = np.random.randn(5, 2)
Y = np.random.randn(5, 5)


def kernel(alpha):
    N, D = X.shape
    return alpha[0] * np.exp(-0.5 * alpha[1] * ((X.reshape(N, 1, D) - X.reshape(1, N, D))**2).sum(-1))


def kernel_grad(alpha):
    N, D = X.shape
    diff = ((X.reshape(N, 1, D) - X.reshape(1, N, D)) ** 2).sum(-1)
    return [np.exp(-0.5 * alpha[1] * diff), -0.5 * diff * alpha[0] * np.exp(-0.5 * alpha[1] * diff)]


def lgp(beta):
    # alpha = np.r_[1.]
    N, Ydim = Y.shape
    K = kernel(np.r_[beta[1], beta[2]])
    K += np.eye(N) / beta[0]
    Kinv = np.linalg.inv(K)
    # + 0.5 * Ydim * N * np.log(2*np.pi)
    return 0.5 * Ydim * np.log(np.linalg.det(K)) + 0.5 * np.sum(Y * np.dot(Kinv, Y))


def lgp_grad(beta):
    # alpha = np.r_[1.]
    N, Xdim = X.shape
    Ydim = Y.shape[1]
    K = kernel(np.r_[beta[1], beta[2]])
    K += np.eye(N) / beta[0]
    Kinv = np.linalg.inv(K)
    alphalphK = np.dot(np.dot(Kinv, Y), np.dot(Y.T, Kinv)) - Ydim * Kinv
    grads = kernel_grad(np.r_[beta[1], beta[2]])
    return [0.5 * np.trace(alphalphK) / beta[0]**2, -0.5 * np.sum(alphalphK * grads[0]), -0.5 * np.sum(alphalphK * grads[1])]


# print(np.dot(kernel(1., X), np.linalg.inv(kernel(1., X))))
# print(optimize.check_grad(model.GP.ll, model.GP.ll_grad, np.r_[1.], X, Y))
# print(optimize.check_grad(kernel, kernel_grad, np.array([1.]), X))
print(optimize.check_grad(lgp, lgp_grad, [10., 3., 3.]))

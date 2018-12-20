import numpy as np
from scipy import optimize
from SGPLVM import SGPLVM

Y = np.random.rand(5, 10)
model = SGPLVM(Y, 2)

result = optimize.check_grad(model.GP.ll, model.GP.ll_grad, np.r_[1.,1.])
print(result)
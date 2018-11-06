import numpy as np
import GPy

# 

if __name__ == "__main__":   
    model = GPy.models.GPLVM.load_model('mymodel.zip')
    print(model)

    model.optimize(message=1)
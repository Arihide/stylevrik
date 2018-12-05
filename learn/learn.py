import numpy as np
from matplotlib import pyplot as plt
import GPy
from scaled_gplvm import ScaledGPLVM

GPy.plotting.change_plotting_library('plotly')
# from bvh_reader import BVHReader
from skeleton import BVHReader
import math
import mathutils
import mathfunc

# 角速度にするか時間差分にするか？
# https://stackoverflow.com/questions/24197182/efficient-quaternion-angular-velocity ???


def motion_to_features(motion, add_velocity=False, add_acceleration=False):
    # Shape of motion data is (frame, pose)
    if add_velocity == True:
        # velocity = np.gradient(motion, axis=0)
        velocity = quaternion_to_angular_velocity(motion)
        motion = np.hstack((motion, velocity))

        if add_acceleration == True:
            acceleration = np.gradient(velocity, axis=0)
            motion = np.hstack((motion, acceleration))

    return motion


def quaternion_to_angular_velocity(motions):

    result = []

    gradient = np.gradient(motions, axis=0) * (1/0.01)

    for j, frame in enumerate(motions):
        rframe = []
        for i in range(0, len(frame), 3):
            q = mathutils.Quaternion((frame[i], frame[i+1], frame[i+2]))
            dqdt = mathutils.Quaternion(
                (gradient[j][i], gradient[j][i+1], gradient[j][i+2]))

            q_ang_vel = 0.5 * q.inverted() * dqdt

            rframe.extend([q_ang_vel.x, q_ang_vel.y, q_ang_vel.z])

        result.append(rframe)

    return np.asarray(result)


def calculate_effector_velocity(joint, br):
    positions = []

    for frame, _ in enumerate(br.motions):
        br.skeleton.apply_frame(br.motions[frame][3:])
        br.skeleton.root.update_global_transform()
        pos = br.skeleton.joints[joint].global_transform.to_translation()
        positions.append([pos.x, pos.y, pos.z])

    positions = np.asarray(positions)
    velocities = np.gradient(positions, axis=0) / 0.01

    return velocities


def save_model(model, Y, mean, std):
    import json
    output_filename = 'testmodel'

    output_dict = {}
    output_dict["X"] = model.X.values.tolist()
    # output_dict["Y"] = model.Y.values.tolist()
    output_dict["Y"] = Y.tolist()
    output_dict["kernel"] = model.kern.to_dict()
    output_dict["likelihood"] = model.likelihood.to_dict()
    output_dict["mean"] = mean.tolist()
    output_dict["std"] = std.tolist()

    # output_dict["class"] = "GPy.core.GP"
    # output_dict["name"] = "stylevrikmodel"
    # output_dict["inference_method"] = model.inference_method.to_dict()

    with open(output_filename + ".json", "w") as outfile:
        json.dump(output_dict, outfile)


def add_gaussian_noise(arr):
    return arr + np.random.normal(0.0, 0.05, arr.shape)


def select_active_set(model, initial_idx=0):
    mu, var = model.predict(model.X)
    indices = var.flatten().argsort()[:30]
    X = model.X[indices]
    Y = model.Y_normalized[indices]

    return GPy.core.GP(
        X, Y, model.kern, model.likelihood, inference_method=model.inference_method
    )


if __name__ == "__main__":
    br = BVHReader('bvh/walk00.bvh')
    # br = BVHReader('bvh/handcrafted_cyclewalk.bvh')
    br.read()

    # データの下処理
    Y = np.asarray(br.motions)
    Y = np.asarray(mathfunc.eulers_to_expmap(Y))
    # Y = np.hstack((Y, calculate_effector_velocity(16, br)))
    # Y = np.hstack((Y, calculate_effector_velocity(39, br)))
    Y = Y[::5]

    kernel = GPy.kern.RBF(input_dim=2, lengthscale=None, ARD=False)

    Y_mean = Y.mean(0)

    # 分散が０のものは１でよい？
    Y_std = Y.std(0)
    Y_std[Y_std == 0] = 1.

    # これおかしい？
    Y_normalized = np.divide(Y-Y_mean, Y_std)
    # Y_normalized = Y-Y_mean

    # model = ScaledGPLVM(Y, 2, kernel=kernel)
    model = GPy.models.GPLVM(Y_normalized, 2, kernel=kernel)
    model.optimize(messages=1, max_iters=5e20)

    # smooth model
    # model.Y_normalized = add_gaussian_noise(Y_normalized)
    # model.unlink_parameter(model.X)
    # model.optimize(messages=1, max_iters=5e20)

    figure = GPy.plotting.plotting_library().figure(1, 2,
                                                    shared_yaxes=True,
                                                    shared_xaxes=True
                                                    )

    canvas = model.plot_latent(labels=np.zeros(
        model.Y_normalized.shape[0]), figure=figure, legend=False)

    GPy.plotting.show(canvas, filename='wishart_metric_notebook')

    # model = select_active_set(model)

    save_model(model, Y, Y_mean, 1./Y_std)

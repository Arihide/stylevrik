from mathutils import Vector, Quaternion
from bvh_reader import BVHReader
import math
import mathfunc
import numpy as np

br = BVHReader('bvh/walk00.bvh')
br.read()

Y = np.asarray(br.motions)
Y = mathfunc.eulers_to_expmap(Y)
Y = np.asarray(Y)

framerate = 120
q = Quaternion(tuple(Y[0][:3]))
dqdt = (Quaternion(tuple(Y[1][:3])) - q)  # * (1 /0.01)


def slerp(q1, q2, t):
    dot = q1.dot(q2)

    if dot < 0.0:
        q2 = -q2
        dot = -dot

    if dot > 0.9995:
        return (q1 + t*(q2 - q1)).normalized()

    theta_0 = math.acos(dot)
    theta = theta_0 * t
    sin_theta = math.sin(theta)
    sin_theta_0 = math.sin(theta_0)
    s0 = math.cos(theta) - dot * sin_theta / sin_theta_0
    s1 = sin_theta / sin_theta_0

    return (s0 * q1) + (s1 * q2)


def dslerp(q1, q2, t):
    dot = q1.dot(q2)

    if dot < 0.0:
        q2 = -q2
        dot = -dot

    if dot > 0.9995:
        return q2 - q1

    theta_0 = math.acos(dot)
    theta = theta_0 * t
    cos_theta = math.cos(theta)
    sin_theta_0 = math.sin(theta_0)

    s0 = -theta_0 * math.sin(theta) - dot * theta_0 * cos_theta / sin_theta_0
    s1 = theta_0 * cos_theta / sin_theta_0

    return (s0 * q1) + (s1 * q2)


def _slerp(q1, q2, t):
    dtheta = math.acos((q2 * q1.inverted()).w * 2)
    return (math.sin((1-t)*dtheta) * q1 + math.sin(t*dtheta) * q2) * (1 / math.sin(dtheta))

# 微分したやつ


def _dslerp(q1: Quaternion, q2: Quaternion, t):
    dtheta = math.acos((q2 * q1.inverted()).w * 2)
    sin_dtheta = math.sin(dtheta)
    return (q1 * math.cos((1-t)*dtheta) * -dtheta + q2 * math.cos(t*dtheta) * dtheta) * (1 / sin_dtheta)


def quaternion_to_angular_velocity(motions):

    result = []

    _motions = np.asarray(motions)
    _motions = _motions.T

    for j, frame in enumerate(_motions):
        rframe = []
        pq = Quaternion((frame[0], frame[1], frame[2]))
        for i in range(0, len(frame), 3):
            q = Quaternion((frame[i], frame[i+1], frame[i+2]))

            q_ang_vel = 0.5 * slerp(pq, q, 1.0).inverted() * dslerp(pq, q, 1.0)

            rframe.extend([q_ang_vel.x, q_ang_vel.y, q_ang_vel.z])

            pq = q

        result.append(rframe)

    return np.asarray(result)


br = BVHReader('bvh/walk00.bvh')
br.read()

print(quaternion_to_angular_velocity(br.motions)[1])

import math
import mathutils
import numpy as np

# 初期姿勢は向かい合っている
# x右　y上　z手前

# Unity　x右　y上　z奥
# ZXY!!

def eulers_to_expmap(eulerArray):

    result = []

    for e in eulerArray:

        tmp = []
        # tmp = [e[0], e[1], e[2]]
        for i in range(3, len(e), 3):
            quat = mathutils.Euler(
                (math.radians(e[i+1]),
                 -math.radians(e[i]),
                 -math.radians(e[i+2])), 'ZXY').to_quaternion()

            # yrot = mathutils.Euler((0, math.pi, 0)).to_quaternion()
            # zrot = mathutils.Euler((0, 0, math.pi)).to_quaternion()
            # quat = yrot * zrot * quat

            axis, angle = quat.to_axis_angle()

            expmap = axis * angle

            # expmap = quat.to_exponential_map()

            tmp.append(expmap.x)
            tmp.append(expmap.y)
            tmp.append(expmap.z)

        result.append(tmp)

    return result


def expmap_to_euler(expmapArray):

    result = []

    for e in expmapArray:
        for i in range(3, len(e), 3):
            euler = mathutils.Vector((e[i], e[i+1], e[i+2]))
            euler = euler / euler.length
            result.append(math.degrees(euler.y))
            result.append(math.degrees(euler.x))
            result.append(math.degrees(euler.z))

    return result

# この関数は正しいはず。ブレンダーで確認済み


def partial_R_partial_EM3(v, i):

    # https://github.com/erich666/jgt-code/blob/master/Volume_03/Number_3/Grassia1998/exp-map.c

    q = mathutils.Quaternion(v)
    dqdvi = partial_Q_partial_3V(v, i)

    dRdEM3 = partial_R_partial_Vi(q, dqdvi)

    return dRdEM3


def partial_R_partial_Vi(q, dqdvi):

    dRdvi = mathutils.Matrix.Identity(4)

    prod0 = -4.0 * q.x * dqdvi.x
    prod1 = -4.0 * q.y * dqdvi.y
    prod2 = -4.0 * q.z * dqdvi.z
    prod3 = 2.0 * (q.y * dqdvi.x + q.x * dqdvi.y)
    prod4 = 2.0 * (q.w * dqdvi.z + q.z * dqdvi.w)
    prod5 = 2.0 * (q.z * dqdvi.x + q.x * dqdvi.z)
    prod6 = 2.0 * (q.w * dqdvi.y + q.y * dqdvi.w)
    prod7 = 2.0 * (q.z * dqdvi.y + q.y * dqdvi.z)
    prod8 = 2.0 * (q.w * dqdvi.x + q.x * dqdvi.w)

    dRdvi[0][0] = prod1 + prod2
    dRdvi[0][1] = prod3 - prod4
    dRdvi[0][2] = prod5 + prod6

    dRdvi[1][0] = prod3 + prod4
    dRdvi[1][1] = prod0 + prod2
    dRdvi[1][2] = prod7 - prod8

    dRdvi[2][0] = prod5 - prod6
    dRdvi[2][1] = prod7 + prod8
    dRdvi[2][2] = prod0 + prod1

    for i in range(3):
        dRdvi[3][i] = dRdvi[i][3] = 0.0

    dRdvi[3][3] = 0.0

    return dRdvi


def partial_Q_partial_3V(v, i):
    theta = v.length
    cosp = math.cos(theta * 0.5)
    sinp = math.sin(theta * 0.5)

    dqdx = mathutils.Quaternion()

    if theta < 0.00001:
        i2 = (i+1) % 3
        i3 = (i+2) % 3
        Tsinc = 0.5 - theta*theta/48.0
        vTerm = v[i] * (theta*theta/40.0 - 1.0) / 24.0

        dqdx.w = -0.5*v[i]*Tsinc
        dqdx[i+1] = v[i] * vTerm + Tsinc
        dqdx[i2+1] = v[i2]*vTerm
        dqdx[i3+1] = v[i3]*vTerm
    else:
        i2 = (i+1) % 3
        i3 = (i+2) % 3
        ang = 1.0 / theta
        ang2 = ang * ang * v[i]
        sang = ang * sinp
        cterm = ang2 * (0.5 * cosp - sang)

        dqdx[i+1] = cterm * v[i] + sang
        dqdx[i2+1] = cterm * v[i2]
        dqdx[i3+1] = cterm * v[i3]
        dqdx.w = -0.5 * v[i] * sang

    return dqdx


def calc_jacobian_matrix(pose, br):

    # offset1 = mathutils.Vector(br._root.offset)
    offset2 = mathutils.Vector(br._root.children[0].offset)
    # 太ももから膝
    offset3 = mathutils.Vector(br._root.children[0].children[0].offset)
    # 膝から足首
    offset4 = mathutils.Vector(
        br._root.children[0].children[0].children[0].offset)

    localMatrices = [
        mathutils.Quaternion((pose[4], pose[3], pose[5])).to_matrix().to_4x4(
        ) * mathutils.Matrix.Translation((pose[0], pose[1], pose[2])),
        mathutils.Quaternion((pose[7], pose[6], pose[8])).to_matrix(
        ).to_4x4() * mathutils.Matrix.Translation(offset2),
        mathutils.Quaternion((pose[10], pose[9], pose[11])).to_matrix(
        ).to_4x4() * mathutils.Matrix.Translation(offset3),
    ]

    worldPositions = [
        localMatrices[0] * offset2,
        localMatrices[0] * localMatrices[1] * offset3,
        localMatrices[0] * localMatrices[1] * localMatrices[2] * offset4
    ]

    jacobian = np.zeros((3, 9))

    bp = mathutils.Matrix.Identity(4)
    for i in range(6, -3, -3):

        v = mathutils.Vector((pose[i], pose[i+1], pose[i+2]))

        for j in range(3):
            dRdEM3 = partial_R_partial_EM3(v, j)

            dv = bp * dRdEM3 * worldPositions[int(i / 3)]
            jacobian[0, i + j] = dv.x
            jacobian[1, i + j] = dv.y
            jacobian[2, i + j] = dv.z

        bp = bp * localMatrices[int(i / 3)]

    return jacobian

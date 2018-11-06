from skeleton import BVHReader
import numpy as np

def calculate_effector_velocity(joint, br):
    positions = []

    for frame,_ in enumerate(br.motions):
        br.skeleton.apply_frame(br.motions[frame][3:])
        br.skeleton.root.update_global_transform()
        pos = br.skeleton.joints[joint].global_transform.to_translation()
        positions.append([pos.x, pos.y, pos.z])

    positions = np.asarray(positions)
    velocities = np.gradient(positions, axis=0) / 0.01

    print(positions)
    print(velocities)

    return positions


br = BVHReader('bvh/walk00.bvh')
br.read()
calculate_effector_velocity(16, br)
# print(calculate_effector_velocity(16))
import sys
import json
from skeleton import BVHReader



if __name__ == "__main__":

    args = sys.argv

    br = BVHReader(args[1])
    # br = BVHReader('bvh/walk00.bvh')
    br.read()

    br.skeleton.apply_frame([0 for x in range(177)])
    # br.skeleton.apply_frame(br.motions[0][3:])
    br.skeleton.root.update_global_transform()

    skeleton = []

    for joint in br.skeleton.joints:

        joint_dict = {
            'name': joint.name,
            # Unityは左手系なので、xを反転
            'translation': [
                -joint.transform.translation.x,
                joint.transform.translation.y,
                joint.transform.translation.z
            ],
            'globalTranslation': [
                -joint.global_transform.translation.x,
                joint.global_transform.translation.y,
                joint.global_transform.translation.z
            ]
            # 'parent': joint.parent
        }
        skeleton.append(joint_dict)

    with open('skeleton.json', 'w') as file:
        file.write(json.dumps(skeleton, indent=1))

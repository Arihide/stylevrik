with open('bvh/handcrafted_cyclewalk.bvh', 'r') as fsource:
    with open('test.bvh', 'w') as ftarget:

        delList = ['RightHandThumb1']

        while True:
            line = fsource.readline()

            if not line:
                break

            isExist = False
            for word in delList:
                if word in line:
                    isExist = True

            if isExist:
                fsource.readline()
                fsource.readline()
                continue

            ftarget.write(line)

        while True:
            line = fsource.readline()
import sys
import json
import numpy as np

if len(sys.argv) <= 1:
    sys.exit()

with open(sys.argv[1]) as fin:
    s = fin.read()
    j = json.loads(s)

    indices = [i for i, x in enumerate(j["mean"]) if x == 0]

    Y = []
    for row in j["Y"]:
        Y.append([x for i, x in enumerate(row) if i not in indices])

    mean = [x for i, x in enumerate(j["mean"]) if i not in indices]
    std = [x for i, x in enumerate(j["std"]) if i not in indices]
    
    j["Y"] = Y
    j["mean"] = mean
    j["std"] = std

    with open("%s_compressed.json" % sys.argv[1][:-5], 'w') as fout:
        fout.write(json.dumps(j))
import sys
import json
import numpy as np

if len(sys.argv) <= 1:
    sys.exit()

with open(sys.argv[1]) as fin:
    s = fin.read()
    j = json.loads(s)
    
    for i in [7, 8, 16, 17]:

        for row in j["Y"]:
            row.insert(i, 0)
        j["mean"].insert(i, 0)
        j["std"].insert(i, 1)

    with open("%s_added.json" % sys.argv[1][:-5], 'w') as fout:
        fout.write(json.dumps(j))
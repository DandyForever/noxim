import numpy as np
from math import *

path = "sim/"
file = "lat.log"

latencies = list()

with open(path + file, 'r') as f:
  data_lines = f.readlines()
  for line in data_lines:
    lat_pos = line.find(':')
    print (line.split())
    latencies.append([int(i) for i in line.split() if i.isdigit()])

data = np.array(latencies)

print(np.mean(data))
print(sqrt(np.var(data)))

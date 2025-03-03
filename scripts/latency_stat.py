import numpy as np
import pandas as pd
from math import *
from collections import Counter

path = "../sim/results/latencies/"
latency_types = ["latency"]
# pir_list = [25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325,
#             350, 375, 400, 425, 450, 475, 500, 600, 700, 800, 900, 1000]
pir_list = [500]
burst_list = [1]
# pir_list = [500]
# burst_list = [1, 2]
traffic_burst = 1
most_common_percentage_threshold = 0.0015

mean_latencies_burst = list()
disp_latencies_burst = list()

file_prefix = "mod_dor_mesh_10x10_MOD_DOR_mp_1"

for latency_type in latency_types:
    for burst in burst_list:
        mean_latencies = list()
        disp_latencies = list()
        for pir in pir_list:
            file = '_'.join([file_prefix, "pir", str(pir), "ps", str(
                burst), "bs", str(traffic_burst), latency_type + ".log"])

            latencies = list()
            lat_distribution = dict()

            with open(path + file, 'r') as f:
                data_lines = f.readlines()
                for line in data_lines:
                    latency = int(line.split()[-1])
                    latencies.append(latency)
                latency_counter = Counter(latencies)
                threshold = latency_counter.most_common(
                    1)[0][1] * most_common_percentage_threshold
                latencies_ = np.array(latencies)
                latencies_.sort()
                min_num = 0
                for i, latency in enumerate(latencies_[::-1]):
                    if latency_counter[latency] < threshold:
                        min_num = latencies_.size - i
                    else:
                        break
                if min_num == 0:
                    min_num = latencies_.size
                latencies_ = latencies_[:min_num]
            data = np.array(latencies_)
            min_num = int(data.size)
            data.sort()
            data = data[:min_num]
            mean_latencies.append(np.mean(data))
            disp_latencies.append(sqrt(np.var(data)))
        print(burst)
        print(mean_latencies)
        print(disp_latencies)
        mean_latencies_burst.append(mean_latencies)
        disp_latencies_burst.append(disp_latencies)

df = pd.DataFrame(mean_latencies_burst)
df.to_excel(path + file_prefix + "latency.xlsx")

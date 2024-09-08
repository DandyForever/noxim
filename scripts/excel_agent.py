import csv

mesh_x = 18
mesh_y = 10

eu_h = 12 * 2
eu_v = 4 * 2
eu_a = 0

pir_list = [50, 100, 150, 200, 250, 300,
            350, 400, 500, 600, 700, 800, 900, 1000]

path = "sim/results/"
pref = "burst"
routing_list = ["DOR"]
mesh = 'x'.join([str(mesh_x), str(mesh_y)])
suff_list = ["mp_4_sv_3_sh_3"]
result_names = [0, mesh_y+1, 2 *
                (mesh_y+1), 3*(mesh_y+1), 4*(mesh_y+1), 5*(mesh_y+1)]
burst_list = [1, 2]
traffic_burst = 2

results = dict()

for burst in burst_list:
    for suff in suff_list:
        for routing in routing_list:
            results['pir'] = pir_list
            pir_map = dict()
            for pir in pir_list:
                file_name = '_'.join(
                    [pref, "mesh", mesh, routing, suff, "pir", str(pir), "ps", str(burst), "bs", str(traffic_burst)])

                data = list()
                data.append(list())
                data.append(list())
                data.append(list())
                data.append(list())
                data.append(list())
                data.append(list())

                row_idx = 0
                data_idx = -1

                with open(path + file_name + ".csv", newline='') as csvfile:
                    spamreader = csv.reader(
                        csvfile, delimiter=';', quotechar=';')
                    for row in spamreader:
                        if row_idx not in result_names:
                            data[data_idx].append(list(map(int, row[:-1])))
                        else:
                            data_idx += 1
                        row_idx += 1
                data.append(list())
                for i in range(mesh_y):
                    data[6].append(list())
                    for j in range(mesh_x):
                        data[3][i][j] *= burst
                        data[6][i].append(data[2][i][j] + data[3][i][j])
                pir_map[pir] = {'x': data[2], 'y': data[3], 'all': data[6]}
            for dir in pir_map[pir]:
                for key in pir_map:
                    results['h'] = list()
                    results['v'] = list()
                    results['t'] = list()
                    results['a'] = list()
                for key in pir_map:
                    total_horizontal = 0
                    total_vertical = 0
                    total_angle = pir_map[key][dir][0][0] + pir_map[key][dir][0][mesh_x-1] + \
                        pir_map[key][dir][mesh_y-1][0] + \
                        pir_map[key][dir][mesh_y-1][mesh_x-1]
                    for i in range(mesh_x):
                        total_horizontal += pir_map[key][dir][0][i] + \
                            pir_map[key][dir][mesh_y-1][i]
                    for i in range(mesh_y):
                        total_vertical += pir_map[key][dir][i][0] + \
                            pir_map[key][dir][i][mesh_x-1]
                    total_horizontal -= total_angle
                    total_vertical -= total_angle
                    results['t'].append(
                        (total_horizontal+total_vertical+total_angle) / (eu_v + eu_h + eu_a) / 100)
                    results['h'].append(total_horizontal / eu_h / 100)
                    results['v'].append(total_vertical / eu_v / 100)
                    if eu_a:
                        results['a'].append(total_angle / eu_a / 100)
                # print(dir)
                with open(path + mesh + dir + suff + routing + str(burst) + '.csv', 'w', newline='') as csvfile:
                    csv_writer = csv.writer(csvfile, delimiter=';',
                                            quotechar='|', quoting=csv.QUOTE_MINIMAL)
                    for key in results:
                        csv_writer.writerow([dir, key] + results[key])

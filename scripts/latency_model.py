import numpy as np

NORTH_IDX = 0
SOUTH_IDX = 1
WEST_IDX = 2
EAST_IDX = 3
LOCAL_IDX = 4
direction_num = 5

mesh_dim_x = 10
mesh_dim_y = 10
tile_num = mesh_dim_x * mesh_dim_y

pir = 0.1


class Coord:
    def __init__(self, id=0):
        self.x = id % mesh_dim_x
        self.y = id // mesh_dim_y

    def __repr__(self):
        return f"Coordinate(x={self.x}, y={self.y})"

    def is_perimeter(self):
        return (self.x == 0) | (self.x == mesh_dim_x - 1) | (self.y == 0) | (self.y == mesh_dim_y - 1)

    def id(self):
        return self.y * mesh_dim_y + self.x

# Returns next tile for given current and destination ones
# Algorith DOR


def next_tile_gen(current_tile, dst_tile):
    next = Coord(current_tile.id())
    if current_tile.y < dst_tile.y:
        next.y += 1
    elif current_tile.y > dst_tile.y:
        next.y -= 1
    elif current_tile.x < dst_tile.x:
        next.x += 1
    elif current_tile.x > dst_tile.x:
        next.x -= 1
    return next

# Returns directions current_tile output ID and next_tile input ID


def encode_dirs(current_tile, next_tile):
    if current_tile.y < next_tile.y:
        return (SOUTH_IDX, NORTH_IDX)
    elif current_tile.y > next_tile.y:
        return (NORTH_IDX, SOUTH_IDX)
    elif current_tile.x < next_tile.x:
        return (EAST_IDX, WEST_IDX)
    elif current_tile.x > next_tile.x:
        return (WEST_IDX, EAST_IDX)
    return (LOCAL_IDX, LOCAL_IDX)


# 1 dim - s - source tile
# 2 dim - d - destination tile
# 3 dim - r - router tile
# 4 dim - i - router input channel
# 5 dim - o - router output channel
# `1` if routing path from `s` to `d` goes through `r` from `i` to `o`
# `0` otherwise
routing_tensor = np.zeros(
    (tile_num, tile_num, tile_num, direction_num, direction_num))

for s in range(tile_num):
    for d in range(tile_num):
        s_coord = Coord(s)
        d_coord = Coord(d)
        if not (s_coord.is_perimeter() and not d_coord.is_perimeter()):
            continue
        current_tile = s_coord
        i_cur_dir = LOCAL_IDX
        while current_tile.id() != d:
            next_tile = next_tile_gen(current_tile, d_coord)
            (o_cur_dir, i_nxt_dir) = encode_dirs(current_tile, next_tile)
            routing_tensor[s][d][current_tile.id()][i_cur_dir][o_cur_dir] = 1
            current_tile = Coord(next_tile.id())
            i_cur_dir = i_nxt_dir
        routing_tensor[s][d][d][i_cur_dir][LOCAL_IDX] = 1

print("Routing tensor done")

# 1 dim - s - source tile
# 2 dim - d - destination tile
traffic_matrix = np.zeros((tile_num, tile_num))

pir_per_mem_tile = pir / (mesh_dim_x - 2) / (mesh_dim_y - 2)

for s in range(tile_num):
    for d in range(tile_num):
        s_coord = Coord(s)
        d_coord = Coord(d)
        if not (s_coord.is_perimeter() and not d_coord.is_perimeter()):
            continue
        traffic_matrix[s][d] = pir_per_mem_tile

print("Traffic matrix done")

# 1 dim - r - router tile
# 2 dim - i - router input channel
# 3 dim - o - router output channel
router_payload_tensor = np.zeros((tile_num, direction_num, direction_num))

for s in range(tile_num):
    for d in range(tile_num):
        for r in range(tile_num):
            for i in range(direction_num):
                for o in range(direction_num):
                    router_payload_tensor[r][i][o] += traffic_matrix[s][d] * \
                        routing_tensor[s][d][r][i][o]

print("Router payload tensor done")

# 1 dim - r - router tile
# 2 dim - i - router input channel
router_input_payload_tensor = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        router_input_payload_tensor[r][i] += np.sum(
            router_payload_tensor[r][i])

print("Router input payload tensor done")


def calc_blocking_possibility_internal(request_possibility, i, o, other_num):
    """
    Compute d(i, j, c) for the current F.
    F is a 5x5 array (F[i][j] for i,j=0..4).
    i, j are the main indices in 0..4.
    c is in 0..4.
    """
    # The four other indices besides i
    others = [x for x in range(5) if x != i]

    total = 0.0
    # Loop over all 16 combinations of (k1, k2, k3, k4) in {0,1}^4
    for mask in range(16):  # from 0 to 15
        # Count how many bits are 1
        # and build the product F[...]^(k_t) * (1-F[...])^(1-k_t)
        ksum = 0
        prob_product = 1.0
        for bit_idx in range(4):
            # k_t is either 0 or 1
            k_t = (mask >> bit_idx) & 1
            p = request_possibility[others[bit_idx]][o]
            if k_t == 1:
                prob_product *= p
                ksum += 1
            else:
                prob_product *= (1 - p)

        # If k_1 + k_2 + k_3 + k_4 = c, add to sum
        if ksum == other_num:
            total += prob_product
    return total


def calc_blocking_possibility(request_possibility, i, o):
    blocking_possibility = 0.0
    for other_num in range(1, 5):
        blocking_possibility_internal = calc_blocking_possibility_internal(
            request_possibility, i, o, other_num)
        blocking_possibility += blocking_possibility_internal * \
            (other_num / (other_num + 1))
    return blocking_possibility


def solve_request_possibility(payload_tensor, max_iter=1000, tol=1e-8):
    """
    Solve for F using the fixed-point iteration:
       F[i][j] = a[i][j] / (1 - D[i][j]),
    with D[i][j] computed via the sums over the 4 other indices.

    :param a: known 5x5 numpy array
    :param max_iter: maximum number of iterations
    :param tol: convergence tolerance
    :return: F as a 5x5 numpy array
    """
    # 1) Initialize F (for example, uniform 0.5)
    request_possibility = payload_tensor.copy()

    for it in range(max_iter):
        request_possibility_old = request_possibility.copy()

        for i in range(5):
            for o in range(5):
                blocking_possibility = calc_blocking_possibility(
                    request_possibility, i, o)

                denom = 1.0 - blocking_possibility
                if abs(denom) < 1e-14:
                    # Avoid dividing by zero.
                    # Could set F[i][j] to some fallback value.
                    request_possibility[i][o] = 0.999999 if denom < 0 else 0.0
                else:
                    request_possibility[i][o] = payload_tensor[i][o] / denom

        # Check for convergence
        diff = np.linalg.norm(request_possibility - request_possibility_old)
        if diff < tol:
            break

    print(f"Finished in {it+1} iterations with diff={diff}")
    return request_possibility


blocking_possibility = np.zeros((tile_num, direction_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        for o in range(direction_num):
            request_possibility_solution = solve_request_possibility(
                router_payload_tensor[r], max_iter=500, tol=1e-10)
            blocking_possibility[r][i][o] = calc_blocking_possibility(
                request_possibility_solution, i, o)

print("Blocking possibility done")

blocking_time = blocking_possibility.copy()

for r in range(tile_num):
    for i in range(direction_num):
        for o in range(direction_num):
            blocking_time[r][i][o] /= (1 - blocking_time[r][i][o])

print("Blocking time done")

request_handle_time = blocking_time.copy()

for r in range(tile_num):
    for i in range(direction_num):
        for o in range(direction_num):
            request_handle_time[r][i][o] += 1.

print("Request handle time done")

mean_input_handle_time = np.zeros((tile_num, direction_num))
mean_sqr_input_handle_time = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        sum_input_handle_time = 0.
        sum_sqr_input_handle_time = 0.
        for o in range(direction_num):
            sum_input_handle_time += router_payload_tensor[r][i][o] * \
                request_handle_time[r][i][o]
            sum_sqr_input_handle_time += router_payload_tensor[r][i][o] * \
                request_handle_time[r][i][o]**2
        mean_input_handle_time[r][i] = 0. if (abs(sum_input_handle_time) < 1e-9) else sum_input_handle_time / \
            router_input_payload_tensor[r][i]
        mean_sqr_input_handle_time[r][i] = 0. if (abs(
            sum_sqr_input_handle_time) < 1e-9) else sum_sqr_input_handle_time / router_input_payload_tensor[r][i]

print("Mean request handle time done")

variance_input_handle_time = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        sum_input_handle_time = 0.
        for o in range(direction_num):
            deviation = (request_handle_time[r][i]
                         [o] - mean_input_handle_time[r][i])
            sum_input_handle_time += router_payload_tensor[r][i][o] * \
                deviation * deviation
        variance_input_handle_time[r][i] = 0. if (abs(sum_input_handle_time) < 1e-9) else sum_input_handle_time / \
            router_input_payload_tensor[r][i]

print("Variance request handle time done")


def mg1n_queue(lambda_arrival, E_T, D_T, N, E_T2):
    """
    Computes key performance metrics for an M/G/1/N queue.

    :param lambda_arrival: Arrival rate (Poisson process)
    :param E_T: Expected service time E(T)
    :param D_T: Variance of service time D(T)
    :param N: System capacity (including server)
    :return: Dictionary with performance metrics
    """

    utilization = lambda_arrival * E_T  # Utilization factor

    print("utilization: ", utilization)

    if abs(utilization) < 1e-9:
        return (0.0, 0.0)

    # Probability of k packets in an infinite buffer M/G/1 system
    chi = np.exp(2 * (lambda_arrival - utilization) /
                 (lambda_arrival + D_T * E_T**3))

    print("chi: ", chi)

    def pk_star(k):
        """ Probability of k packets in an M/G/1/∞ system """
        if k == 0:
            return 1 - utilization
        else:
            return utilization * (1 - chi) * chi**(k - 1)

    # Compute pk* for all k
    pk_star_values = [pk_star(k) for k in range(N)]

    print("pk_star_values: ", pk_star_values)

    # Compute normalizing factor c
    c = (1 - utilization * (1 - sum(pk_star_values)))**-1

    print("c: ", c)

    # Compute pk for the finite queue M/G/1/N
    pk_values = [c * pk_star_values[k] for k in range(N)]
    pk_values.append(1 - (1 - c * (1 - utilization))
                     / utilization)  # p_N

    print("pk_values: ", pk_values)

    # Compute average number of packets in system
    mean_depth = sum((k - 1) * pk_values[k] for k in range(1, N+1))

    print("mean_depth: ", mean_depth)

    # Compute mean waiting time in queue
    W_s = sum(k * pk_values[k]
              for k in range(N+1)) / (lambda_arrival * (1 - pk_values[N]))
    print("W_s: ", W_s)
    W_q = W_s - E_T

    print("W_q: ", W_q)
    # Compute residual service time
    mean_residual_time = E_T2 / (2. * E_T)
    print("mean_residual_time: ", mean_residual_time)

    return (mean_depth, mean_residual_time)


def mm1n_queue(lambda_arrival, E_T, N):
    utilization = lambda_arrival * E_T
    pk_values = [(1 - utilization) * utilization**k / (1 - utilization**(N+1))
                 for k in range(N+1)]
    mean_depth = sum((k - 1) * pk_values[k] for k in range(1, N+1))
    return mean_depth


mean_input_queue_depth = np.zeros((tile_num, direction_num))
mean_input_residual_time = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        (mean_input_queue_depth[r][i], mean_input_residual_time[r][i]) = mg1n_queue(
            router_input_payload_tensor[r][i], mean_input_handle_time[r][i], variance_input_handle_time[r][i], 2, mean_sqr_input_handle_time[r][i])

print("M/G/1/N queues done")

mean_input_queue_depth = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        mean_input_queue_depth[r][i] = mm1n_queue(
            router_input_payload_tensor[r][i], mean_input_handle_time[r][i], 2)


def calc_average_latency(routing_tensor, mean_input_queue_depth, mean_input_handle_time):
    average_latency = 0.0
    for r in range(tile_num):
        for i in range(direction_num):
            if abs(routing_tensor[r][i].sum()) > 1e-9:
                average_latency += (mean_input_queue_depth[r][i] + 1.) * \
                    mean_input_handle_time[r][i] + 1.
    return average_latency


average_latency = np.zeros((tile_num, tile_num))
count = 0.

for s in range(tile_num):
    for d in range(tile_num):
        if Coord(s).is_perimeter() and not Coord(d).is_perimeter():
            average_latency[s][d] = calc_average_latency(
                routing_tensor[s][d], mean_input_queue_depth, mean_input_handle_time)
            count += 1
            print(Coord(s), Coord(d), average_latency[s][d])

print("Average latency done")

print("Average latency: ", average_latency.sum() / count)

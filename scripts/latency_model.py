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

print(router_payload_tensor[45])

# 1 dim - r - router tile
# 2 dim - i - router input channel
router_input_payload_tensor = np.zeros((tile_num, direction_num))

for r in range(tile_num):
    for i in range(direction_num):
        router_input_payload_tensor[r][i] += np.sum(
            router_payload_tensor[r][i])

print(router_input_payload_tensor[45])


def d_ijc(F, i, j, c):
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
            p = F[others[bit_idx]][j]
            if k_t == 1:
                prob_product *= p
                ksum += 1
            else:
                prob_product *= (1 - p)

        # If k_1 + k_2 + k_3 + k_4 = c, add to sum
        if ksum == c:
            total += prob_product
    return total


def D_ij(F, i, j):
    D_val = 0.0
    for c in range(1, 5):
        d_val = d_ijc(F, i, j, c)
        D_val += d_val * (c / (c + 1))
    return D_val


def solve_F(a, max_iter=1000, tol=1e-8):
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
    F = np.full((5, 5), 0.5)

    for it in range(max_iter):
        F_old = F.copy()

        for i in range(5):
            for j in range(5):
                # Compute D[i][j]
                Dij = D_ij(F_old, i, j)

                denom = 1.0 - Dij
                if abs(denom) < 1e-14:
                    # Avoid dividing by zero.
                    # Could set F[i][j] to some fallback value.
                    F[i][j] = 0.999999 if denom < 0 else 0.0
                else:
                    F[i][j] = a[i][j] / denom

        # Check for convergence
        diff = np.linalg.norm(F - F_old)
        if diff < tol:
            print(f"Converged in {it+1} iterations with diff={diff}")
            break

    return F


if __name__ == "__main__":
    np.random.seed(0)
    a = np.random.rand(5, 5)  # Example known array

    F_solution = solve_F(a, max_iter=500, tol=1e-10)
    print("Solution F:\n", F_solution)

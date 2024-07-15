../build/noxim \
    -config ../config_examples/mesh_10x10_interliving.yaml \
    -pir 0.25 poisson \
    -routing MOD_DOR \
    -size 1 2 \
    -interliving_reps 0 \
    -sim 10000 \
    -detailed \
    -mem_ports 4 \
    -switch_horizontal_masters 3 \
    -switch_vertical_masters 0 \
    -switch_angle_masters 3 \
    -req_ack_mode 1 \
    -both_phys_req_mode 1 \
    -switch_debug 0 \
    -traffic_verbose 0 \
    -flit_dump 0 \
    -buffer_verbose 0 \
    -log_file_name ./results/validation

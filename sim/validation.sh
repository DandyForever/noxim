# ../build/noxim \
#     -config ../config_examples/task_3804/mesh_18x10_dor.yaml \
#     -pir 0.5 poisson \
#     -routing DOR \
#     -size 1 1 \
#     -traffic_burst_size 1 \
#     -interliving_reps 0 \
#     -sim 10000 \
#     -detailed \
#     -mem_ports 1 \
#     -switch_horizontal_masters 0 \
#     -switch_vertical_masters 0 \
#     -switch_angle_masters 0 \
#     -req_ack_mode 1 \
#     -both_phys_req_mode 1 \
#     -switch_debug 0 \
#     -traffic_verbose 0 \
#     -flit_dump 0 \
#     -buffer_verbose 0 \
#     -buffer_mid 0 \
#     -log_file_name ./results/validation

# ../build/noxim \
#     -config ../config_examples/latencies/mesh_10x10_dor.yaml \
#     -pir 0.7 poisson \
#     -sim 10000 \
#     -detailed \
#     -req_ack_mode 1 \
#     -both_phys_req_mode 1 \
#     -log_file_name ./results/latencies/validation_short

# ../build/noxim \
#     -config ../config_examples/latencies/mesh_10x10_dor.yaml \
#     -pir 0.7 poisson \
#     -sim 100000 \
#     -detailed \
#     -req_ack_mode 1 \
#     -both_phys_req_mode 1 \
#     -log_file_name ./results/latencies/validation_long

../build/noxim \
    -config ../config_examples/strawman/mesh_18x18_6ch_dor.yaml \
    -pir 1.0 poisson \
    -detailed \
    -req_ack_mode 1 \
    -both_phys_req_mode 1 \
    -log_file_name ./results/strawman/validation_6ch


# ../bin/noxim \
#     -config ../config_examples/mesh_14x14_interliving.yaml \
#     -pir 1.00 poisson \
#     -routing DOR \
#     -interliving_reps 0 \
#     -sim 10000 \
#     -detailed \
#     -mem_ports 1 \
#     -switch_horizontal_masters 0 \
#     -switch_vertical_masters 0 \
#     -switch_angle_masters 0 \
#     -req_ack_mode 1 \
#     -switch_debug 0 \
#     -log_file_name ./results/validation_14x14_switch_vertical_all_ro.csv

../bin/noxim \
    -config ../config_examples/mesh_14x14_interliving.yaml \
    -pir 1.00 poisson \
    -routing MOD_DOR \
    -interliving_reps 0 \
    -sim 10000 \
    -detailed \
    -mem_ports 4 \
    -switch_horizontal_masters 2 \
    -switch_vertical_masters 0 \
    -switch_angle_masters 0 \
    -req_ack_mode 0 \
    -switch_debug 0 \
    -log_file_name ./results/validation_14x14_switch_vertical_all_wo.csv

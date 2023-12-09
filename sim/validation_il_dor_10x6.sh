# Interliving feature pipeline valildation
../bin/noxim \
    -config ../config_examples/mesh_10x6.yaml \
    -pir 1.00 poisson \
    -routing DOR \
    -interliving_reps 0 \
    -sim 1000 \
    -detailed \
    -mem_ports 4 \
    -switch_horizontal_masters 0 \
    -log_file_name ./results/validation_mdor_10x6.csv

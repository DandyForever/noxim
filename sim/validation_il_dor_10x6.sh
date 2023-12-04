# Interliving feature pipeline valildation
../bin/noxim \
    -config ../config_examples/mesh_10x6.yaml \
    -pir 1.00 poisson \
    -routing DOR \
    -interliving_reps 0 \
    -sim 10000 \
    -detailed \
    -mem_ports 4 \
    -log_file_name ./results/validation_mdor_10x6.csv

# Interliving feature pipeline valildation
../bin/noxim \
    -config ../config_examples/mesh_10x10_interliving.yaml \
    -pir 1.00 poisson \
    -routing DOR \
    -interliving_reps 1 \
    -sim 10000 \
    -detailed \
    -mem_ports 1 \
    -log_file_name ./results/validation_il_10x6.csv

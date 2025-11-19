../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_mod_dor_q1.yaml \
    -pir 0.5 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q1 &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_dor_q2.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q2 &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_dor_q4.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q4 &

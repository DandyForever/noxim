../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_dor_q1.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q1_dor &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_dor_q2.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q2_dor &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_dor_q4.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q4_dor &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_mod_dor_q1.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q1_mod_dor &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_mod_dor_q2.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q2_mod_dor &

../../build/noxim \
    -config ../../config_examples/quadrants/mesh_10x10_mod_dor_q4.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/quadrants/validation_q4_mod_dor &

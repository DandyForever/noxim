../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_18x10_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_base &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_18x10_mod_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_perfect &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_24x10_piramide.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_piramide &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_24x10_stair.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_stair &

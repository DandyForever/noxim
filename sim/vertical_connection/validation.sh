../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_18x10_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_base &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_18x10_mod_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_perfect &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_24x10_piramide.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_piramide &

../../build/noxim \
    -config ../../config_examples/vertical_connection/mesh_24x10_stair.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/vertical_connection/validation_stair &

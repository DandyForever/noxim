../../build/noxim \
    -config ../../config_examples/perfect_routing/mesh_18x10_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/perfect_routing/horizontal_dor &

../../build/noxim \
    -config ../../config_examples/perfect_routing/mesh_18x10_xy.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/perfect_routing/vertical_xy &

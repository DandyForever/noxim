../../build/noxim \
    -config ../../config_examples/phd_2/mesh_10x10_dor_1p_xbar.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_2/validation_dor_1p_xbar &

../../build/noxim \
    -config ../../config_examples/phd_2/mesh_10x10_dor_1p.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_2/validation_dor_1p &

../../build/noxim \
    -config ../../config_examples/phd_2/mesh_10x10_dor_4p.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_2/validation_dor_4p &

../../build/noxim \
    -config ../../config_examples/phd_2/mesh_10x10_mod_dor_1p_xbar.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_2/validation_mod_dor_1p_xbar &

../../build/noxim \
    -config ../../config_examples/phd_2/mesh_10x10_mod_dor_1p.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_2/validation_mod_dor_1p &

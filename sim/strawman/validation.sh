../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x10_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_1 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x10_mod_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_2 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x18_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_3 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x18_mod_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_4 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x18_6ch_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_5 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x18_6ch_mod_dor.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_6 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x10_horizontal_yx.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_7_1 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x10_vertical_xy.yaml \
    -pir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_7_2 &

../../build/noxim \
    -config ../../config_examples/strawman/mesh_18x10_mod_dor.yaml \
    -pir 0.5 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/strawman/validation_8 &

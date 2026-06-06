../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x6_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_1 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x6_mod_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_2 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x10_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_3 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x10_mod_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_4 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x10_6ch_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_5 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x10_6ch_mod_dor.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_6 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x6_horizontal_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_7_1 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x6_vertical_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_7_2 &

../../build/noxim \
    -config ../../config_examples/phd_4/mesh_10x6_mod_dor.yaml \
    -ir 0.5 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/phd_4/validation_8 &

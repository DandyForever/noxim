../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_2vc_yx_yx_xy_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_2vc_yx_yx_xy_xy &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_2vc_yx_yx_yx_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_2vc_yx_yx_yx_yx &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_3vc_yx_xy_xy_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_3vc_yx_xy_xy_xy &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_3vc_yx_xy_yx_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_3vc_yx_xy_yx_yx &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_3vc_yx_yx_xy_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_3vc_yx_yx_xy_yx &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_3vc_yx_yx_yx_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_3vc_yx_yx_yx_xy &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_4vc_xy_yx_xy_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_4vc_xy_yx_xy_yx &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_4vc_xy_yx_yx_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_4vc_xy_yx_yx_xy &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_4vc_yx_xy_xy_yx.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_4vc_yx_xy_xy_yx &

../../build/noxim \
    -config ../../config_examples/routing_study/mesh_10x10_4vc_yx_xy_yx_xy.yaml \
    -ir 1.0 poisson \
    -sim 10000 \
    -detailed \
    -log_file_name ../results/routing_study/validation_4vc_yx_xy_yx_xy &

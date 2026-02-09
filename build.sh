docker run --rm \
    -e MESON_ARGS='--verbose -Dbuildtype=debug -Dpython:optimize=false -Db_sanitize=address,undefined -fsanitize=address,undefined -fno-omit-frame-pointer' \
    -v $(pwd):/retrace -v $(pwd)/dist:/output -w /retrace \
    retrace-build:latest \
    pip wheel -Csetup-args="-Dbuildtype=debug" --no-build-isolation --wheel-dir=/output .

#!/bin/bash
set -euo pipefail

# Configuration
COMPILER_PATH=${COMPILER_PATH:-"/opt/gcc-7.3.0-x86_64/bin"}
PATH=${COMPILER_PATH}:$PATH

VERSION=$(make ARCH=x86_64 kernelversion)
[[ -z "$VERSION" ]] && { echo "Error: Couldn't determine kernel version"; exit 1; }

VERSION_PATCH=${VERSION%.*}
LOAD_DIR="/home/public/RND/loads/linux/${VERSION_PATCH}"

# Find the latest build ID by checking existing directories
find_latest_build() {
    local last_id=0
    if [[ -d "${LOAD_DIR}" ]]; then
        # Use find to avoid ARG_MAX issues with large number of directories
        while IFS= read -r -d '' dir; do
            local id="${dir##*.solos}"
            [[ "$id" =~ ^[0-9]+$ ]] && ((id > last_id)) && last_id=$id
        done < <(find "${LOAD_DIR}" -maxdepth 1 -type d -name "${VERSION}.solos*" -print0)
    fi
    echo "$last_id"
}

# Update .config with new build version
update_config_version() {
    local build_id=$1
    [[ -f .config ]] && mv -f .config old.config
    sed "s/^CONFIG_LOCALVERSION=.*$/CONFIG_LOCALVERSION=\".solos${build_id}\"/" \
        old.config > .config
}

# Setup new version
setup_version() {
    local last_build_id
    last_build_id=$(find_latest_build)
    echo "Last build ID: ${last_build_id}"

    NEW_BUILD_ID=$((last_build_id + 1))
    NEW_VERSION="${VERSION}.solos${NEW_BUILD_ID}"
    echo "New version: ${NEW_VERSION}"

    update_config_version "$NEW_BUILD_ID"
}

# Run the actual kernel build
run_build() {
    local build_dir=$1
    shift
    ./build-env/run-dev-env-ol9-kernel make -C "$build_dir" ARCH=x86_64 "$@"
}

# Handle CI build (with BUILD_NUMBER set)
ci_build() {
    echo "CI build detected, publishing to ${LOAD_DIR}"

    # Tag and push
    git tag -a "v${NEW_VERSION}" -m "Build solos${NEW_BUILD_ID}"
    git push origin "v${NEW_VERSION}"

    # Setup build directory
    mkdir -p "$LOAD_DIR"
    local build_dir="${LOAD_DIR}/${NEW_VERSION}"
    mkdir "$build_dir"

    # Copy source and build
    echo "Copying kernel source to ${build_dir}"
    cp -diR . "$build_dir"

    local old_pwd="$PWD"
    cd "$build_dir"
    run_build "$build_dir" "$@"
    local rc=$?

    # Create source archive
    if [[ $rc -eq 0 ]]; then
        echo "Creating source archive..."
        git archive "v${NEW_VERSION}" | gzip > "kernel-${NEW_VERSION}-src.tar.gz"
        touch .keepme
    fi

    cd "$old_pwd"
    echo "Done!"
    return $rc
}

# Main execution
echo "Publish location: ${LOAD_DIR}"
setup_version

if [[ -n "${BUILD_NUMBER:-}" ]]; then
    ci_build "$@"
    rc=$?
else
    run_build . "$@"
    rc=$?
fi

exit $rc

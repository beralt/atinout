#!/bin/bash

# Exit on error and enable tracing
set -ex

# Name used for docker container
PROJECT=sabanto-modem-config
# Name used for docker container's image
PROJECT_IMAGE="${PROJECT}_image"

# Get the script's current directory
SCRIPT_DIR="$(dirname -- $(readlink -f "${0}"))"
# Get the script's parent directory
PARENT_DIR="$(dirname -- "${SCRIPT_DIR}")"
# Get the script's parent directory name
PARENT_DIR_NAME="$(basename -- "${PARENT_DIR}")"

# Stop the docker container if it's still running
docker stop "${PROJECT}" > /dev/null 2>&1 || true
# Build the docker container's image
docker buildx build \
--progress plain \
-t "${PROJECT_IMAGE}" \
"${PARENT_DIR}"
# Remove the container with this name if it already exists
docker rm -f "${PROJECT}" > /dev/null 2>&1 || true
# Remove unused data
docker system prune -f
mkdir -p ${PARENT_DIR}/build
# Run the docker container in detached mode with a bind mounted build directory
docker run --name "${PROJECT}" -u $(id -u):$(id -g) -v \
"${PARENT_DIR}:/tmp/${PARENT_DIR_NAME}" "${PROJECT_IMAGE}" \
bash -c "/tmp/atinout/scripts/build-deb.sh && cp /tmp/*.deb /tmp/atinout/build/ && \
make -C /tmp/atinout/telit-fw all && cp /tmp/atinout/telit-fw/build/m2mapz.bin /tmp/atinout/build/"

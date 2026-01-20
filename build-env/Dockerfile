ARG BASE_REGISTRY
ARG BASE_REPO
ARG BASE_TAG

FROM ${BASE_REGISTRY}/${BASE_REPO}:${BASE_TAG}

# Install additional packages needed for kernel building
RUN dnf reinstall -y flex \
    && dnf clean all

WORKDIR /root

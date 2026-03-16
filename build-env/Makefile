include version.mk

.PHONY: all
all: docker-build

.PHONY: docker-pull
docker-pull:
	@echo "Checking if base image exists locally..."
	@if ! docker image inspect $(OL9_RUN_DEV_ENV_REGISTRY)/$(OL9_RUN_DEV_ENV_REPO):$(OL9_RUN_DEV_ENV_REPO_TAG) >/dev/null 2>&1; then \
		echo "Base image not found locally. Pulling $(OL9_RUN_DEV_ENV_REGISTRY)/$(OL9_RUN_DEV_ENV_REPO):$(OL9_RUN_DEV_ENV_REPO_TAG)..."; \
		docker pull $(OL9_RUN_DEV_ENV_REGISTRY)/$(OL9_RUN_DEV_ENV_REPO):$(OL9_RUN_DEV_ENV_REPO_TAG); \
	else \
		echo "Base image already exists locally."; \
	fi

.PHONY: docker-build
docker-build: docker-pull
	docker build ./ \
		-t $(OL9_RUN_DEV_ENV_REGISTRY)/$(OL9_RUN_DEV_ENV_REPO):$(OL9_RUN_DEV_ENV_KERNEL_REPO_TAG) \
		--build-arg BASE_REGISTRY=$(OL9_RUN_DEV_ENV_REGISTRY) \
		--build-arg BASE_REPO=$(OL9_RUN_DEV_ENV_REPO) \
		--build-arg BASE_TAG=$(OL9_RUN_DEV_ENV_REPO_TAG)

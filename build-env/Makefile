include version.mk

.PHONY: all
all: docker-build

.PHONY: docker-build
docker-build:
	docker build ./ \
		-t $(OL9_RUN_DEV_ENV_REGISTRY)/$(OL9_RUN_DEV_ENV_REPO):$(OL9_RUN_DEV_ENV_KERNEL_REPO_TAG) \
		--build-arg BASE_REGISTRY=$(OL9_RUN_DEV_ENV_REGISTRY) \
		--build-arg BASE_REPO=$(OL9_RUN_DEV_ENV_REPO) \
		--build-arg BASE_TAG=$(OL9_RUN_DEV_ENV_REPO_TAG)

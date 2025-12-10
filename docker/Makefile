ifneq (,$(wildcard .env))
    include .env
endif

setup-env-image:
	docker buildx build \
		--build-arg VULKAN_SDK_VERSION="$(VULKAN_SDK_VERSION)" \
		-f docker/env.Dockerfile \
		-t toki_env_image .

create-env-container: setup-env-image
	docker create --name toki_env_container toki_env_image 2>/dev/null || true
	docker cp toki_env_container:/env_dir/vendor .

arch-image:
	docker buildx build \
		-f docker/arch.Dockerfile \
		-t toki_arch_image .

arch-build: arch-image
	docker run --env-file=.env --volume .:/toki toki_arch_image:latest

debian-image:
	docker buildx build \
		-f docker/debian.Dockerfile \
		-t toki_debian_image .

debian-build: debian-image
	docker run --env-file=.env --volume .:/toki toki_debian_image:latest

fedora-image:
	docker buildx build \
		-f docker/fedora.Dockerfile \
		-t toki_fedora_image .

fedora-build: fedora-image
	docker run --env-file=.env --volume .:/toki toki_fedora_image:latest

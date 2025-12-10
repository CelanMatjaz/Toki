FROM debian:stable

RUN apt update
# VULKAN SDK packages
RUN apt install -y libglm-dev cmake libxcb-dri3-0 libxcb-present0 libpciaccess0 \
libpng-dev libxcb-keysyms1-dev libxcb-dri3-dev libx11-dev g++ gcc \
libwayland-dev libxrandr-dev libxcb-randr0-dev libxcb-ewmh-dev \
git python-is-python3 bison libx11-xcb-dev liblz4-dev libzstd-dev \
ocaml-core ninja-build pkg-config libxml2-dev wayland-protocols python3-jsonschema \
clang-format qtbase5-dev qt6-base-dev xz-utils wget clang python3-pip 

RUN apt install -y libxcb-xinput0 libxcb-xinerama0 libxcb-cursor-dev

# GLFW packages
RUN apt install -y libwayland-dev libxkbcommon-dev xorg-dev

WORKDIR /toki

COPY scripts scripts
RUN pip install --no-cache-dir --break-system-packages -r scripts/requirements.txt

ENV TARGET_OS="debian"
ENV VENDOR_DIR=/toki/vendor
ENV BUILD_DIR=/toki/build/${TARGET_OS}
ENV CMAKE_FLAGS=-DTOKI_USE_GLFW=ON

COPY .env /tmp
RUN . /tmp/.env

CMD ["sh", "-c", "cmake -S . -B ${BUILD_DIR} ${CMAKE_FLAGS} && cmake --build ${BUILD_DIR}"]

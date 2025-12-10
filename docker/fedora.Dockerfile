FROM fedora:44

RUN dnf -y update

# VULKAN_SDK packages
RUN dnf -y install glibc-devel @development-tools glm-devel cmake libpng-devel wayland-devel \
libpciaccess-devel libX11-devel libXpresent libxcb xcb-util libxcb-devel libXrandr-devel \
xcb-util-keysyms-devel xcb-util-wm-devel python3 git lz4-devel libzstd-devel python3-distutils-extra qt \
gcc-g++ wayland-protocols-devel ninja-build python3-jsonschema qt5-qtbase-devel qt6-qtbase-devel python3 python3-pip clang
RUN dnf -y install xinput libXinerama xcb-util-cursor

# GLFW packages
RUN dnf -y install wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel

WORKDIR /toki

COPY scripts scripts
RUN pip install --no-cache-dir --break-system-packages -r scripts/requirements.txt

ENV TARGET_OS="fedora"
ENV VENDOR_DIR=/toki/vendor
ENV BUILD_DIR=/toki/build/${TARGET_OS}
ENV CMAKE_FLAGS=-DTOKI_USE_GLFW=ON -DCMAKE_EXE_LINKER_FLAGS="-lm"

COPY .env /tmp
RUN source /tmp/.env

CMD ["sh", "-c", "cmake -S . -B ${BUILD_DIR} ${CMAKE_FLAGS} && cmake --build ${BUILD_DIR}"]

FROM archlinux:multilib-devel

# VULKAN SDK packages
RUN pacman --noconfirm -Syyu base-devel cmake libpng wayland libpciaccess libx11 libxpresent \
libxcb xcb-util libxrandr xcb-util-keysyms xcb-util-wm python git lz4 zstd python-distlib qt5-base \
wayland-protocols ninja python3 python-pip clang
RUN pacman --noconfirm -S libxcb libxinerama xcb-util-cursor

# GLFW packages
RUN pacman --noconfirm -S wayland wayland-protocols libxkbcommon libxcb xcb-util mesa libxcursor

WORKDIR /toki

COPY scripts scripts
RUN pip install --no-cache-dir --break-system-packages -r scripts/requirements.txt

ENV TARGET_OS="arch"
ENV VENDOR_DIR=/toki/vendor
ENV BUILD_DIR=/toki/build/${TARGET_OS}
ENV CMAKE_FLAGS=-DTOKI_USE_GLFW=ON

COPY .env /tmp
RUN source /tmp/.env

CMD ["sh", "-c", "cmake -S . -B ${BUILD_DIR} ${CMAKE_FLAGS} && cmake --build ${BUILD_DIR}"]

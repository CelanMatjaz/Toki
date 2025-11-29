FROM ubuntu:24.04

ARG TARGET=Linux

RUN apt update
RUN apt install -y mesa-vulkan-drivers vulkan-tools
RUN apt install -y xz-utils
RUN apt install -y cmake libxcb-present0 libpciaccess0 \
libpng-dev libxcb-keysyms1-dev libxcb-dri3-dev libx11-dev g++ gcc \
libwayland-dev libxrandr-dev libxcb-randr0-dev libxcb-ewmh-dev \
git python3 bison libx11-xcb-dev liblz4-dev libzstd-dev python-is-python3 \
ocaml-core ninja-build pkg-config libxml2-dev wayland-protocols qtcreator \
qtbase5-dev qt5-qmake qtbase5-dev-tools
RUN apt install -y pip
RUN apt install -y libxkbcommon0 libxkbcommon-dev libxinerama-dev
RUN apt install -y libwayland-dev libxkbcommon-dev xorg-dev
RUN apt install -y clang g++

WORKDIR /toki

COPY ./scripts /toki/scripts
COPY ./Makefile /toki/Makefile
COPY ./.env /toki/.env

RUN pip install --break-system-packages -r /toki/scripts/requirements.txt

CMD ["make", "docker-build"]

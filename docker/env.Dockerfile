FROM alpine:latest

ARG VULKAN_SDK_VERSION=1.4.335.0

WORKDIR /env_dir
RUN apk add wget python3 py3-pip

COPY scripts /scripts
RUN pip3 install --no-cache-dir --break-system-packages -r /scripts/requirements.txt

RUN python /scripts/setup_vulkan.py

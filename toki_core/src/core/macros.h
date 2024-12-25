#pragma once

#define DELETE_MOVE(type)        \
    type(type&& other) = delete; \
    type& operator=(type&& other) = delete;

#define DELETE_COPY(type)        \
    type(const type& other) = delete; \
    type& operator=(const type& other) = delete;

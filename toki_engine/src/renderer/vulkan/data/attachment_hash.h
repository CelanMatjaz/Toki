#pragma once

#include <vector>

#include "core/base.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/defines.h"

namespace toki {

class AttachmentsHash {
public:
    AttachmentsHash() = default;
    AttachmentsHash(const std::vector<Attachment>& attachments);
    ~AttachmentsHash() = default;

    bool operator==(const AttachmentsHash& other) const;
    bool operator<(const AttachmentsHash& other) const;
    u64 operator()(const AttachmentsHash& h) const;

    operator bool() const;

private:
    union {
        u64 combined;
        u8 seperate[MAX_FRAMEBUFFER_ATTACHMENTS];
    } m_data{};
};

}  // namespace toki

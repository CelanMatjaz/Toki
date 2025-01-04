#include "mapping_functions.h"

#include <utility>

namespace toki {

VkFormat map_format(ColorFormat format) {
    switch (format) {
        case ColorFormat::R8:
            return VK_FORMAT_R8_SRGB;
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;
    }

    std::unreachable();
}

VkAttachmentLoadOp map_attachment_load_op(AttachmentLoadOp op) {
    switch (op) {
        case AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    std::unreachable();
}

VkAttachmentStoreOp map_attachment_load_op(AttachmentStoreOp op) {
    switch (op) {
        case AttachmentStoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case AttachmentStoreOp::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    std::unreachable();
}

}  // namespace toki

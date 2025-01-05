#include "attachment_hash.h"

#include "core/assert.h"
#include "core/macros.h"

namespace toki {

AttachmentsHash::AttachmentsHash(const std::vector<Attachment>& attachments) {
    TK_ASSERT(
        attachments.size() <= MAX_FRAMEBUFFER_ATTACHMENTS,
        "Only {} framebuffer attachments supported, provided attachment count: {}",
        MAX_FRAMEBUFFER_ATTACHMENTS,
        attachments.size());

    for (u32 i = 0; i < attachments.size(); ++i) {
        // https://docs.vulkan.org/spec/latest/chapters/renderpass.html#renderpass-compatibility
        // Add high byte for sample count if ever added to Attachment struct
        // Code to add to end of line: | HIGH_BYTE(attachments[i].sampleCount)
        m_data.seperate[i] = LOW_BYTE(attachments[i].colorFormat);
    }
}

bool AttachmentsHash::operator==(const AttachmentsHash& other) const {
    return m_data.combined == other.m_data.combined;
}

bool AttachmentsHash::operator<(const AttachmentsHash& other) const {
    return m_data.combined < other.m_data.combined;
}

u64 AttachmentsHash::operator()(const AttachmentsHash& h) const {
    return h.m_data.combined;
}

AttachmentsHash::operator bool() const {
    return m_data.combined != 0;
}

}  // namespace toki

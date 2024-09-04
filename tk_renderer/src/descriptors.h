#include <toki/core.h>

#include "renderer_types.h"

namespace Toki {

// TODO: Update functions to and out param and remove descriptor pool from VulkanState
TkError create_descriptor_pool(VulkanState* state);
void destroy_descriptor_pool(VulkanState* state);

}  // namespace Toki

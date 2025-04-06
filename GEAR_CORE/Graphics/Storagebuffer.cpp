#include "Graphics/Storagebuffer.h"

using namespace gear;
using namespace graphics;

template class ARC_EXPORT Storagebuffer<UniformBufferStructures::Camera>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::Lights>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::SpecularIrradianceInfo>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::BloomInfo>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::ProbeInfo>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::DebugProbeInfo>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::Model>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::HDRInfo>;
template class ARC_EXPORT Storagebuffer<UniformBufferStructures::PBRConstants>;
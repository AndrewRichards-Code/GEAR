#include "Graphics/Uniformbuffer.h"

using namespace gear;
using namespace graphics;

template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::Camera>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::Lights>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::BloomInfo>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::ProbeInfo>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::DebugProbeInfo>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::Model>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::HDRInfo>;
template class ARC_EXPORT Uniformbuffer<UniformBufferStructures::PBRConstants>;
#ifndef SAMPLERS
#define SAMPLERS

#include "../../../bufferslots.h"

SamplerState SmpUnfiltered	: SMP_REGISTER(SMP_UNFILTERED);
SamplerState SmpFiltered	: SMP_REGISTER(SMP_FILTERED);
SamplerState SmpConfig		: SMP_REGISTER(SMP_CONFIG); // if theres an option to enable/disable texture filtering
SamplerState SmpShadow		: SMP_REGISTER(SMP_SHADOWMAP);
SamplerState SmpRenderScale	: SMP_REGISTER(SMP_RENDER_SCALE);


#endif
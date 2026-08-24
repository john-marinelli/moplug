#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* MojoDSPHandle;

MojoDSPHandle mojo_dsp_create();

void mojo_dsp_destroy(MojoDSPHandle handle);

void mojo_dsp_prepare(
    MojoDSPHandle handle,
    double sampleRate,
    int32_t maxBlockSize,
    int32_t inputChannels,
    int32_t outputChannels
);

void mojo_dsp_reset(
    MojoDSPHandle handle
);

void mojo_dsp_process_f32(
    MojoDSPHandle handle,
    float** inputs,
    float** outputs,
    int32_t inputChannels,
    int32_t outputChannels,
    int32_t frameCount
);

void mojo_dsp_set_parameter(
    MojoDSPHandle handle,
    uint32_t parameterId,
    double normalizedValue
);

size_t mojo_dsp_state_size(
    MojoDSPHandle handle
);

bool mojo_dsp_serialize_state(
    MojoDSPHandle handle,
    void* destination,
    size_t destinationSize
);

bool mojo_dsp_deserialize_state(
    MojoDSPHandle handle,
    const void* source,
    size_t sourceSize
);

#ifdef __cplusplus
}
#endif

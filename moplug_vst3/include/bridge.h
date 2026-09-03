#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MOPLUG_SAMPLE_F32,
    MOPLUG_SAMPLE_F64
} MoPlugSampleFormat;

typedef struct
{
    void** channels;
    int32_t channel_count;
} MoPlugAudioBus;

typedef enum
{
    MOPLUG_EVENT_NOTE_ON,
    MOPLUG_EVENT_NOTE_OFF,
    MOPLUG_EVENT_POLY_PRESSURE,
    MOPLUG_EVENT_OTHER
} MoPlugEventType;

typedef struct
{
    MoPlugEventType type;

    int32_t sample_offset;
    int32_t channel;

    int32_t note;
    int32_t note_id;

    float value;
} MoPlugEvent;

typedef struct
{
    uint32_t id;
    double value;
    int32_t sample_offset;
} MoPlugParamChange;

typedef struct
{
    uint32_t state;
    double sample_rate;

    int64_t project_time_samples;
    int64_t continuous_time_samples;

    double project_time_beats;
    double bar_position_beats;

    double tempo;

    int32_t time_sig_numerator;
    int32_t time_sig_denominator;

    double cycle_start_beats;
    double cycle_end_beats;

} MoPlugTransport;

typedef struct
{
    int32_t frames;
    MoPlugSampleFormat sample_format;

    MoPlugAudioBus* inputs;
    int32_t input_bus_count;

    MoPlugAudioBus* outputs;
    int32_t output_bus_count;

    const MoPlugEvent* input_events;
    int32_t input_event_count;

    MoPlugEvent* output_events;
    int32_t output_event_capacity;
    int32_t output_event_count;
    
    const MoPlugParamChange* input_params;
    int32_t input_param_count;

    MoPlugParamChange* output_params;
    int32_t output_param_capacity;
    int32_t output_param_count;

    const MoPlugTransport* transport;
} MoPlugProcessData;

typedef void* MojoDSPHandle;

typedef struct ParamDescriptor
{
    uint32_t id;
    const char* name;
    const char* units;
    double default_value;
    uint32_t step_count;
    uint32_t flags;
} ParamDescriptor;

typedef struct PluginDescriptor
{
    const char* stable_id;
    const char* display_name;
    const char* version;

    uint32_t parameter_count;
    const ParamDescriptor* parameters;
} PluginInfo;

typedef struct {
    float** inputs;
    float** outputs;

    int32_t input_channels;
    int32_t output_channels;
    int32_t frames;

} VstAudioBlock;

const PluginDescriptor* mojo_get_plugin_descriptor();

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
    VstAudioBlock* block
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

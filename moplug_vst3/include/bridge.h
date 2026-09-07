#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MOPLUG_SAMPLE_F32 = 0,
    MOPLUG_SAMPLE_F64 = 1
} MoPlugSampleFormat;

typedef struct
{
    void** channels;
    int32_t channel_count;
} MoPlugAudioBus;

typedef enum
{
    MOPLUG_EVENT_NOTE_ON = 0,
    MOPLUG_EVENT_NOTE_OFF = 1,
    MOPLUG_EVENT_POLY_PRESSURE = 2,
    MOPLUG_EVENT_OTHER = 3
} MoPlugEventType;

typedef struct
{
    MoPlugEventType type;

    int32_t sample_offset;
    int32_t bus_index;

    int16_t channel;
    int16_t pitch;

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

typedef struct MoPlugParamDescriptor
{
    uint32_t id;

    const char* name;
    const char* units;

    double default_value;

    uint32_t step_count;
    uint32_t flags;
} MoPlugParamDescriptor;

typedef enum
{
    MOPLUG_BUS_AUDIO = 0,
    MOPLUG_BUS_EVENT = 1
} MoPlugBusType;

typedef enum
{
    MOPLUG_BUS_INPUT = 0,
    MOPLUG_BUS_OUTPUT = 1
} MoPlugBusDirection;

typedef struct MoPlugBusDescriptor
{
    const char* name;

    MoPlugBusType type;
    MoPlugBusDirection direction;

    int32_t channels;

    uint32_t flags;
} MoPlugBusDescriptor;

typedef struct MoPlugDescriptor
{
    const char* stable_id;
    const char* display_name;
    const char* version;

    const char* company;
    const char* website;
    const char* email;

    uint32_t bus_count;
    const MoPlugBusDescriptor* buses;

    uint32_t param_count;
    const MoPlugParamDescriptor* params;
} MoPlugDescriptor;

typedef struct {
    float** inputs;
    float** outputs;

    int32_t input_channels;
    int32_t output_channels;
    int32_t frames;

} VstAudioBlock;

const MoPlugDescriptor* mojo_get_plugin_descriptor();

MojoDSPHandle mojo_dsp_create();

void mojo_runtime_initialize(void);

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

void mojo_dsp_process(
    MojoDSPHandle handle,
    MoPlugProcessData* block
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

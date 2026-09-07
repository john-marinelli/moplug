from std.memory import Pointer, bitcast, Layout, alloc
from std.origin import MutUntrackedOrigin, ImmUntrackedOrigin
from std.traits import ImplicitlyCopyable

comptime FFloatPtr = Pointer[Float32, MutUntrackedOrigin]
comptime FChannelPtr = Pointer[FFloatPtr, MutUntrackedOrigin]
comptime FBytePtr = Pointer[UInt8, MutUntrackedOrigin]
comptime VstAudioBlockPtr = Pointer[VstAudioBlock, MutUntrackedOrigin]
comptime VoidDoublePtr = Pointer[Pointer[NoneType, MutUntrackedOrigin], MutUntrackedOrigin]
comptime F32ChannelsPtr = Pointer[Pointer[Float32, MutUntrackedOrigin], MutUntrackedOrigin]
comptime F64ChannelsPtr = Pointer[Pointer[Float64, MutUntrackedOrigin], MutUntrackedOrigin]
comptime ImmCStr = Pointer[UInt8, ImmUntrackedOrigin]

comptime MoPlugSampleFormat = Int32
comptime MoPlugEventType = Int32
comptime MoPlugBusType = Int32
comptime MoPlugBusDirection = Int32

def leak_c_str(text: String) -> ImmCStr:
    var bytes = text.as_bytes()
    var owned = alloc(Layout[UInt8](count=text.byte_length()+1))
    var ptr = owned^.unsafe_leak()

    for i in range(text.byte_length()):
        ptr[unsafe_offset=i] = bytes[i]
    ptr[unsafe_offset=text.byte_length()] = 0

    return ptr.as_imm().unsafe_origin_cast[ImmUntrackedOrigin]()

@fieldwise_init
struct MoPlugBusDescriptor:
    var name: ImmCStr

    var type: MoPlugBusType
    var direction: MoPlugBusDirection

    var channels: Int32

    var flags: UInt32

@fieldwise_init
struct MoPlugParamDescriptor:
    var id: UInt32

    var name: ImmCStr
    var units: ImmCStr

    var default_value: Float64

    var step_count: UInt32
    var flags: UInt32


@fieldwise_init
struct MoPlugDescriptor(ImplicitlyCopyable):
    var stable_id: ImmCStr
    var display_name: ImmCStr
    var version: ImmCStr

    var company: ImmCStr
    var website: ImmCStr
    var email: ImmCStr

    var bus_count: UInt32
    var buses: Pointer[MoPlugBusDescriptor, ImmUntrackedOrigin]

    var param_count: UInt32
    var params: Pointer[MoPlugParamDescriptor, ImmUntrackedOrigin]


@fieldwise_init
struct MoPlugAudioBusF32:
    var channels: F32ChannelsPtr
    var channel_count: Int32

    def __init__(out self, ref other: MoPlugAudioBusRaw):
        self.channel_count = other.channel_count
        self.channels = other.channels.unsafe_bitcast[FFloatPtr]()


@fieldwise_init
struct MoPlugAudioBusF64:
    var channels: F64ChannelsPtr
    var channel_count: Int32

@fieldwise_init
struct MoPlugAudioBusRaw:
    var channels: VoidDoublePtr
    var channel_count: Int32

@fieldwise_init
struct MoPlugEvent:
    var type: MoPlugEventType

    var sample_offset: Int32
    var bus_index: Int32
    
    var channel: Int16
    var pitch: Int16

    var note_id: Int32

    var value: Float32


@fieldwise_init
struct MoPlugParamChange:
    var id: UInt32
    var value: Float64
    var sample_offset: Int32

@fieldwise_init
struct MoPlugTransport:
    var state: UInt32
    var sample_rate: Float64

    var project_time_samples: Int64
    var continuous_time_samples: Int64
    
    var project_time_beats: Float64
    var bar_position_beats: Float64

    var tempo: Float64

    var time_sig_numerator: Int32
    var time_sig_denominator: Int32

    var cycle_start_beats: Float64
    var cycle_end_beats: Float64



struct MoPlugProcessData:
    var frames: Int32
    var sample_format: MoPlugSampleFormat

    var inputs: Pointer[MoPlugAudioBusF32, ImmUntrackedOrigin]
    var input_bus_count: Int32
    
    var outputs: Pointer[MoPlugAudioBusF32, MutUntrackedOrigin]
    var output_bus_count: Int32

    var input_events: Pointer[MoPlugEvent, MutUntrackedOrigin]
    var input_event_count: Int32

    var output_events: Pointer[MoPlugEvent, MutUntrackedOrigin]
    var output_event_capacity: Int32
    var output_event_count: Int32

    var input_params: Pointer[MoPlugParamChange, ImmUntrackedOrigin]
    var input_param_count: Int32

    var output_params: Pointer[MoPlugParamChange, MutUntrackedOrigin]
    var output_param_capacity: Int32
    var output_param_count: Int32

    var transport: Pointer[MoPlugTransport, MutUntrackedOrigin]

    def __init__(
        out self,
        frames: Int32,
        sample_format: MoPlugSampleFormat,
        inputs: Pointer[MoPlugAudioBusRaw, MutUntrackedOrigin],
        input_bus_count: Int32,
        outputs: Pointer[MoPlugAudioBusRaw, MutUntrackedOrigin],
        output_bus_count: Int32,
        input_events: Pointer[MoPlugEvent, MutUntrackedOrigin],
        input_event_count: Int32,
        output_events: Pointer[MoPlugEvent, MutUntrackedOrigin],
        output_event_capacity: Int32,
        output_event_count: Int32,
        input_params: Pointer[MoPlugParamChange, ImmUntrackedOrigin],
        input_param_count: Int32,
        output_params: Pointer[MoPlugParamChange, MutUntrackedOrigin],
        output_param_capacity: Int32,
        output_param_count: Int32,
        transport: Pointer[MoPlugTransport, MutUntrackedOrigin],
    ):
        self.frames = frames
        self.sample_format = sample_format
        self.inputs = inputs.unsafe_bitcast[MoPlugAudioBusF32]()
        self.input_bus_count = input_bus_count
        self.outputs = outputs.unsafe_bitcast[MoPlugAudioBusF32]()
        self.output_bus_count = output_bus_count
        self.input_events = input_events
        self.input_event_count = input_event_count
        self.output_events = output_events
        self.output_event_capacity = output_event_capacity
        self.output_event_count = output_event_count
        self.input_params = input_params
        self.input_param_count = input_param_count
        self.output_params = output_params
        self.output_param_count = output_param_count
        self.output_param_capacity = output_param_capacity
        self.transport = transport




@fieldwise_init
struct VstAudioBlock(ImplicitlyCopyable):
    var inputs: FChannelPtr
    var outputs: FChannelPtr

    var input_channels: Int32
    var output_channels: Int32
    var frames: Int32

    def _frame_in_bounds(self, channel: Int32, frame: Int32, channels: Int32, frames: Int32) -> Bool:
        if channel < 0 or channel >= channels:
            return False

        if frame < 0 or frame >= frames:
            return False

        return True

    def set_out_frame(mut self, channel: Int32, frame: Int32, value: Float32):

        if not self._frame_in_bounds(channel, frame, self.output_channels, self.frames):
            return

        self.outputs[unsafe_offset=channel][unsafe_offset=frame] = value

    def get_in_frame(self, channel: Int32, frame: Int32) -> Float32:

        if not self._frame_in_bounds(channel, frame, self.input_channels, self.frames):
            return 0.0

        return self.inputs[unsafe_offset=channel][unsafe_offset=frame]

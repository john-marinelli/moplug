from std.memory import Pointer, alloc, Layout
from std.origin import MutUntrackedOrigin

from mojo_bridge.types import FChannelPtr, VstAudioBlock


# TODO: import actual plugin, this is a testing stub
struct DspBridge:
    var sample_rate: Float64
    var max_block_size: Int

    var input_channels: Int
    var output_channels: Int

    var gain: Float32

    def __init__(out self):
        self.sample_rate = 44100.0
        self.max_block_size = 0
        self.input_channels = 0
        self.output_channels = 0

        self.gain = 1.0

    def prepare(
        mut self,
        sample_rate: Float64,
        max_block_size: Int,
        input_channels: Int,
        output_channels: Int,
    ):
        self.sample_rate = sample_rate
        self.max_block_size = max_block_size

        self.input_channels = input_channels
        self.output_channels = output_channels

    def reset(mut self):
        pass

    def set_parameter(
        mut self, parameter_id: UInt32, normalized_value: Float64
    ):
        if parameter_id == 0:
            self.gain = Float32(normalized_value)

    def process(
        mut self,
        block: VstAudioBlock,
    ):
        var channels = block.input_channels

        if block.output_channels < channels:
            channels = block.output_channels

        for channel in range(channels):
            var input = block.inputs[unsafe_offset=channel]
            var output = block.outputs[unsafe_offset=channel]

            for frame in range(block.frames):
                output[unsafe_offset=frame] = (
                    input[unsafe_offset=frame] * self.gain
                )


comptime DspPointer = Pointer[DspBridge, MutUntrackedOrigin]


def create_dsp() -> DspPointer:
    var allocation = alloc(Layout[DspBridge](count=1))

    var pointer = allocation^.unsafe_leak()
    pointer.unsafe_write(DspBridge())

    return pointer


def destroy_dsp(pointer: DspPointer):
    pointer.unsafe_deinit_pointee()
    pointer.unsafe_free()


def prepare_dsp(
    pointer: DspPointer,
    sample_rate: Float64,
    max_block_size: Int32,
    input_channels: Int32,
    output_channels: Int32,
):
    pointer[].prepare(
        sample_rate,
        Int(max_block_size),
        Int(input_channels),
        Int(output_channels),
    )


def reset_dsp(pointer: DspPointer):
    pointer[].reset()


def set_dsp_parameter(
    pointer: DspPointer, parameter_id: UInt32, normalized_value: Float64
):
    pointer[].set_parameter(parameter_id, normalized_value)


def process_dsp(
    pointer: DspPointer,
    inputs: FChannelPtr,
    outputs: FChannelPtr,
    input_channels: Int32,
    output_channels: Int32,
    frames: Int32,
):
    var block = VstAudioBlock(
        inputs, outputs, Int(input_channels), Int(output_channels), Int(frames)
    )

    pointer[].process(block)

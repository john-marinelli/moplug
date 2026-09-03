from std.memory import Pointer
from std.origin import MutUntrackedOrigin
from std.traits import ImplicitlyCopyable

comptime FFloatPtr = Pointer[Float32, MutUntrackedOrigin]
comptime FChannelPtr = Pointer[FFloatPtr, MutUntrackedOrigin]
comptime FBytePtr = Pointer[UInt8, MutUntrackedOrigin]
comptime VstAudioBlockPtr = Pointer[VstAudioBlock, MutUntrackedOrigin]

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

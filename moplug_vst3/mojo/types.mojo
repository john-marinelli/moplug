from std.memory import Pointer
from std.origin import MutUntrackedOrigin

comptime FFloatPtr = Pointer[Float32, MutUntrackedOrigin]
comptime FChannelPtr = Pointer[FFloatPtr, MutUntrackedOrigin]
comptime FBytePtr = Pointer[UInt8, MutUntrackedOrigin]


struct VstAudioBlock:
    var inputs: FChannelPtr
    var outputs: FChannelPtr

    var input_channels: Int
    var output_channels: Int
    var frames: Int

    def __init__(
        out self,
        inputs: FChannelPtr,
        outputs: FChannelPtr,
        input_channels: Int,
        output_channels: Int,
        frames: Int,
    ):
        self.inputs = inputs
        self.outputs = outputs
        self.input_channels = input_channels
        self.output_channels = output_channels

        self.frames = frames

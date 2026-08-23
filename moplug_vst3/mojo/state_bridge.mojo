from std.memory import Pointer
from std.origin import MutUntrackedOrigin, ImmUntrackedOrigin
from std.memory import bitcast

from .dsp_bridge import DspPointer

comptime MutBytes = Pointer[UInt8, MutUntrackedOrigin]
comptime ImmBytes = Pointer[UInt8, ImmUntrackedOrigin]

# TODO: This will not be constant,
# it will be a function of the number and type of parameters
comptime STATE_SIZE = 4


def dsp_state_size(dsp: DspPointer) -> UInt:
    return UInt(STATE_SIZE)


# TODO: These will also need to be refactored
# to take care of a variable number/types of params
def write_dsp_state(
    dsp: DspPointer,
    dest: MutBytes,
    dest_size: UInt,
) -> Bool:
    if dest_size < UInt(STATE_SIZE):
        return False

    var bits = bitcast[DType.uint32](dsp[].gain)

    dest[unsafe_offset=0] = UInt8(bits & 0xFF)
    dest[unsafe_offset=1] = UInt8((bits >> 8) & 0xFF)
    dest[unsafe_offset=2] = UInt8((bits >> 16) & 0xFF)
    dest[unsafe_offset=3] = UInt8((bits >> 24) & 0xFF)

    return True


def read_dsp_state(
    dsp: DspPointer,
    src: ImmBytes,
    src_size: UInt,
) -> Bool:
    if src_size < UInt(STATE_SIZE):
        return False

    var bits = (
        UInt32(src[unsafe_offset=0])
        | (UInt32(src[unsafe_offset=1]) << 8)
        | (UInt32(src[unsafe_offset=2]) << 16)
        | (UInt32(src[unsafe_offset=3]) << 24)
    )

    dsp[].gain = bitcast[DType.float32](bits)

    return True

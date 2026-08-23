from std.runtime import initialize_runtime
from std.memory import MutOpaquePointer, ImmOpaquePointer, bitcast
from std.origin import MutUntrackedOrigin, ImmUntrackedOrigin
from .types import FChannelPtr
from .dsp_bridge import (
    DspPointer,
    DspBridge,
    create_dsp,
    destroy_dsp,
    prepare_dsp,
    reset_dsp,
    process_dsp,
    set_dsp_parameter,
)
from .state_bridge import (
    MutBytes,
    ImmBytes,
    dsp_state_size,
    write_dsp_state,
    read_dsp_state,
)

comptime MutHandle = MutOpaquePointer[MutUntrackedOrigin]
comptime ImmHandle = ImmOpaquePointer[ImmUntrackedOrigin]


def _dsp_from_handle(handle: MutHandle) -> DspPointer:
    return handle.unsafe_bitcast[DspBridge]()


@export("mojo_runtime_initialize")
def mojo_runtime_initialize() abi("C"):
    initialize_runtime()


@export("mojo_dsp_create")
def mojo_dsp_create() abi("C") -> MutHandle:
    var pointer = create_dsp()

    return pointer.unsafe_bitcast[NoneType]()


@export("mojo_dsp_destroy")
def mojo_dsp_destroy(handle: MutHandle) abi("C"):
    var pointer = _dsp_from_handle(handle)

    destroy_dsp(pointer)


@export("mojo_dsp_prepare")
def mojo_dsp_prepare(
    handle: MutHandle,
    sample_rate: Float64,
    max_block_size: Int32,
    input_channels: Int32,
    output_channels: Int32,
) abi("C"):
    var pointer = _dsp_from_handle(handle)

    prepare_dsp(
        pointer,
        sample_rate,
        max_block_size,
        input_channels,
        output_channels,
    )


@export("mojo_dsp_reset")
def mojo_dsp_reset(handle: MutHandle) abi("C"):
    reset_dsp(_dsp_from_handle(handle))


@export("mojo_dsp_set_parameter")
def mojo_dsp_set_parameter(
    handle: MutHandle,
    parameter_id: UInt32,
    normalized_value: Float64,
) abi("C"):
    set_dsp_parameter(_dsp_from_handle(handle), parameter_id, normalized_value)


@export("mojo_dsp_process_f32")
def mojo_dsp_process_f32(
    handle: MutHandle,
    inputs: FChannelPtr,
    outputs: FChannelPtr,
    input_channels: Int32,
    output_channels: Int32,
    frames: Int32,
) abi("C"):
    process_dsp(
        _dsp_from_handle(handle),
        inputs,
        outputs,
        input_channels,
        output_channels,
        frames,
    )


@export("mojo_dsp_state_size")
def mojo_dsp_state_size(
    handle: MutHandle,
) abi("C") -> UInt:
    return dsp_state_size(_dsp_from_handle(handle))


@export("mojo_dsp_serialize_state")
def mojo_dsp_serialize_state(
    handle: MutHandle,
    destination: MutBytes,
    destination_size: UInt,
) abi("C") -> Bool:
    return write_dsp_state(
        _dsp_from_handle(handle), destination, destination_size
    )


@export("mojo_dsp_deserialize_state")
def mojo_dsp_deserialize_state(
    handle: MutHandle,
    source: ImmBytes,
    source_size: UInt,
) abi("C") -> Bool:
    return read_dsp_state(
        _dsp_from_handle(handle),
        source,
        source_size,
    )

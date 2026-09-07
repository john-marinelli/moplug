from std.runtime import initialize_runtime
from std.memory import MutOpaquePointer, ImmOpaquePointer, bitcast, Layout, alloc
from std.origin import MutUntrackedOrigin, ImmUntrackedOrigin
from mojo_bridge.types import FChannelPtr, MoPlugDescriptor, MoPlugBusDescriptor, MoPlugParamDescriptor, MoPlugProcessData, leak_c_str
from mojo_bridge.dsp_bridge import (
    DspPointer,
    DspBridge,
    create_dsp,
    destroy_dsp,
    prepare_dsp,
    reset_dsp,
    process_dsp,
    set_dsp_parameter,
)
from mojo_bridge.state_bridge import (
    MutBytes,
    ImmBytes,
    dsp_state_size,
    write_dsp_state,
    read_dsp_state,
)
from mojo_bridge.types import VstAudioBlockPtr

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


@export("mojo_dsp_process")
def mojo_dsp_process(
    handle: MutHandle,
    block_ptr: Pointer[MoPlugProcessData, MutUntrackedOrigin],
) abi("C"):
    process_dsp(
        _dsp_from_handle(handle),
        block_ptr,
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

@export("mojo_get_plugin_descriptor")
def mojo_get_plugin_descriptor() abi("C") -> Pointer[MoPlugDescriptor, ImmUntrackedOrigin]:
    var bus_owned = alloc(Layout[MoPlugBusDescriptor](count=2))
    var buses = bus_owned^.unsafe_leak()
    buses.unsafe_offset(0).unsafe_write(MoPlugBusDescriptor(leak_c_str("Input"), 0, 0, 2, 0))
    buses.unsafe_offset(1).unsafe_write(MoPlugBusDescriptor(leak_c_str("Output"), 0, 1, 2, 0))
    var param_owned = alloc(Layout[MoPlugParamDescriptor](count=1))
    var params = param_owned^.unsafe_leak()
    params.unsafe_offset(0).unsafe_write(MoPlugParamDescriptor(0, leak_c_str("Gain"), leak_c_str("dB"), 0.5, 0, 1))
    var desc = MoPlugDescriptor(
        leak_c_str("testing.com.whatever"),
        leak_c_str("Testing Gain"),
        leak_c_str("0.0.1"),
        leak_c_str("Company"),
        leak_c_str("website.com"),
        leak_c_str("email@email.com"),
        2,
        buses,
        1,
        params,
    )

    var allocation = alloc(Layout[MoPlugDescriptor](count=1))
    var pointer = allocation^.unsafe_leak()
    pointer.unsafe_write(desc)

    return pointer.as_imm().unsafe_origin_cast[ImmUntrackedOrigin]()



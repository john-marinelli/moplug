

struct FloatParam:
    var id: UInt32
    var name: String
    var units: String

    var default_value: Float64

    var max_value: Float64
    var min_value: Float64

    var value: Float64
    
    var automatable: Bool
    var read_only: Bool
    var wrap_around: Bool
    var hidden: Bool
    var program_change: Bool
    var bypass: Bool


struct BypassParam:
    var id: UInt32
    var name: String

    var default_value: Bool

    var value: Bool

    var automatable: Bool

    def __init__(
        out self,
        name: String,
        default_value: Bool = False,
        automatable: Bool = True
    ):
        self.id = 
        self.name = name
        self.default_value = default_value
        self.value = self.default_value
        self.automatable = automatable





struct SelectParam:
    var id: UInt32
    var name: String
    var units: String
    var default_value: Float64

    var step_count: UInt32
    var stepped: Bool

    def __init__(
        out self,
        name: String,
        units: String = "",
        default_value: Float64 = 0.0,
        step_count: UInt32 = 0,
    ):
        pass



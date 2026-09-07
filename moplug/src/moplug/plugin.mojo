trait MoPlugPlugin:
    def prepare(mut self, sample_rate: Float64): ...

    # TODO: fill out this trait, not sure whether prepare should have more than just
    # sample rate

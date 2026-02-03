import PyOpenColorIO as ocio

class OcioProfile:

    @property
    def config(self: OcioProfile) -> ocio.Config:
        ...

    @config.setter
    def config(self: OcioProfile, config: ocio.Config) -> None:
        ...

    @property
    def working_space(self: OcioProfile) -> str:
        ...

    @working_space.setter
    def working_space(self: OcioProfile, value: str) -> None:
        ...

    @property
    def view_transform(self: OcioProfile) -> str:
        ...

    @view_transform.setter
    def view_transform(self: OcioProfile, value: str) -> None:
        ...

    @property
    def display_transform(self: OcioProfile) -> str:
        ...

    @display_transform.setter
    def display_transform(self: OcioProfile, value: str) -> None:
        ...
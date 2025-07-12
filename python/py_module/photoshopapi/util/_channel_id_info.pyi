import psapi.enum


class ChannelIDInfo:

    @property
    def id(self: ChannelIDInfo) -> psapi.enum.ChannelID:
        ...

    @id.setter
    def id(self: ChannelIDInfo, other: psapi.enum.ChannelID, color_mode: psapi.enum.ColorMode) -> None:
        ...

    @property
    def index(self: ChannelIDInfo) -> int:
        ...

    @index.setter
    def index(self: ChannelIDInfo, other: int, color_mode: psapi.enum.ColorMode) -> None:
        ...

    def __eq__(self, other: ChannelIDInfo) -> bool: 
        ...
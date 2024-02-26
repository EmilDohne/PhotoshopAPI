import psapi.enum


class ChannelIDInfo:
    '''
    Utility class which stores both the ID of the channel as well its logical index.
    This is done to allow for custom channels which have will have :class:`psapi.enum.ChannelID.Custom`
    as ID and then the corresponding index from 0-56.
    '''

    @property
    def id(self: ChannelIDInfo) -> psapi.enum.ChannelID:
        ...

    @id.setter
    def id(self: ChannelIDInfo, other: psapi.enum.ChannelID, color_mode: psapi.enum.ColorMode) -> None:
        '''
        When setting this property the ``index`` property is updated automatically to reflect this change
        '''
        ...

    @property
    def index(self: ChannelIDInfo) -> int:
        ...

    @index.setter
    def index(self: ChannelIDInfo, other: int, color_mode: psapi.enum.ColorMode) -> None:
        '''
        When setting this property the ``id`` property is updated automatically to reflect this change
        '''
        ...

    def __eq__(self, other: ChannelIDInfo) -> bool: 
        ...
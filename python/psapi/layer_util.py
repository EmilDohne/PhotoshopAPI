import psapi.types


def is_image_layer(layer: psapi.types.Layer) -> bool:
    """
    Check whether the provided layer is an image layer
    """
    return isinstance(layer, (psapi.ImageLayer_8bit, psapi.ImageLayer_16bit, psapi.ImageLayer_32bit))


def is_group_layer(layer: psapi.types.Layer) -> bool:
    """
    Check whether the provided layer is a group layer
    """
    return isinstance(layer, (psapi.GroupLayer_8bit, psapi.GroupLayer_16bit, psapi.GroupLayer_32bit))
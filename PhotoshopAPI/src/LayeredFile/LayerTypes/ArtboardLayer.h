#pragma once


// This struct holds no data, we just use it to identify its type
// Artboards are a distinct type of group with children and a predefined size which they are clipped to
// Artboards may include any other type of layers, but not other artboard layers
template <typename T>
struct ArtboardLayer
{

};
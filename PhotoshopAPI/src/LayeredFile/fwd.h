#pragma once

#include "Macros.h"
#include "concepts.h"

PSAPI_NAMESPACE_BEGIN

template <typename T>
	requires concepts::bit_depth<T>
struct LayeredFile;


/// Layer types

template <typename T>
	requires concepts::bit_depth<T>
struct Layer;

template <typename T>
	requires concepts::bit_depth<T>
struct GroupLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct ImageLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct SmartObjectLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct AdjustmentLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct ShapeLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct ArtboardLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct SectionDividerLayer;

template <typename T>
	requires concepts::bit_depth<T>
struct TextLayer;

PSAPI_NAMESPACE_END
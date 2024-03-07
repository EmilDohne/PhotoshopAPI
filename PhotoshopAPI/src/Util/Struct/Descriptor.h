#pragma once

#include "Macros.h"

#include <string>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

enum class DescriptorType
{
	Reference,
	Descriptor,
	List,
	Double,
	UnitFloat,
	String,
	Enumerated,
	Integer,
	LargeInteger,
	Boolean,
	GlobalObject,
	Class,
	Alias,
	Raw
};


/// Descriptors are Photoshops way of storing arbitrary data, without defining a solid structure for it
/// These have a variable length and start with a UnicodeString as well as a string for the classID.
/// These may be several layers of nesting deep
struct DescriptorStruct
{
	DescriptorType type;
	std::vector<DescriptorStruct> children;

	template <typename T>
	T operator[](const std::string key);

	void read(File& document);
};


PSAPI_NAMESPACE_END
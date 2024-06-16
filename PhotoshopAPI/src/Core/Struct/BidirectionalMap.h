#pragma once

#include "Macros.h"

#include <unordered_map>


PSAPI_NAMESPACE_BEGIN


namespace
{
	template <typename T, typename U>
	constexpr bool is_different_v = !std::is_same_v<T, U>;
}


/// Bidirectional unordered map which internally stores two maps in both directions allowing for fast lookups at the cost of 
/// storing the maps' contents twice. This is not meant for exceedingly large maps and provides a strong guarantee that 
/// an item is present in both maps and lookups can be performed with either template type. The only limitation is that 
/// we do not allow for maps with the same key and value as we are unable to disambiguate the two. So a 
/// bidirectional_unordered_map<char, char> would be invalid but a bidirectional_unordered_map<char, int> would be valid
template <typename T, typename U> requires is_different_v<T, U>
struct bidirectional_unordered_map
{
	bidirectional_unordered_map() = default;

	bidirectional_unordered_map(std::initializer_list<std::pair<T, U>> init)
	{
		for (auto& item : init)
		{
			insert(item);
		}
	}
	bidirectional_unordered_map(std::initializer_list<std::pair<U, T>> init)
	{
		for (auto& item : init)
		{
			insert(item);
		}
	}


	/// Add a pair of values to both maps for later retrieval
	void insert(const T& val1, const U& val2) noexcept
	{
		if (m_MapForward.count(val1))
			PSAPI_LOG_WARNING("BidirectionalMap", "Inserting value which already exists in this map");
		m_MapForward[val1] = val2;
		m_MapBackwards[val2] = val1;
	}
	void insert(const U& val1, const T& val2) noexcept { insert(val2, val1); }
	void insert(const std::pair<const T, const U>& val) noexcept { insert(std::get<0>(val), std::get<1>(val)); }
	void insert(const std::pair<const U, const T>& val) noexcept { insert(std::get<0>(val), std::get<1>(val)); }

	/// Access an element without bounds checks
	const T& operator[](const U& key) { return m_MapBackwards[key]; }
	/// Access an element without bounds checks
	const U& operator[](const T& key) { return m_MapForward[key]; }

	/// Access an element with bounds checks
	T& at(const U& key) { return m_MapBackwards.at(key); }
	/// Access an element with bounds checks
	U& at(const T& key) { return m_MapForward.at(key); }

	size_t size() const noexcept { return m_MapForward.size(); }

private:
	std::unordered_map<T, U> m_MapForward;
	std::unordered_map<U, T> m_MapBackwards;
};


PSAPI_NAMESPACE_END
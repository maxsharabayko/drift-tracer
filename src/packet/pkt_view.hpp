#pragma once
#include <iostream>
#include "../bits.hpp"
#include "../buf_view.hpp"


/// Packet field description.
/// @tparam val_type defines the value of the field, e.g. uint32_t, uint8_t.
/// @tparam byte_offset defines the offset to the field in bytes.
template <typename val_type, ptrdiff_t byte_offset>
struct pkt_field
{
	static constexpr ptrdiff_t offset = byte_offset;
	typedef val_type           type;
};

/// Packet view class provides read and write operations
/// on a buffer, holding packet data.
/// @details
/// Usage: Use with <tt>const_buffer</tt> or <tt>mutable_buffer</tt>.
///
/// @tparam storage can be mutable_buffer or const_buffer
///
template <class storage>
class pkt_view
{
public:
	pkt_view(const storage &buffer)
		: view_(buffer)
		, len_(buffer.size())
	{
	}

	pkt_view(const storage &&buffer)
		: view_(buffer)
		, len_(buffer.size())
	{
	}

	pkt_view(const pkt_view &other)
		: view_(other.view_)
		, len_(other.len_)
	{
	}

	pkt_view(const pkt_view &&) = delete;

public:
	/// Set the field to the value provided.
	/// @details
	/// Using SFINAE to disable function for const_buffer.
	/// @note is_same_v (C++17) is equal to is_same<..>::value (C++11).
	/// @note enable_if_t (C++14) is equal to enable_if<..>::type (C++11).
	template <class field_desc, typename T = storage, std::enable_if_t<std::is_same_v<T, mut_bufv>, bool> = false>
	void set_field(typename field_desc::type val)
	{
		using valtype = typename field_desc::type;
		valtype *ptr  = reinterpret_cast<valtype *>(view_.data()) + field_desc::offset / sizeof(valtype);
		*ptr          = bswap<valtype>(val);
	}

	/// Get the value in the field.
	template <class field_desc>
	typename field_desc::type get_field() const
	{
		using valtype      = typename field_desc::type;
		const valtype *ptr = reinterpret_cast<const valtype *>(view_.data()) + field_desc::offset / sizeof(valtype);
		return bswap<valtype>(*ptr);
	}

	/// Get the value in the field.
	uint32_t get_value() const
	{
		return 0;
	}

protected:
	storage view_; ///< buffer view
	size_t  len_;  ///< actual length of content
};

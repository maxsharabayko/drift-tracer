#pragma once
#include <cinttypes>

/// A class template that holds a pointer to a buffer,
/// but does not own it.
///
/// @details
/// The class provides a safe representation of a buffer that can (mut_bufv)
/// or can't (const_bufv) be modified. It does not own the underlying data,
/// and so is cheap to copy or assign.
///
/// @par Accessing Buffer Contents
///
/// The contents of a buffer may be accessed using the @c data() and @c size()
/// member functions:
///
/// @code mut_bufv b1(...);
/// std::size_t s1 = b1.size();
/// unsigned storage* p1 = b1.data();
/// @endcode
///
/// The @c data() member function permits violations of type safety, so uses of
/// it in application code should be carefully considered.
///
/// @note std::ranges::view is C++20
///
template <typename storage>
class buf_view
{
public:
	/// Construct an empty buffer.
	buf_view() noexcept
		: ptr_(0)
		, size_(0)
	{
	}

	/// Construct a buffer to represent a given memory range.
	buf_view(storage *ptr, size_t size) noexcept
		: ptr_(ptr)
		, size_(size)
	{
	}

	/// Get a pointer to the beginning of the memory range.
	storage *data() const noexcept { return ptr_; }

	storage *at(ptrdiff_t pos) const { return ptr_ + pos; }

	/// Get the size of the memory range.
	size_t size() const noexcept { return size_; }

	/// Move the start of the buffer by the specified number of bytes.
	buf_view &operator+=(std::size_t n) noexcept
	{
		std::size_t offset = n < size_ ? n : size_;
		ptr_               = ptr_ + offset;
		size_ -= offset;
		return *this;
	}

	/// Move the start of the buffer by the specified number of bytes.
	buf_view operator+(std::size_t n) noexcept
	{
		std::size_t offset = n < size_ ? n : size_;
		return buf_view(ptr_ + offset, size_ - offset);
	}

	storage *begin() const { return ptr_; }

	storage *end() const { return ptr_ + size_; }

private:
	storage *   ptr_;
	std::size_t size_;
};

/// Mutable buffer view allows to modify the contents.
typedef buf_view<uint8_t> mut_bufv;

/// Constant buffer view does not allow to modify the contents.
typedef buf_view<const uint8_t> const_bufv;


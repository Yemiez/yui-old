#pragma once
#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <new>
#include "Types.h"

namespace yui {

	template<typename T>
	class Vector
	{
	public:
		using ValueType = T;
		using SizeType = u32;
		using Iterator = T*;
		using ConstIterator = const Iterator;
		
	public:
		Vector() = default;
		Vector(std::initializer_list<T> initializer_list)
		{
			copy_initial(initializer_list.begin(), initializer_list.end());
		}

		u32 size() const { return m_size; }
		u32 capacity() const { return m_capacity; }
		bool empty() const { return m_size == 0; }
		bool is_null() const { return m_buffer == nullptr; }
		ConstIterator begin() const { return m_buffer; }
		ConstIterator end() const { return m_buffer + m_size; }
		Iterator begin() { return m_buffer; }
		Iterator end() { return m_buffer + m_size; }
		const T& at(u32 index) const { return m_buffer[index]; }
		T& at(u32 index) { return m_buffer[index]; }
		void reserve(u32 new_capacity);
		void resize(u32 new_size);
		void resize(u32 new_size, const T& value);
		void clear();
		void prepend(T value);
		void append(T value);
		T& emplace_back(T&& value);
		T& insert(u32 where, const T& value);
		T& insert(u32 where, T&& value);
		T* erase(u32 where);
		void pop_back();

	public: // Finding functions etc
		Iterator find(const T& va) const;
		template<typename Callable>
		Iterator find_if(Callable predicate) const;

		bool contains(const T& value) const
		{
			return find(value) != end();
		}
		
	private:
		// This *has* to be called with a currently NULL buffer.
		void copy_initial(const T* begin, const T* end);

		// This function will copy the range begin to end into where + distance(begin, end).
		// It will not perform checks whether it fits. Just simply copy the data.
		// Hence why the function is static.
		static void copy_unchecked(T* where, const T* begin, const T* end);

		// Allocate N elements of T.
		static T* allocate(u32 size) { return static_cast<T*>(malloc(size * sizeof(T))); }
	private: // heap memory related functions
		void grow_capacity(u32 new_size);
		void shrink_capacity(u32 new_size);
		void ensure_capacity(u32 capacity);
		void grow_size(u32 new_size, const T& value);
		void shrink_size(u32 new_size);
		void clear_and_keep_constraints();

		T* slot(u32 index) { return &m_buffer[index]; }
		const T* slot(u32 index) const { return &m_buffer[index]; }
	private:
		T *m_buffer{nullptr};
		u32 m_size{0};
		u32 m_capacity{0};
	};

	template <typename T>
	void Vector<T>::reserve(u32 new_capacity)
	{
		if (new_capacity > m_capacity) grow_capacity(new_capacity);
		else if (new_capacity > m_size) shrink_capacity(new_capacity);
		// Can't shrink.
	}

	template <typename T>
	void Vector<T>::resize(u32 new_size)
	{
		if (new_size < m_size) return shrink_size(new_size);
		else return grow_size(new_size, {});
	}

	template <typename T>
	void Vector<T>::resize(u32 new_size, const T& value)
	{
		if (new_size < m_size) return shrink_size(new_size); // Possibly an error because it passed a default value which will be ignored.
		else return grow_size(new_size, value);
	}

	template <typename T>
	void Vector<T>::clear()
	{
		clear_and_keep_constraints();
		m_size = m_capacity = 0;
	}

	template <typename T>
	void Vector<T>::prepend(T value)
	{
		insert(0, std::move(value));
	}

	template <typename T>
	void Vector<T>::append(T value)
	{
		ensure_capacity(m_size + 1);
		new (slot(m_size)) T(std::move(value));
		++m_size;
	}

	template <typename T>
	T& Vector<T>::emplace_back(T&& value)
	{
		ensure_capacity(m_size + 1);
		auto *ptr = new (slot(m_size++)) T(std::move(value));
		return *ptr;
	}

	template <typename T>
	T& Vector<T>::insert(u32 where, const T& value)
	{
		auto tmp = value;
		return insert(where, std::move(tmp));
	}

	template <typename T>
	T& Vector<T>::insert(u32 where, T&& value)
	{
		if (where == m_size) return emplace_back(std::move(value));
		
		ensure_capacity(m_size + 1);
		for (auto i = m_size; i >= where; --i) {
			new (slot(i + 1)) T (std::move(at(i)));
		}
		++m_size;
		return *(new (slot(where)) T(std::move(value)));
	}

	template <typename T>
	T* Vector<T>::erase(u32 where)
	{
		if (where == m_size - 1) {
			// Last element
			pop_back();
			return end();
		}

		at(where).~T();

		for (auto i = where + 1; i < m_size; ++i) {
			new (slot(i - 1)) T(std::move(at(i)));
		}
		--m_size;
		return nullptr;
	}

	template <typename T>
	void Vector<T>::pop_back()
	{
		at(--m_size).~T();
	}

	template <typename T>
	typename Vector<T>::Iterator Vector<T>::find(const T& va) const
	{
		return std::find_if(begin(), end(), [&va](const auto &vb) {
			return vb == va;
		});
	}

	template <typename T>
	template <typename Callable>
	typename Vector<T>::Iterator Vector<T>::find_if(Callable predicate) const
	{
		return std::find_if(begin(), end(), predicate);
	}

	template <typename T>
	void Vector<T>::copy_initial(const T* begin, const T* end)
	{
		m_size = std::distance(begin, end);
		m_capacity = m_size;
		m_buffer = allocate(m_capacity);

		copy_unchecked(this->begin(), begin, end);
	}

	template <typename T>
	void Vector<T>::copy_unchecked(T* where, const T* begin, const T* end)
	{
		for (; begin != end; ++begin, ++where) {
			new (where)T(*begin);
		}
	}

	template <typename T>
	void Vector<T>::grow_capacity(u32 new_size)
	{
		auto *tmp_buffer = allocate(new_size);
		if (is_null()) {
			m_buffer = tmp_buffer;
			m_capacity = new_size;
		}
		else {
			copy_unchecked(tmp_buffer, begin(), end());
			clear_and_keep_constraints();
			m_buffer = tmp_buffer;
			m_capacity = new_size;
			// Size stays the same.
		}
	}

	template <typename T>
	void Vector<T>::shrink_capacity(u32 new_size)
	{
		auto *tmp_buffer = allocate(new_size);
		copy_unchecked(tmp_buffer, begin(), begin() + new_size);
		m_capacity = new_size;
		m_size = new_size;
	}

	template <typename T>
	void Vector<T>::ensure_capacity(u32 capacity)
	{
		if (capacity > m_capacity) {
			grow_capacity(std::max(m_capacity * 2, capacity));
		}
	}

	template <typename T>
	void Vector<T>::grow_size(u32 new_size, const T& value)
	{
		ensure_capacity(new_size);
		for (auto i = m_size; i < new_size; ++i) {
			new (slot(i)) T(value);
		}
		m_size = new_size;
	}

	template <typename T>
	void Vector<T>::shrink_size(u32 new_size)
	{
		for (auto i = new_size; i < m_size; ++i) {
			at(i).~T();
		}
		m_size = new_size;
	}

	template <typename T>
	void Vector<T>::clear_and_keep_constraints()
	{
		for (auto i = 0u; i < m_size; ++i) {
			at(i).~T();
		}
		delete m_buffer;
		m_buffer = nullptr;
		// Simply deletes the buffer.
	}
}

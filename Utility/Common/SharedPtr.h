#pragma once

template <class T>
class SharedPtr
{
private:
	template <class T, class... TArgs>
	friend SharedPtr<T> MakeShared(TArgs&&... args);

	mutable uint32_t* counter;
	T* pointer;

	SharedPtr(T& data) :
		counter{ new uint32_t{ 1 } },
		pointer{ &data }
	{

	}

public:
	SharedPtr() :
		counter{ nullptr },
		pointer{ nullptr }
	{

	}

	SharedPtr(std::nullptr_t) :
		SharedPtr{}
	{

	}

	SharedPtr(const SharedPtr& y) :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		if (counter)
			*counter += 1;
	}

	SharedPtr(SharedPtr&& y) :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		y.counter = nullptr;
		y.pointer = nullptr;
	}

	SharedPtr& operator=(const SharedPtr& y)
	{
		if (this == &y)
			return;

		~SharedPtr();

		counter = y.counter;
		pointer = y.pointer;

		if (counter)
			*counter += 1;

		return *this;
	}

	SharedPtr& operator=(SharedPtr&& y)
	{
		if (this == &y)
			return *this;

		counter = y.counter;
		pointer = y.pointer;

		y.counter = nullptr;
		y.pointer = nullptr;
	}

	~SharedPtr()
	{
		if (!counter)
			return;

		*counter -= 1;

		if (*counter == 0)
		{
			delete counter;
			delete pointer;
		}
	}

	bool operator==(const SharedPtr& y) const
	{
		return pointer == y.pointer;
	}

	bool operator!=(const SharedPtr& y) const
	{
		return pointer != y.pointer;
	}

	T& operator*() const
	{
		return *pointer;
	}

	T* operator->() const
	{
		return pointer;
	}

	T* Get() const
	{
		return pointer;
	}
};

template <class T, class... TArgs>
SharedPtr<T> MakeShared(TArgs&&... args)
{
	T* pointer{ new T{ std::forward<TArgs>(args)... } };
	return SharedPtr{ *pointer };
}
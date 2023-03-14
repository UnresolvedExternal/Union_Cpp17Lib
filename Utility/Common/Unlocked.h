#pragma once

template <class T>
struct Unlocked
{
private:
	TMemUnlocker unlocker;

public:
	Unlocked(uint32_t address = 0x00000000u) :
		unlocker{ reinterpret_cast<void32>(address), sizeof(T), address }
	{
		
	}

	Unlocked(Unlocked&& y) :
		unlocker{ y.unlocker }
	{
		y.unlocker.ulAddress = nullptr;
		y.unlocker.ulSize = 0u;
		y.unlocker.ulOldRoot = 0u;
	}

	Unlocked& operator=(Unlocked&& y)
	{
		unlocker.~TMemUnlocker();

		unlocker.ulAddress = y.unlocker.ulAddress;
		unlocker.ulSize = y.unlocker.ulSize;
		unlocker.ulOldRoot = y.unlocker.ulOldRoot;

		y.unlocker.ulAddress = nullptr;
		y.unlocker.ulSize = 0u;
		y.unlocker.ulOldRoot = 0u;
	}

	Unlocked& operator=(const T& value)
	{
		**this = value;
		return *this;
	}

	operator T() const
	{
		return *GetTypedAddress();
	}

	T& operator*()
	{
		return *GetTypedAddress();
	}

	T* operator->()
	{
		return GetTypedAddress();
	}

	int GetAddress() const
	{
		return reinterpret_cast<int>(unlocker.ulAddress);
	}

	T* GetTypedAddress() const
	{
		return reinterpret_cast<T*>(unlocker.ulAddress);
	}

	int GetSize() const
	{
		return sizeof(T);
	}

	int GetNextAddress() const
	{
		return GetAddress() + GetSize();
	}
};

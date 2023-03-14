#include <functional>

namespace std
{
	template <>
	struct hash<NAMESPACE::zSTRING>
	{
		size_t operator()(const NAMESPACE::zSTRING& x) const
		{
			size_t hash = 5381;
			const char* str = x.ToChar();

			while (size_t c = *str++)
				hash = ((hash << 5) + hash) + reinterpret_cast<const unsigned char&>(c);

			return hash;
		}
	};

	template <>
	struct less<NAMESPACE::zSTRING>
	{
		bool operator()(const NAMESPACE::zSTRING& x, const NAMESPACE::zSTRING& y) const
		{
			return strcmp(x.ToChar(), y.ToChar()) < 0;
		}
	};

	template <>
	struct equal_to<NAMESPACE::zSTRING>
	{
		bool operator()(const NAMESPACE::zSTRING& x, const NAMESPACE::zSTRING& y) const
		{
			return x.Compare(y);
		}
	};
}

#include <string>

namespace NAMESPACE
{
	CCmd& operator<<(CCmd& cmd, const zVEC3& value)
	{
		return cmd << value[0] << " / " << value[1] << " / " << value[2];
	}

	CCmd& operator<<(CCmd& cmd, const std::string& value)
	{
		return cmd << const_cast<char* const>(value.c_str());
	}
}

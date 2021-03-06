#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <Windows.h>
#include <accctrl.h>

namespace tokenDumper{

	HANDLE GetProcessHandle(DWORD pid);
	std::string ConvertSidToString(const PSID pSid);
	std::string ConvertLuidToString(const PLUID pLuid);
	std::string LookupAccount(const PSID pSid);
	std::string TrusteeToString(const PTRUSTEE_A pTrustee);

	class win32_exception : public std::runtime_error {
	public:
		win32_exception(DWORD lastError, const std::string& msg) : runtime_error(makeMessage(lastError, msg)) {
		}
	private:
		std::string makeMessage(DWORD lastError, const std::string & what_arg)
		{
			std::stringstream ss;
			//TODO: Make a message
	//		ss << what_arg << " Error: (" << lastError << ")" << Utils::GetErrorTextA(lastError);
			ss << "Err: (" << lastError << ")";
			return ss.str();
		}
	};

	template <typename T>
	class ArrayDeleter 
	{
	public:
		explicit ArrayDeleter(T* obj) : m_pObj(obj) {}
		~ArrayDeleter() {
			delete[] m_pObj;
		}

		//non copyable
		ArrayDeleter(const ArrayDeleter&)				= delete;
		ArrayDeleter& operator=(const ArrayDeleter&)	= delete;

		const T* Get() const { return m_pObj; }
		T* Get() { return m_pObj; }

	private:
		T* m_pObj;
	};
};


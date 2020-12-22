#include <windows.h>
#include <SimConnect.h>

#include "SkyConnect.h"

namespace {
  const char *ConnectionName = "SkyConnect";
  const int WM_USER_SIMCONNECT = 0x0402;
}

// PUBLIC

SkyConnect::SkyConnect()
    : m_simConnectHandler(nullptr)
{

}

SkyConnect::~SkyConnect() {
    this->close();
}

bool SkyConnect::open() {
    HRESULT result;
    HANDLE hEventHandle = nullptr;
    HWND hWnd = nullptr;

    result = SimConnect_Open(&m_simConnectHandler, ::ConnectionName, hWnd, ::WM_USER_SIMCONNECT, hEventHandle, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    return result == S_OK;
}

bool SkyConnect::close() {
    HRESULT result;
    if (m_simConnectHandler != nullptr) {
        result = SimConnect_Close(m_simConnectHandler);
        m_simConnectHandler = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

bool SkyConnect::isConnected() const {
    return m_simConnectHandler != nullptr;
}

#include "darkTheme.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Windows.h>
#include <dwmapi.h>

void glfwSetWindowTheme(GLFWwindow* window, int theme)
{
    if (theme == GLFW_THEME_AUTO)
        theme = glfwGetSystemTheme();

    if (theme == GLFW_THEME_LIGHT)
        theme = 0;
    else if (theme == GLFW_THEME_DARK)
        theme = 1;

    DwmSetWindowAttribute(glfwGetWin32Window(window),
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &theme, sizeof(theme));

    int width = 0, height = 0;
    glfwGetWindowSize(window, &width, &height);
    SetWindowPos(glfwGetWin32Window(window), NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

int glfwGetSystemTheme()
{
    BOOL useLightMode;
    BOOL success = FALSE;
    HKEY hRootKey = HKEY_CURRENT_USER;
    const wchar_t* lpSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    const wchar_t* lpValueName = L"AppsUseLightTheme";
    DWORD result;

    HKEY hKey = 0;
    if (RegOpenKeyExW(hRootKey, lpSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwBufferSize = sizeof(DWORD);
        DWORD dwData = 0;
        if (RegQueryValueExW(hKey, lpValueName, 0, NULL, (LPBYTE)(&dwData), &dwBufferSize) == ERROR_SUCCESS)
        {
            result = dwData;
            success = TRUE;
        }
        RegCloseKey(hKey);
    }
    useLightMode = success && result != 0;
    return useLightMode ? GLFW_THEME_LIGHT : GLFW_THEME_DARK;
}

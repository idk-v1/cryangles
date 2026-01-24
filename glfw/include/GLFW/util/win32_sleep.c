#include "sleep.h"
#include <Windows.h>

void glfwSleep(double time)
{
    DWORD t;

    if (time == 0.0)
    {
        t = 0;
    }
    else if (time < 0.001)
    {
        t = 1;
    }
    else if (time > 2147483647.0)
    {
        t = 2147483647;
    }
    else
    {
        t = (DWORD)(time * 1000.0 + 0.5);
    }
    Sleep(t);
}
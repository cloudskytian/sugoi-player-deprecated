#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")

#include <tchar.h>
#include <Windows.h>

int main(int argc, char *argv[])
{
#if defined(_WIN64) && defined(_DEBUG)
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoi64d.dll"));
#elif defined(_WIN64) && !defined(_DEBUG)
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoi64.dll"));
#elif !defined(_WIN64) && defined(_DEBUG)
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoid.dll"));
#elif !defined(_WIN64) && !defined(_DEBUG)
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoi.dll"));
#endif
    int ret = -1;
    if (hDll != nullptr)
    {
        typedef int(*guardApp)(int, char *[]);
        guardApp app = (guardApp)GetProcAddress(hDll, "sugoiGuardMain");
        ret = app(argc, argv);
        FreeLibrary(hDll);
    }
    else
    {
        MessageBox(nullptr, TEXT("Failed to load dll. Contact with the developers for more details."), TEXT("Error"), MB_OK | MB_ICONERROR);
    }
    return ret;
}

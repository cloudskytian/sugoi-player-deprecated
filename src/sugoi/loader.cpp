#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")

#include <string.h>
#include <tchar.h>
#include <Windows.h>

typedef int(*appMain)(int, char *[]);

int main(int argc, char *argv[])
{
#ifdef _WIN64
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoi64.dll"));
#else
    HINSTANCE hDll = LoadLibrary(TEXT("Sugoi.dll"));
#endif
    int exec = -1;
    bool guardMode = false;
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "--guard") == 0)
            {
                guardMode = true;
                break;
            }
        }
    }
    if (hDll != nullptr)
    {
        appMain app;
        if (guardMode)
        {
            app = (appMain)GetProcAddress(hDll, "sugoiGuardMain");
        }
        else
        {
            app = (appMain)GetProcAddress(hDll, "sugoiAppMain");
        }
        exec = app(argc, argv);
        FreeLibrary(hDll);
    }
    else
    {
        MessageBox(nullptr, TEXT("ERROR! Failed to load main module \"Sugoi[64].dll\".\r\nContact the developers for technical support."), TEXT("Error"), MB_OK | MB_ICONERROR);
    }
    return exec;
}

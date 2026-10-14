#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <wchar.h>

static wchar_t logPath[MAX_PATH];
static void LogPlugin(const wchar_t* name,DWORD error) {
    wchar_t line[160];
    const int n=swprintf_s(line,L"%s: %s (error %lu)\r\n",name,error==ERROR_SUCCESS?L"loaded":(error==ERROR_FILE_NOT_FOUND?L"disabled (file absent)":L"failed"),error);
    HANDLE file=CreateFileW(logPath,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file!=INVALID_HANDLE_VALUE) {DWORD written;if(n>0) WriteFile(file,line,static_cast<DWORD>(n*sizeof(wchar_t)),&written,nullptr);CloseHandle(file);}
}
static HMODULE realVersion;
static FARPROC forwards[17];
static const char* exportNames[]={
 "GetFileVersionInfoA","GetFileVersionInfoByHandle","GetFileVersionInfoExA","GetFileVersionInfoExW",
 "GetFileVersionInfoSizeA","GetFileVersionInfoSizeExA","GetFileVersionInfoSizeExW","GetFileVersionInfoSizeW",
 "GetFileVersionInfoW","VerFindFileA","VerFindFileW","VerInstallFileA","VerInstallFileW",
 "VerLanguageNameA","VerLanguageNameW","VerQueryValueA","VerQueryValueW"
};
#define FORWARD(N,OFFSET) extern "C" __declspec(naked) void Proxy##N() { __asm { jmp dword ptr [forwards+OFFSET] } }
FORWARD(0,0) FORWARD(1,4) FORWARD(2,8) FORWARD(3,12) FORWARD(4,16) FORWARD(5,20)
FORWARD(6,24) FORWARD(7,28) FORWARD(8,32) FORWARD(9,36) FORWARD(10,40) FORWARD(11,44)
FORWARD(12,48) FORWARD(13,52) FORWARD(14,56) FORWARD(15,60) FORWARD(16,64)

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        wchar_t path[MAX_PATH];
        UINT n=GetSystemDirectoryW(path,MAX_PATH);
        if(!n || n+13>=MAX_PATH) return FALSE;
        memcpy(path+n,L"\\version.dll",13*sizeof(wchar_t));
        realVersion=LoadLibraryW(path);
        if(!realVersion) return FALSE;
        for(unsigned i=0;i<17;i++) forwards[i]=GetProcAddress(realVersion,exportNames[i]);
        if(!forwards[0] || !forwards[4] || !forwards[15]) return FALSE;
        // Load from our own directory. Missing plugins are intentionally disabled.
        wchar_t pluginPath[MAX_PATH];
        if (!GetModuleFileNameW(instance,pluginPath,MAX_PATH)) return FALSE;
        wchar_t* slash=wcsrchr(pluginPath,L'\\');
        if (!slash) return FALSE;
        const size_t prefix=static_cast<size_t>(slash+1-pluginPath);
        memcpy(logPath,pluginPath,prefix*sizeof(wchar_t));
        if(wcscpy_s(logPath+prefix,MAX_PATH-prefix,L"mod_loader\\runtime.log")!=0) return FALSE;
        const wchar_t* plugins[]={L"ds_resolution.dll",L"ds_damage_numbers.dll"};
        for(const auto name:plugins) {
            if(wcscpy_s(pluginPath+prefix,MAX_PATH-prefix,name)!=0) return FALSE;
            // Keep modules resident: their code is referenced by game hooks.
            if(GetFileAttributesW(pluginPath)==INVALID_FILE_ATTRIBUTES) {LogPlugin(name,GetLastError());continue;}
            const HMODULE plugin=LoadLibraryExW(pluginPath,nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
            LogPlugin(name,plugin?ERROR_SUCCESS:GetLastError());
        }
    }
    return TRUE;
}

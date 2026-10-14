#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

struct GameString { char* text; uint32_t spare; };
struct Mode { uint32_t width, height, value; };
struct GameVector { uint32_t allocator; unsigned char* begin; unsigned char* end; unsigned char* capacity; };
static_assert(sizeof(GameString)==8 && sizeof(GameVector)==16, "Build x86");

using AssignString = void* (__thiscall*)(GameString*, const char*, unsigned);
using DestroyString = void (__thiscall*)(GameString*);
using InsertString = void* (__thiscall*)(GameVector*, void*, const GameString*);
using AppendMode = void (__thiscall*)(GameVector*, const Mode*);
static AssignString assignString;
static DestroyString destroyString;
static InsertString insertString;
static AppendMode appendMode;
static uintptr_t menuResume, uiShellSlot;
struct Resolution { uint32_t width, height; const char* menuText; };
static const Resolution resolutions[]={
    {1280,720,"1280x720x32"},
    {1280,800,"1280x800x32"},
    {1280,1024,"1280x1024x32"},
    {1360,768,"1360x768x32"},
    {1366,768,"1366x768x32"},
    {1440,900,"1440x900x32"},
    {1600,900,"1600x900x32"},
    {1680,1050,"1680x1050x32"},
    {1920,1080,"1920x1080x32"},
    {1920,1200,"1920x1200x32"},
    {2048,1080,"2048x1080x32"},
    {2560,1080,"2560x1080x32"},
    {2560,1440,"2560x1440x32"},
    {2560,1600,"2560x1600x32"},
    {3440,1440,"3440x1440x32"},
    {3840,1600,"3840x1600x32"},
    {3840,2160,"3840x2160x32"},
    {5120,1440,"5120x1440x32"}
};

static void __cdecl AddMenuResolution(void* options) {
    auto v = reinterpret_cast<GameVector*>(static_cast<unsigned char*>(options)+0x44);
    for(const auto& resolution:resolutions) {
        bool present=false;
        for (auto p=v->begin; p!=v->end; p+=sizeof(GameString)) {
            auto s=reinterpret_cast<GameString*>(p);
            if (s->text && strcmp(s->text,resolution.menuText)==0) {present=true;break;}
        }
        if(present) continue;
        GameString s={};
        assignString(&s,resolution.menuText,static_cast<unsigned>(strlen(resolution.menuText)));
        if (s.text) insertString(v,v->end,&s);
        destroyString(&s); // The game's insert retains its own string reference.
    }
}

static void __cdecl AddRendererResolution(void* renderer) {
    // This table contains (width,height,max_back_buffers), NOT display bpp.
    auto v = reinterpret_cast<GameVector*>(static_cast<unsigned char*>(renderer)+0x6a4);
    for(const auto& resolution:resolutions) {
        bool present=false;
        for(auto p=v->begin; p!=v->end; p+=sizeof(Mode)) {
            auto m=reinterpret_cast<Mode*>(p);
            if(m->width==resolution.width && m->height==resolution.height) {present=true;break;}
        }
        if(present) continue;
        const Mode m={resolution.width,resolution.height,1};
        appendMode(v,&m);
    }
    // Physical display modes remain subject to the original DirectDraw checks.
}

__declspec(naked) static void MenuHook() {
    __asm {
        pushfd
        pushad
        push dword ptr [ebp-8] // Options object saved by the original function.
        call AddMenuResolution
        add esp,4
        popad
        popfd
        mov ecx,uiShellSlot
        mov ecx,dword ptr [ecx] // Replay overwritten mov ecx,[UIShell singleton].
        jmp dword ptr [menuResume]
    }
}
__declspec(naked) static void RendererHook() {
    __asm {
        pushfd
        pushad
        push edi // Renderer still in EDI at the original epilogue.
        call AddRendererResolution
        add esp,4
        popad
        popfd
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

struct Patch { uint32_t rva; unsigned size; unsigned char oldBytes[6]; unsigned char newBytes[6]; };
static void RelativeJump(unsigned char* bytes,uintptr_t from,uintptr_t to,unsigned n) {
    memset(bytes,0x90,n); bytes[0]=0xe9;
    auto rel=static_cast<uint32_t>(to-from-5);
    memcpy(bytes+1,&rel,4);
}
static bool InstallPatches(unsigned char* base) {
    auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>4096) return false;
    auto pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
       pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC || pe->OptionalHeader.SizeOfImage!=0x3b4000) return false;
    // All sites must match before any write. Different game builds fail closed.
    Patch patches[]={
        {0x13c2e,4,{0x80,0x02,0,0},{0x80,0x07,0,0}},
        {0x13c55,4,{0xe0,0x01,0,0},{0x38,0x04,0,0}},
        {0xb4a5d,6,{0x8b,0x0d,0x5c,0x06,0x7a,0},{ }},
        {0x251416,5,{0x5f,0x5e,0x5d,0x5b,0xc3},{ }}
    };
    for(const auto& p:patches) if(memcmp(base+p.rva,p.oldBytes,p.size)) return false;
    assignString=reinterpret_cast<AssignString>(base+0x2be9);
    destroyString=reinterpret_cast<DestroyString>(base+0x2cb6);
    insertString=reinterpret_cast<InsertString>(base+0xba838);
    appendMode=reinterpret_cast<AppendMode>(base+0x195fbf);
    menuResume=reinterpret_cast<uintptr_t>(base+0xb4a63);
    uiShellSlot=reinterpret_cast<uintptr_t>(base+0x3a065c);
    RelativeJump(patches[2].newBytes,reinterpret_cast<uintptr_t>(base+patches[2].rva),reinterpret_cast<uintptr_t>(MenuHook),6);
    RelativeJump(patches[3].newBytes,reinterpret_cast<uintptr_t>(base+patches[3].rva),reinterpret_cast<uintptr_t>(RendererHook),5);
    // Two fallback immediates share one page; protect/restore each page once.
    SYSTEM_INFO systemInfo; GetSystemInfo(&systemInfo);
    uintptr_t pages[4]={}; unsigned pageCount=0;
    for(const auto& p:patches) {
        uintptr_t page=reinterpret_cast<uintptr_t>(base+p.rva)&~(static_cast<uintptr_t>(systemInfo.dwPageSize)-1);
        bool present=false;
        for(unsigned i=0;i<pageCount;i++) if(pages[i]==page) present=true;
        if(!present) pages[pageCount++]=page;
    }
    DWORD oldProtect[4]={};
    unsigned writable=0;
    for(;writable<pageCount;writable++) {
        if(!VirtualProtect(reinterpret_cast<void*>(pages[writable]),systemInfo.dwPageSize,PAGE_EXECUTE_READWRITE,&oldProtect[writable])) break;
    }
    if(writable==pageCount) for(auto& p:patches) memcpy(base+p.rva,p.newBytes,p.size);
    for(unsigned i=0;i<writable;i++) {DWORD ignored; VirtualProtect(reinterpret_cast<void*>(pages[i]),systemInfo.dwPageSize,oldProtect[i],&ignored);}
    if(writable==pageCount) FlushInstructionCache(GetCurrentProcess(),base,pe->OptionalHeader.SizeOfImage);
    return writable==pageCount;
}

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        InstallPatches(reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr)));
    }
    return TRUE;
}

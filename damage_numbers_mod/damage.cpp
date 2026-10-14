#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>
struct GameString { char* text; uint32_t spare; };
using AssignString = void* (__thiscall*)(GameString*, const char*, unsigned);
using DestroyString = void (__thiscall*)(GameString*);
static_assert(sizeof(GameString)==8,"Build x86");
static void RelativeJump(unsigned char* bytes,uintptr_t from,uintptr_t to,unsigned n) {
    memset(bytes,0x90,n);bytes[0]=0xe9;
    const auto rel=static_cast<uint32_t>(to-from-5);memcpy(bytes+1,&rel,4);
}
#include <stdio.h>
#include <math.h>
#include <float.h>

struct Vec3 { float x,y,z; };
struct Hit { Vec3 world; float loss; DWORD born; bool active; };
static Hit hits[128];
static unsigned nextHit;
static SRWLOCK hitLock=SRWLOCK_INIT;
static unsigned char* gameBase;
static uintptr_t lifeResume, cursorDraw;
static void* textWindow;
static volatile LONG effectsDisabled;
static const char* volatile stage="idle";
static wchar_t diagnosticPath[MAX_PATH];
template<class T> static T Native(unsigned rva) { return reinterpret_cast<T>(gameBase+rva); }
template<class T> static T Field(void* p,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
using UnaryBool=bool (__thiscall*)(void*);
using Position=void* (__thiscall*)(void*);
using Transform=Vec3* (__thiscall*)(void*,Vec3*);
using Project=Vec3* (__thiscall*)(void*,Vec3*,const Vec3*);
// The retail factory ends in RET 4: the callee removes its enum argument.
using CreateText=void* (__stdcall*)(unsigned);
using TextArg=void (__thiscall*)(void*,const GameString*);
using SetText=void (__thiscall*)(void*,const GameString*,bool);
using NativeDrawText=void (__thiscall*)(void*);
using NativeSetRect=void (__thiscall*)(void*,int,int,int,int,bool);

static bool Offline() { return !Native<bool (__cdecl*)()>(0x175afa)(); }
static bool Finite(float f) { return _finite(f)!=0; }
static void __cdecl CaptureLoss(void* aspect,float newLife) {
    stage="capture health";
    const float oldLife=Field<float>(aspect,0x70);
    if(!Finite(oldLife)||!Finite(newLife)||newLife>=oldLife||oldLife<=0||!Offline()) return;
    void* go=Field<void*>(aspect,4);
    if(!go || !Field<void*>(go,0) || Native<UnaryBool>(0x1aeed6)(go)) return;
    void* placement=Field<void*>(go,0x40);
    if(!placement || !Field<void*>(gameBase,0x3a05fc)) return;
    Vec3 world={};
    stage="get placement";
    void* pos=Native<Position>(0x137075)(placement);
    stage="transform hit position";
    Native<Transform>(0x26653b)(pos,&world);
    if(!Finite(world.x)||!Finite(world.y)||!Finite(world.z)) return;
    world.y+=2.0f;
    const float remaining=newLife>0?newLife:0;
    const float loss=oldLife-remaining;
    if(loss<=0 || !Finite(loss)) return;
    Hit hit={world,loss,GetTickCount(),true};
    AcquireSRWLockExclusive(&hitLock);
    hits[nextHit++%128]=hit;
    ReleaseSRWLockExclusive(&hitLock);
}

static void SetNativeString(GameString* s,const char* value) {
    Native<AssignString>(0x2be9)(s,value,static_cast<unsigned>(strlen(value)));
}
static void __cdecl RenderHits() {
    Hit visible[128]; unsigned count=0;
    const DWORD now=GetTickCount();
    const bool offline=Offline();
    AcquireSRWLockExclusive(&hitLock);
    for(auto& h:hits) {
        if(!offline || now-h.born>=1150) h.active=false;
        if(h.active) visible[count++]=h;
    }
    ReleaseSRWLockExclusive(&hitLock);
    if(!count) return;
    void* shell=Field<void*>(gameBase,0x3a065c);
    void* services=Field<void*>(gameBase,0x3a0644);
    if(!shell || !services) return;
    void* renderer=Field<void*>(services,0x1e0);
    if(!renderer) return;
    if(!textWindow) {
        stage="create native UIText";
        textWindow=Native<CreateText>(0x2dfc7f)(7); // Retail UIText factory.
        if(!textWindow) return;
        *reinterpret_cast<unsigned char*>(static_cast<unsigned char*>(textWindow)+0x108)=1;
        *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(textWindow)+0x1bc)=2;
    }
    GameString font={};
    stage="assign font string";
    SetNativeString(&font,"b_gui_fnt_16p_copperplate_light");
    stage="set native font";
    Native<TextArg>(0x2e7da5)(textWindow,&font);
    Native<DestroyString>(0x2cb6)(&font);
    const int width=Field<int>(shell,0x60),height=Field<int>(shell,0x64);
    for(unsigned i=0;i<count;i++) {
        const Hit& h=visible[i];
        Vec3 screen={};
        stage="project hit position";
        Native<Project>(0x2536f2)(renderer,&screen,&h.world);
        if(!Finite(screen.x)||!Finite(screen.y)||screen.x<0||screen.x>width||screen.y<0||screen.y>height) continue;
        const float t=static_cast<float>(now-h.born)/1150.0f;
        const int x=static_cast<int>(screen.x);
        const int y=static_cast<int>(screen.y-42*t);
        char number[32];
        if(h.loss<1) sprintf_s(number,"%.1f",static_cast<double>(h.loss));
        else sprintf_s(number,"%.0f",static_cast<double>(h.loss));
        stage="assign number string";
        GameString value={}; SetNativeString(&value,number);
        stage="set native text";
        Native<SetText>(0x7dfba)(textWindow,&value,true);
        Native<DestroyString>(0x2cb6)(&value);
        // Native rect order is left,right,top,bottom; standalone window owns no UI tree.
        stage="set native text rectangle";
        Native<NativeSetRect>(0x2e83f6)(textWindow,x-64,x+64,y-12,y+12,false);
        const unsigned alpha=static_cast<unsigned>(255*(1-t));
        *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(textWindow)+0x1b4)=(alpha<<24)|0xffd060;
        stage="draw native text";
        Native<NativeDrawText>(0x2e7c47)(textWindow);
    }
}

static int RecordFailure(EXCEPTION_POINTERS* info) {
    InterlockedExchange(&effectsDisabled,1);
    char entry[1024];
    HMODULE module=nullptr;char modulePath[MAX_PATH]={};
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(info->ExceptionRecord->ExceptionAddress),&module);
    if(module)GetModuleFileNameA(module,modulePath,MAX_PATH);
    int n=sprintf_s(entry,"Damage numbers disabled after hook exception. stage=%s code=%08lX address=%p module=%s offset=%08lX EAX=%08lX ECX=%08lX EDX=%08lX ESP=%08lX EBP=%08lX\r\n",
        stage,info->ExceptionRecord->ExceptionCode,info->ExceptionRecord->ExceptionAddress,modulePath,
        static_cast<unsigned long>(reinterpret_cast<uintptr_t>(info->ExceptionRecord->ExceptionAddress)-reinterpret_cast<uintptr_t>(module)),
        info->ContextRecord->Eax,info->ContextRecord->Ecx,info->ContextRecord->Edx,info->ContextRecord->Esp,info->ContextRecord->Ebp);
    HANDLE file=CreateFileW(diagnosticPath,FILE_APPEND_DATA,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file!=INVALID_HANDLE_VALUE){DWORD written;WriteFile(file,entry,static_cast<DWORD>(n),&written,nullptr);CloseHandle(file);}
    return EXCEPTION_EXECUTE_HANDLER;
}
static void __cdecl SafeCaptureLoss(void* aspect,float life) {
    if(effectsDisabled)return;
    __try {CaptureLoss(aspect,life);}
    __except(RecordFailure(GetExceptionInformation())) {}
}
static void __cdecl SafeRenderHits() {
    if(effectsDisabled)return;
    __try {RenderHits();}
    __except(RecordFailure(GetExceptionInformation())) {}
}

__declspec(naked) static void LifeHook() {
    __asm {
        mov eax,dword ptr [esp+8]
        pushfd
        pushad
        mov ebx,esp
        sub esp,528
        and esp,-16
        fxsave [esp]
        fninit
        cld
        push eax
        push esi
        call SafeCaptureLoss
        add esp,8
        fxrstor [esp]
        mov esp,ebx
        popad
        popfd
        mov dword ptr [esi+70h],eax
        jmp dword ptr [lifeResume]
    }
}
__declspec(naked) static void FrameHook() {
    __asm {
        pushfd
        pushad
        mov ebx,esp
        sub esp,528
        and esp,-16
        fxsave [esp]
        fninit
        cld
        call SafeRenderHits
        fxrstor [esp]
        mov esp,ebx
        popad
        popfd
        jmp dword ptr [cursorDraw]
    }
}

static bool InstallDamage(unsigned char* base) {
    const auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>4096) return false;
    const auto pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
       pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC || pe->OptionalHeader.SizeOfImage!=0x3b4000) return false;
    const unsigned char life[]={0x8b,0x44,0x24,0x08,0x89,0x46,0x70};
    const unsigned char frame[]={0xe8,0x80,0x32,0,0};
    if(memcmp(base+0x1323c7,life,7)||memcmp(base+0x2e039d,frame,5)) return false;
    gameBase=base;lifeResume=reinterpret_cast<uintptr_t>(base+0x1323ce);
    cursorDraw=reinterpret_cast<uintptr_t>(base+0x2e3622);
    unsigned char patchLife[7],patchFrame[5];
    RelativeJump(patchLife,reinterpret_cast<uintptr_t>(base+0x1323c7),reinterpret_cast<uintptr_t>(LifeHook),7);
    RelativeJump(patchFrame,reinterpret_cast<uintptr_t>(base+0x2e039d),reinterpret_cast<uintptr_t>(FrameHook),5);
    patchFrame[0]=0xe8;
    DWORD oldLife=0,oldFrame=0;
    if(!VirtualProtect(base+0x1323c7,7,PAGE_EXECUTE_READWRITE,&oldLife)) return false;
    if(!VirtualProtect(base+0x2e039d,5,PAGE_EXECUTE_READWRITE,&oldFrame)) {
        DWORD ignored;VirtualProtect(base+0x1323c7,7,oldLife,&ignored);return false;
    }
    memcpy(base+0x1323c7,patchLife,7);memcpy(base+0x2e039d,patchFrame,5);
    DWORD ignored;VirtualProtect(base+0x1323c7,7,oldLife,&ignored);
    VirtualProtect(base+0x2e039d,5,oldFrame,&ignored);
    FlushInstructionCache(GetCurrentProcess(),base,pe->OptionalHeader.SizeOfImage);
    return true;
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved) {
    (void)reserved;
    if(reason==DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        GetModuleFileNameW(instance,diagnosticPath,MAX_PATH);
        wchar_t* last=wcsrchr(diagnosticPath,L'\\');
        if(last) wcscpy_s(last+1,MAX_PATH-static_cast<size_t>(last+1-diagnosticPath),L"damage_numbers_mod\\runtime.log");
        InstallDamage(reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr)));
    }
    return TRUE;
}

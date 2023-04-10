#pragma once
/**
 * Some macros to help new users.
 * These should also be useful to experienced users
 * as it should help in keeping names consistent.
 *
 */
#include <API/ARK/Ark.h>

#define COMMA ,
#define CAST(as, from) reinterpret_cast<as>(from)
#define FORCECAST(as, from) *CAST(as*, CAST(void*, &from))
#define DSTR(str) #str
#define STR(str) std::to_string(str)
#define FIELD(type, cls, field) type ## & field ## Field() { \
    static auto offset = calculateFieldOffset(this, DSTR(cls) "." DSTR(field)); \
    return *((type ## *)(reinterpret_cast<size_t *>(this) + offset)); \
}

#define DELAYEXECUTE API::Timer::Get().DelayExecute

#define SET_HOOK(className, functionName) \
ArkApi::GetHooks().SetHook(DSTR(className) "." DSTR(functionName), &Hook_ ## className ## _ ## functionName, & className ## _ ## functionName ## _original)
/* Macro to remove a hook easily. */
#define DISABLE_HOOK(className, functionName) \
ArkApi::GetHooks().DisableHook(DSTR(className) "." DSTR(functionName), &Hook_ ## className ## _ ## functionName)
/* Shortcut */
#define HOOKS \
ArkApi::GetHooks()
/* Shortcut */
#define COMMANDS \
ArkApi::GetCommands()
/* Shortcut */
#define API_UTILS \
ArkApi::GetApiUtils()
/* Shortcut */
#define LOG \
Log::GetLog()
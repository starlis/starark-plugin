#pragma once
#include "includes.h"


namespace SAOmega {
    std::string getDinoName(APrimalDinoCharacter* dino);
    inline long long calculateFieldOffset(void* _this, const char *field) {
        size_t *actualPtr = reinterpret_cast<size_t *>(GetNativePointerField<void*>(_this, field));
        return actualPtr - reinterpret_cast<size_t *>(_this);
    }

	BOOL Load();
	BOOL Unload();
    namespace Breeding {
        void Load();
        void Unload();
    }
    namespace Spyglass {
        void Load();
        void Unload();
    }
    namespace Experience {
        void Load();
        void Unload();
    }
    namespace Loot {
        void Load();
        void Unload();
    }
    namespace WorldSave {
        void Load();
        void Unload();
    }
}
using namespace SAOmega;

extern "C"  __declspec(dllexport) void plugin_load();
extern "C"  __declspec(dllexport) void plugin_unload();
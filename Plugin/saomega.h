#pragma once
#include "includes.h"
#include <TaskExecutor.h>

namespace SA {

    extern bool exiting;
    extern volatile bool shuttingDown;
    extern nlohmann::json config;
    extern inline long long calculateFieldOffset(void* _this, const char *field);
    extern inline uint64_t timestamp();
    extern std::string serverDir;
    extern SA::TaskExecutor taskExecutor;

    std::string getDinoName(APrimalDinoCharacter* dino);
    void writeFile(const std::string& file, const std::string& data);
    void writeStatus(const std::string& status);
    extern inline unsigned int getThreadId();

	void Load();
	void Unload();
    namespace Config {
        void ReadConfig();
    }
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
        void Exit();
        void Load();
        void Unload();
    }
    namespace Commands {
        void Load();
        void Unload();
    }
}
using namespace SA;
#pragma once
#include <sstream>
#include "saomega.h"

namespace SA {
    inline long long calculateFieldOffset(void* _this, const char *field) {
        size_t *actualPtr = reinterpret_cast<size_t *>(GetNativePointerField<void*>(_this, field));
        return actualPtr - reinterpret_cast<size_t *>(_this);
    }
    inline uint64_t timestamp() {
        return std::time(nullptr);
    }
    inline unsigned int getThreadId() {
        auto threadId = std::this_thread::get_id();
        return *static_cast<unsigned int*>(static_cast<void*>(&threadId));;
    }

    void writeFile(const std::string& file, const std::string& data) {
        taskExecutor.run([file, data]() {
            std::ofstream myfile;
            myfile.open(serverDir + file);
            myfile << data + "\n";
            myfile.close();
        });
    }
    void writeStatus(const std::string& status) {
        writeFile("status.txt", status);
    }

    std::string getDinoName(APrimalDinoCharacter* dino) {
        FString name;
        dino->GetDescriptiveName(&name);
        return dino->DinoNameTagField().ToString().ToString() + "(" + name.ToString() + ")";
    }

}

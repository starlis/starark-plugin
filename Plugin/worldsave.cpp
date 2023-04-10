#include "saomega.h"
#include <chrono>
#include "SafeQueue.hpp"

using namespace std;

namespace SAOmega::WorldSave {

    volatile bool shuttingDown = false;

    struct __declspec(align(8)) FGraphEvent {
        uint8 _padding[0x40];
    };
    struct FWriteFileTaskInfo {
        uint8 padding[0x58];
    };
    struct __declspec(align(8)) FWriteFileTask {
        TSharedRef<FWriteFileTaskInfo,0> TaskInfo;
        FString FileName;
        FString TempFileName;
        bool bCopy;
    };

    DECLARE_HOOK(UWorld_SaveToFile, void*, UWorld*, void*, FString *, FString *, const void *);
    DECLARE_HOOK(FWriteFileTask_DoTask, void, FWriteFileTask*, ENamedThreads::Type, const void*);

    void Hook_FWriteFileTask_DoTask(FWriteFileTask *_this, ENamedThreads::Type CurrentThread, const void* MyCompletionGraphEvent) {
        auto start = std::chrono::high_resolution_clock::now();
        FWriteFileTask_DoTask_original(_this, CurrentThread, MyCompletionGraphEvent);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<int64_t, std::nano> dur_ns = (end - start);
        int64_t measured_ns = dur_ns.count();
        LOG->info("SaveToFile: " + STR(measured_ns));
    }
    void* Hook_UWorld_SaveToFile(UWorld *_this, void *result, FString *filename, FString *tempFilename, const void *typesToSave) {
        auto start = std::chrono::high_resolution_clock::now();

        void* res = UWorld_SaveToFile_original(_this, result, filename, tempFilename, typesToSave);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<int64_t, std::nano> dur_ns = (end - start);
        int64_t measured_ns = dur_ns.count();
        LOG->info("SaveToFile: " + STR(measured_ns));
        return res;
    }
    void Load() {
        //SET_HOOK(UWorld, SaveToFile);
        //SET_HOOK(FWriteFileTask, DoTask);
    }

    void Unload() {
        shuttingDown = true;
        //DISABLE_HOOK(UWorld, SaveToFile);
        //DISABLE_HOOK(FWriteFileTask, DoTask);
    }
}
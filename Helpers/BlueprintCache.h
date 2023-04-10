//
// Created by Aikar on 4/2/2023.
//

#ifndef SAOMEGA_BLUEPRINTCACHE_H
#define SAOMEGA_BLUEPRINTCACHE_H

#include "includes.h"

class BlueprintCache {
private:
    FString path;
    UClass* cls;
public:
    BlueprintCache(char* path) {
        this->path = path;
    }
    BlueprintCache(FString path) {
        this->path = path;
    }
    UClass* Get() {
        if (!cls) {
            cls = UVictoryCore::BPLoadClass(&path);
        }
        return cls;
    }
};

#endif //SAOMEGA_BLUEPRINTCACHE_H

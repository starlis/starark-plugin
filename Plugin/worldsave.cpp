#include "saomega.h"
#include <chrono>
#include "SafeQueue.hpp"

using namespace std;

namespace SA::WorldSave {
    struct WorldSerializedObject {
        bool ShouldSpawn;
        __int64 DataOffset;
        int LoadFlags;
        UObject *Object;
        unsigned int Index;
        int CachedSaveVersion;
        static __int64 __fastcall GetSerializedObjectsNum() {

        }
    };

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

    DECLARE_HOOK(APrimalDinoCharacter_TakeDamage, float, APrimalDinoCharacter*, float, UClass **, AController *, AActor *);
    DECLARE_HOOK(AShooterGameMode_SaveWorld, void, AShooterGameMode*, bool);
    DECLARE_HOOK(TestOnlineGameSettings_TestOnlineGameSettings, void, AShooterGameMode*, bool);
    DECLARE_HOOK(AShooterGameMode_PreLogin, void, AShooterGameMode*, FString*, FString*, TSharedPtr<FUniqueNetId, 0>*, FString*, FString*);
    DECLARE_HOOK(AShooterGameMode_StartNewShooterPlayer, void, AShooterGameMode*, APlayerController*, bool, bool, const FPrimalPlayerCharacterConfigStruct&, UPrimalPlayerData*);
    DECLARE_HOOK(UShooterCheatManager_ServerChat, void, UShooterCheatManager*, FString*);
    DECLARE_HOOK(UShooterCheatManager_Broadcast, void, UShooterCheatManager*, FString*);



    uint64_t lastDmg = 0;
    uint64_t lastSave = 0;
    bool hasPendingSave = false;
    bool pendingForceSave = false;
    float Hook_APrimalDinoCharacter_TakeDamage(APrimalDinoCharacter *_this, float DamageAmount, UClass **DamageEvent, AController *EventInstigator, AActor *DamageCauser) {
        int base = _this->AbsoluteBaseLevelField();
        if (base == 1 && DamageCauser && DamageCauser->TargetingTeamField() >= 50000) {
            lastDmg = timestamp();
        }
        //LOG->info(getDinoName(_this) + " " + STR(base) + " took dmg " + STR(DamageAmount) + " from "
        //+ (DamageCauser ? DamageCauser->ClassField()->NameField().ToString().ToString() : "nothing") + " - " + STR(lastDmg));
        return APrimalDinoCharacter_TakeDamage_original(_this, DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    }

    FString shuttingDownMsg = "The server is currently shutting down. Reconnect in a few minutes.";
    void Hook_AShooterGameMode_PreLogin(AShooterGameMode *_this, FString* Options, FString* Address, TSharedPtr<FUniqueNetId, 0>* UniqueId, FString* authToken, FString* ErrorMessage) {
        if (shuttingDown) {
            *ErrorMessage = shuttingDownMsg;
            return;
        }
        AShooterGameMode_PreLogin_original(_this, Options, Address, UniqueId, authToken, ErrorMessage);
    }

    void Hook_AShooterGameMode_StartNewShooterPlayer(AShooterGameMode* _this, APlayerController* newPlayer, bool forceCreateNewPlayerData, bool isFromLogin, const FPrimalPlayerCharacterConfigStruct& characterConfig, UPrimalPlayerData* playerData) {
        AShooterPlayerController *playerController = static_cast<AShooterPlayerController *>(newPlayer);
        AShooterGameMode_StartNewShooterPlayer_original(_this, newPlayer, forceCreateNewPlayerData, isFromLogin,
                                                        characterConfig, playerData);
        AShooterCharacter *playerPawn = playerController->LastControlledPlayerCharacterField().Get();
        auto now = timestamp();
        if (!playerPawn || isFromLogin || now - lastSave > 60*8 || hasPendingSave || pendingForceSave) {
            return;
        }
        hasPendingSave = true;
        pendingForceSave = true;
        DELAYEXECUTE([]() {
            hasPendingSave = false;
            pendingForceSave = true;
            ArkApi::GetApiUtils().GetShooterGameMode()->SaveWorld(false);
        }, 30);
    }

    void Hook_UShooterCheatManager_Broadcast(UShooterCheatManager* _this, FString* msg) {
        Hook_UShooterCheatManager_ServerChat(_this, msg);
    }
    void Hook_UShooterCheatManager_ServerChat(UShooterCheatManager* _this, FString* msg) {
        if (msg->Contains("will shutdown in 1 minute")) {
            shuttingDown = true;
            ArkApi::GetApiUtils().GetCheatManager()->SaveWorldDisableTransfer();
        } else if (!shuttingDown && msg->Contains("Server is about to shutdown, performing a world save")) {
            shuttingDown = true;
            ArkApi::GetApiUtils().GetCheatManager()->SaveWorldDisableTransfer();
        } else if (msg->Contains("no longer shutting down")) {
            shuttingDown = false;
            return;
        } else if (!shuttingDown && msg->Contains("Server shutdown required.")) {
            shuttingDown = true;
        }
        if (!msg->Contains("A world save is about to be performed")) {
            UShooterCheatManager_ServerChat_original(_this, msg);
        }
    }

    void Hook_AShooterGameMode_SaveWorld(AShooterGameMode *_this, bool forceWaitOnSave) {
        auto now = timestamp();
        auto sinceDmg = now - lastDmg;
        auto sinceSave = now - lastSave;
        const auto saveRate = 60*5;
        const auto dmgRate = 60;
        if (!forceWaitOnSave && !shuttingDown && !pendingForceSave && sinceSave < 60*20) {
            if (sinceDmg < dmgRate || sinceSave < saveRate) {
                if (!hasPendingSave && sinceSave >= saveRate) {
                    LOG->info("Delaying world save due to boss dmg or last save recently " + STR(now) + ":" +
                                  STR(sinceDmg) + ":" + STR(sinceSave));
                    hasPendingSave = true;
                    DELAYEXECUTE([]() {
                        hasPendingSave = false;
                        ArkApi::GetApiUtils().GetShooterGameMode()->SaveWorld(false);
                    }, dmgRate - static_cast<int>(sinceDmg));
                }
                return;
            }
            auto color = FLinearColor(255, 0, 0, 255);
            ArkApi::GetApiUtils().SendNotificationToAll(color, 1.1f, 10, nullptr, "Server is performing a world save.");
            lastSave = now;
            DELAYEXECUTE([]() {
                pendingForceSave = true; // ensure this call isn't skipped
                ArkApi::GetApiUtils().GetShooterGameMode()->SaveWorld(false);
            }, 0);
        } else {
            lastSave = now;
            pendingForceSave = false;
            hasPendingSave = false;
            AShooterGameMode_SaveWorld_original(_this, forceWaitOnSave);
        }
    }
    void Exit() {
        //ArkApi::GetApiUtils().GetShooterGameMode()->SaveWorld(true);
    }
    void Load() {
        SET_HOOK(APrimalDinoCharacter, TakeDamage);
        SET_HOOK(AShooterGameMode, PreLogin);
        SET_HOOK(AShooterGameMode, SaveWorld);
        SET_HOOK(UShooterCheatManager, ServerChat);
        SET_HOOK(UShooterCheatManager, Broadcast);
        SET_HOOK(AShooterGameMode, StartNewShooterPlayer);
    }

    void Unload() {
        shuttingDown = true;
        DISABLE_HOOK(APrimalDinoCharacter, TakeDamage);
        DISABLE_HOOK(AShooterGameMode, PreLogin);
        DISABLE_HOOK(AShooterGameMode, SaveWorld);
        DISABLE_HOOK(UShooterCheatManager, ServerChat);
        DISABLE_HOOK(UShooterCheatManager, Broadcast);
        DISABLE_HOOK(AShooterGameMode, StartNewShooterPlayer);
    }
}
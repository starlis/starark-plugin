#include "saomega.h"
#include <chrono>
#include "SafeQueue.hpp"

using namespace std;

namespace SAOmega::WorldSave {
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

    DECLARE_HOOK(APrimalDinoCharacter_TakeDamage, float, APrimalDinoCharacter*, float, UClass **, AController *, AActor *);
    DECLARE_HOOK(AShooterGameMode_SaveWorld, void, AShooterGameMode*, bool);
    DECLARE_HOOK(AShooterGameMode_BeginPlay, void, AShooterGameMode*);
    DECLARE_HOOK(TestOnlineGameSettings_TestOnlineGameSettings, void, AShooterGameMode*, bool);
    DECLARE_HOOK(AShooterGameMode_PreLogin, void, AShooterGameMode*, FString*, FString*, TSharedPtr<FUniqueNetId, 0>*, FString*, FString*);
    DECLARE_HOOK(UShooterCheatManager_ServerChat, void, UShooterCheatManager*, FString*);

    /*AShooterGameMode* gameMode;
    void Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this) {
        gameMode = _this;
        AShooterGameMode_BeginPlay_original(_this);
    }*/

    uint64_t lastDmg = 0;
    uint64_t lastSave = 0;
    bool hasPendingSave = false;
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

    void Hook_UShooterCheatManager_ServerChat(UShooterCheatManager* _this, FString* msg) {
        if (msg->Contains("will shutdown in 1 minute") || msg->Contains("Server is about to shutdown, performing a world save")) {
            shuttingDown = true;
            /*if (gameMode) {
                CAST(AShooterGameState*, gameMode->GameStateField())->ServerSessionNameField()
                gameMode->GameSessionField()->UpdateSessionJoinability();
            }*/
        } else if (msg->Contains("no longer shutting down")) {
            shuttingDown = false;

            return;
        }
        if (!msg->Contains("A world save is about to be performed")) {
            UShooterCheatManager_ServerChat_original(_this, msg);
        }
    }

    void Hook_AShooterGameMode_SaveWorld(AShooterGameMode *_this, bool forceWaitOnSave) {
        auto now = timestamp();
        if (!forceWaitOnSave && !shuttingDown) {
            auto sinceDmg = now - lastDmg;
            auto sinceSave = now - lastSave;
            const auto saveRate = 60*5;
            const auto dmgRate = 60;
            if (sinceDmg < dmgRate || sinceSave < saveRate) {
                if (!hasPendingSave && sinceSave >= saveRate && sinceSave < 60*20) {
                    LOG->info("Delaying world save due to boss dmg or last save recently " + STR(now) + ":" +
                                  STR(sinceDmg) + ":" + STR(sinceSave));
                    hasPendingSave = true;
                    DELAYEXECUTE([_this]() {
                        hasPendingSave = false;
                        Hook_AShooterGameMode_SaveWorld(_this, false);
                    }, 60);
                }
                return;
            }
            auto color = FLinearColor(255, 0, 0, 255);
            ArkApi::GetApiUtils().SendNotificationToAll(color, 1.1f, 10, nullptr, "Server is performing a world save.");
            lastSave = now;
            DELAYEXECUTE([_this]() {
                AShooterGameMode_SaveWorld_original(_this, false);
            }, 0);
        } else {
            lastSave = now;
            AShooterGameMode_SaveWorld_original(_this, true);
        }
    }
    void Load() {
        SET_HOOK(APrimalDinoCharacter, TakeDamage);
        SET_HOOK(AShooterGameMode, PreLogin);
        SET_HOOK(AShooterGameMode, SaveWorld);
        //SET_HOOK(AShooterGameMode, BeginPlay);
        SET_HOOK(UShooterCheatManager, ServerChat);
    }

    void Unload() {
        shuttingDown = true;
        DISABLE_HOOK(APrimalDinoCharacter, TakeDamage);
        DISABLE_HOOK(AShooterGameMode, PreLogin);
        DISABLE_HOOK(AShooterGameMode, SaveWorld);
        //DISABLE_HOOK(AShooterGameMode, BeginPlay);
        DISABLE_HOOK(UShooterCheatManager, ServerChat);
    }
}
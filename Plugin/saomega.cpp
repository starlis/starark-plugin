#pragma once
/*
 *								READ ME
 *	This file controls what is loaded and unloaded in the plugin.
 *	You should put your hooks/commands in here.
*/
#include "saomega.h"
#include <fstream>
#include <shellapi.h>

#pragma comment(lib, "mysqlclient.lib")
#pragma comment(lib, "ArkApi.lib")

namespace SA {
    using namespace SA;
    bool exiting = false;
    volatile bool shuttingDown;
    std::string serverDir = ArkApi::Tools::GetCurrentDir() + + R"(\..\..\..\)";
    TaskExecutor taskExecutor(2);

    DECLARE_HOOK(AShooterGameMode_AreTribesAllied, bool, AShooterGameMode*, int, int);
    DECLARE_HOOK(AShooterPlayerState_IsAlliedWith, bool, AShooterPlayerState*, int);
    DECLARE_HOOK(APrimalCharacter_IsAlliedWithOtherTeam, bool, APrimalCharacter*, int);
    DECLARE_HOOK(FEngineLoop_Exit, void, void*);
    DECLARE_HOOK(AShooterGameMode_BeginPlay, void, AShooterGameMode*);
    DECLARE_HOOK(APrimalCharacter_TakeDamage, float, APrimalCharacter*, float, UClass **, AController *, AActor *);

    FString ServerKey;
    uint64_t lastTickStatus = 0;


    void onReady();
    void Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this) {
        AShooterGameMode_BeginPlay_original(_this);
        onReady();
    }
    void onTick(float delta_seconds) {
        auto now = timestamp();
        if (now - lastTickStatus > 15) {
            lastTickStatus = now;
            writeFile("tick.txt", STR(now));
        }
    }

    const int adminTribeId = 1830631037;

    bool isEitherAdminTribe(int tribe1, int tribe2) {
        return tribe1 >= 50000 && tribe2 >= 50000 && tribe1 != tribe2 && (tribe1 == adminTribeId || tribe2 == adminTribeId);
    }

    bool Hook_AShooterGameMode_AreTribesAllied(AShooterGameMode* _this, int tribe1, int tribe2) {
        if (isEitherAdminTribe(tribe1, tribe2)) {
            return true;
        } else {
            return AShooterGameMode_AreTribesAllied_original(_this, tribe1, tribe2);
        }
    }
    bool Hook_AShooterPlayerState_IsAlliedWith(AShooterPlayerState* _this, int tribe2) {
        if (isEitherAdminTribe(_this->TargetingTeamField(), tribe2)) {
            return true;
        } else {
            return AShooterPlayerState_IsAlliedWith_original(_this, tribe2);
        }
    }
    bool Hook_APrimalCharacter_IsAlliedWithOtherTeam(APrimalCharacter* _this, int tribe2) {
        if (isEitherAdminTribe(_this->TargetingTeamField(), tribe2)) {
            return true;
        } else {
            return APrimalCharacter_IsAlliedWithOtherTeam_original(_this, tribe2);
        }
    }

    /*
     * bool IsAlliedWith(int OtherTeam) { return NativeCall<bool, int>(this, "AShooterPlayerState.IsAlliedWith", OtherTeam); }
	bool IsAlliedWithOtherTeam(int OtherTeamID) { return NativeCall<bool, int>(this, "APrimalCharacter.IsAlliedWithOtherTeam", OtherTeamID); }
     */

    float Hook_APrimalCharacter_TakeDamage(APrimalCharacter *_this, float amt, UClass **event, AController *instigator, AActor *causer) {
        if (_this->IsA(AShooterCharacter::StaticClass()) && (!_this->ControllerField() || !_this->ControllerField()->IsA(AShooterPlayerController::StaticClass()))) {
            return 0.0f;
        }
        return APrimalCharacter_TakeDamage_original(_this, amt, event, instigator, causer);
    }
    
    void Hook_FEngineLoop_Exit(void* _this) {
        exiting = true;
        taskExecutor.shutdown();
        if (shuttingDown) {
            writeStatus("cleanshutdown");
        } else {
            writeStatus("unexpectedshutdown");
        }

        //SA::WorldSave::Exit();
        FEngineLoop_Exit_original(_this);
    }
    void onReady() {
       writeStatus("ready");
       LOG->info("SA Ready");
    }

    void Load()
	{
		Log::Get().Init("SA");
        writeStatus("load");

        LPWSTR* argv;
        {
            int argc;
            int i;
            FString param(L"-serverkey=");

            argv = CommandLineToArgvW(GetCommandLineW(), &argc);
            if (nullptr != argv) {
                for (i = 0; i < argc; i++) {
                    FString arg(argv[i]);
                    if (arg.Contains(param)) {
                        if (arg.RemoveFromStart(param)) {
                            ServerKey = arg;
                            break;
                        }
                    }
                }

                LocalFree(argv);
            }
        }

        LOG->info("SA Loading - " + ServerKey.ToString());
        try {
            SA::Config::ReadConfig();
        } catch (std::exception& e) {
            LOG->error("SA Exception: " + std::string(e.what()));
        }
        ArkApi::GetCommands().AddOnTickCallback("SA::OnTick", &SA::onTick);

        SA::Breeding::Load();
        SA::Spyglass::Load();
        SA::Loot::Load();
        SA::Experience::Load();
        SA::Commands::Load();
        SA::WorldSave::Load();
        SET_HOOK(APrimalCharacter, TakeDamage);
        SET_HOOK(AShooterGameMode, BeginPlay);
        SET_HOOK(AShooterGameMode, AreTribesAllied);
        SET_HOOK(AShooterPlayerState, IsAlliedWith);
        SET_HOOK(APrimalCharacter, IsAlliedWithOtherTeam);
        SET_HOOK(FEngineLoop, Exit);

        if (ArkApi::GetApiUtils().GetStatus() == ArkApi::ServerStatus::Ready) {
            onReady();
        }
	}

	void Unload()
	{
        LOG->info("SA Unloading");
        ArkApi::GetCommands().RemoveOnTickCallback("SA::OnTick");
        SA::Breeding::Unload();
        SA::Spyglass::Unload();
        SA::Loot::Unload();
        SA::Experience::Unload();
        SA::Commands::Unload();
        SA::WorldSave::Unload();
        DISABLE_HOOK(APrimalCharacter, TakeDamage);
        DISABLE_HOOK(AShooterGameMode, BeginPlay);
        DISABLE_HOOK(AShooterGameMode, AreTribesAllied);
        DISABLE_HOOK(AShooterPlayerState, IsAlliedWith);
        DISABLE_HOOK(APrimalCharacter, IsAlliedWithOtherTeam);
        DISABLE_HOOK(FEngineLoop, Exit);

        taskExecutor.shutdown();

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
		LOG->info("SA Unloaded");
	}
}
extern "C"  __declspec(dllexport) void __fastcall Plugin_Init() {
    SA::Load();
}
extern "C"  __declspec(dllexport) void __fastcall Plugin_Unload() {
    SA::Unload();
}
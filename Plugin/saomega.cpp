#pragma once
/*
 *								READ ME
 *	This file controls what is loaded and unloaded in the plugin.
 *	You should put your hooks/commands in here.
*/
#include "saomega.h"
#include <fstream>
#include <shellapi.h>

namespace SAOmega {
    DECLARE_HOOK(AShooterGameMode_AreTribesAllied, bool, AShooterGameMode*, int, int);
    nlohmann::json config;

    std::string getDinoName(APrimalDinoCharacter* dino) {
        FString name;
        dino->GetDescriptiveName(&name);
        return dino->DinoNameTagField().ToString().ToString() + "(" + name.ToString() + ")";
    }

    FString ServerKey;
    void ReadConfig() {
		const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/SAOmega/config.json";
		std::ifstream file{ config_path };
		if (!file.is_open()) {
            config = nlohmann::json({});
            LOG->warn("config.json missing");
            return;
        }


		file >> config;
		file.close();
	}

	void ReloadConfig(APlayerController* player_controller, FString*, bool)
	{
		auto shooter_controller = static_cast<AShooterPlayerController*>(player_controller);
		try { ReadConfig(); }
		catch (const std::exception& error)
		{
			API_UTILS.SendServerMessage(shooter_controller, FColorList::Red, "Failed to reload config");
			Log::GetLog()->error(error.what());
			return;
		}
		API_UTILS.SendServerMessage(shooter_controller, FColorList::Green, "Reloaded config");
	}

    void sendTags(AActor *target) {

    }

    AActor *getTargetActor(AShooterCharacter *pchar) {
        AActor* target = pchar->GetAimedActor(ECollisionChannel::ECC_GameTraceChannel2, nullptr, 0.0, 0.0, nullptr, nullptr, false, false, false);
        return target;
    }

    void onDumpTarget(AShooterPlayerController* player, FString* args, EChatSendMode::Type type) {
        auto* pchar = player->LastControlledPlayerCharacterField().Get();
        if (!pchar) {
            LOG->info("No Char");
            return;
        }
        AActor *target = getTargetActor(pchar);
        if (!target) {
            LOG->info("No Target");
            return;
        }

        LOG->info(target->NameField().ToString().ToString());
        LOG->info("-----------------");
        for (auto& tag : target->TagsField()) {
            LOG->info("Tag: " + tag.ToString().ToString());
        }
        LOG->info("CustomTag: " + target->CustomTagField().ToString().ToString());
        if (!target->IsA(APrimalCharacter::StaticClass())) {
            return;
        }
        auto* tchar = CAST(APrimalCharacter*, target);
        for (auto buff : tchar->BuffsField()) {
            LOG->info("Buff: " + buff->NameField().ToString().ToString());
            if (buff->MyBuffPersistentDataField() != nullptr) {
                auto pd = buff->MyBuffPersistentDataField();
            }
        }
    }
    void onDumpSouls(AShooterPlayerController* player, FString* args, EChatSendMode::Type type) {
        for (auto&& item : player->GetPlayerInventoryComponent()->InventoryItemsField()) {

        }
    }
    const int adminTribeId = 1830631037;
    bool Hook_AShooterGameMode_AreTribesAllied(AShooterGameMode* _this, int tribe1, int tribe2) {
        if (tribe1 >= 50000 && tribe2 >= 50000 && tribe1 != tribe2 && (tribe1 == adminTribeId || tribe2 == adminTribeId)) {
            return true;
        } else {
            return AShooterGameMode_AreTribesAllied_original(_this, tribe1, tribe2);
        }
    }

	BOOL Load()
	{
		Log::Get().Init("SAOmega");
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

        LOG->info("SAOmega Loading - " + ServerKey.ToString());
        /*try {
            ReadConfig();
        } catch (std::exception& e) {
            LOG->error("SAOmega Exception: " + std::string(e.what()));
        }*/

        SAOmega::Breeding::Load();
        SAOmega::Spyglass::Load();
        SAOmega::Loot::Load();
        SAOmega::Experience::Load();
        SAOmega::Commands::Load();
        SAOmega::WorldSave::Load();
        SET_HOOK(AShooterGameMode, AreTribesAllied);

        //COMMANDS.AddChatCommand("dumpsouls", &onDumpSouls);
        //COMMANDS.AddChatCommand("/dumptarget", &onDumpTarget);
		//COMMANDS.AddConsoleCommand("SAOmega.Reload", ReloadConfig);
		return TRUE;
	}

	BOOL Unload()
	{
        LOG->info("SAOmega Unloading");

        SAOmega::Breeding::Unload();
        SAOmega::Spyglass::Unload();
        SAOmega::Loot::Unload();
        SAOmega::Experience::Unload();
        SAOmega::Commands::Unload();
        SAOmega::WorldSave::Unload();
        DISABLE_HOOK(AShooterGameMode, AreTribesAllied);


        //COMMANDS.RemoveChatCommand("dumpsouls");
        //COMMANDS.RemoveChatCommand("dumptarget");
		//COMMANDS.RemoveConsoleCommand("SAOmega.Reload");
		LOG->info("SAOmega Unloaded");
		return TRUE;
	}
}
extern "C"  __declspec(dllexport) void Plugin_Init() {
    SAOmega::Load();
}
extern "C"  __declspec(dllexport) void Plugin_Unload() {
    SAOmega::Unload();
}
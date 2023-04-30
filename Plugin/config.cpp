#include "saomega.h"

namespace SA {
    nlohmann::json config;
}
namespace SA::Config {
    void ReadConfig() {
        const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/SA/config.json";
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

    void Load() {
        COMMANDS.AddConsoleCommand("SA.Reload", ReloadConfig);
    }
    void Unload() {
        COMMANDS.RemoveConsoleCommand("SA.Reload");
    }
}
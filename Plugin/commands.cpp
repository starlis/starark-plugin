#include "saomega.h"

namespace SA::Commands {

    void sendTags(AActor *target) {

    }

    AActor *getTargetActor(AShooterCharacter *pchar) {
        AActor* target = pchar->GetAimedActor(ECollisionChannel::ECC_GameTraceChannel2, nullptr, 0.0, 0.0, nullptr, nullptr, false, false, false);
        return target;
    }

    void onDumpTarget(AShooterPlayerController* player, FString* args, EChatSendMode::Type type) {
        if (!player->NameField().ToString().Equals("Aikar")) {
            return;
        }
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
        if (!player->NameField().ToString().Equals("Aikar")) {
            return;
        }
        for (auto&& item : player->GetPlayerInventoryComponent()->InventoryItemsField()) {

        }
    }
    void SendNotification(RCONClientConnection* rcon_connection, RCONPacket* rcon_packet, UWorld*) {
        FString msg = rcon_packet->Body;

        TArray<FString> parsed;
        msg.ParseIntoArray(parsed, L"|", true);
        if (parsed.Num() < 4) {
            FString reply = "Invalid Params: r,g,b,a|size|duration|Message";
            rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
            return;
        }
        float r = 255;
        float g = 255;
        float b = 255;
        float a = 255;
        TArray<FString> colorParsed;
        parsed[0].RemoveFromStart("SendNotification ");
        parsed[0].ParseIntoArray(colorParsed, L",", true);
        if (colorParsed.Num() >= 3) {
            try {
                r = std::stof(colorParsed[0].ToString());
            } catch (std::exception& e) {
                FString reply = "Invalid Params: r color: <" + colorParsed[0] + "> - " + e.what();
                rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
                return;
            }
            try {
                g = std::stof(colorParsed[1].ToString());
            } catch (std::exception& e) {
                FString reply = "Invalid Params: g color: " + colorParsed[1] + " - " + e.what();
                rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
                return;
            }
            try {
                b = std::stof(colorParsed[2].ToString());
            } catch (std::exception& e) {
                FString reply = "Invalid Params: b color: " + colorParsed[2] + " - " + e.what();
                rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
                return;
            }
        }
        auto color = FLinearColor(r, g, b, a);
        float scale = 1;
        float time = 15;
        try {
            scale = std::stof(parsed[1].ToString());
        } catch (std::exception& e) {
            FString reply = "Invalid Params: scale: " + parsed[1] + " - " + e.what();
            rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
            return;
        }
        try {
            time = std::stof(parsed[2].ToString());
        } catch (std::exception& e) {
            FString reply = "Invalid Params: time: " + parsed[2] + " - " + e.what();
            rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
            return;
        }
        std::string sendmsg = "";
        for (int i = 3; i < parsed.Num(); i++) {
            if (i >= 4) {
                sendmsg += " ";
            }
            sendmsg += parsed[i].ToString();
        }
        ArkApi::GetApiUtils().SendNotificationToAll(color, scale, time, nullptr, sendmsg.c_str());

    }

    void Load() {
        COMMANDS.AddRconCommand("SendNotification", &SendNotification);

        //COMMANDS.AddChatCommand("dumpsouls", &onDumpSouls);
        COMMANDS.AddChatCommand("/dumptarget", &onDumpTarget);
    }

    void Unload() {
        COMMANDS.RemoveRconCommand("SendNotification");
        //COMMANDS.RemoveChatCommand("dumpsouls");
        COMMANDS.RemoveChatCommand("dumptarget");
    }
}
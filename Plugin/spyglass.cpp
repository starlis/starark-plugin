#pragma once
#include "saomega.h"


namespace SAOmega::Spyglass {

    DECLARE_HOOK(AShooterGameMode_StartNewShooterPlayer, void, AShooterGameMode*, APlayerController*, bool, bool, const FPrimalPlayerCharacterConfigStruct&, UPrimalPlayerData*);
    DECLARE_HOOK(UPrimalInventoryComponent_NotifyItemAdded, void, UPrimalInventoryComponent*, UPrimalItem*, bool);
    DECLARE_HOOK(AShooterPlayerController_ReceivedPlayerState, void, AShooterPlayerController*);

    static BlueprintCache spyGlassBuffBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/AwesomeSpyGlass_Buff.AwesomeSpyGlass_Buff'");
    static BlueprintCache spyGlassBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/PrimalItem_AwesomeSpyGlass.PrimalItem_AwesomeSpyGlass'");

    bool activateSpyglass(UPrimalItem *item, AShooterCharacter *owner) {
        static auto spyGlassClass = spyGlassBlueprint.Get();


        if (!owner || !item->IsA(spyGlassClass) || item->bIsEngram()() || item->bIsBlueprint()())
            return false;


        item->Use(true);
        return true;
    }

    void Hook_AShooterGameMode_StartNewShooterPlayer(AShooterGameMode* _this, APlayerController* newPlayer, bool forceCreateNewPlayerData, bool isFromLogin, const FPrimalPlayerCharacterConfigStruct& characterConfig, UPrimalPlayerData* playerData) {
        AShooterPlayerController* playerController = static_cast<AShooterPlayerController*>(newPlayer);
        AShooterGameMode_StartNewShooterPlayer_original(_this, newPlayer, forceCreateNewPlayerData, isFromLogin, characterConfig, playerData);
        AShooterCharacter* playerPawn = playerController->LastControlledPlayerCharacterField().Get();
        if (!playerPawn) {
            return;
        }
        static auto spyGlassBuffClass = spyGlassBuffBlueprint.Get();
        static auto spyGlassClass = spyGlassBlueprint.Get();
        for (auto &&item: playerPawn->MyInventoryComponentField()->InventoryItemsField()) {
            if (!item->IsA(spyGlassClass) || item->bIsEngram()() || item->bIsBlueprint()())
                continue;

            playerPawn->DeactivateBuffs(spyGlassBuffClass, item, true);

            API::Timer::Get().DelayExecute([item]() {
                item->Use(true);
            }, 1);
            return;
        }
    }


    void Hook_UPrimalInventoryComponent_NotifyItemAdded(UPrimalInventoryComponent* _this, UPrimalItem* item, bool bEquipItem) {
        UPrimalInventoryComponent_NotifyItemAdded_original(_this, item, bEquipItem);

        AActor* owner = _this->GetOwner();
        if (!item && !owner || !owner->IsA(AShooterCharacter::StaticClass()))
            return;

        AShooterCharacter* playerCharacter = static_cast<AShooterCharacter*>(owner);
        activateSpyglass(item, playerCharacter);
    }

    void Load() {
        SET_HOOK(AShooterGameMode, StartNewShooterPlayer);
        SET_HOOK(UPrimalInventoryComponent, NotifyItemAdded);
    }
    void Unload() {
        DISABLE_HOOK(AShooterGameMode, StartNewShooterPlayer);
        DISABLE_HOOK(UPrimalInventoryComponent, NotifyItemAdded);
    }
}
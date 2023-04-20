#pragma once
#include "saomega.h"


namespace SAOmega::Spyglass {

    DECLARE_HOOK(AShooterGameMode_StartNewShooterPlayer, void, AShooterGameMode*, APlayerController*, bool, bool, const FPrimalPlayerCharacterConfigStruct&, UPrimalPlayerData*);
    DECLARE_HOOK(UPrimalInventoryComponent_NotifyItemAdded, void, UPrimalInventoryComponent*, UPrimalItem*, bool);

    static BlueprintCache spyGlassBuffBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/AwesomeSpyGlass_Buff.AwesomeSpyGlass_Buff'");
    static BlueprintCache spyGlassBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/PrimalItem_AwesomeSpyGlass.PrimalItem_AwesomeSpyGlass'");

    bool activateSpyglass(UPrimalItem *item, AShooterCharacter *owner) {




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

            // Player logged out likely with buff we have to clear it or it doesn't get refreshed correctly
            // This method is dangerous to call if a UI is open as it will freeze the UI, so we need to do this part immediately on login and no delay.
            playerPawn->DeactivateBuffs(spyGlassBuffClass, item, true);

            // But delay the buff apply
            API::Timer::Get().DelayExecute([item]() {
                item->Use(true);
            }, 5);
            return;
        }
    }

    void Hook_UPrimalInventoryComponent_NotifyItemAdded(UPrimalInventoryComponent* _this, UPrimalItem* item, bool bEquipItem) {
        UPrimalInventoryComponent_NotifyItemAdded_original(_this, item, bEquipItem);

        AActor* owner = _this->GetOwner();
        static auto spyGlassClass = spyGlassBlueprint.Get();
        if (!item && !owner || !owner->IsA(AShooterCharacter::StaticClass()))
            return;
        if (!item->IsA(spyGlassClass) || item->bIsEngram()() || item->bIsBlueprint()())
            return;

        API::Timer::Get().DelayExecute([item]() {
            item->Use(true);
        }, 2);
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
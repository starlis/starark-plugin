#pragma once
#include "saomega.h"


namespace SAOmega::Spyglass {

    DECLARE_HOOK(AShooterGameMode_StartNewShooterPlayer, void, AShooterGameMode*, APlayerController*, bool, bool, const FPrimalPlayerCharacterConfigStruct&, UPrimalPlayerData*);
    DECLARE_HOOK(UPrimalInventoryComponent_NotifyItemAdded, void, UPrimalInventoryComponent*, UPrimalItem*, bool);
    DECLARE_HOOK(AShooterPlayerController_ReceivedPlayerState, void, AShooterPlayerController*);

    static BlueprintCache spyGlassBuffBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/AwesomeSpyGlass_Buff.AwesomeSpyGlass_Buff'");
    static BlueprintCache spyGlassBlueprint("Blueprint'/Game/Mods/AwesomeSpyglass/PrimalItem_AwesomeSpyGlass.PrimalItem_AwesomeSpyGlass'");

    void tryActivateSpyglass(AShooterPlayerController *controller) {
        AShooterCharacter* playerPawn = controller->LastControlledPlayerCharacterField().Get();
        if (!playerPawn) {
            return;
        }

        auto spyGlassClass = spyGlassBlueprint.Get();
        auto spyGlassBuffClass = spyGlassBuffBlueprint.Get();

        for (auto &&item: playerPawn->MyInventoryComponentField()->InventoryItemsField()) {
            if (!item->IsA(spyGlassClass) || item->bIsEngram()() || item->bIsBlueprint()())
                continue;

            playerPawn->DeactivateBuffs(spyGlassBuffClass, item, true);
            item->Use(true);
            return;
        }
    }

    void Hook_AShooterGameMode_StartNewShooterPlayer(AShooterGameMode* _this, APlayerController* newPlayer, bool forceCreateNewPlayerData, bool isFromLogin, const FPrimalPlayerCharacterConfigStruct& characterConfig, UPrimalPlayerData* playerData) {
        AShooterPlayerController* playerController = static_cast<AShooterPlayerController*>(newPlayer);
        AShooterGameMode_StartNewShooterPlayer_original(_this, newPlayer, forceCreateNewPlayerData, isFromLogin, characterConfig, playerData);
        API::Timer::Get().DelayExecute([playerController]() {
            tryActivateSpyglass(playerController);
        }, 5);
        API::Timer::Get().DelayExecute([playerController]() {
            tryActivateSpyglass(playerController);
        }, 10);
        API::Timer::Get().DelayExecute([playerController]() {
            tryActivateSpyglass(playerController);
        }, 15);
    }

    void Hook_UPrimalInventoryComponent_NotifyItemAdded(UPrimalInventoryComponent* _this, UPrimalItem* item, bool bEquipItem) {
        auto spyGlassClass = spyGlassBlueprint.Get();
        auto spyGlassBuffClass = spyGlassBuffBlueprint.Get();

        UPrimalInventoryComponent_NotifyItemAdded_original(_this, item, bEquipItem);
        if (!_this || !item || !item->IsA(spyGlassClass) || item->bIsEngram()() || item->bIsBlueprint()())
            return;

        AActor* owner = _this->GetOwner();
        if (!owner || !owner->IsA(AShooterCharacter::StaticClass()))
            return;

        AShooterCharacter* playerCharacter = static_cast<AShooterCharacter*>(owner);
        playerCharacter->DeactivateBuffs(spyGlassBuffClass, item, true);
        item->Use(true);
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
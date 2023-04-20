#pragma once
//
// Created by Aikar on 4/1/2023.
//
#include "saomega.h"


namespace SAOmega::Loot {
    BlueprintCache bpPath("Blueprint'/Game/PrimalEarth/CoreBlueprints/Inventories/DinoDropInventoryComponent_BP_Base.DinoDropInventoryComponent_BP_Base'");
    struct FSupplyCrateItemEntry {
        uint8 _padding[0x18];
        TArray<TSubclassOf<UPrimalItem>> Items;
        uint8 _padding2[0x68];
    };


    struct FSupplyCrateItemSet {
        uint8 _padding[0x40];
        TArray<FSupplyCrateItemEntry, FDefaultAllocator>& ItemEntriesField() {
            static auto offset = calculateFieldOffset(this, "FSupplyCrateItemSet.ItemEntries");
            return *((TArray<FSupplyCrateItemEntry, FDefaultAllocator>*)((size_t*)this + offset));
        }
    };

    bool shouldFilter(FString& name) {
        return (name.Contains("_ApexDrop_")
        || name.Contains("RecipeNote_")
        || name.Contains("PrimalItemCostume_")
        || name.Contains("PrimalItemSkin_")
        || name.Contains("PrimalItemArmor_Cloth")
        || name.Contains("PrimalItemArmor_Hide")
        || name.Contains("PrimalItemArmor_Fur")
        || name.Contains("PrimalItemArmor_Chitin")
        || name.Contains("PrimalItemArmor_Metal")
        || name.Contains("PrimalItemArmor_Scuba")
        || name.Contains("PrimalItemConsumable_Berry")
        || name.Contains("PrimalItem_WeaponStone")
        || name.Contains("PrimalItem_WeaponMetal")
        || name.Contains("PrimalItem_WeaponGPS")
        || name.Contains("PrimalItem_WeaponCompass")
        || name.Contains("PrimalItem_WeaponSickle")
        || name.Contains("PrimalItem_WeaponBow_")
        || name.Contains("PrimalItem_WeaponTorch_")
        || name.Contains("PrimalItem_WeaponCrossBow_")
        || name.Contains("PrimalItem_WeaponBoomerang")
        || name.Contains("PrimalItem_WeaponMachined")
        || name.Contains("PrimalItem_WeaponGun")
        || name.Contains("PrimalItem_WeaponPike")
        || name.Contains("PrimalItem_WeaponGrenade")
        || name.Contains("PrimalItem_WeaponProd_")
        || name.Contains("PrimalItem_WeaponSpear")
        || name.Contains("PrimalItem_WeaponSlingshot")
        || name.Contains("_NamelessVenom_")
        || name.Contains("_Wishbone_")
       );
    }
    struct RemoveUndesiredItems
    {
        bool operator()(const TSubclassOf<UPrimalItem>& item) const {
            if (!item.uClass) {
                return false;
            }
            auto name = item.uClass->NameField().ToString();
            bool b = shouldFilter(name);
            if (b) {
                //LOG->info(name.ToString() + " removed");
            }
            return b;
        }
    };

    void removeItems() {
        for (auto i = 0; i < Globals::GUObjectArray()().ObjObjects.NumElements; i++)
        {
            auto obj = Globals::GUObjectArray()().ObjObjects.GetObjectPtr(i)->Object;
            if (obj == nullptr) {
                continue;
            }

            // Skip invalid UObjects
            if (!obj->IsValidLowLevelFast(true))
                continue;

            // Skip over UObjects that aren't CDOs
            if (obj != obj->ClassField()->ClassDefaultObjectField())
                continue;

            // Skip over UObjects that aren't an APrimalDinoCharacter
            if (!obj->IsA(APrimalDinoCharacter::StaticClass()))
                continue;

            APrimalDinoCharacter* dinoCDO = (APrimalDinoCharacter*)obj;

            static auto dinoDropClass = bpPath.Get();
            for (UObject* listItem : dinoCDO->DeathInventoryTemplatesField().AssociatedObjects) {
                // DeathInventoryTemplates.AssociatedObjects is an array of UBlueprint*
                UBlueprint *bp = (UBlueprint *) listItem;

                // The BP class is a sublcass of UPrimalInventoryComponent
                if (!bp->GeneratedClassField().uClass->IsChildOf(dinoDropClass)) {
                    continue;
                }

                // Get the CDO for this inventory component and remove meat from its item sets
                UPrimalInventoryComponent *component = (UPrimalInventoryComponent *) bp->GeneratedClassField().uClass->ClassDefaultObjectField();

                auto& itemSets = FORCECAST(TArray<SAOmega::Loot::FSupplyCrateItemSet>, component->ItemSetsField());
                auto& additionalSets = FORCECAST(TArray<SAOmega::Loot::FSupplyCrateItemSet>,
                                                component->AdditionalItemSetsField());
                int count = 0;
                for (auto &set: itemSets) {
                    for (auto &entry: set.ItemEntriesField()) {
                        count += entry.Items.RemoveAll(RemoveUndesiredItems());

                    }
                }
                for (auto &set: additionalSets) {
                    for (auto &entry: set.ItemEntriesField()) {
                        count +=entry.Items.RemoveAll(RemoveUndesiredItems());
                    }
                }
                if (count > 0) {
                    //LOG->info("Removed " + STR(count) + " from " + component->NameField().ToString().ToString());
                }
            }
        }
    }

    void Load() {
        API::Timer::Get().DelayExecute(&removeItems, 0);
    }
    void Unload() {
    }
}
#pragma once
#include "saomega.h"

//
// Created by Aikar on 4/1/2023.
//
namespace SA::Experience {
    DECLARE_HOOK(UPrimalCharacterStatusComponent_AddExperience, void, UPrimalCharacterStatusComponent*, float, bool, EXPType::Type);

    float addExpAmount = 0.0f;
    void Hook_UPrimalCharacterStatusComponent_AddExperience(UPrimalCharacterStatusComponent* _this, float howMuch, bool bShareWithTribe, EXPType::Type type) {
        auto character = _this->GetPrimalCharacter();
        auto isPlayer = character->IsA(AShooterCharacter::StaticClass());
        auto isDino = character->IsA(APrimalDinoCharacter::StaticClass());
        auto origHowMuch = howMuch;
        auto origAddExpAmount = addExpAmount;
        if ((isPlayer || isDino) && character->TargetingTeamField() >= 50000) {
            FString name;
            auto level = _this->GetCharacterLevel();
            float levelBasedCap = (_this->GetExperienceRequiredForNextLevelUp() - _this->GetExperienceRequiredForPreviousLevelUp());
            float multipliers = 0.0f;
            if (type == EXPType::XP_ALPHAKILL) {
                multipliers += 10.0f;
            }
            if (level < 25) {
                multipliers += 4.0f;
                howMuch *= 6;
            } else if (level < 50) {
                multipliers += 3.0f;
                howMuch *= 4;
            } else if (level <= 100) {
                multipliers += 2.0f;
                howMuch *= 2;
            }
            if (isPlayer) {
                auto player = static_cast<AShooterCharacter*>(character);
                if (!player->IsRider()) {
                    multipliers += 5.0f;
                }
                name = player->PlayerNameField();
                if (addExpAmount > 0 && player->IsRider()) {
                    howMuch = max(addExpAmount / 2.0f, howMuch);
                }
            } else {
                name = character->NameField().ToString();
            }
            multipliers = max(1.0f, multipliers);
            float cap = min(isPlayer ? 200000.0f : 50000.0f, levelBasedCap * multipliers);
            if (level < 30) {
                cap = max(5000, cap);
            } else {
                cap = max(20000, cap);
            }

            if (cap > 1.0 && howMuch > cap) {
                if (levelBasedCap > 0) {
                    LOG->info(
                            "XPOverCap " + name.ToString() + " xp: " + STR(howMuch) + " - origXp: " + STR(origHowMuch) +
                            " - cap: " + STR(cap) + " level: " +
                            STR(level) + " levelBasedCap: " + STR(levelBasedCap) + " multipliers: " + STR(multipliers));
                }
                howMuch = min(howMuch, cap);
            }
        }
        addExpAmount = origHowMuch;
        UPrimalCharacterStatusComponent_AddExperience_original(_this, howMuch, bShareWithTribe, type);
        addExpAmount = origAddExpAmount;
    }

    void Load() {
        SET_HOOK(UPrimalCharacterStatusComponent, AddExperience);
    }

    void Unload() {
        DISABLE_HOOK(UPrimalCharacterStatusComponent, AddExperience);
    }
}
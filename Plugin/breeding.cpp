#pragma once
//
// Created by Aikar on 4/1/2023.
//
#include "saomega.h"

namespace SAOmega::Breeding {
    DECLARE_HOOK(APrimalDinoCharacter_DoMate, void, APrimalDinoCharacter*, APrimalDinoCharacter*);
    DECLARE_HOOK(APrimalDinoCharacter_BeginPlay, void, APrimalDinoCharacter*);
    DECLARE_HOOK(APrimalDinoCharacter_UpdateBabyCuddling, void, APrimalDinoCharacter*, long double, char, TSubclassOf<UPrimalItem>);

    void Hook_APrimalDinoCharacter_DoMate(APrimalDinoCharacter* femaleDino, APrimalDinoCharacter* maleDino) {
        auto max = femaleDino->MaxAllowedRandomMutationsField();
        auto femaleCountMale = femaleDino->RandomMutationsMaleField();
        auto femaleCountFemale = femaleDino->RandomMutationsFemaleField();
        auto femaleCount = femaleCountMale + femaleCountFemale;

        auto maleCountMale = maleDino->RandomMutationsMaleField();
        auto maleCountFemale = maleDino->RandomMutationsFemaleField();
        auto maleCount = maleCountMale + maleCountFemale;

        auto totalNextMutations = maleCount + femaleCount;
        auto femaleRolls = femaleDino->RandomMutationRollsField();
        auto maleRolls = maleDino->RandomMutationRollsField();
        //LOG->info("female: " + getDinoName(femaleDino) + " " + STR(femaleCount) + " rolls " + STR(femaleRolls));
        //LOG->info("male: " + getDinoName(maleDino) + " " + STR(maleCount) + " rolls " + STR(maleRolls));
        if (femaleRolls == 3 && maleRolls == 3) { // Isn't even trying for mutations, QOL to never get
            femaleDino->RandomMutationRollsField() = 0;
            maleDino->RandomMutationRollsField() = 0;
        } else if (totalNextMutations >= max) {
            LOG->info("Mating Mutations " + STR(totalNextMutations) + " max " + STR(max) + " for " + getDinoName(femaleDino));
            femaleDino->RandomMutationRollsField() = 0;
            maleDino->RandomMutationRollsField() = 0;
        } else {
            //LOG->info("Mating Mutations " + STR(totalNextMutations) + " less than max " + STR(max) + " for " + getDinoName(femaleDino));
        }
        APrimalDinoCharacter_DoMate_original(femaleDino, maleDino);
        // Restore back
        femaleDino->RandomMutationRollsField() = femaleRolls;
        maleDino->RandomMutationRollsField() = maleRolls;
    }

    void Hook_APrimalDinoCharacter_BeginPlay(APrimalDinoCharacter* _this) {
        APrimalDinoCharacter_BeginPlay_original(_this);
        _this->FemaleMatingRangeAdditionField() = max(_this->FemaleMatingRangeAdditionField(), 1024); // default 80
        //LOG->info("RANGE: " + getDinoName(_this) + STR(orig) + ":" + STR(_this->FemaleMatingRangeAdditionField()));
    }

    void Hook_APrimalDinoCharacter_UpdateBabyCuddling(APrimalDinoCharacter* _this, long double NewBabyNextCuddleTime, char NewBabyCuddleType, TSubclassOf<UPrimalItem> NewBabyCuddleFood) {
        // Always cuddles
        APrimalDinoCharacter_UpdateBabyCuddling_original(_this, NewBabyNextCuddleTime, EBabyCuddleType::PET, NewBabyCuddleFood);
    }

    void Load() {
        SET_HOOK(APrimalDinoCharacter, DoMate);
        SET_HOOK(APrimalDinoCharacter, BeginPlay);
        SET_HOOK(APrimalDinoCharacter, UpdateBabyCuddling);
    }
    void Unload() {
        DISABLE_HOOK(APrimalDinoCharacter, DoMate);
        DISABLE_HOOK(APrimalDinoCharacter, BeginPlay);
        DISABLE_HOOK(APrimalDinoCharacter, UpdateBabyCuddling);
    }
}
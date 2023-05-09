#ifndef GUARD_ARCHIPELAGO_H
#define GUARD_ARCHIPELAGO_H

#include "global.h"

struct ArchipelagoOptions
{
    /* 0x00 */ bool8 advanceTextWithHoldA;
    /* 0x01 */ bool8 isFerryEnabled;
    /* 0x02 */ bool8 areTrainersBlind;
    /* 0x03 */ bool8 canFlyWithoutBadge;
    /* 0x04 */ u16 expMultiplierNumerator;
    /* 0x06 */ u16 expMultiplierDenominator;
    /* 0x08 */ u16 birchPokemon;
    /* 0x0A */ bool8 guaranteedCatch;
    /* 0x0B */ bool8 betterShopsEnabled;
    /* 0x0C */ bool8 eliteFourRequiresGyms;
    /* 0x0D */ u8 eliteFourRequiredCount;
    /* 0x0E */ bool8 normanRequiresGyms;
    /* 0x0F */ u8 normanRequiredCount;
    /* 0x10 */ u8 startingBadges;
    /* 0x11 */ u8 receivedItemMessageFilter; // 0 = Show All; 1 = Show Progression Only; 2 = Show None
    /* 0x12 */ bool8 reusableTms;
    /* 0x13 */ u16 removedBlockers;
} __attribute__((packed));

extern const struct ArchipelagoOptions gArchipelagoOptions;
extern const u16 gBlockerBitToFlagMap[];

#endif // GUARD_ARCHIPELAGO_H

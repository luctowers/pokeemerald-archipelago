#ifndef GUARD_ARCHIPELAGO_H
#define GUARD_ARCHIPELAGO_H

#include "global.h"

struct ArchipelagoOptions
{
    bool8 advanceTextWithHoldA;
    bool8 isFerryEnabled;
    bool8 areTrainersBlind;
    bool8 canFlyWithoutBadge;
    u16 expMultiplierNumerator;
    u16 expMultiplierDenominator;
    u16 birchPokemon;
    bool8 guaranteedCatch;
    bool8 betterShopsEnabled;
} __attribute__((packed));

extern const struct ArchipelagoOptions gArchipelagoOptions;

#endif // GUARD_ARCHIPELAGO_H

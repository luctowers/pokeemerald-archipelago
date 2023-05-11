#include "archipelago.h"

const struct ArchipelagoOptions gArchipelagoOptions = {
    .advanceTextWithHoldA = FALSE,
    .isFerryEnabled = FALSE,
    .areTrainersBlind = FALSE,
    .canFlyWithoutBadge = FALSE,
    .expMultiplierNumerator = 100,
    .expMultiplierDenominator = 100,
    .birchPokemon = SPECIES_LOTAD,
    .guaranteedCatch = FALSE,
    .betterShopsEnabled = FALSE,
    .eliteFourRequiresGyms = FALSE,
    .eliteFourRequiredCount = 8,
    .normanRequiresGyms = FALSE,
    .normanRequiredCount = 4,
    .startingBadges = 0,
    .receivedItemMessageFilter = 0,
    .reusableTms = FALSE,
    .addRoute115Boulders = FALSE,
    .removedBlockers = 0,
    .freeFlyLocation = 0,
};

const struct ArchipelagoInfo gArchipelagoInfo = {
    .slotName = {0},
};

const u16 gBlockerBitToFlagMap[16] = {
    [0] = FLAG_HIDE_SAFARI_ZONE_SOUTH_CONSTRUCTION_WORKERS,
    [1] = FLAG_HIDE_LILYCOVE_CITY_WAILMER,
    [2] = FLAG_HIDE_ROUTE_110_TEAM_AQUA,
    [3] = FLAG_HIDE_AQUA_HIDEOUT_1F_GRUNTS_BLOCKING_ENTRANCE,
    [4] = FLAG_HIDE_ROUTE_119_TEAM_AQUA_BRIDGE,
    [5] = FLAG_HIDE_ROUTE_112_TEAM_MAGMA,
};

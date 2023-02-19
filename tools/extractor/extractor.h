#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>

#define NUM_LAND_ENCOUNTER_SLOTS 12
#define NUM_WATER_ENCOUNTER_SLOTS 5
#define NUM_FISHING_ENCOUNTER_SLOTS 10

struct ItemInfo {
    uint32_t ram_address;
    uint32_t rom_address;
    std::string flag_name;
    std::string name;
    uint16_t default_item;
};

struct EncounterSlotInfo {
    uint16_t default_species;
    uint8_t min_level;
    uint8_t max_level;
};

struct EncounterTableInfo {
    uint32_t ram_address;
    uint32_t rom_address;
    bool exists;
    EncounterSlotInfo encounter_slots[NUM_LAND_ENCOUNTER_SLOTS];
};

struct MapEncounterInfo {
    std::string map_name;
    EncounterTableInfo land_encounters;
    EncounterTableInfo water_encounters;
    EncounterTableInfo fishing_encounters;
};

#endif

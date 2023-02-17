#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>

struct ItemInfo {
    uint32_t ram_address;
    uint32_t rom_address;
    std::string flag_name;
    uint16_t default_item;
};

struct EncounterSlotInfo {
    uint16_t default_species;
    uint8_t min_level;
    uint8_t max_level;
};

struct EncounterTableInfo {
    std::string map_name;
    uint32_t land_pokemon_ram_address;
    uint32_t land_pokemon_rom_address;
    uint32_t water_pokemon_ram_address;
    uint32_t water_pokemon_rom_address;
    uint32_t fishing_pokemon_ram_address;
    uint32_t fishing_pokemon_rom_address;
    bool land_encounters_exist;
    bool water_encounters_exist;
    bool fishing_encounters_exist;
    EncounterSlotInfo land_encounters[12];
    EncounterSlotInfo water_encounters[5];
    EncounterSlotInfo fishing_encounters[10];
};

#endif

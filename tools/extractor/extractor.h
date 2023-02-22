#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <vector>

#include <json.hpp>

#define NUM_LAND_ENCOUNTER_SLOTS 12
#define NUM_WATER_ENCOUNTER_SLOTS 5
#define NUM_FISHING_ENCOUNTER_SLOTS 10

enum LocationType {
    GROUND_ITEM,
    HIDDEN_ITEM,
    NPC_GIFT,
    BADGE
};

std::string location_type_to_string (LocationType lt);

class LocationInfo {
    public:
        std::string name;
        LocationType type;
        uint16_t flag;
        uint32_t ram_address;
        uint32_t rom_address;
        uint16_t default_item;

        nlohmann::json to_json ();
};

struct EncounterSlotInfo {
    uint16_t default_species;
    uint8_t min_level;
    uint8_t max_level;
};

class EncounterTableInfo {
    public:
        uint32_t ram_address;
        uint32_t rom_address;
        bool exists;
        std::vector<std::shared_ptr<EncounterSlotInfo>> encounter_slots;

        nlohmann::json to_json ();
};

class MapInfo {
    public:
        std::string name;
        EncounterTableInfo land_encounters;
        EncounterTableInfo water_encounters;
        EncounterTableInfo fishing_encounters;
        // std::vector<std::string> warps;

        nlohmann::json to_json ();
};

#endif

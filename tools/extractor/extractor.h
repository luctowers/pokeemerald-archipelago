#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <vector>

#include <json.hpp>

#define NUM_LAND_ENCOUNTER_SLOTS 12
#define NUM_WATER_ENCOUNTER_SLOTS 5
#define NUM_FISHING_ENCOUNTER_SLOTS 10

class LocationInfo {
    public:
        std::string name;
        uint16_t flag;
        uint32_t ram_address;
        uint32_t rom_address;
        uint16_t default_item;

        nlohmann::json to_json ();
};

class WarpInfo {
    public:
        bool is_one_way = true;
        std::string source_map;
        std::vector<int> source_indices;
        std::vector<std::tuple<int, int>> source_coordinates;
        std::string dest_map;
        std::vector<int> dest_indices;
        std::vector<std::tuple<int, int>> dest_coordinates;

        bool connects_to (const WarpInfo &other);
        std::string encode ();
        static WarpInfo decode (std::string s);
};

class LearnsetInfo {
    public:
        uint32_t rom_address;
        std::vector<std::tuple<uint8_t, uint16_t>> moves;

        nlohmann::json to_json ();
};

class SpeciesInfo {
    public:
        uint32_t rom_address;
        uint16_t id;
        uint8_t base_stats[6];
        uint8_t catch_rate;
        uint8_t abilities[2];
        uint8_t types[2];

        LearnsetInfo learnset_info;

        nlohmann::json to_json ();
};

enum TrainerPokemonInfoType {
    NO_ITEM_DEFAULT_MOVES,
    NO_ITEM_CUSTOM_MOVES,
    ITEM_DEFAULT_MOVES,
    ITEM_CUSTOM_MOVES
};

class TrainerPokemonInfo {
    public:
        uint16_t species;
        uint8_t moves[4];
};

class TrainerInfo {
    public:
        std::vector<TrainerPokemonInfo> party;
        TrainerPokemonInfoType pokemon_data_type;
        uint32_t rom_address;
        uint32_t party_rom_address;
        uint32_t battle_script_rom_address;

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

class StaticEncounterInfo {
    public:
        uint32_t rom_address;
        uint16_t species;
        uint8_t level;
        uint16_t flag;

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

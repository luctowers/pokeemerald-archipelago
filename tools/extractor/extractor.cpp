#include "extractor.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <regex>

#include <json.hpp>
using json = nlohmann::json;

int main (int argc, char *argv[])
{
    std::filesystem::path root_dir;

    if (argc == 1)
    {
        root_dir = std::filesystem::path(".");
    }
    else if (argc == 2)
    {
        root_dir = std::filesystem::path(argv[1]);
    }
    else
    {
        fprintf(stderr, "Too many arguments\nUSAGE: extractor [root directory]\n");
        exit(1);
    }

    // ------------------------------------------------------------------------
    // Getting macros
    // ------------------------------------------------------------------------
    std::ifstream macro_file(root_dir / "tools/extractor/macros.json");
    if (macro_file.fail())
    {
        fprintf(stderr, "Could not find macros.json\n");
        exit(1);
    }
    json macros_json = json::parse(macro_file);

    // ------------------------------------------------------------------------
    // Reading symbols
    // ------------------------------------------------------------------------
    std::ifstream symbol_map_file(root_dir / "pokeemerald-archipelago.sym");
    std::regex symbol_map_regex("^([0-9a-fA-F]+) [lg] [0-9a-fA-F]+ ([a-zA-Z0-9_]+)$");
    std::map<std::string, uint32_t> symbol_map;

    std::string line;
    while (std::getline(symbol_map_file, line))
    {
        std::smatch m;
        if (std::regex_match(line, m, symbol_map_regex))
        {
            symbol_map[m[2]] = std::stoi(m[1], nullptr, 16);
        }
    }

    std::vector<std::shared_ptr<ItemInfo>> npc_gifts;
    for (auto const& [symbol, address] : symbol_map)
    {
        if (symbol.substr(0, 28) == "Archipelago_Target_NPC_Gift_")
        {
            std::shared_ptr<ItemInfo> item(new ItemInfo());
            item->ram_address = address + 3;
            item->rom_address = item->ram_address - 0x8000000;
            item->flag_name = symbol.substr(28);
            npc_gifts.push_back(item);
        }
    }

    std::map<std::string, uint32_t> misc_ram_addresses = {
        { "gSaveblock1", symbol_map["gSaveblock1"] },
        { "gArchipelagoReceivedItem", symbol_map["gArchipelagoReceivedItem"] },
        { "gGymBadgeItems", symbol_map["gGymBadgeItems"] },
    };

    // ------------------------------------------------------------------------
    // Reading map.json files
    // ------------------------------------------------------------------------
    std::vector<std::shared_ptr<ItemInfo>> ball_items;
    std::vector<std::shared_ptr<ItemInfo>> hidden_items;

    for(const auto& entry: std::filesystem::directory_iterator(root_dir / "data/maps/"))
    {
        if (entry.is_directory())
        {
            std::ifstream map_file(entry.path() / "map.json");
            json map_data_json = json::parse(map_file);

            json object_events_json = map_data_json["object_events"];
            for (const auto& event_json: object_events_json) {
                if (event_json["flag"].get<std::string>().substr(0, 9) == "FLAG_ITEM")
                {
                    std::shared_ptr<ItemInfo> item(new ItemInfo());
                    item->flag_name = event_json["flag"];
                    item->ram_address = symbol_map[event_json["script"]] + 3;
                    item->rom_address = item->ram_address - 0x8000000;
                    ball_items.push_back(item);
                }
            }

            json bg_events_json = map_data_json["bg_events"];
            for (const auto& event_json: bg_events_json) {
                if (event_json["type"] == "hidden_item")
                {
                    std::shared_ptr<ItemInfo> item(new ItemInfo());
                    item->flag_name = event_json["flag"];
                    item->ram_address = symbol_map["Archipelago_Target_Hidden_Item" + item->flag_name] + 8;
                    item->rom_address = item->ram_address - 0x8000000;
                    item->default_item = macros_json["items"][event_json["item"].get<std::string>()];
                    hidden_items.push_back(item);
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // Reading encounter tables
    // ------------------------------------------------------------------------
    std::vector<std::shared_ptr<MapEncounterInfo>> encounter_tables;

    std::ifstream wild_encounters_file(root_dir / "src/data/wild_encounters.json");
    json wild_encounters_json = json::parse(wild_encounters_file);
    
    for (const auto& map_json: wild_encounters_json["wild_encounter_groups"][0]["encounters"]) {
        std::shared_ptr<MapEncounterInfo> map(new MapEncounterInfo());

        std::string base_symbol = map_json["base_label"];

        map->land_encounters.ram_address = symbol_map[base_symbol + "_LandMons"];
        map->land_encounters.rom_address = map->land_encounters.ram_address - 0x8000000;
        map->water_encounters.ram_address = symbol_map[base_symbol + "_WaterMons"];
        map->water_encounters.rom_address = map->water_encounters.ram_address - 0x8000000;
        map->fishing_encounters.ram_address = symbol_map[base_symbol + "_FishingMons"];
        map->fishing_encounters.rom_address = map->fishing_encounters.ram_address - 0x8000000;

        map->map_name = map_json["map"];

        uint i;

        i = 0;
        try
        {
            for (const auto& encounter_slot_json: map_json.at("land_mons")["mons"]) {
                map->land_encounters.encounter_slots[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                map->land_encounters.encounter_slots[i].min_level = encounter_slot_json["min_level"];
                map->land_encounters.encounter_slots[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            map->land_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        i = 0;
        try
        {
            for (const auto& encounter_slot_json: map_json.at("water_mons")["mons"]) {
                map->water_encounters.encounter_slots[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                map->water_encounters.encounter_slots[i].min_level = encounter_slot_json["min_level"];
                map->water_encounters.encounter_slots[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            map->water_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        i = 0;
        try
        {
            for (const auto& encounter_slot_json: map_json.at("fishing_mons")["mons"]) {
                map->fishing_encounters.encounter_slots[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                map->fishing_encounters.encounter_slots[i].min_level = encounter_slot_json["min_level"];
                map->fishing_encounters.encounter_slots[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            map->fishing_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        encounter_tables.push_back(map);
    }

    // ------------------------------------------------------------------------
    // Reading ROM
    // ------------------------------------------------------------------------
    std::ifstream rom(root_dir / "pokeemerald-archipelago.gba", std::ios::binary);
    if (rom.fail())
    {
        fprintf(stderr, "Could not open rom file\n");
        exit(1);
    }
    
    for (const auto& item: ball_items)
    {
        rom.seekg(item->rom_address, rom.beg);
        rom.read((char*)&(item->default_item), 2);
    }

    for (const auto& item: npc_gifts)
    {
        rom.seekg(item->rom_address, std::ios::beg);
        rom.read((char*)&(item->default_item), 2);
    }

    // ------------------------------------------------------------------------
    // Creating output
    // ------------------------------------------------------------------------
    json npc_gifts_json;
    for (const auto& item: npc_gifts)
    {
        npc_gifts_json[item->flag_name.substr(5)] = {
            { "flag", macros_json["flags"][item->flag_name] },
            { "ram_address", item->ram_address },
            { "rom_address", item->rom_address },
            { "default_item", item->default_item },
        };
    }

    json ball_items_json;
    for (const auto& item: ball_items)
    {
        ball_items_json[item->flag_name.substr(5)] = {
            { "flag", macros_json["flags"][item->flag_name] },
            { "ram_address", item->ram_address },
            { "rom_address", item->rom_address },
            { "default_item", item->default_item },
        };
    }

    json hidden_items_json;
    for (const auto& item: hidden_items)
    {
        hidden_items_json[item->flag_name.substr(5)] = {
            { "flag", macros_json["flags"][item->flag_name] },
            { "ram_address", item->ram_address },
            { "rom_address", item->rom_address },
            { "default_item", item->default_item },
        };
    }

    json encounter_tables_json;
    for (const auto& table: encounter_tables)
    {
        json map = {};
        
        if (table->land_encounters.exists) {
            map["land_encounters"] = {
                { "ram_address", table->land_encounters.ram_address },
                { "rom_address", table->land_encounters.rom_address },
                { "encounter_slots", json::array() },
            };
            for (uint j = 0; j < NUM_LAND_ENCOUNTER_SLOTS; ++j) {
                map["land_encounters"]["encounter_slots"].push_back(table->land_encounters.encounter_slots[j].default_species);
            }
        }
        
        if (table->water_encounters.exists) {
            map["water_encounters"] = {
                { "ram_address", table->water_encounters.ram_address },
                { "rom_address", table->water_encounters.rom_address },
                { "encounter_slots", json::array() },
            };
            for (uint j = 0; j < NUM_WATER_ENCOUNTER_SLOTS; ++j) {
                map["water_encounters"]["encounter_slots"].push_back(table->water_encounters.encounter_slots[j].default_species);
            }
        }
        
        if (table->fishing_encounters.exists) {
            map["fishing_encounters"] = {
                { "ram_address", table->fishing_encounters.ram_address },
                { "rom_address", table->fishing_encounters.rom_address },
                { "encounter_slots", json::array() },
            };
            for (uint j = 0; j < NUM_FISHING_ENCOUNTER_SLOTS; ++j) {
                map["fishing_encounters"]["encounter_slots"].push_back(table->fishing_encounters.encounter_slots[j].default_species);
            }
        }

        encounter_tables_json[table->map_name] = map;
    }

    json output_json = {
        { "misc_ram_addresses", misc_ram_addresses },
        { "npc_gifts", npc_gifts_json },
        { "ball_items", ball_items_json },
        { "hidden_items", hidden_items_json },
        { "encounter_tables", encounter_tables_json },
        { "constants", {
            { "items", macros_json["items"] },
            { "flags", macros_json["flags"] },
            { "species", macros_json["species"] },
        }},
    };

    std::ofstream outfile(root_dir / "data.json");
    outfile << std::setw(2) << output_json << std::endl;
}

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

    std::vector<std::shared_ptr<ItemInfo>> ball_items;
    std::vector<std::shared_ptr<ItemInfo>> hidden_items;

    std::ifstream macro_file(root_dir / "tools/extractor/macros.json");
    if (macro_file.fail())
    {
        fprintf(stderr, "Could not find macros.json\n");
        exit(1);
    }
    json macros_json = json::parse(macro_file);

    std::cout << "Looking up symbols..." << std::endl;

    std::ifstream symbol_map_file(root_dir / "pokeemerald.sym");
    std::map<std::string, uint32_t> symbol_map;
    std::regex symbol_map_regex("^([0-9a-fA-F]+) [lg] [0-9a-fA-F]+ ([a-zA-Z0-9_]+)$");
    std::string line;
    while (std::getline(symbol_map_file, line))
    {
        std::smatch m;
        if (std::regex_match(line, m, symbol_map_regex))
        {
            symbol_map[m[2]] = std::stoi(m[1], nullptr, 16);
        }
    }

    std::cout << "Reading map files..." << std::endl;

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
                    item->ram_address = symbol_map["Archipelago_Target_" + item->flag_name] + 8;
                    item->rom_address = item->ram_address - 0x8000000;
                    item->default_item = macros_json["items"][event_json["item"].get<std::string>()];
                    hidden_items.push_back(item);
                }
            }
        }
    }

    std::cout << "Reading wild encounters..." << std::endl;

    std::vector<std::shared_ptr<EncounterTableInfo>> encounter_tables;

    std::ifstream wild_encounters_file(root_dir / "src/data/wild_encounters.json");
    json wild_encounters_json = json::parse(wild_encounters_file);
    
    for (const auto& table_json: wild_encounters_json["wild_encounter_groups"][0]["encounters"]) {
        std::shared_ptr<EncounterTableInfo> table(new EncounterTableInfo());
        std::string base_symbol = table_json["base_label"];

        table->land_pokemon_ram_address = symbol_map[base_symbol + "_LandMons"];
        table->water_pokemon_ram_address = symbol_map[base_symbol + "_WaterMons"];
        table->fishing_pokemon_ram_address = symbol_map[base_symbol + "_FishingMons"];

        table->map_name = table_json["map"];

        uint i;

        i = 0;
        try
        {
            for (const auto& encounter_slot_json: table_json.at("land_mons")["mons"]) {
                table->land_encounters[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                table->land_encounters[i].min_level = encounter_slot_json["min_level"];
                table->land_encounters[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            table->land_encounters_exist = true;
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
            table_json.at("water_mons");
            for (const auto& encounter_slot_json: table_json.at("water_mons")["mons"]) {
                table->water_encounters[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                table->water_encounters[i].min_level = encounter_slot_json["min_level"];
                table->water_encounters[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            table->water_encounters_exist = true;
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
            for (const auto& encounter_slot_json: table_json.at("fishing_mons")["mons"]) {
                table->fishing_encounters[i].default_species = macros_json["species"][encounter_slot_json["species"].get<std::string>()];
                table->fishing_encounters[i].min_level = encounter_slot_json["min_level"];
                table->fishing_encounters[i].max_level = encounter_slot_json["max_level"];
                ++i;
            }
            table->fishing_encounters_exist = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        encounter_tables.push_back(table);
    }

    std::cout << "Reading ROM..." << std::endl;

    std::ifstream rom(root_dir / "pokeemerald.gba", std::ios::binary);
    
    for (const auto& item: ball_items)
    {
        rom.seekg(item->rom_address, std::ios::beg);
        rom >> item->default_item;
    }

    std::cout << "Creating JSON..." << std::endl;

    json ball_items_json;
    for (const auto& item: ball_items)
    {
        ball_items_json.push_back({
            { "flag", item->flag_name },
            { "ram_address", item->ram_address },
            { "rom_address", item->rom_address },
            { "default_item", item->default_item },
        });
    }

    json hidden_items_json;
    for (const auto& item: hidden_items)
    {
        hidden_items_json.push_back({
            { "flag", item->flag_name },
            { "ram_address", item->ram_address },
            { "rom_address", item->rom_address },
            { "default_item", item->default_item },
        });
    }

    json encounter_tables_json;
    uint i = 0;
    for (const auto& table: encounter_tables)
    {
        encounter_tables_json.push_back({
            { "map_name", table->map_name },
        });
        
        if (table->land_encounters_exist) {
            encounter_tables_json[i]["land_pokemon_ram_address"] = table->land_pokemon_ram_address;
            encounter_tables_json[i]["land_encounters"] = json::array();
            for (uint j = 0; j < 12; ++j) {
                encounter_tables_json[i]["land_encounters"].push_back(table->land_encounters[j].default_species);
            }
        }

        if (table->water_encounters_exist) {
            encounter_tables_json[i]["water_pokemon_ram_address"] = table->water_pokemon_ram_address;
            encounter_tables_json[i]["water_encounters"] = json::array();
            for (uint j = 0; j < 5; ++j) {
                encounter_tables_json[i]["water_encounters"].push_back(table->water_encounters[j].default_species);
            }
        }

        if (table->fishing_encounters_exist) {
            encounter_tables_json[i]["fishing_pokemon_ram_address"] = table->fishing_pokemon_ram_address;
            encounter_tables_json[i]["fishing_encounters"] = json::array();
            for (uint j = 0; j < 10; ++j) {
                encounter_tables_json[i]["fishing_encounters"].push_back(table->fishing_encounters[j].default_species);
            }
        }

        ++i;
    }

    json output_json = {
        { "ball_items", ball_items_json },
        { "hidden_items", hidden_items_json },
        { "encounter_tables", encounter_tables_json },
        { "items", macros_json["items"] },
        { "flags", macros_json["flags"] },
    };

    std::ofstream outfile(root_dir / "data.json");
    outfile << std::setw(2) << output_json << std::endl;
}

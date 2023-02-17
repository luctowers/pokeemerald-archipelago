#include "extractor.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <regex>

#include <json.hpp>
using json = nlohmann::json;

std::regex create_regex_for_symbol (std::string symbol) {
    return std::regex("^[ ]*(0x[0-9a-fA-F]+)[ ]+" + symbol + ".*$");
}

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

    std::vector<std::shared_ptr<BallItemInfo>> ball_items;
    std::vector<std::shared_ptr<HiddenItemInfo>> hidden_items;

    std::ifstream macro_file(root_dir / "tools/extractor/macros.json");
    if (macro_file.fail())
    {
        fprintf(stderr, "Could not find macros.json\n");
        exit(1);
    }
    json macros_json = json::parse(macro_file);

    std::cout << "Reading map files..." << std::endl;

    for(const auto& entry: std::filesystem::directory_iterator(root_dir / "data/maps/"))
    {
        if (entry.is_directory())
        {
            std::ifstream map_file(entry.path() / "map.json");
            json map_data_json = json::parse(map_file);

            json object_events_json = map_data_json["object_events"];
            for (const auto& event: object_events_json) {
                if (event["flag"].get<std::string>().substr(0, 9) == "FLAG_ITEM")
                {
                    std::shared_ptr<BallItemInfo> item(new BallItemInfo());
                    item->script_name = event["script"].get<std::string>();
                    item->script_symbolmap_regex = create_regex_for_symbol(item->script_name);
                    item->flag_name = event["flag"].get<std::string>();
                    ball_items.push_back(item);
                }
            }

            json bg_events_json = map_data_json["bg_events"];
            for (const auto& event: bg_events_json) {
                if (event["type"].get<std::string>() == "hidden_item")
                {
                    std::shared_ptr<HiddenItemInfo> item(new HiddenItemInfo());
                    item->flag_name = event["flag"].get<std::string>();
                    item->item_symbolmap_regex = create_regex_for_symbol("Archipelago_Target_" + item->flag_name);
                    item->default_item = macros_json["items"][event["item"].get<std::string>()];
                    hidden_items.push_back(item);
                }
            }
        }
    }

    std::cout << "Looking up symbols..." << std::endl;

    std::ifstream symbol_map(root_dir / "pokeemerald.map");
    std::string line;
    while (std::getline(symbol_map, line))
    {
        for (const auto& item: ball_items)
        {
            std::smatch m;
            if (std::regex_match(line, m, item->script_symbolmap_regex))
            {
                item->ram_address = std::stoi(m[1], nullptr, 16) + 3;
                item->rom_address = item->ram_address - 0x8000000;
            }
        }
        
        for (const auto& item: hidden_items)
        {
            std::smatch m;
            if (std::regex_match(line, m, item->item_symbolmap_regex))
            {
                item->ram_address = std::stoi(m[1], nullptr, 16) + 8;
                item->rom_address = item->ram_address - (0x8000000 - 0x30); // Not sure why there's an extra 0x30 offset
            }
        }
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

    json output_json = {
        { "ball_items", ball_items_json },
        { "hidden_items", hidden_items_json },
        { "items", macros_json["items"] },
        { "flags", macros_json["flags"] }
    };

    std::ofstream outfile(root_dir / "data.json");
    outfile << std::setw(2) << output_json << std::endl;
}

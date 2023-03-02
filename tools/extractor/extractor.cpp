#include "extractor.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <set>
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
    // Getting constants
    // ------------------------------------------------------------------------
    std::cout << "Loading constants..." << std::endl;
    std::ifstream macro_file(root_dir / "constants.json");
    if (macro_file.fail())
    {
        fprintf(stderr, "Could not find constants.json\n");
        exit(1);
    }
    json constants_json = json::parse(macro_file);

    // ------------------------------------------------------------------------
    // Reading symbols
    // ------------------------------------------------------------------------
    std::cout << "Reading symbols..." << std::endl;
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

    // NPC Gifts
    std::vector<std::shared_ptr<LocationInfo>> npc_gifts;
    for (auto const& [symbol, address] : symbol_map)
    {
        if (symbol.substr(0, 28) == "Archipelago_Target_NPC_Gift_")
        {
            std::shared_ptr<LocationInfo> item(new LocationInfo());
            item->name = "NPC_GIFT_" + symbol.substr(33);
            item->type = NPC_GIFT;
            item->flag = constants_json[symbol.substr(28)];
            item->ram_address = address + 3;
            item->rom_address = item->ram_address - 0x8000000;
            npc_gifts.push_back(item);
        }
    }

    // Badges
    std::vector<std::shared_ptr<LocationInfo>> badges;
    for (auto const& [symbol, address] : symbol_map)
    {
        if (symbol.substr(0, 25) == "Archipelago_Target_Badge_")
        {
            std::shared_ptr<LocationInfo> item(new LocationInfo());
            item->name = "BADGE_" + symbol.substr(25);
            item->type = BADGE;
            item->flag = constants_json["FLAG_BADGE0" + symbol.substr(25) + "_GET"];
            item->ram_address = address + 3;
            item->rom_address = item->ram_address - 0x8000000;
            badges.push_back(item);
        }
    }

    std::map<std::string, uint32_t> misc_ram_addresses = {
        { "gSaveblock1", symbol_map["gSaveblock1"] },
        { "gArchipelagoReceivedItem", symbol_map["gArchipelagoReceivedItem"] },
    };

    std::map<std::string, uint32_t> misc_rom_addresses = {
        { "sExpMultiplier", symbol_map["sExpMultiplier"] - 0x8000000 },
    };

    // ------------------------------------------------------------------------
    // Reading map.json files
    // ------------------------------------------------------------------------
    std::cout << "Reading maps..." << std::endl;
    std::vector<std::shared_ptr<LocationInfo>> ball_items;
    std::vector<std::shared_ptr<LocationInfo>> hidden_items;
    std::map<std::string, std::shared_ptr<MapInfo>> maps;
    std::vector<std::shared_ptr<WarpInfo>> warps;

    for(const auto& entry: std::filesystem::directory_iterator(root_dir / "data/maps/"))
    {
        if (entry.is_directory())
        {
            std::ifstream map_file(entry.path() / "map.json");
            json map_data_json = json::parse(map_file);

            std::shared_ptr<MapInfo> map(new MapInfo());
            map->name = map_data_json["id"];
            maps[map->name] = map;

            // ----------------------------------------------------------------
            // Warps
            // ----------------------------------------------------------------

            // Many warps are actually two or three events acting as one logical warp.
            // Doorways, for example, are often 2 tiles wide indoors but
            // only 1 tile wide outdoors. Both indoor warps point to the
            // outdoor warp, and the outdoor warp points to only one of the
            // indoor warps. There are also warps that are 2 tiles wide and lead to
            // a corresponding pair of warps. We want to describe warps logically in a way that
            // retains information about individual warp events.
            //
            // This is how warps are encoded:
            //
            // {source_map}:{source_warp_ids}/{dest_map}:{dest_warp_id}[!]
            //    source_map:       The map the warp events are located in
            //    source_warp_ids:  The ids of all adjacent warp events in source_map
            //                      (these must be in ascending order)
            //    dest_map:         The map of the warp event to which this one is connected
            //    dest_warp_ids:     The ids of the warp events in dest_map
            //    [!]:              If the warp expects to lead to a destination which does
            //                      not lead back to it, add a ! to the end
            //
            // Example:   MAP_LAVARIDGE_TOWN_HOUSE:0,1/MAP_LAVARIDGE_TOWN:4
            // Example 2: MAP_AQUA_HIDEOUT_B1F:14/MAP_AQUA_HIDEOUT_B1F:12!
            //
            // Note: A warp must have its destination set as another warp event.
            // However, that does not guarantee that the destination warp event
            // will warp back to the source. There are (few) one-way warps.
            //
            // Note2: Some warp destinations go to the map "MAP_DYNAMIC" and
            // have a warp id which is not a number. These edge cases are:
            //   - The Moving Truck
            //   - Terra Cave
            //   - Marine Cave
            //   - The Department Store Elevator
            //   - Secret Bases
            //   - The Trade Center
            //   - The Union Room
            //   - The Record Corner
            //   - 2P/4P Battle Colosseum

            json warp_events_json = map_data_json["warp_events"];
            // (id, x, y, destination_map, destination_id)
            std::vector<std::shared_ptr<WarpInfo>> map_warps;
            uint i = 0;
            for (const auto& warp_json: warp_events_json)
            {
                std::shared_ptr<WarpInfo> warp(new WarpInfo);
                warp->source_map = map->name;
                warp->source_indices.push_back(i);
                warp->source_coordinates.push_back(std::tuple<int, int>(warp_json["x"], warp_json["y"]));
                warp->dest_map = warp_json["dest_map"];
                if (warp_json["dest_warp_id"] == "WARP_ID_DYNAMIC")
                {
                    warp->dest_indices.push_back(-1);
                }
                else if (warp_json["dest_warp_id"] == "WARP_ID_SECRET_BASE")
                {
                    warp->dest_indices.push_back(-2);
                }
                else
                {
                    warp->dest_indices.push_back(std::stoi(static_cast<std::string>(warp_json["dest_warp_id"])));
                }

                map_warps.push_back(warp);
                ++i;
            }

            // Sort so that adjacency checker only needs to check against the
            // previously found matching warp. Otherwise would have to do a
            // recursive flood of some sort.
            std::sort(
                map_warps.begin(), map_warps.end(),
                [](std::shared_ptr<WarpInfo> a, std::shared_ptr<WarpInfo> b)
                {
                    return std::get<0>(a->source_coordinates[0]) == std::get<0>(b->source_coordinates[0])
                        ? std::get<1>(a->source_coordinates[0]) < std::get<1>(b->source_coordinates[0])
                        : std::get<0>(a->source_coordinates[0]) < std::get<0>(b->source_coordinates[0]);
                }
            );

            // Group warps by whether they're logically the same
            std::vector<std::shared_ptr<WarpInfo>> grouped_warps;
            std::vector<bool> is_collected(map_warps.size());
            for (uint i = 0; i < map_warps.size(); ++i)
            {
                if (is_collected[i]) continue;

                const auto warp = map_warps[i];
                is_collected[i] = true;

                for (uint j = i + 1; j < map_warps.size(); ++j)
                {
                    const auto other_warp = map_warps[j];

                    // Check destination map to exit early, but we're assuming that adjacent
                    // warps are always part of the same logical warp
                    if (warp->dest_map != other_warp->dest_map) continue;
                    // Check adjacency
                    if (
                        abs(std::get<0>(warp->source_coordinates.back()) - std::get<0>(other_warp->source_coordinates[0])) +
                        abs(std::get<1>(warp->source_coordinates.back()) - std::get<1>(other_warp->source_coordinates[0])) > 1
                    ) continue;

                    if (
                        map->name == "MAP_ROUTE110_TRICK_HOUSE_PUZZLE7" &&
                        (warp->source_indices[0] == 9 || warp->source_indices[0] == 11)
                    ) continue; // These are the only two warps in the game which are adjacent but go to completely different places

                    warp->source_indices.push_back(other_warp->source_indices[0]);
                    warp->dest_indices.push_back(other_warp->dest_indices[0]);
                    warp->source_coordinates.push_back(other_warp->source_coordinates[0]);
                    is_collected[j] = true;
                }
                grouped_warps.push_back(warp);
            }

            for (const auto warp: grouped_warps)
            {
                warps.push_back(warp);
            }

            // ----------------------------------------------------------------
            // Items
            // ----------------------------------------------------------------
            json object_events_json = map_data_json["object_events"];
            for (const auto& event_json: object_events_json)
            {
                std::string flag_name = event_json["flag"].get<std::string>();
                if (flag_name.substr(0, 9) == "FLAG_ITEM")
                {
                    std::shared_ptr<LocationInfo> item(new LocationInfo());
                    item->flag = constants_json[flag_name];
                    item->name = flag_name.substr(5);
                    item->type = GROUND_ITEM;
                    item->ram_address = symbol_map[event_json["script"]] + 3;
                    item->rom_address = item->ram_address - 0x8000000;
                    ball_items.push_back(item);
                }
            }

            json bg_events_json = map_data_json["bg_events"];
            for (const auto& event_json: bg_events_json)
            {
                if (event_json["type"] == "hidden_item")
                {
                    std::string flag_name = event_json["flag"].get<std::string>();
                    std::shared_ptr<LocationInfo> item(new LocationInfo());
                    item->flag = constants_json[flag_name];
                    item->name = flag_name.substr(5);
                    item->type = HIDDEN_ITEM;
                    item->ram_address = symbol_map["Archipelago_Target_Hidden_Item_" + flag_name] + 8;
                    item->rom_address = item->ram_address - 0x8000000;
                    item->default_item = constants_json[event_json["item"].get<std::string>()];
                    hidden_items.push_back(item);
                }
            }
        }
    }

    // Now that all warps are created we can check 1-way
    for (const auto &warp: warps)
    {
        for (const auto &other_warp: warps)
        {
            if (warp == other_warp) continue;
            if (warp->connects_to(*other_warp))
            {
                // Found our destination
                if (other_warp->connects_to(*warp))
                {
                    warp->is_one_way = false;
                }

                break;
            }
        }
    }

    // ------------------------------------------------------------------------
    // Reading encounter tables
    // ------------------------------------------------------------------------
    std::cout << "Reading encounter tables..." << std::endl;
    std::ifstream wild_encounters_file(root_dir / "src/data/wild_encounters.json");
    json wild_encounters_json = json::parse(wild_encounters_file);
    
    for (const auto& map_json: wild_encounters_json["wild_encounter_groups"][0]["encounters"]) {
        std::shared_ptr<MapInfo> map = maps[map_json["map"]];

        // Altering Cave is the only map with multiple encounter tables.
        // It is supposed to switch between them based on a value set by an unreleased event.
        // The only vanilla table is the first one, with all Zubats.
        if (map->name == "MAP_ALTERING_CAVE" && map_json["base_label"] != "gAlteringCave1") continue;

        std::string base_symbol = map_json["base_label"];

        map->land_encounters.ram_address = symbol_map[base_symbol + "_LandMons"];
        map->land_encounters.rom_address = map->land_encounters.ram_address - 0x8000000;
        map->water_encounters.ram_address = symbol_map[base_symbol + "_WaterMons"];
        map->water_encounters.rom_address = map->water_encounters.ram_address - 0x8000000;
        map->fishing_encounters.ram_address = symbol_map[base_symbol + "_FishingMons"];
        map->fishing_encounters.rom_address = map->fishing_encounters.ram_address - 0x8000000;

        try
        {
            for (const auto& encounter_slot_json: map_json.at("land_mons")["mons"]) {
                auto slot = std::shared_ptr<EncounterSlotInfo>(new EncounterSlotInfo());
                map->land_encounters.encounter_slots.push_back(slot);
                slot->default_species = constants_json[encounter_slot_json["species"].get<std::string>()];
                slot->min_level = encounter_slot_json["min_level"];
                slot->max_level = encounter_slot_json["max_level"];
            }
            map->land_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        try
        {
            for (const auto& encounter_slot_json: map_json.at("water_mons")["mons"]) {
                auto slot = std::shared_ptr<EncounterSlotInfo>(new EncounterSlotInfo());
                map->water_encounters.encounter_slots.push_back(slot);
                slot->default_species = constants_json[encounter_slot_json["species"].get<std::string>()];
                slot->min_level = encounter_slot_json["min_level"];
                slot->max_level = encounter_slot_json["max_level"];
            }
            map->water_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }

        try
        {
            for (const auto& encounter_slot_json: map_json.at("fishing_mons")["mons"]) {
                auto slot = std::shared_ptr<EncounterSlotInfo>(new EncounterSlotInfo());
                map->fishing_encounters.encounter_slots.push_back(slot);
                slot->default_species = constants_json[encounter_slot_json["species"].get<std::string>()];
                slot->min_level = encounter_slot_json["min_level"];
                slot->max_level = encounter_slot_json["max_level"];
            }
            map->fishing_encounters.exists = true;
        }
        catch (const json::exception &e)
        {
            if (e.id != 403) {
                throw e;
            }
        }
    }

    // ------------------------------------------------------------------------
    // Reading ROM
    // ------------------------------------------------------------------------
    std::cout << "Reading ROM..." << std::endl;
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

    for (const auto& item: badges)
    {
        rom.seekg(item->rom_address, std::ios::beg);
        rom.read((char*)&(item->default_item), 2);
    }

    // ------------------------------------------------------------------------
    // Creating output
    // ------------------------------------------------------------------------
    std::cout << "Creating JSON..." << std::endl;
    json maps_json;
    for (const auto& map_tuple: maps)
    {
        maps_json[map_tuple.first] = map_tuple.second->to_json();
    }

    json locations_json;
    for (const auto& location: npc_gifts)
    {
        locations_json[location->name] = location->to_json();
    }
    for (const auto& location: ball_items)
    {
        locations_json[location->name] = location->to_json();
    }
    for (const auto& location: hidden_items)
    {
        locations_json[location->name] = location->to_json();
    }
    for (const auto& location: badges)
    {
        locations_json[location->name] = location->to_json();
    }

    std::vector<std::string> encoded_warps;
    for (const auto& warp: warps)
    {
        encoded_warps.push_back(warp->encode());
    }
    std::sort(encoded_warps.begin(), encoded_warps.end());

    json output_json = {
        { "_comment", "DO NOT MODIFY. This file was auto-generated. Your changes will likely be overwritten." },
        { "maps", maps_json },
        { "misc_ram_addresses", misc_ram_addresses },
        { "misc_rom_addresses", misc_rom_addresses },
        { "locations", locations_json },
        { "warps", encoded_warps },
        { "constants", constants_json },
    };

    std::cout << "Writing file..." << std::endl;
    std::ofstream outfile(root_dir / "extracted_data.json");
    outfile << std::setw(2) << output_json << std::endl;
}

std::string location_type_to_string (LocationType lt)
{
    switch (lt)
    {
        case GROUND_ITEM: return "GROUND_ITEM";
        case HIDDEN_ITEM: return "HIDDEN_ITEM";
        case NPC_GIFT: return "NPC_GIFT";
        case BADGE: return "BADGE";
        default: return "UNKNOWN";
    }
}

json LocationInfo::to_json ()
{
    return {
        { "type", location_type_to_string(this->type) },
        { "flag", this->flag },
        { "ram_address", this->ram_address },
        { "rom_address", this->rom_address },
        { "default_item", this->default_item },
    };
}

bool WarpInfo::connects_to (const WarpInfo &other)
{
    if (this->dest_map != other.source_map) return false;

    bool contains_all_indices = true;
    for (uint dest_i: this->dest_indices)
    {
        bool found_index = false;
        for (uint other_source_i: other.source_indices)
        {
            if (dest_i == other_source_i)
            {
                found_index = true;
                break;
            }
        }

        if (!found_index) {
            contains_all_indices = false;
            break;
        }
    }

    return contains_all_indices;
}

std::string WarpInfo::encode ()
{
    std::string result = "";

    result += this->source_map + ":";
    std::set<int> sorted_source_indices(this->source_indices.begin(), this->source_indices.end());
    for (int i: sorted_source_indices)
    {
        result += std::to_string(i) + ",";
    }
    result.pop_back();

    result += "/";

    result += this->dest_map + ":";
    std::set<int> sorted_dest_indices(this->dest_indices.begin(), this->dest_indices.end());
    for (int i: sorted_dest_indices)
    {
        result += std::to_string(i) + ",";
    }
    result.pop_back();

    if (this->is_one_way)
    {
        result += '!';
    }

    return result;
}

WarpInfo WarpInfo::decode (std::string s)
{
    bool is_one_way = false;
    if (s.back() == '!')
    {
        is_one_way = true;
        s.pop_back();
    }

    size_t slash_i = s.find('/');
    std::string source = s.substr(0, slash_i);
    std::string dest = s.substr(slash_i + 1);

    size_t source_colon_i = source.find(':');
    std::string source_map = source.substr(0, source_colon_i);
    std::string source_indices_string = source.substr(source_colon_i + 1);
    size_t dest_colon_i = dest.find(':');
    std::string dest_map = dest.substr(0, dest_colon_i);
    std::string dest_indices_string = dest.substr(dest_colon_i + 1);

    size_t i = 0;
    size_t prev_i = 0;
    std::vector<int> source_indices;
    while ((i = source_indices_string.find(',', i)) < source_indices_string.size())
    {
        source_indices.push_back(std::stoi(source_indices_string.substr(prev_i, i - prev_i)));
        prev_i = i + 1;
    }

    i = 0;
    prev_i = 0;
    std::vector<int> dest_indices;
    while ((i = dest_indices_string.find(',', i)) < dest_indices_string.size())
    {
        dest_indices.push_back(std::stoi(dest_indices_string.substr(prev_i, i - prev_i)));
        prev_i = i + 1;
    }

    WarpInfo warp_info;
    warp_info.is_one_way = is_one_way;
    warp_info.source_map = source_map;
    warp_info.source_indices = source_indices;
    warp_info.dest_map = dest_map;
    warp_info.dest_indices = dest_indices;

    return warp_info;
}

json MapInfo::to_json ()
{
    return {
        { "land_encounters", this->land_encounters.to_json() },
        { "water_encounters", this->water_encounters.to_json() },
        { "fishing_encounters", this->fishing_encounters.to_json() },
    };
}

json EncounterTableInfo::to_json ()
{
    if (!this->exists) return nullptr;
    json slots_json = json::array();
    for (const auto &encounter_slot: this->encounter_slots)
    {
        slots_json.push_back(encounter_slot->default_species);
    }

    return {
        { "encounter_slots", slots_json },
        { "ram_address", this->ram_address },
        { "rom_address", this->rom_address },
    };
}

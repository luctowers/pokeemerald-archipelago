#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <regex>

struct BallItemInfo {
    public:
        std::string script_name;
        std::regex script_symbolmap_regex;
        uint32_t ram_address;
        uint32_t rom_address;
        std::string flag_name;
        uint8_t default_item;
};

struct HiddenItemInfo {
    public:
        std::regex item_symbolmap_regex;
        uint32_t ram_address;
        uint32_t rom_address;
        std::string flag_name;
        uint8_t default_item;
};

#endif

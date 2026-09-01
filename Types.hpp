#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

enum class CellType : int { 
    LIBERA = 0, 
    OSTACOLO = 1 
};

enum class Action : int { 
    NORD = 0, 
    SUD = 1, 
    EST = 2, 
    OVEST = 3, 
    NONE = -1 
};

struct Position {
    int r{0};
    int c{0};

    constexpr bool operator==(const Position& other) const noexcept {
        return r == other.r && c == other.c;
    }

    constexpr bool operator!=(const Position& other) const noexcept {
        return !(*this == other);
    }
};

constexpr std::string_view actionToString(Action a) noexcept {
    switch (a) {
        case Action::NORD:  return "^";
        case Action::SUD:   return "v";
        case Action::EST:   return ">";
        case Action::OVEST: return "<";
        default:            return ".";
    }
}

using Grid = std::vector<std::vector<int>>;
using ValueMatrix = std::vector<std::vector<double>>;
using PolicyMatrix = std::vector<std::vector<Action>>;
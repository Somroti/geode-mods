#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>

using namespace geode::prelude;

// Structure pour stocker les données de rating
struct RatingData {
    int stars;
    int difficulty;
    int status;
    int demon_type; // 0=non-demon, 1=Easy, 2=Medium, 3=Hard, 4=Insane, 5=Extreme
    int dib;
};

// Déclarations des fonctions
void loadRatingsFromFile();
void parseRatingsJson(const std::string& jsonContent);
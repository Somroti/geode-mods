#include "NewRatedLevels.hpp"
#include <regex>
#include <fstream>
#include <sstream>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Cache pour les ratings
static std::unordered_map<int, RatingData> ratingsCache;
static bool cacheLoaded = false;

// Déclarations
void createResourcesStructure();
void createDefaultRateJson(const std::filesystem::path& filePath);
void loadRatingsFromFile();
void parseRatingsJson(const std::string& jsonContent);
void createFakeRateSavedJson();
GJDifficulty getDifficultyFromValue(int difficulty);
int getDemonTypeFromDifficulty(int difficulty);
static void applyRatingToLevel(GJGameLevel* level, const RatingData& rating);

$on_mod(Loaded) {
    createResourcesStructure();
    loadRatingsFromFile();
    createFakeRateSavedJson();
}

void createResourcesStructure() {
    try {
        auto resourcesDir = Mod::get()->getResourcesDir();
        auto modSubDir = resourcesDir / "somroteam_dev.new_rated_levels";
        auto ratingsFile = modSubDir / "rate.json";

        if (!std::filesystem::exists(resourcesDir)) {
            std::filesystem::create_directories(resourcesDir);
        }

        if (!std::filesystem::exists(modSubDir)) {
            std::filesystem::create_directories(modSubDir);
        }

        if (!std::filesystem::exists(ratingsFile)) {
            createDefaultRateJson(ratingsFile);
        }
    } catch (const std::exception& e) {
        // Erreur silencieuse
    }
}

void createDefaultRateJson(const std::filesystem::path& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) return;

        file << R"({
    "levels": {
        "117448912": {
            "stars": 10,
            "difficulty": 7,
            "status": 0,
            "dib": 3
        },
        "117251972": {
            "stars": 7,
            "difficulty": 4,
            "status": 1,
            "dib": 0
        },
        "119154457": {
            "stars": 10,
            "difficulty": 8,
            "status": 1,
            "dib": 6
        },
        "119183902": {
            "stars": 5,
            "difficulty": 3,
            "status": 1,
            "dib": 0
        },
        "114552735": {
            "stars": 4,
            "difficulty": 3,
            "status": 1,
            "dib": 0
        },
        "113620475": {
            "stars": 5,
            "difficulty": 3,
            "status": 0,
            "dib": 0
        },
        "112856885": {
            "stars": 1,
            "difficulty": 1,
            "status": 0,
            "dib": 0
        },
        "120897907": {
            "stars": 2,
            "difficulty": 1,
            "status": 0,
            "dib": 0
        }
    }
})";
        file.close();
    } catch (const std::exception& e) {
        // Erreur silencieuse
    }
}

void loadRatingsFromFile() {
    auto resourcesDir = Mod::get()->getResourcesDir();
    auto modSubDir = resourcesDir / "somroteam_dev.new_rated_levels";
    auto ratingsFile = modSubDir / "rate.json";

    if (!std::filesystem::exists(ratingsFile)) return;

    try {
        std::ifstream file(ratingsFile);
        if (!file.is_open()) return;

        std::string content;
        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        content = contentStream.str();
        file.close();

        if (!content.empty()) {
            parseRatingsJson(content);
            cacheLoaded = true;
        }
    } catch (const std::exception& e) {
        // Erreur silencieuse
    }
}

void parseRatingsJson(const std::string& jsonContent) {
    ratingsCache.clear();

    std::regex levelRegex("\"(\\d+)\"\\s*:\\s*\\{[^}]*\"stars\"\\s*:\\s*(\\d+)[^}]*\"difficulty\"\\s*:\\s*(\\d+)[^}]*\"status\"\\s*:\\s*(\\d+)[^}]*\"dib\"\\s*:\\s*(\\d+)[^}]*\\}");

    std::sregex_iterator iter(jsonContent.begin(), jsonContent.end(), levelRegex);
    std::sregex_iterator end;

    while (iter != end) {
        std::smatch match = *iter;

        try {
            int levelID = std::stoi(match[1].str());
            int stars = std::stoi(match[2].str());
            int difficulty = std::stoi(match[3].str());
            int status = std::stoi(match[4].str());
            int dib = std::stoi(match[5].str());

            if (stars >= 0 && stars <= 10 && difficulty >= 0 && difficulty <= 10 && status >= 0 && status <= 4 && dib >= 0) {
                ratingsCache[levelID] = {stars, difficulty, status, dib};
            }
        } catch (const std::exception& e) {
            // Ignorer les erreurs de parsing
        }

        ++iter;
    }
}

void createFakeRateSavedJson() {
    if (!cacheLoaded || ratingsCache.empty()) return;

    // Utiliser le répertoire de sauvegarde de l'utilisateur pour les mods
    auto geodeDir = geode::dirs::getSaveDir();
    auto fakeRateDir = geodeDir / "geode" / "mods" / "hiimjustin000.fake_rate";
    auto savedJsonFile = fakeRateDir / "saved.json";

    try {
        // Créer le dossier Fake Rate s'il n'existe pas
        if (!std::filesystem::exists(fakeRateDir)) {
            std::filesystem::create_directories(fakeRateDir);
        }

        std::ofstream file(savedJsonFile);
        if (!file.is_open()) return;

        file << "{\n    \"fake-rate\": [";

        bool first = true;
        for (const auto& [levelID, rating] : ratingsCache) {
            if (!first) {
                file << ",";
            }
            first = false;

            // Calculer more-difficulties-override en fonction des étoiles
            int moreDifficultiesOverride = 0;
            if (rating.stars == 4 || rating.stars == 7 || rating.stars == 9) {
                moreDifficultiesOverride = rating.stars;
            }

            file << "\n        {\n";
            file << "            \"id\": " << levelID << ",\n";
            file << "            \"stars\": " << rating.stars << ",\n";
            file << "            \"feature\": " << rating.status << ",\n";
            file << "            \"difficulty\": " << rating.difficulty << ",\n";
            file << "            \"more-difficulties-override\": " << moreDifficultiesOverride << ",\n";
            file << "            \"grandpa-demon-override\": 0,\n";
            file << "            \"demons-in-between-override\": " << rating.dib << ",\n";
            file << "            \"gddp-integration-override\": 0,\n";
            file << "            \"coins\": " << (rating.stars > 0 ? "true" : "false") << "\n";
            file << "        }";
        }

        file << "\n    ]\n}";
        file.close();
    } catch (const std::exception& e) {
        // Erreur silencieuse
    }
}

GJDifficulty getDifficultyFromValue(int difficulty) {
    switch (difficulty) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10: return GJDifficulty::Demon;
        default: return GJDifficulty::Normal;
    }
}

int getDemonTypeFromDifficulty(int difficulty) {
    switch (difficulty) {
        case 6: return 0; // Hard Demon
        case 7: return 3; // Easy Demon
        case 8: return 4; // Medium Demon
        case 9: return 5; // Insane Demon
        case 10: return 6; // Extreme Demon
        default: return 0;
    }
}

static void applyRatingToLevel(GJGameLevel* level, const RatingData& rating) {
    if (!level) return;

    // Configuration des étoiles seulement - laisser Fake Rate gérer les icônes
    level->m_stars = rating.stars;
    level->m_starsRequested = rating.stars;
    level->m_ratingsSum = rating.stars * 10;

    // User coins
    if (level->m_coins > 0) {
        level->m_coinsVerified = level->m_coins;
    }
}

// Hooks
class $modify(NewRatedLevelsLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (cacheLoaded && level) {
            auto it = ratingsCache.find(level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(level, it->second);
            }
        }

        bool result = LevelInfoLayer::init(level, challenge);

        if (result && cacheLoaded && level) {
            auto it = ratingsCache.find(level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(level, it->second);
            }
        }

        return result;
    }

    void updateLabelValues() {
        if (this->m_level && cacheLoaded) {
            auto it = ratingsCache.find(this->m_level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(this->m_level, it->second);
            }
        }

        LevelInfoLayer::updateLabelValues();
    }
};

class $modify(NewRatedLevelsLevelBrowserLayer, LevelBrowserLayer) {
    void loadPage(GJSearchObject* searchObj) {
        LevelBrowserLayer::loadPage(searchObj);

        if (cacheLoaded) {
            Loader::get()->queueInMainThread([this]() {
                this->applyRatingsToLoadedLevels();
            });
        }
    }

    void setupLevelBrowser(cocos2d::CCArray* levels) {
        if (cacheLoaded && levels) {
            for (int i = 0; i < levels->count(); i++) {
                GJGameLevel* level = static_cast<GJGameLevel*>(levels->objectAtIndex(i));
                if (level) {
                    auto it = ratingsCache.find(level->m_levelID);
                    if (it != ratingsCache.end()) {
                        applyRatingToLevel(level, it->second);
                    }
                }
            }
        }

        LevelBrowserLayer::setupLevelBrowser(levels);
    }

    void applyRatingsToLoadedLevels() {
        if (!this->m_levels) return;

        for (int i = 0; i < this->m_levels->count(); i++) {
            GJGameLevel* level = static_cast<GJGameLevel*>(this->m_levels->objectAtIndex(i));
            if (level) {
                auto it = ratingsCache.find(level->m_levelID);
                if (it != ratingsCache.end()) {
                    applyRatingToLevel(level, it->second);
                }
            }
        }
    }
};

class $modify(NewRatedLevelsLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        if (cacheLoaded && level) {
            auto it = ratingsCache.find(level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(level, it->second);
            }
        }

        LevelCell::loadFromLevel(level);

        if (cacheLoaded && level) {
            auto it = ratingsCache.find(level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(level, it->second);
            }
        }
    }
};

class $modify(NewRatedLevelsPlayLayer, PlayLayer) {
    void levelComplete() {
        RatingData customRating = {0, 0, 0, 0};
        bool hasCustomRating = false;

        if (this->m_level && cacheLoaded) {
            auto it = ratingsCache.find(this->m_level->m_levelID);
            if (it != ratingsCache.end()) {
                hasCustomRating = true;
                customRating = it->second;
            }
        }

        PlayLayer::levelComplete();

        if (hasCustomRating && this->m_level) {
            Loader::get()->queueInMainThread([this, customRating]() {
                if (this->m_level) {
                    applyRatingToLevel(this->m_level, customRating);
                }
            });
        }
    }

    void onQuit() {
        if (this->m_level && cacheLoaded) {
            auto it = ratingsCache.find(this->m_level->m_levelID);
            if (it != ratingsCache.end()) {
                applyRatingToLevel(this->m_level, it->second);
            }
        }

        PlayLayer::onQuit();
    }
};
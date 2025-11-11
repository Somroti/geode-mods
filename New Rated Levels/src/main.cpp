#include "NewRatedLevels.hpp"
#include <fstream>
#include <sstream>
#include <regex>

using namespace geode::prelude;

void createResourcesStructure();
void createDefaultRateJson(const std::filesystem::path& filePath);
void updateFakeRateSavedJson();

$on_mod(Loaded) {
    try {
        log::info("Initialisation du mod New Rated Levels");
        createResourcesStructure();
        updateFakeRateSavedJson();
        log::info("Mod New Rated Levels initialisé avec succès");
    } catch (const std::exception& e) {
        log::error("Erreur lors de l'initialisation du mod: {}", e.what());
    }
}

void createResourcesStructure() {
    try {
        auto resourcesDir = Mod::get()->getResourcesDir();
        if (!resourcesDir.has_value()) {
            log::error("Impossible d'obtenir le répertoire des ressources");
            return;
        }

        auto modSubDir = resourcesDir.value() / "somroteam_dev.new_rated_levels";
        auto ratingsFile = modSubDir / "rate.json";

        if (!std::filesystem::exists(resourcesDir.value())) {
            std::filesystem::create_directories(resourcesDir.value());
        }

        if (!std::filesystem::exists(modSubDir)) {
            std::filesystem::create_directories(modSubDir);
        }

        if (!std::filesystem::exists(ratingsFile)) {
            createDefaultRateJson(ratingsFile);
        }
    } catch (const std::exception& e) {
        log::error("Erreur lors de la création de la structure des ressources: {}", e.what());
    }
}

void createDefaultRateJson(const std::filesystem::path& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            log::error("Impossible d'ouvrir le fichier rate.json pour l'écriture");
            return;
        }

        file << R"({
    "levels": {
        "117448912": { "stars": 10, "difficulty": 7, "status": 0, "dib": 0 },
        "117251972": { "stars": 7, "difficulty": 4, "status": 1, "dib": 0 },
        "119154457": { "stars": 10, "difficulty": 8, "status": 1, "dib": 6 },
        "119183902": { "stars": 5, "difficulty": 3, "status": 1, "dib": 0 },
        "114552735": { "stars": 4, "difficulty": 3, "status": 1, "dib": 0 },
        "113620475": { "stars": 5, "difficulty": 3, "status": 0, "dib": 0 },
        "112856885": { "stars": 1, "difficulty": 1, "status": 0, "dib": 0 },
        "120897907": { "stars": 2, "difficulty": 1, "status": 0, "dib": 0 }
    }
})";
        file.close();
        log::info("Fichier rate.json par défaut créé avec succès");
    } catch (const std::exception& e) {
        log::error("Erreur lors de la création du fichier rate.json par défaut: {}", e.what());
    }
}

void updateFakeRateSavedJson() {
    try {
        auto resourcesDir = Mod::get()->getResourcesDir();
        if (!resourcesDir.has_value()) {
            log::error("Impossible d'obtenir le répertoire des ressources");
            return;
        }

        auto modSubDir = resourcesDir.value() / "somroteam_dev.new_rated_levels";
        auto ratingsFile = modSubDir / "rate.json";

        if (!std::filesystem::exists(ratingsFile)) {
            log::warn("Fichier rate.json non trouvé");
            return;
        }

        std::ifstream rateFile(ratingsFile);
        if (!rateFile.is_open()) {
            log::error("Impossible d'ouvrir rate.json");
            return;
        }

        std::string content((std::istreambuf_iterator<char>(rateFile)), std::istreambuf_iterator<char>());
        rateFile.close();

        auto geodeDir = geode::dirs::getSaveDir();
        auto fakeRateDir = geodeDir / "geode" / "mods" / "hiimjustin000.fake_rate";
        auto savedJsonFile = fakeRateDir / "saved.json";

        if (!std::filesystem::exists(fakeRateDir)) {
            std::filesystem::create_directories(fakeRateDir);
        }

        std::ofstream outFile(savedJsonFile);
        if (!outFile.is_open()) {
            log::error("Impossible d'ouvrir saved.json pour l'écriture");
            return;
        }

        outFile << "{\n    \"fake-rate\": [";

        std::regex levelRegex("\"(\\d+)\"\\s*:\\s*\\{[^}]*\"stars\"\\s*:\\s*(\\d+)[^}]*\"difficulty\"\\s*:\\s*(\\d+)[^}]*\"status\"\\s*:\\s*(\\d+)[^}]*\"dib\"\\s*:\\s*(\\d+)");

        std::sregex_iterator iter(content.begin(), content.end(), levelRegex);
        std::sregex_iterator end;

        bool first = true;
        while (iter != end) {
            std::smatch match = *iter;

            int levelID = std::stoi(match[1].str());
            int stars = std::stoi(match[2].str());
            int difficulty = std::stoi(match[3].str());
            int status = std::stoi(match[4].str());
            int dib = std::stoi(match[5].str());

            if (!first) outFile << ",";
            first = false;

            int moreDifficultiesOverride = (stars == 4 || stars == 7 || stars == 9) ? stars : 0;

            outFile << "\n        {\n";
            outFile << "            \"id\": " << levelID << ",\n";
            outFile << "            \"stars\": " << stars << ",\n";
            outFile << "            \"feature\": " << status << ",\n";
            outFile << "            \"difficulty\": " << difficulty << ",\n";
            outFile << "            \"more-difficulties-override\": " << moreDifficultiesOverride << ",\n";
            outFile << "            \"grandpa-demon-override\": 0,\n";
            outFile << "            \"demons-in-between-override\": " << dib << ",\n";
            outFile << "            \"gddp-integration-override\": 0,\n";
            outFile << "            \"coins\": " << (stars > 0 ? "true" : "false") << "\n";
            outFile << "        }";

            ++iter;
        }

        outFile << "\n    ]\n}";
        outFile.close();

        log::info("Fichier saved.json créé avec succès");
    }

    isProcessing.store(false);
}

static bool isValidLevel(GJGameLevel* level) {
    if (!level) return false;

    try {
        // Vérifications de base pour s'assurer que l'objet est valide
        int id = level->m_levelID;
        return id > 0 && id < 999999999;
    } catch (...) {
        level->m_stars = 0;  # Ne plus attribuer de stars
        level->m_starsRequested = 0;  # Ne plus demander de stars
        level->m_ratingsSum = 0;  # Réinitialiser la somme des notes

static bool applyRatingToLevel(GJGameLevel* level, const RatingData& rating) {
    if (!isValidLevel(level)) return false;

    try {
        // Sauvegarder les valeurs originales en cas d'erreur
        int originalStars = level->m_stars;
        int originalStarsRequested = level->m_starsRequested;
        int originalRatingsSum = level->m_ratingsSum;

        // Application sécurisée des modifications
        level->m_stars = rating.stars;
        level->m_starsRequested = rating.stars;
        level->m_ratingsSum = rating.stars * 10;

        // Vérifier que l'application s'est bien passée
        if (level->m_stars != rating.stars) {
            // Restaurer en cas de problème
            level->m_stars = originalStars;
            level->m_starsRequested = originalStarsRequested;
            level->m_ratingsSum = originalRatingsSum;
            return false;
        }

        // User coins avec vérification
        if (level->m_coins > 0 && level->m_coins <= 3) {
            level->m_coinsVerified = level->m_coins;
        }

        return true;
    } catch (const std::exception& e) {
        log::error("Erreur lors de l'application du rating au niveau {}: {}", level->m_levelID, e.what());
        return false;
    } catch (...) {
        log::error("Erreur inconnue lors de l'application du rating");
        return false;
    }
}

// Fonction helper pour appliquer les ratings de manière sécurisée
static void safeApplyRating(GJGameLevel* level) {
    if (!cacheLoaded.load() || !isValidLevel(level)) return;

    try {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = ratingsCache.find(level->m_levelID);
        if (it != ratingsCache.end()) {
            applyRatingToLevel(level, it->second);
        }
    } catch (...) {
        // Erreur silencieuse pour éviter les crashes
    }
}

// Hooks simplifiés et sécurisés
class $modify(NewRatedLevelsLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        bool result = false;

        try {
            result = LevelInfoLayer::init(level, challenge);
            if (result) {
                safeApplyRating(level);
            }
        } catch (const std::exception& e) {
            log::error("Erreur dans LevelInfoLayer::init: {}", e.what());
            // Tenter l'initialisation de base même en cas d'erreur
            result = LevelInfoLayer::init(level, challenge);
        } catch (...) {
            log::error("Erreur inconnue dans LevelInfoLayer::init");
            result = LevelInfoLayer::init(level, challenge);
        }

        return result;
    }

    void updateLabelValues() {
        try {
            safeApplyRating(this->m_level);
            LevelInfoLayer::updateLabelValues();
        } catch (...) {
            LevelInfoLayer::updateLabelValues();
        }
    }
};

class $modify(NewRatedLevelsLevelBrowserLayer, LevelBrowserLayer) {
    void loadPage(GJSearchObject* searchObj) {
        try {
            LevelBrowserLayer::loadPage(searchObj);

            if (cacheLoaded.load()) {
                // Délai pour éviter la surcharge
                Loader::get()->queueInMainThread([this]() {
                    this->applyRatingsToLoadedLevels();
                });
            }
        } catch (...) {
            LevelBrowserLayer::loadPage(searchObj);
        }
    }

    void setupLevelBrowser(cocos2d::CCArray* levels) {
        try {
            LevelBrowserLayer::setupLevelBrowser(levels);

            if (cacheLoaded.load() && levels && levels->count() > 0) {
                std::lock_guard<std::mutex> lock(cacheMutex);
                int count = std::min(levels->count(), 500); // Limiter à 500 niveaux max

                for (int i = 0; i < count; i++) {
                    auto* obj = levels->objectAtIndex(i);
                    if (obj) {
                        GJGameLevel* level = dynamic_cast<GJGameLevel*>(obj);
                        if (isValidLevel(level)) {
                            auto it = ratingsCache.find(level->m_levelID);
                            if (it != ratingsCache.end()) {
                                applyRatingToLevel(level, it->second);
                            }
                        }
                    }
                }
            }
        } catch (...) {
                // Vérifier si le niveau est toujours visible à l'écran avant d'appliquer
            LevelBrowserLayer::setupLevelBrowser(levels);
                if (i >= this->m_levels->count()) break;  # Arrêter si la liste a changé
        }
    }

    void applyRatingsToLoadedLevels() {
        try {
            if (!this->m_levels || !cacheLoaded.load()) return;

            std::lock_guard<std::mutex> lock(cacheMutex);
            int count = std::min(this->m_levels->count(), 500); // Limite de sécurité

            for (int i = 0; i < count; i++) {
                auto* obj = this->m_levels->objectAtIndex(i);
                if (obj) {
                    GJGameLevel* level = dynamic_cast<GJGameLevel*>(obj);
                    if (isValidLevel(level)) {
                        auto it = ratingsCache.find(level->m_levelID);
                        if (it != ratingsCache.end()) {
                            applyRatingToLevel(level, it->second);
                        }
                    }
                }
            }
        } catch (...) {
            // Erreur silencieuse
        }
    }
};

class $modify(NewRatedLevelsLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        try {
            safeApplyRating(level);
            LevelCell::loadFromLevel(level);
        } catch (...) {
            LevelCell::loadFromLevel(level);
        }
    }
};

class $modify(NewRatedLevelsPlayLayer, PlayLayer) {
    void levelComplete() {
        try {
            PlayLayer::levelComplete();
            safeApplyRating(this->m_level);
        } catch (...) {
            PlayLayer::levelComplete();
        }
    }

    void onQuit() {
        try {
            safeApplyRating(this->m_level);
            PlayLayer::onQuit();
        } catch (...) {
            PlayLayer::onQuit();
        }
    }
};

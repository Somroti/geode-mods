#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace geode::prelude;

class $modify(SaveProgressMod, PlayLayer) {
public:
    std::unordered_map<std::string, int> readSaveData() {
        std::unordered_map<std::string, int> data;
        std::ifstream file("save_progress.json");
        if (!file.is_open()) return data;

        std::string line;
        while (std::getline(file, line)) {
            size_t quote1 = line.find('\"');
            size_t quote2 = line.find('\"', quote1 + 1);
            size_t colon = line.find(':', quote2);

            if (quote1 != std::string::npos && quote2 != std::string::npos && colon != std::string::npos) {
                std::string key = line.substr(quote1 + 1, quote2 - quote1 - 1);
                std::string valueStr = line.substr(colon + 1);
                int value = std::stoi(valueStr);
                data[key] = value;
            }
        }

        file.close();
        return data;
    }

    void writeSaveData(const std::unordered_map<std::string, int>& data) {
        std::ofstream file("save_progress.json");
        file << "{\n";
        bool first = true;
        for (auto& [key, value] : data) {
            if (!first) file << ",\n";
            first = false;
            file << "  \"" << key << "\": " << value;
        }
        file << "\n}\n";
        file.close();
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        std::string levelID = std::to_string(level->m_levelID.value());
        auto data = readSaveData();

        int saved = data.contains(levelID) ? data[levelID] : 0;
        int current = level->m_normalPercent;

        if (saved > current) {
            level->m_normalPercent = saved;
        }

        return true;
    }

    void onExit() {
        PlayLayer::onExit();

        std::string levelID = std::to_string(m_level->m_levelID.value());
        int progress = m_level->m_normalPercent;

        auto data = readSaveData();
        if (!data.contains(levelID) || progress > data[levelID]) {
            data[levelID] = progress;
            writeSaveData(data);
        }
    }
};

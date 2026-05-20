#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::string encodeSpaces(const std::string& text) {
    std::string result;

    for (char c : text) {
        if (c == ' ') {
            result += "%20";
        } else {
            result += c;
        }
    }

    return result;
}

void printPlayerStatsFromMatch(const nlohmann::json& matchData, const std::string& puuid) {
    auto participants = matchData["info"]["participants"];

    for (const auto& participant : participants) {
        if (participant["puuid"] == puuid) {
            std::string championName = participant["championName"];
            bool win = participant["win"];

            int kills = participant["kills"];
            int deaths = participant["deaths"];
            int assists = participant["assists"];

            int totalMinionsKilled = participant["totalMinionsKilled"];
            int neutralMinionsKilled = participant["neutralMinionsKilled"];
            int totalCS = totalMinionsKilled + neutralMinionsKilled;

            int visionScore = participant["visionScore"];
            int damage = participant["totalDamageDealtToChampions"];

            double gameDurationSeconds = matchData["info"]["gameDuration"];
            double gameMinutes = gameDurationSeconds / 60.0;

            double kda = static_cast<double>(kills + assists) / std::max(1, deaths);
            double csPerMin = totalCS / gameMinutes;

            std::cout << "Champion: " << championName << std::endl;
            std::cout << "Result: " << (win ? "Win" : "Loss") << std::endl;
            std::cout << "KDA: " << kills << "/" << deaths << "/" << assists << std::endl;
            std::cout << "Calculated KDA: " << kda << std::endl;
            std::cout << "CS: " << totalCS << std::endl;
            std::cout << "CS/min: " << csPerMin << std::endl;
            std::cout << "Vision score: " << visionScore << std::endl;
            std::cout << "Damage to champions: " << damage << std::endl;

            return;
        }
    }

    std::cout << "Player not found in this match." << std::endl;
}

int main() {
    std::cout << "Rift Therapist is running!" << std::endl;

    const char* apiKey = std::getenv("RIOT_API_KEY");

    if (apiKey == nullptr) {
        std::cout << "API key missing" << std::endl;
        return 1;
    }

    std::cout << "API key found" << std::endl;

    std::string gameName;
    std::string tagLine;

    std::cout << "Enter game name: ";
    std::getline(std::cin, gameName);

    std::cout << "Enter tag line: ";
    std::getline(std::cin, tagLine);

    // Step 1: Get PUUID from Riot ID
    std::string accountUrl =
        "https://asia.api.riotgames.com/riot/account/v1/accounts/by-riot-id/"
        + encodeSpaces(gameName)
        + "/"
        + encodeSpaces(tagLine);

    cpr::Response accountResponse = cpr::Get(
        cpr::Url{accountUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "\nAccount status code: " << accountResponse.status_code << std::endl;
    std::cout << "Account response body:" << std::endl;
    std::cout << accountResponse.text << std::endl;

    std::string puuid;

    if (accountResponse.status_code == 200) {
        nlohmann::json accountData = nlohmann::json::parse(accountResponse.text);
        puuid = accountData["puuid"];

        std::cout << "PUUID: " << puuid << std::endl;
    } else {
        std::cout << "Account request failed" << std::endl;
        return 1;
    }

    // Step 2: Get recent match IDs
    std::string numberOfMatches = "10";

    std::string matchesUrl =
        "https://sea.api.riotgames.com/lol/match/v5/matches/by-puuid/"
        + puuid
        + "/ids?start=0&count="
        + numberOfMatches;

    std::cout << "\nMatches URL: " << matchesUrl << std::endl;

    cpr::Response matchListResponse = cpr::Get(
        cpr::Url{matchesUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "Match list status code: " << matchListResponse.status_code << std::endl;
    std::cout << "Match list response body:" << std::endl;
    std::cout << matchListResponse.text << std::endl;

    nlohmann::json matchIds;

    if (matchListResponse.status_code == 200) {
        matchIds = nlohmann::json::parse(matchListResponse.text);

        if (matchIds.empty()) {
            std::cout << "No matches found." << std::endl;
            return 1;
        }
    } else {
        std::cout << "Match list request failed" << std::endl;
        return 1;
    }


    // Step 4: Loop through all matches, including the first one again
    std::cout << "\n=== Recent Match Summaries ===" << std::endl;

    for (const auto& matchIdJson : matchIds) {
        std::string matchId = matchIdJson;

        std::string matchDetailUrl =
            "https://sea.api.riotgames.com/lol/match/v5/matches/"
            + matchId;

        cpr::Response matchDetails = cpr::Get(
            cpr::Url{matchDetailUrl},
            cpr::Header{
                {"X-Riot-Token", apiKey}
            }
        );

        if (matchDetails.status_code != 200) {
            std::cout << "\nFailed to fetch match: " << matchId << std::endl;
            continue;
        }

        nlohmann::json matchData = nlohmann::json::parse(matchDetails.text);

        std::cout << "\nMatch: " << matchId << std::endl;
        printPlayerStatsFromMatch(matchData, puuid);
    }

    return 0;
}
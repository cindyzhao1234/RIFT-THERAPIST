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

bool printPlayerStatsFromMatch(
    const nlohmann::json& matchData,
    const std::string& puuid,
    int& totalKills,
    int& totalDeaths,
    int& totalAssists,
    int& totalVisionScore,
    double& totalCsPerMin,
    int& gamesAnalysed
) {
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

            totalKills += kills;
            totalDeaths += deaths;
            totalAssists += assists;
            totalVisionScore += visionScore;
            totalCsPerMin += csPerMin;
            gamesAnalysed++;

            std::cout << "Champion: " << championName << std::endl;
            std::cout << "Result: " << (win ? "Win" : "Loss") << std::endl;
            std::cout << "KDA: " << kills << "/" << deaths << "/" << assists << std::endl;
            std::cout << "Calculated KDA: " << kda << std::endl;
            std::cout << "CS: " << totalCS << std::endl;
            std::cout << "CS/min: " << csPerMin << std::endl;
            std::cout << "Vision score: " << visionScore << std::endl;
            std::cout << "Damage to champions: " << damage << std::endl;

            return win;
        }
    }

    std::cout << "Player not found in this match." << std::endl;
    return false;
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

    int winCount = 0;
    int lossCount = 0;
    int gamesAnalysed = 0;

    int totalKills = 0;
    int totalDeaths = 0;
    int totalAssists = 0;
    int totalVisionScore = 0;

    double totalCsPerMin = 0.0;

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

        bool win = printPlayerStatsFromMatch(
            matchData,
            puuid,
            totalKills,
            totalDeaths,
            totalAssists,
            totalVisionScore,
            totalCsPerMin,
            gamesAnalysed
        );

        if (win) {
            winCount++;
        } else {
            lossCount++;
        }
    }

    if (gamesAnalysed == 0) {
        std::cout << "No valid games were analysed." << std::endl;
        return 1;
    }

    double winRate = static_cast<double>(winCount) / gamesAnalysed * 100.0;
    double averageDeaths = static_cast<double>(totalDeaths) / gamesAnalysed;
    double averageKda = static_cast<double>(totalKills + totalAssists) / std::max(1, totalDeaths);
    double averageCsPerMin = totalCsPerMin / gamesAnalysed;
    double averageVisionScore = static_cast<double>(totalVisionScore) / gamesAnalysed;

    std::cout << "\n=== Rift Therapist Diagnosis ===" << std::endl;

    std::cout << "Games analysed: " << gamesAnalysed << std::endl;
    std::cout << "Wins: " << winCount << std::endl;
    std::cout << "Losses: " << lossCount << std::endl;
    std::cout << "Win rate: " << winRate << "%" << std::endl;
    std::cout << "Average KDA: " << averageKda << std::endl;
    std::cout << "Average deaths: " << averageDeaths << std::endl;
    std::cout << "Average CS/min: " << averageCsPerMin << std::endl;
    std::cout << "Average vision score: " << averageVisionScore << std::endl;

    std::cout << "\nAdvice:" << std::endl;

    if (winRate >= 60.0) {
        std::cout << "- You are on a strong run. Keep your champion pool and playstyle consistent." << std::endl;
    } else if (winRate >= 50.0) {
        std::cout << "- Your recent games are balanced. Small improvements could turn this into a climb." << std::endl;
    } else {
        std::cout << "- Your recent win rate is below 50%. Focus on one clear improvement before queueing again." << std::endl;
    }

    if (averageDeaths >= 7.0) {
        std::cout << "- Your average deaths are quite high. You may be taking too many risky fights." << std::endl;
    } else if (averageDeaths <= 4.0) {
        std::cout << "- Your deaths are controlled. You are generally staying alive well." << std::endl;
    }

    if (averageCsPerMin < 5.5) {
        std::cout << "- Your CS/min is low. Farming consistency should be a main focus." << std::endl;
    } else if (averageCsPerMin >= 7.0) {
        std::cout << "- Your CS/min is strong. You are keeping up well economically." << std::endl;
    }

    if (averageVisionScore < 15.0) {
        std::cout << "- Your vision score is low. Try placing more wards and buying control wards." << std::endl;
    } else {
        std::cout << "- Your vision score looks decent across these games." << std::endl;
    }

    std::cout << "\nRift Therapist says: ";
    
    if (winRate < 50.0 && averageDeaths >= 7.0) {
        std::cout << "Tilt Queue Warning. You are probably fighting too much while behind." << std::endl;
    } else if (winRate >= 60.0 && averageKda >= 3.0) {
        std::cout << "Main Character Arc. You are carrying more than inting right now." << std::endl;
    } else if (averageCsPerMin < 5.5) {
        std::cout << "Farming Therapy Required. Minions are being left unattended." << std::endl;
    } else if (averageVisionScore < 15.0) {
        std::cout << "Map Blindness Detected. Wards are cheaper than losing LP." << std::endl;
    } else {
        std::cout << "Training Arc. Nothing is completely doomed, but there is room to improve." << std::endl;
    }

    return 0;
}
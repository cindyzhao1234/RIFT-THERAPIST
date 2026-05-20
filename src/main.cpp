#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

struct MatchStats {
    std::string matchId;
    std::string championName;
    bool win;

    int kills;
    int deaths;
    int assists;
    int cs;
    int visionScore;
    int damage;

    double kda;
    double csPerMin;
};

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

bool extractPlayerStatsFromMatch(
    const nlohmann::json& matchData,
    const std::string& matchId,
    const std::string& puuid,
    MatchStats& stats
) {
    auto participants = matchData["info"]["participants"];

    for (const auto& participant : participants) {
        if (participant["puuid"] == puuid) {
            int kills = participant["kills"];
            int deaths = participant["deaths"];
            int assists = participant["assists"];

            int totalMinionsKilled = participant["totalMinionsKilled"];
            int neutralMinionsKilled = participant["neutralMinionsKilled"];
            int totalCS = totalMinionsKilled + neutralMinionsKilled;

            double gameDurationSeconds = matchData["info"]["gameDuration"];
            double gameMinutes = gameDurationSeconds / 60.0;

            stats.matchId = matchId;
            stats.championName = participant["championName"];
            stats.win = participant["win"];

            stats.kills = kills;
            stats.deaths = deaths;
            stats.assists = assists;
            stats.cs = totalCS;
            stats.visionScore = participant["visionScore"];
            stats.damage = participant["totalDamageDealtToChampions"];

            stats.kda = static_cast<double>(kills + assists) / std::max(1, deaths);
            stats.csPerMin = totalCS / gameMinutes;

            return true;
        }
    }

    return false;
}

void printMatchStats(const MatchStats& stats) {
    std::cout << "\nMatch: " << stats.matchId << std::endl;
    std::cout << "Champion: " << stats.championName << std::endl;
    std::cout << "Result: " << (stats.win ? "Win" : "Loss") << std::endl;
    std::cout << "KDA: " << stats.kills << "/" << stats.deaths << "/" << stats.assists << std::endl;
    std::cout << "Calculated KDA: " << stats.kda << std::endl;
    std::cout << "CS: " << stats.cs << std::endl;
    std::cout << "CS/min: " << stats.csPerMin << std::endl;
    std::cout << "Vision score: " << stats.visionScore << std::endl;
    std::cout << "Damage to champions: " << stats.damage << std::endl;
}

void printRiftTherapistDiagnosis(const std::vector<MatchStats>& allMatches) {
    if (allMatches.empty()) {
        std::cout << "No valid matches were analysed." << std::endl;
        return;
    }

    int winCount = 0;
    int lossCount = 0;

    int totalKills = 0;
    int totalDeaths = 0;
    int totalAssists = 0;
    int totalVisionScore = 0;

    int winDeaths = 0;
    int lossDeaths = 0;
    int winGames = 0;
    int lossGames = 0;

    double totalCsPerMin = 0.0;

    for (const MatchStats& match : allMatches) {
        if (match.win) {
            winCount++;
            winGames++;
            winDeaths += match.deaths;
        } else {
            lossCount++;
            lossGames++;
            lossDeaths += match.deaths;
        }

        totalKills += match.kills;
        totalDeaths += match.deaths;
        totalAssists += match.assists;
        totalVisionScore += match.visionScore;
        totalCsPerMin += match.csPerMin;
    }

    int gamesAnalysed = allMatches.size();

    double winRate = static_cast<double>(winCount) / gamesAnalysed * 100.0;
    double averageDeaths = static_cast<double>(totalDeaths) / gamesAnalysed;
    double averageKda = static_cast<double>(totalKills + totalAssists) / std::max(1, totalDeaths);
    double averageCsPerMin = totalCsPerMin / gamesAnalysed;
    double averageVisionScore = static_cast<double>(totalVisionScore) / gamesAnalysed;

    double averageDeathsInWins = 0.0;
    double averageDeathsInLosses = 0.0;

    if (winGames > 0) {
        averageDeathsInWins = static_cast<double>(winDeaths) / winGames;
    }

    if (lossGames > 0) {
        averageDeathsInLosses = static_cast<double>(lossDeaths) / lossGames;
    }

    std::cout << "\n=== Rift Therapist Diagnosis ===" << std::endl;

    std::cout << "Games analysed: " << gamesAnalysed << std::endl;
    std::cout << "Wins: " << winCount << std::endl;
    std::cout << "Losses: " << lossCount << std::endl;
    std::cout << "Win rate: " << winRate << "%" << std::endl;
    std::cout << "Average KDA: " << averageKda << std::endl;
    std::cout << "Average deaths: " << averageDeaths << std::endl;
    std::cout << "Average deaths in wins: " << averageDeathsInWins << std::endl;
    std::cout << "Average deaths in losses: " << averageDeathsInLosses << std::endl;
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

    if (lossGames > 0 && winGames > 0 && averageDeathsInLosses >= averageDeathsInWins + 2.0) {
        std::cout << "- You are dying much more in losses than wins. When a game starts going badly, focus on slowing the game down instead of forcing fights." << std::endl;
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
    } else if (lossGames > 0 && winGames > 0 && averageDeathsInLosses >= averageDeathsInWins + 2.0) {
        std::cout << "Death Gap Detected. Your losses are likely becoming worse because you keep taking risky fights." << std::endl;
    } else if (winRate >= 60.0 && averageKda >= 3.0) {
        std::cout << "Main Character Arc. You are carrying more than inting right now." << std::endl;
    } else if (averageCsPerMin < 5.5) {
        std::cout << "Farming Therapy Required. Minions are being left unattended." << std::endl;
    } else if (averageVisionScore < 15.0) {
        std::cout << "Map Blindness Detected. Wards are cheaper than losing LP." << std::endl;
    } else {
        std::cout << "Training Arc. Nothing is completely doomed, but there is room to improve." << std::endl;
    }
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

    std::string puuid;

    if (accountResponse.status_code == 200) {
        nlohmann::json accountData = nlohmann::json::parse(accountResponse.text);
        puuid = accountData["puuid"];

        std::cout << "PUUID: " << puuid << std::endl;
    } else {
        std::cout << "Account request failed" << std::endl;
        std::cout << accountResponse.text << std::endl;
        return 1;
    }

    std::string numberOfMatches = "10";

    std::string matchesUrl =
        "https://sea.api.riotgames.com/lol/match/v5/matches/by-puuid/"
        + puuid
        + "/ids?start=0&count="
        + numberOfMatches;

    cpr::Response matchListResponse = cpr::Get(
        cpr::Url{matchesUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "\nMatch list status code: " << matchListResponse.status_code << std::endl;

    nlohmann::json matchIds;

    if (matchListResponse.status_code == 200) {
        matchIds = nlohmann::json::parse(matchListResponse.text);

        if (matchIds.empty()) {
            std::cout << "No matches found." << std::endl;
            return 1;
        }
    } else {
        std::cout << "Match list request failed" << std::endl;
        std::cout << matchListResponse.text << std::endl;
        return 1;
    }

    std::vector<MatchStats> allMatches;

    std::cout << "\n=== Recent Match Summaries ===" << std::endl;

    for (const auto& matchIdJson : matchIds) {
        std::string matchId = matchIdJson;

        std::string matchDetailUrl =
            "https://sea.api.riotgames.com/lol/match/v5/matches/"
            + matchId;

        cpr::Response matchDetailsResponse = cpr::Get(
            cpr::Url{matchDetailUrl},
            cpr::Header{
                {"X-Riot-Token", apiKey}
            }
        );

        if (matchDetailsResponse.status_code != 200) {
            std::cout << "\nFailed to fetch match: " << matchId << std::endl;
            std::cout << matchDetailsResponse.text << std::endl;
            continue;
        }

        nlohmann::json matchData = nlohmann::json::parse(matchDetailsResponse.text);

        MatchStats stats;

        bool foundPlayer = extractPlayerStatsFromMatch(
            matchData,
            matchId,
            puuid,
            stats
        );

        if (foundPlayer) {
            allMatches.push_back(stats);
            printMatchStats(stats);
        } else {
            std::cout << "\nPlayer not found in match: " << matchId << std::endl;
        }
    }

    printRiftTherapistDiagnosis(allMatches);

    return 0;
}
#include <iostream>
#include <cstdlib>
#include <string>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::string encodeSpaces(const std::string& text){
    std::string result;

    for(char c : text){
        if(c == ' '){
            result += "%20";
        } else{
            result += c;
        }
    }
    return result;
}


int main() {
    std::cout << "Rift Therapist is running!" << std::endl;
    
    const char* apiKey = std::getenv("RIOT_API_KEY");

    if(apiKey == nullptr){
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

    std::string accountUrl = "https://asia.api.riotgames.com/riot/account/v1/accounts/by-riot-id/" + encodeSpaces(gameName) + "/" + encodeSpaces(tagLine);

    cpr::Response response = cpr::Get(
        cpr::Url{accountUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "Status code: " << response.status_code << std::endl;
    std::cout << "Response body:" << std::endl;
    std::cout << response.text << std::endl;

    std::string puuid;

    if(response.status_code == 200){
        nlohmann::json data = nlohmann::json::parse(response.text);
        
        puuid = data["puuid"];

        std::cout<< "PUUID: " << puuid << std::endl;
    } else{
        std::cout << "Request failed" << std::endl;
        return 1;
    }


    std::string numberofMatches = "10";
    std::string matchesUrl = "https://sea.api.riotgames.com/lol/match/v5/matches/by-puuid/" + puuid + "/ids?start=0&count=" + numberofMatches;

    std::cout << "Match Url: " << matchesUrl << std::endl;

    cpr::Response matchResponse = cpr::Get(
        cpr::Url{matchesUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "Status code: " << matchResponse.status_code << std::endl;
    std::cout << "Response body:" << std::endl;
    std::cout << matchResponse.text << std::endl;


    std::string firstMatch;
    if(matchResponse.status_code == 200){
        nlohmann::json matchIds = nlohmann::json::parse(matchResponse.text);
        
        firstMatch = matchIds[0];

        std::cout<< "First match: " << firstMatch << std::endl;
    } else{
        std::cout << "Request failed" << std::endl;
        return 1;
    }

    std::string matchDetailUrl = "https://sea.api.riotgames.com/lol/match/v5/matches/" + firstMatch;

    cpr::Response firstMatchDetails = cpr::Get(
        cpr::Url{matchDetailUrl},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    if(firstMatchDetails.status_code != 200){
        std::cout << "Request failed" << std::endl;
        return 1;
    } 

    nlohmann::json matchData = nlohmann::json::parse(firstMatchDetails.text);
    auto participants = matchData["info"]["participants"];

    for(const auto& participant : participants){
        if(participant["puuid"] == puuid){
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

            break;
        }
    }

    return 0;
}
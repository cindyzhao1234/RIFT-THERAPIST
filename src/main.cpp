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

    std::string url = "https://asia.api.riotgames.com/riot/account/v1/accounts/by-riot-id/" + encodeSpaces(gameName) + "/" + encodeSpaces(tagLine);

    cpr::Response response = cpr::Get(
        cpr::Url{url},
        cpr::Header{
            {"X-Riot-Token", apiKey}
        }
    );

    std::cout << "Status code: " << response.status_code << std::endl;
    std::cout << "Response body:" << std::endl;
    std::cout << response.text << std::endl;
    return 0;
}
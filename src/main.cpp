#include <iostream>
#include <cstdlib>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

int main() {
    std::cout << "Rift Therapist is running!" << std::endl;
    
    const char* apiKey = std::getenv("RIOT_API_KEY");

    if(apiKey == nullptr){
        std::cout << "API key missing" << std::endl;
    } else{
        std::cout << "API key found" << std::endl;
    }
    return 0;
}
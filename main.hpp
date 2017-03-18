#include <iostream>
#include <set>

struct Flight {
    std::string source;
    std::string destination;
    uint16_t departure;
    uint16_t price;
    float pheromone;
};

struct Ant {
    std::string actualCity;
    std::string nextCity;
    std::set<std::string> nonVisitedCities;
    std::vector<Flight> path;
    int cost;
};

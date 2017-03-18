#include <iostream>
#include <set>
#include <exception>

//TODO inicializovat spravne premenne
int ANT_COUNT = 1;
int BETA = 2;
float PHEROMONE_INIT_VALUE = 1;
float ALPHA = 0.1;
float RHO = 0.1;

struct MissingColumnException : public std::exception {
    const char * what () const throw () {
        return "Malformed line in CSV input. There was expected another column.";
    }
};

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
        uint16_t cost;
        bool active;
};

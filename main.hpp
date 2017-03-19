#include <iostream>
#include <set>
#include <exception>
#include <string>
#include <vector>


int ANT_COUNT = 10;
int BETA = 2;
float PHEROMONE_INIT_VALUE = 10;
float ALPHA = 0.1;
float RHO = 0.5;

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

void printResults();

void printPath(std::vector<Flight> path);

void sig_handler(int signal);

void readCsv();

void printCities();

void initAnts();

void init();

std::string getNextCity(std::string source, uint16_t departure, std::set<std::string> nonVisitedCities);

void evaluateAnt(int antIndex);

void evaluate();

#include <csignal>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>


#include "main.hpp"

using namespace std;

map<string, map <string , map <uint16_t , Flight> > > citiesMap;
string start = "";
int bestCost = UINT16_MAX;
int cityCount = 0;
vector<Flight> bestPath;
vector<Ant> ants;
set<string> tmpCities;
std::mutex mu;

void printResults() {
    cout << bestCost <<endl;
    vector<Flight>::iterator it;
    for(it = bestPath.begin(); it != bestPath.end();it++){
        cout << it->source <<" "<<it->destination <<" "<< it->departure<<" "<< it->price << endl;
    }
}

void printPath(vector<Flight> path) {
    vector<Flight>::iterator it;
    for(it = path.begin(); it != path.end();it++){
        cout << it->source <<" "<<it->destination <<" "<< it->departure<<" "<< it->price << endl;
    }

}

void sig_handler(int signal) {
    if (signal == SIGTERM){
        printResults();
        exit(0);
    }
}

void readCsv() {
    std::getline(cin, start);
    for(string line; std::getline(cin, line);) {
        string source, destination, departure, price;
        istringstream iss(line);

        if (!(iss >> source >> destination >> departure >> price)) {
            throw MissingColumnException();
        }

        //Skip flight from start which is not 0.day
        if(source.compare(start) == 0 && (uint16_t) stoi(departure) !=0){
            continue;
        }
        //Skip flight from city(not start) which is 0.day
        if(source.compare(start) != 0 && (uint16_t) stoi(departure) ==0){
            continue;
        }

        citiesMap[source][destination][(uint16_t) stoi(departure)] = {source, destination, (uint16_t) stoi(departure), (uint16_t) stoi(price), PHEROMONE_INIT_VALUE};
    }

    cityCount = citiesMap.size();
}

void printCities() {

    map<string, map <string , map <uint16_t , Flight > > >::const_iterator its;

    //Iterate oved map key=source
    for( its=citiesMap.begin(); its!=citiesMap.end(); its++) {
        cout << its->first << endl;

        map <string , map <uint16_t , Flight > >::const_iterator itdes;

        //Iterate oved map key=destination
        for( itdes=citiesMap[its->first].begin(); itdes!=citiesMap[its->first].end(); itdes++) {

            map <uint16_t , Flight > ::const_iterator itdep;

            //Iterate oved map key=departure
            for( itdep=citiesMap[its->first][itdes->first].begin(); itdep!=citiesMap[its->first][itdes->first].end(); itdep++) {

                //Iterate oved vector with flight
                Flight flight = citiesMap[its->first][itdes->first][itdep->first];
                cout << "{source: "<<flight.source << ", destination: " << flight.destination << ", price: " << flight.price << ", departure: " << flight.departure << ", pheromone: " << flight.pheromone << "}" << endl;
            }
        }
    }
}

void initAnts() {

    //Initialization of ants
    ants.clear();
    for(int i=0; i<ANT_COUNT; i++){
        Ant tmpAnt;

        tmpAnt.actualCity = start;
        tmpAnt.nextCity = "";
        tmpAnt.cost = 0;
        tmpAnt.nonVisitedCities = tmpCities;
        tmpAnt.active = true;

        ants.push_back(tmpAnt);
    }
}

void init() {

    map<string, map <string , map <uint16_t , Flight > > >::const_iterator its;
    for( its=citiesMap.begin(); its!=citiesMap.end(); its++) {
        if(its->first != start) {
            tmpCities.insert(its->first);
        }
    }

    initAnts();
}

string getNextCity(string source, uint16_t departure, set<string> nonVisitedCities) {

    vector<string> availableCities;
    vector<float> availableCitiesRating;
    float availableCitiesRatingSum = 0;

    // Count probability of choosing for all available cities = not yet visited
    set<string>::const_iterator itdes;
    for( itdes=nonVisitedCities.begin(); itdes != nonVisitedCities.end(); itdes++) {

        //Iba ak existuje taky let
        if(citiesMap[source][*itdes][departure].source.compare("") != 0){
            mu.lock();
            float pheromone = citiesMap[source][*itdes][departure].pheromone;
            mu.unlock();

            float cityRating = pheromone * (1 / pow(citiesMap[source][*itdes][departure].price, BETA));
            availableCities.push_back(*itdes);
            availableCitiesRating.push_back(cityRating);
            availableCitiesRatingSum += cityRating;
        }
    }

    // Generate random number from <0;1>
    auto r = ((double) rand() / (RAND_MAX));
    for (size_t i = 0; i < availableCitiesRating.size(); i++) {

        // If random number is in interval assigned to this city, return it as next city
        r -= availableCitiesRating[i] / availableCitiesRatingSum;
        if(r < 0) {
            //cout << "Vyberam si teba: " << availableCities[i] << endl;
            return availableCities[i];
        }
    }

    // If no city is available from actual city
    return "";
}

void evaluateAnt(int antIndex) {
    for(int cityIndex = 0; cityIndex < cityCount; cityIndex++) {
        if(cityIndex < (cityCount -1) ) {

            // Skip ant if is lost / inactive
            if(ants[antIndex].active){

                // Find next city for ant
                ants[antIndex].nextCity = getNextCity(ants[antIndex].actualCity,cityIndex,ants[antIndex].nonVisitedCities);

                // Ant is lost ?
                if(ants[antIndex].nextCity.compare("") == 0){
                    ants[antIndex].active = false;
                } else {
                    ants[antIndex].nonVisitedCities.erase(ants[antIndex].nextCity);
                    ants[antIndex].path.push_back(citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex]);
                    ants[antIndex].cost += citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex].price;
                }
            }
        } else {

            // In the end append last flight to route - flight back to start city
            // Skip ant if is lost / inactive
            if(ants[antIndex].active){
                ants[antIndex].nextCity = start;
                if(citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex].source.compare("") != 0){
                    ants[antIndex].path.push_back(citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex]);
                    ants[antIndex].cost += citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex].price;
                } else {

                    // If no next city, then ant is lost / inactive
                    ants[antIndex].active = false;
                }
            }
        }

        // Local update pheromone and position
        // Skip ant if is lost / inactive
        if(ants[antIndex].active){
            mu.lock();
            float pheromoneActual = citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex].pheromone;
            citiesMap[ants[antIndex].actualCity][ants[antIndex].nextCity][cityIndex].pheromone = (1 - RHO) *  pheromoneActual + RHO * PHEROMONE_INIT_VALUE;
            mu.unlock();
            ants[antIndex].actualCity = ants[antIndex].nextCity ;
        }
    }
}

void evaluate() {

    // Create ants = threads
    std::thread threads[ANT_COUNT];
    for(int antIndex = 0; antIndex < ANT_COUNT; antIndex++) {
        threads[antIndex] = thread(evaluateAnt, antIndex);
    }

    // Wait until all of ants ends their route
    for(int antIndex = 0; antIndex < ANT_COUNT; antIndex++) {
        threads[antIndex].join();
    }

    // Update cost of ant path
    for(int k = 0; k < ANT_COUNT; k++){
        if(ants[k].active){
            ants[k].cost = 0;
            for(uint16_t j=0; j<ants[k].path.size() ;j++){
                ants[k].cost += ants[k].path[j].price;
            }
        }
    }

    // Find best solution across ants
    int bestIndex = -1;
    for(int k = 0; k < ANT_COUNT; k++){
        if(ants[k].active){
            if(bestIndex == -1 || ants[k].cost < ants[bestIndex].cost ){
                bestIndex=k;
            }
        }
    }

    if(ants[bestIndex].cost < bestCost) {
        bestCost = ants[bestIndex].cost;
        bestPath = ants[bestIndex].path;
    }

    // If best cost is zero, that means no route was found
    if(bestCost != 0) {

        // Update global pheromones
        vector<Flight>::const_iterator flight_it;
        for( flight_it=bestPath.begin(); flight_it!=bestPath.end(); flight_it++) {
            float pheromoneActual = citiesMap[flight_it->source][flight_it->destination][flight_it->departure].pheromone;
            citiesMap[flight_it->source][flight_it->destination][flight_it->departure].pheromone = ( 1 - ALPHA ) * pheromoneActual + (1 / bestCost);
        }
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    // Register signal SIGTERM and signal handler
    if (signal(SIGTERM, sig_handler) == SIG_ERR) {
        printf("\nCan't catch SIGINT\n");
    }

    try{
        readCsv();
    } catch(exception& e){
        cout << e.what() << endl;
    }

    init();

    // Infinite loop - SIGTERM will end this program
    while(true){
        initAnts();
        evaluate();
        printResults();
    }

    return 0;
}

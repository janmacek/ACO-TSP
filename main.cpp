#include <csignal>
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>

#include "main.hpp"

using namespace std;

map<string, map <string , map <uint16_t , Flight> > > citiesMap;
string start = "";
int bestCost = UINT16_MAX;
vector<Flight> bestPath;
vector<Ant> ants;
int cityCount = 0;
set<string> tmpCities;

//TODO zmazat nakonci
int lostAnts = 0;


void printResults() {
    cout << bestCost <<endl;
    vector<Flight>::iterator it;
    for(it = bestPath.begin(); it != bestPath.end();it++){
        cout << it->source <<" "<<it->destination <<" "<< it->departure<<" "<< it->price << endl;
    }

}

void sig_handler(int signal) {
    if (signal == SIGINT){
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

void initAnts(){
    //Initialization of ants
    ants.clear();
    for(int i=0; i<ANT_COUNT; i++){
        Ant tmpAnt;

        tmpAnt.actualCity = start;
        tmpAnt.nextCity = "";
        tmpAnt.cost = UINT16_MAX;
        tmpAnt.nonVisitedCities = tmpCities;
        tmpAnt.active = true;

        ants.push_back(tmpAnt);
    }
}

void init(){

    map<string, map <string , map <uint16_t , Flight > > >::const_iterator its;
    for( its=citiesMap.begin(); its!=citiesMap.end(); its++) {
        if(its->first != start) {
            tmpCities.insert(its->first);
        }
    }

    initAnts();
}



string getNextCity(string source, uint16_t departure, set<string> nonVisitedCities){

    vector<string> availableCities;
    vector<float> availableCitiesRating;
    float availableCitiesRatingSum = 0;

    // Count probability of choosing for all available cities = not yet visited
    set<string>::const_iterator itdes;
    for( itdes=nonVisitedCities.begin(); itdes != nonVisitedCities.end(); itdes++) {

        float cityRating = citiesMap[source][*itdes][departure].pheromone * (1 / pow(citiesMap[source][*itdes][departure].price, BETA));
        availableCities.push_back(*itdes);
        availableCitiesRating.push_back(cityRating);
        availableCitiesRatingSum += cityRating;
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

void evaluate() {

    for(int i = 0; i < cityCount; i++) {
        if(i < (cityCount -1) ) {

            // Find next city for all ants
            for(int k = 0; k < ANT_COUNT; k++){

                // Ant is lost ?
                if(ants[k].active){
                    ants[k].nextCity = getNextCity(ants[k].actualCity,i,ants[k].nonVisitedCities);
                    // Ant is lost ?
                    if(ants[k].nextCity.compare("") == 0){
                        ants[k].active=false;
                    }else{
                        ants[k].nonVisitedCities.erase(ants[k].nextCity);
                        ants[k].path.push_back(citiesMap[ants[k].actualCity][ants[k].nextCity][i]);
                    }
                }
            }
        } else {

            // Return all ants to start city
            for(int k = 0; k < ANT_COUNT; k++){

                // Ant is not lost ?
                if(ants[k].active){
                    ants[k].nextCity = start;
                    if(citiesMap[ants[k].actualCity][ants[k].nextCity][i].source.compare("") != 0){
                        ants[k].path.push_back(citiesMap[ants[k].actualCity][ants[k].nextCity][i]);
                    }else{
                        ants[k].active=false;
                    }


                }

            }
        }

        // Local update pheromone and and position
        for(int k = 0; k < ANT_COUNT; k++){

            //Ant is not lost ?
            if(ants[k].active){
                float pheromoneActual = citiesMap[ants[k].actualCity][ants[k].nextCity][i].pheromone;
                citiesMap[ants[k].actualCity][ants[k].nextCity][i].pheromone = (1 - RHO) *  pheromoneActual + RHO * PHEROMONE_INIT_VALUE;
                ants[k].actualCity = ants[k].nextCity ;
            }
        }
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
    int bestIndex = 0;
    for(int k = 0; k < ANT_COUNT; k++){
        if(ants[k].active){
            if(ants[k].cost < ants[bestIndex].cost ){
                bestIndex=k;
            }
        }
    }

    if(ants[bestIndex].cost < bestCost) {
        bestCost = ants[bestIndex].cost;
        bestPath = ants[bestIndex].path;
    }

    // Update global pheromones
    vector<Flight>::const_iterator flight_it;
    for( flight_it=bestPath.begin(); flight_it!=bestPath.end(); flight_it++) {
        float pheromoneActual = citiesMap[flight_it->source][flight_it->destination][flight_it->departure].pheromone;
        citiesMap[flight_it->source][flight_it->destination][flight_it->departure].pheromone = ( 1 - ALPHA ) * pheromoneActual + (1 / bestCost);
    }
}

int main(int argc, char **argv) {

    srand(time(NULL));

    // register signal SIGINT and signal handler
    if (signal(SIGINT, sig_handler) == SIG_ERR)
      printf("\nCan't catch SIGINT\n");

    try{
        readCsv();
    } catch(exception& e){
        cout << e.what() << endl;
    }



    init();

    cout << "Startujem z: " << start << endl;

    //Infinite loop
    while(true){
        initAnts();
        evaluate();
        printResults();
    }
    //printCities();
    return 0;
}

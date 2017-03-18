#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <map>
#include <list>

#include "main.hpp"

using namespace std;


struct MissingColumnException : public exception {
    const char * what () const throw () {
        return "Malformed line in CSV input. There was expected another column.";
    }
};

map<string, map <string , map <uint16_t , Flight> > > citiesMap;
string start = "";
int bestCost;
vector<Flight> bestPath;
vector<Ant> ants;
int cityCount = 0;
int antCount = 0;
int pheromoneInitValue = 0;
float alpha = 0;
float rho = 0;


void printResults() {
    cout << bestCost <<endl;
    vector<Flight>::iterator it;
    for(it = bestPath.begin(); it != bestPath.end();it++){
        cout << it->source <<" "<<it->destination <<" "<< it->departure<<" "<< it->price << endl;
    }
    exit(0);
}

void sig_handler(int signal)
{
  if (signal == SIGINT)
    printResults();
}

void readCsv() {
    std::getline(cin, start);

    for(string line; std::getline(cin, line);) {
        string source, destination, departure, price;
        istringstream iss(line);

        if (!(iss >> source >> destination >> departure >> price)) {
            throw MissingColumnException();
        }

        citiesMap[source][destination][(uint16_t) stoi(departure)] = {source, destination, (uint16_t) stoi(departure), (uint16_t) stoi(price), 0};
    }
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
          ////Iterate oved vector with flight
          Flight flight = citiesMap[its->first][itdes->first][itdep->first];
          cout << "{source: "<<flight.source << ", destination: " << flight.destination << ", price: " << flight.price << ", departure: " << flight.departure << ", pheromone: " << flight.pheromone << "}" << endl;


        }
      }
    }
}

void init(){

  //TODO inicializovat spravne premenne
  antCount = 1;
  pheromoneInitValue = 1;
  alpha = 0.1;
  rho = 0.1;
  cityCount = citiesMap.size();


  set<string> tmpCities ;

  map<string, map <string , map <uint16_t , Flight > > >::const_iterator its;
  //Initialize edges with pheromone
  for( its=citiesMap.begin(); its!=citiesMap.end(); its++) {

    map <string , map <uint16_t , Flight>  >::const_iterator itdes;
    //Iterate oved map key=destination
    for( itdes=citiesMap[its->first].begin(); itdes!=citiesMap[its->first].end(); itdes++) {

      map <uint16_t , Flight>  ::const_iterator itdep;
      //Iterate oved map key=departure
      for( itdep=citiesMap[its->first][itdes->first].begin(); itdep!=citiesMap[its->first][itdes->first].end(); itdep++) {
        citiesMap[its->first][itdes->first][itdep->first].pheromone = pheromoneInitValue;
      }
    }

    tmpCities.insert(its->first);
  }


  tmpCities.erase(start);

  //Initialize ants
  for(int i=0; i<antCount; i++){
    Ant tmpAnt;
    tmpAnt.actualCity = start;
    tmpAnt.nextCity = "";
    tmpAnt.cost = 0;
    tmpAnt.nonVisitedCities = tmpCities;
    ants.push_back(tmpAnt);

  }

}

string getNextCity(string source, uint16_t departure,set<string> nonVisitedCities){

  vector<Flight> availableCities;

  set<string>::const_iterator itdes;
  //Iterate oved map key=destination
  for( itdes=nonVisitedCities.begin(); itdes!=nonVisitedCities.end(); itdes++) {
    availableCities.push_back(citiesMap[source][*itdes][departure]);
  }


//Helper print 
  for (int i=0; i<availableCities.size();i++){
    cout << availableCities[i].source <<"|"<< availableCities[i].destination << endl;
  }
  if(!availableCities.empty()){
    //TODO vyber naj mesta;
    return "";
  }else{
    return "";
  }


}

void evaluate() {

  //Infinite loop
    while(true){
      for(int i=0; i<cityCount; i++){
            if(i < cityCount){
              //Find next city for all ants
              for(int k = 0; k < antCount; k++){
                //TODO co ak nemoze dalsie mesto
                ants[k].nextCity = getNextCity(ants[k].actualCity,i,ants[k].nonVisitedCities);
                ants[k].nonVisitedCities.erase(ants[k].nextCity);
                ants[k].path[i] = citiesMap[ants[k].actualCity][ants[k].nextCity][i];
              }
            }else{
              //Return all ants to start city
              for(int k = 0; k < antCount; k++){
                ants[k].nextCity = start;
                ants[k].path[i] = citiesMap[ants[k].actualCity][ants[k].nextCity][i];
              }
            }
            //Local update pheromone and and position
            for(int k = 0; k < antCount; k++){

              float pheromoneActual = citiesMap[ants[k].actualCity][ants[k].nextCity][i].pheromone;
              citiesMap[ants[k].actualCity][ants[k].nextCity][i].pheromone = (1 - rho)* pheromoneActual + rho*pheromoneInitValue;
              ants[k].actualCity = ants[k].nextCity ;
            }
      }

      //Update cost of ant path
      for(int k = 0; k < antCount; k++){
          ants[k].cost = 0;
          for(int j=0; j<ants[k].path.size() ;j++){
            ants[k].cost += ants[k].path[j].price;
          }
      }

      //Compute cost of ants tour
      for(int k = 0; k < antCount;k++){
            ants[k].cost = 0;
            for(int j=0; j<ants[k].path.size() ;j++){
                ants[k].cost += ants[k].path[j].price;
            }
      }

      //Get actual best path
      int bestIndex = 0;
      for(int k = 0; k < antCount; k++){
          if(ants[k].cost > ants[bestIndex].cost){
            bestIndex=k;
          }
      }
      bestCost = ants[bestIndex].cost;
      bestPath = ants[bestIndex].path;

      //Update global pheromones
      map<string, map <string , map <uint16_t , Flight > > >::const_iterator its;
      //Initialize edges with pheromone
      for( its=citiesMap.begin(); its!=citiesMap.end(); its++) {
        map <string , map <uint16_t , Flight>  >::const_iterator itdes;
        //Iterate oved map key=destination
        for( itdes=citiesMap[its->first].begin(); itdes!=citiesMap[its->first].end(); itdes++) {

          map <uint16_t , Flight>  ::const_iterator itdep;
          //Iterate oved map key=departure
          for( itdep=citiesMap[its->first][itdes->first].begin(); itdep!=citiesMap[its->first][itdes->first].end(); itdep++) {
            float pheromoneActual = citiesMap[its->first][itdes->first][itdep->first].pheromone;
            citiesMap[its->first][itdes->first][itdep->first].pheromone = (1-alpha) * pheromoneActual + alpha * (1/bestCost);
          }
        }

      }

    }
}

int main(int argc, char **argv) {

    if (signal(SIGINT, sig_handler) == SIG_ERR)
      printf("\nCan't catch SIGINT\n");

    try{
        readCsv();
    } catch(exception& e){
        cout << e.what() << endl;
    }

    init();
    evaluate();
    //printCities();
    return 0;
}

#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <map>

#include "main.hpp"

using namespace std;


struct MissingColumnException : public exception {
    const char * what () const throw () {
        return "Malformed line in CSV input. There was expected another column.";
    }
};

const int COUNT_OF_ANTS = 50;

map<string, vector<Flight> > cities;
string start;

void readCsv() {
    std::getline(cin, start);

    for(string line; std::getline(cin, line);) {
        string source, destination, departure, price;
        istringstream iss(line);

        if (!(iss >> source >> destination >> departure >> price)) {
            throw MissingColumnException();
        }

        cities[source].push_back({destination, (uint16_t) stoi(departure), (uint16_t) stoi(price), 0});
    }
}

void printCities() {
    map<string, vector<Flight> >::iterator it;
    for(it = cities.begin(); it != cities.end(); it++ ) {
        cout << it->first << ":" << endl;
        for (vector<Flight>::iterator itv = it->second.begin() ; itv != it->second.end(); ++itv) {
            cout << "{ destination: " << itv->destination << ", price: " << itv->price << ", departure: " << itv->departure << ", pheromone: " << itv->pheromone << "}" << endl;
        }
        cout << endl;
    }
}

void evaluate() {
    for(size_t antIndex = 0; antIndex < COUNT_OF_ANTS; antIndex++) {
        // TODO mravce bezia o zivot
    }
}

int main(int argc, char **argv) {

    try{
        readCsv();
    } catch(exception& e){
        cout << e.what() << endl;
    }

    evaluate();
    printCities();
    return 0;
}

#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>

using namespace std;

struct MissingColumnException : public exception {
   const char * what () const throw () {
      return "Malformed line in CSV input. There was expected another column.";
   }
};

void readCsv(){
    std::string start;
    std::getline(std::cin, start);

    for (std::string line; std::getline(std::cin, line);) {
        std::string from, to, departure, price;
        std::istringstream iss(line);

        if (!(iss >> from >> to >> departure >> price)) {
            throw MissingColumnException();
        }
        std::cout << line << std::endl;
    }
}

int main(int argc, char **argv) {

    try{
        readCsv();
    } catch(exception& e){
        std::cout << e.what() << std::endl;
    }
    return 0;
}

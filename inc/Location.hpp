#ifndef LOCATION_HPP
#define LOCATION_HPP

# include "Config.hpp"

class Location {

        public:
        
        std::string                 uri;
        std::string                 root;
        std::string                 index;
        std::string                 methods; // Set to vector or tab
        

        // Pour la phase de développement pré-parsing, on utilisera ces locations :

        // Location *newLoc1 = new Location("/", "./www", "index.html", "GET");
        // Location *newLoc2 = new Location("/", "./www", "page.html", "GET POST");

        Location(std::string _uri, std::string _root, std::string _index, std::string _methods) {
            uri = _uri;
            root = _root;
            index = _index;
            methods = _methods;
        };

        private:
            Location();
};

#endif
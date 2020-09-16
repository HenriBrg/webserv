#ifndef LOCATION_HPP
#define LOCATION_HPP

# include "Config.hpp"

class Location {

        public:
        
        std::string                 uri;
        std::string                 root;
        std::string                 index;
        std::vector<std::string>    methods;
        
        Location(std::string uri, std::string root, std::string index, std::string methods) {
            uri = uri;
            root = root;
            index = index;
            methods = methods;
        };

        private:
            Location();
};

#endif
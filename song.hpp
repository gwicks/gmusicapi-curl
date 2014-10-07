#include <iostream>
#include <string>

class Song
{
    public:
        std::string title,artist,album,year,id,pathuri;
        bool isStream;
        Song(std::string,std::string,std::string,std::string,std::string);
        std::string getID();
        bool operator< (const Song &other) const {
            return title < other.title;
        }

};

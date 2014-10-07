#include "song.hpp"

Song::Song (std::string t,std::string art,std::string alb,std::string yr,std::string i)
{
    title = t;
    artist = art;
    album = alb;
    year = yr;
    id = i;
    pathuri = "";
    isStream = false;
}

std::string Song::getID()
{
    return id;
}


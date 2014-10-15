gmusicapi-curl
==============

A C++ implementation of the Google Music API using libcurl.

Prequisites (Base)
==============
Boost Libraries

Curl Development Libraries

TagLib Libraries

JsonCPP

(For cplayer)

Ncurses Development Libraries

mpg123

Installation/Usage
===================
To use this port, simply drop in the GoogleMusicAPI.cpp, song.cpp, and song.hpp files into your project.

cplayer.cpp is an example application that uses the API and is a good example of how to use it.

Issues
===================

Appears to have heavy issues compiling on FreeBSD for unknown reasons (possibly having to do with the port of JsonCPP)

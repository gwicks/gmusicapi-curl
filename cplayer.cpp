#include <iostream>
#include <dirent.h>
#include <vector>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <curl/curl.h>
#include <curses.h>
#include <menu.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include "GoogleMusicAPI.cpp"

void drawFrame(int,int,int,int);

void songSelect(int,int,Song,std::vector<Song>);

int mainMenu(int lastpos,int lastpg,bool doInit,std::vector<Song> lsongs)
{
    std::vector<Song> testItems;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int maxrows = w.ws_row;
    int maxcols = w.ws_col;
    maxrows = maxrows - 3;
    maxcols = maxcols - 3; 
    if (doInit)
    {
        initscr();
        start_color();
        cbreak();
        noecho();
        intrflush(stdscr, false);
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_WHITE);
        keypad(stdscr, true);
    }
    testItems = lsongs;
    drawFrame(0,0,LINES-1,COLS-1);
    move(1,1);
    printw("NCPlayer");

    int i = 0, ppos = 2;
    if (testItems.size() <= maxrows)
    {
        for (i = 0; i < testItems.size(); i++)
        {
            mvprintw(ppos + i,1,testItems.at(i).title.c_str());
        }
    }
    else
    {
        for (i = 0;i < maxrows; i++)
        {
            mvprintw(ppos + i,1,testItems.at(lastpg+i).title.c_str());
        }
    }
    mvchgat(1,1,COLS-2,A_BOLD,1,NULL);
    mvchgat(lastpos + 2,1,COLS-2,A_NORMAL,2,NULL);
    bool done = false;
    int pos = lastpos;
    int startval = lastpg;
    Song currsong = testItems.at(pos);
    while (!done)
    {
        int ch = getch();
        switch (ch)
        {
            case 24:
                done = true;
                break;
            case '\n':
                done = true;
                songSelect(pos,startval,currsong,testItems); 
                break;
            case KEY_UP:
                if (pos > 0)
                { 
                    mvchgat(pos+2,1,COLS-2,A_NORMAL,1,NULL);
                    pos = pos - 1;
                    currsong = testItems.at(startval + pos);
                    mvchgat(pos+2,1,COLS-2,A_NORMAL,2,NULL);
                }
                break;
            case KEY_DOWN:
                if (pos < testItems.size() - 1 && pos <= maxrows - 2)
                {
                    mvchgat(pos+2,1,COLS-2,A_NORMAL,1,NULL);
                    pos = pos + 1;
                    currsong = testItems.at(startval + pos);
                    mvchgat(pos+2,1,COLS-2,A_NORMAL,2,NULL);
                }
                break;
            case KEY_NPAGE:
                if (startval + pos < testItems.size())
                {
                    startval = startval + 1;
                    char blank = ' ';
                    std::string clearStr = std::string(maxcols,blank);
                    for (i = 0;i < maxrows; i++)
                    {
                        mvprintw(ppos + i,1,clearStr.c_str());
                        mvprintw(ppos + i,1,testItems.at(startval+i).title.c_str());
                        currsong = testItems.at(startval + pos);
                        mvchgat(pos+2,1,COLS-2,A_NORMAL,2,NULL);
                    }
                    refresh();
                }
                break;
            case KEY_PPAGE:
                if (startval > 0)
                {
                    startval = startval - 1;
                    char blank = ' ';
                    std::string clearStr = std::string(maxcols,blank);
                    for (i = 0;i < maxrows; i++)
                    { 
                        mvprintw(ppos + i,1,clearStr.c_str());
                        mvprintw(ppos + i,1,testItems.at(startval+i).title.c_str());
                        currsong = testItems.at(startval + pos);
                        mvchgat(pos+2,1,COLS-2,A_NORMAL,2,NULL);
                    }
                }
                break;

        }
        refresh();
    }
    
    endwin();
    return 0;

}


std::vector<Song> listMPG(char* dirname)
{
    std::vector<Song> retVector;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirname)) != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            std::string tempstr = ent->d_name;
            std::string temppath = dirname + tempstr;
            if (tempstr.size() >= 4 && tempstr.substr(tempstr.size() - 4,4) == ".mp3")
            { 
	    	TagLib::FileRef tempref (temppath.c_str());
	    	TagLib::Tag *temptag = tempref.tag();
           
	    	std::string temptitle = temptag->title().toCString();
		std::string tempartist = temptag->artist().toCString();
		std::string tempalbum = temptag->album().toCString();
		std::string tempyear = boost::lexical_cast<std::string>(temptag->year());

	    	Song tsong = Song(temptitle,tempartist,tempalbum,tempyear,"");
            	tsong.pathuri = temppath;
            	tsong.isStream = false;
                retVector.push_back(tsong);
            }
        }
        closedir (dir);
    } else {
        //printf("Cannot open");
    }

    return retVector;

}

std::string user = "";
std::string password = "";
std::string authToken = "";
std::string xtToken = "";

int main()
{
    std::string doLogin = "y";
    std::vector<Song> songs;
    if (doLogin == "y")
    {
    	std::cout << "Google Email: ";
    	std::cin >> user;
    	std::cout << "Google Password: ";
    	std::cin >> password;
        authToken = getAuthToken(user,password);
        xtToken = getXTCookie(authToken); 
    	songs = getAllSongs(authToken,xtToken);
    }
    mainMenu(0,0,true,songs);
    return 0;
}

void songSelect(int lpos,int pgpos,Song s,std::vector<Song> lastSongs)
{
    erase();
    drawFrame(0,0,LINES-1,COLS-1);
    attron(A_BOLD);
    mvprintw(1,1,"Title: %s",s.title.c_str());
    mvprintw(2,1,"Artist: %s",s.artist.c_str());
    mvprintw(3,1,"Album: %s",s.album.c_str());
    mvprintw(4,1,"Year: %s",s.year.c_str());
    attroff(A_BOLD);
    mvprintw(5,1,"<Play>");
    mvprintw(5,9,"<Back>");
    bool isDone = false;
    int mpos = 0;
    mvchgat(5,1,6,A_NORMAL,2,NULL);
    while (!isDone)
    {
        int ch = getch();
        switch(ch)
        {
            case '\n':
                if (mpos == 0)
                {
                    std::string cmd = "";
                    if (s.isStream)
                    {
                        attron(A_BOLD);
                        mvprintw(6,1,"(Now Playing: Press Ctrl-C to Stop; Left and Right Arrows to Seek; Space to Pause)");
                        attroff(A_BOLD);
                        refresh();
                        playSong(getStreamURL(authToken,s.id));
                        mvprintw(6,1,"                      ");
                        refresh();
                    }
                    else
                    {
                        cmd = "mpg123 -q -C '" + s.pathuri + "' > /dev/null/ 2&>1";
                        mvprintw(6,1,"Playing...");
                        refresh();
                        system(cmd.c_str());
                        mvprintw(6,1,"                      ");
                        refresh();
                    }
                }
                else
                {
                    isDone = true;
                }
                break;
            case KEY_LEFT:
                if (mpos == 0)
                {
                    mpos = 1;
                    mvchgat(5,1,6,A_NORMAL,1,NULL);
                    mvchgat(5,9,6,A_NORMAL,2,NULL);
                }
                else
                {
                    mpos = 0;
                    mvchgat(5,1,6,A_NORMAL,2,NULL);
                    mvchgat(5,9,6,A_NORMAL,1,NULL);
                }
                break;
            case KEY_RIGHT:
                if (mpos == 0)
                {
                    mpos = 1;
                    mvchgat(5,1,6,A_NORMAL,1,NULL);
                    mvchgat(5,9,6,A_NORMAL,2,NULL);
                }
                else
                {
                    mpos = 0;
                    mvchgat(5,1,6,A_NORMAL,2,NULL);
                    mvchgat(5,9,6,A_NORMAL,1,NULL);
                }
                break;

        }
        refresh();
    }
    erase();
    mainMenu(lpos,pgpos,false,lastSongs);
} 

void drawFrame(int startrow, int startcol, int endrow, int endcol) {
   int saverow, savecol;
   getyx(stdscr,saverow,savecol);
   mvaddch(startrow,startcol,ACS_ULCORNER);
   for (int i = startcol + 1; i < endcol; i++)
      addch(ACS_HLINE);
   addch(ACS_URCORNER);
   for (int i = startrow +1; i < endrow; i++) {
      mvaddch(i,startcol,ACS_VLINE);
      mvaddch(i,endcol,ACS_VLINE);
   }
   mvaddch(endrow,startcol,ACS_LLCORNER);
   for (int i = startcol + 1; i < endcol; i++)
      addch(ACS_HLINE);
   addch(ACS_LRCORNER);
   move(saverow,savecol);
   refresh();
}



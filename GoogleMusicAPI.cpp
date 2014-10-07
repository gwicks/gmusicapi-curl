#include <stdio.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>

#include "song.cpp"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{ 
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


std::string getAuthToken(std::string uname,std::string pass)
{
    CURL *curl;
    CURLcode res;
    static std::string readBuffer;
    
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_global_init(CURL_GLOBAL_ALL);


    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/accounts/ClientLogin");
        readBuffer.clear();
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA, &readBuffer);
        
        curl_easy_setopt(curl,CURLOPT_POST, 1);
        std::string postData = "Email=" + uname + "&Passwd=" + pass + "&service=sj";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str()); 
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.dat");
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_formfree(formpost);

    }
    std::vector<std::string> authArr;
    boost::split(authArr,readBuffer,boost::is_any_of("\n"));
    std::string authKey = authArr[2]; 
    
    boost::replace_first(authKey,"Auth=","");

    return authKey;
}

std::string getXTCookie(std::string authToken)
{

    CURL *curlx;
    CURLcode resx;
    static std::string readBufferx;
    
    struct curl_httppost *formpostx = NULL;
    struct curl_httppost *lastptrx = NULL;

    std::string headerText = "Authorization: GoogleLogin auth=" + authToken;

    const char *header = headerText.c_str();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers,header);

    struct curl_slist *cookies;

    curl_global_init(CURL_GLOBAL_ALL);


    curlx = curl_easy_init();
    if (curlx)
    {
        curl_easy_setopt(curlx, CURLOPT_URL, "https://play.google.com/music/listen?u=0");
        curl_easy_setopt(curlx, CURLOPT_HTTPHEADER, headers);
        readBufferx.clear();
        curl_easy_setopt(curlx,CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curlx,CURLOPT_WRITEDATA, &readBufferx);
        
        curl_easy_setopt(curlx,CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curlx, CURLOPT_COOKIEJAR, "cookies.dat");
        
        resx = curl_easy_perform(curlx);


        curl_formfree(formpostx);

    }

    CURLcode r = curl_easy_getinfo(curlx, CURLINFO_COOKIELIST,&cookies);
    struct curl_slist *nc;
    int i;

    nc = cookies, i = 1;

    std::string rawXT;

    while (nc)
    {
        std::string currentData = nc->data;
        if (boost::find_first(currentData,"xt"))
        {
            rawXT = currentData;
        }
        nc = nc->next;
        i++;
    }
    std::vector<std::string> parsedCookie;
    boost::split(parsedCookie,rawXT,boost::is_any_of("\t"));

    std::string xtToken = parsedCookie[6];

    curl_easy_cleanup(curlx);

    return xtToken;
}


std::vector<Song> getAllSongs(std::string authToken,std::string xtToken)
{

    CURL *curlt;
    CURLcode rest;
    static std::string readBuffert;
    
    struct curl_httppost *formpostt = NULL;
    struct curl_httppost *lastptrt = NULL;

    std::string headerTextt = "Authorization: GoogleLogin auth=" + authToken;

    const char *headert = headerTextt.c_str();

    struct curl_slist *headerst = NULL;
    headerst = curl_slist_append(headerst,headert);


    curl_global_init(CURL_GLOBAL_ALL);


    curlt = curl_easy_init();
    if (curlt)
    {
        std::string argURL = "https://play.google.com/music/services/streamingloadalltracks?u=0&xt=" + xtToken + "==&format=jsarray";
        curl_easy_setopt(curlt, CURLOPT_URL, argURL.c_str());
        curl_easy_setopt(curlt, CURLOPT_HTTPHEADER, headerst);
        readBuffert.clear();
        curl_easy_setopt(curlt,CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curlt,CURLOPT_WRITEDATA, &readBuffert);
        
        curl_easy_setopt(curlt,CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curlt, CURLOPT_COOKIEJAR, "cookies.dat");
        
        rest = curl_easy_perform(curlt);

        /* always cleanup */
        curl_easy_cleanup(curlt);

        curl_formfree(formpostt);

    }

    std::string songData = readBuffert;

    std::size_t found = songData.find("<script type='text/javascript'>\nwindow.parent['slat_process']([");

    songData = songData.substr(found+63);
    std::size_t foundEnd = songData.find(");\nwindow.parent['slat_progress']");



    songData = songData.substr(0,foundEnd);


    while (boost::find_first(songData,",,"))
    {
        boost::replace_all(songData,",,",",");
    }

    int index = songData.rfind("]\n,");
    songData = songData.substr(0,index);
    songData = "{\"playlist\":" + songData + "]}";


    Json::Value root;
    Json::Reader reader;
    bool parsedSuccess = reader.parse(songData,root,false);

    if (not parsedSuccess)
    {
        std::cout << reader.getFormatedErrorMessages() << std::endl;
    }

    const Json::Value playlist = root["playlist"];

    std::vector<Song> songArray;

    for (unsigned int index=0; index < playlist.size() - 1;++index)
    {
        Json::Value currv = playlist[index];
        int s = 0;
        
        Song tempS ("","","","",""); 
        tempS.title = currv[s+1].asString();
        tempS.artist = currv[s+3].asString();
        tempS.album = currv[s+4].asString();
        if (currv[s+12].asInt() > 1000)
        {
            tempS.year = std::to_string(currv[s+12].asInt());
        }
        else
        {
            tempS.year = "????";
        }
        tempS.id = currv[s].asString();
        tempS.isStream = true; 

        songArray.push_back(tempS);

    }
    std::sort(songArray.begin(), songArray.end());
    return songArray;
}

std::string getStreamURL(std::string authToken,std::string id)
{

    CURL *curlu;
    CURLcode resu;
    static std::string readBufferu;
    
    struct curl_httppost *formpostu = NULL;
    struct curl_httppost *lastptru = NULL;

    std::string headerTextu = "Authorization: GoogleLogin auth=" + authToken;

    const char *headeru = headerTextu.c_str();

    struct curl_slist *headersu = NULL;
    headersu = curl_slist_append(headersu,headeru);


    curl_global_init(CURL_GLOBAL_ALL);


    curlu = curl_easy_init();
    if (curlu)
    {
        std::string argURLu = "https://play.google.com/music/play?u=0&songid=" + id + "&pt=e";
        curl_easy_setopt(curlu, CURLOPT_URL, argURLu.c_str());
        curl_easy_setopt(curlu, CURLOPT_HTTPHEADER, headersu);
        readBufferu.clear();
        curl_easy_setopt(curlu,CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curlu,CURLOPT_WRITEDATA, &readBufferu);
        
        curl_easy_setopt(curlu,CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curlu, CURLOPT_COOKIEFILE, "cookies.dat");
        
        resu = curl_easy_perform(curlu);

        /* always cleanup */
        curl_easy_cleanup(curlu);

        curl_formfree(formpostu);

    }
    //std::cout << readBufferu << std::endl;
    
    Json::Value root;
    Json::Reader reader;
    bool parsedSuccess = reader.parse(readBufferu,root,false);
     
    const Json::Value url = root["url"];

    std::string finalURL = url.asString();

    boost::replace_first(finalURL, "https", "http");

    return finalURL;
}

void playSong(std::string surl)
{
    std::string cmd =  "mpg123 -q -C '" + surl + "' > /dev/null 2>&1";
    system(cmd.c_str());
}


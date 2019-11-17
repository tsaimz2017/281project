// Project Identifier: 01BD41C3BF016AD7E8B6F837DF18926EC3E83350
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <stdio.h> // isalnum()​
#include <ctype.h> // isalnum()​

#include <string.h>
#include <cstring>

using namespace std;

struct Entry{
    long long tsInt = 0;
    string ts;
    string cat;
    string msg;
    unsigned int entryID = 0;
};

struct sCompare{
    vector<Entry>* logFile;
public:
    explicit sCompare(vector<Entry>* fin) : logFile(fin){}
    bool operator()(const unsigned int left, const unsigned int right) const{

        if ((*logFile)[left].tsInt != (*logFile)[right].tsInt) {
            return ((*logFile)[left].tsInt < (*logFile)[right].tsInt);
        }
        else {
//            const char* s1 = (char*)(*logFile)[left].cat.c_str();
//            const char* s2 = (char*)(*logFile)[right].cat.c_str();
            if (strcasecmp((*logFile)[left].cat.c_str(), (*logFile)[right].cat.c_str()) < 0) return true;
            else if (strcasecmp((*logFile)[left].cat.c_str(), (*logFile)[right].cat.c_str()) > 0) return false;
            else { // s1 == s2
                return ((*logFile)[left].entryID < (*logFile)[right].entryID);
            }
        }
    }
};

struct tCompare{
    vector<Entry>* logFile;
public:
    explicit tCompare(vector<Entry>* fin) : logFile(fin){}
    bool operator()(const unsigned int left, const long long val) const{
        return (*logFile)[left].tsInt < val;
    }
    bool operator()(const long long val, const unsigned int left) const{
        return val < (*logFile)[left].tsInt;
    }
};

int main(int argc, char* argv[]){
    std::ios_base::sync_with_stdio(false);

    vector<Entry> logFile;
    unordered_map<string, vector<unsigned int>> catMap;
    unordered_map<string, vector<unsigned int>> keywordMap;
    vector<unsigned int> intersection;

    ifstream infile(argv[1]);
    ifstream cmdfile(argv[3]);
    ofstream outfile(argv[5]);

    Entry temp;


    if (argc < 2){
        cout << "Not enough arg.\n";
        exit(0);
    }
    // need test here
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        cout << "Help message.\n";
        exit(0);
    }

    if (infile.is_open()){
        unsigned int count = 0;
        while(getline(infile, temp.ts, '|')){ // why '' not "" here?
            string ts_str = temp.ts;
            ts_str.erase(2, 1);
            ts_str.erase(4, 1);
            ts_str.erase(6, 1);
            ts_str.erase(8, 1);
            temp.tsInt = stoll(ts_str); // erase semicolon and convert ts string to long long int

            getline(infile, temp.cat, '|');
            getline(infile, temp.msg);

            temp.entryID = count++;
            logFile.emplace_back(temp); // push all entries into vector logFile
        }
        infile.close();
        outfile << count << " entries read\n";
    }
    else cerr << "Error in opening log file!\n";



    deque<unsigned int> excerpt;

    vector<unsigned int> sortedID;

    sortedID.reserve(logFile.size());

    unsigned int k = 0;
    while(k < logFile.size()){
        sortedID.emplace_back(logFile[k].entryID);
        k++;
    }
//    for (int i = 0; i < logFile.size(); i++){
//        sortedID.emplace_back(logFile[i].entryID);
//    }
    sCompare scomp(&logFile);
    sort(sortedID.begin(), sortedID.end(), scomp); // sortedID vector stores every entry's origin location (entryID) in logFile

    string catStr;
    vector<string> keywords;
    for (unsigned int i = 0; i < sortedID.size(); i++){
        catStr = logFile[sortedID[i]].cat;
        transform(catStr.begin(), catStr.end(), catStr.begin(), ::tolower);
        catMap[catStr].emplace_back(i); // catMap stores every entry's <category, location in sortedID vector> pair (location after sorted)
        // just use it to 'c'

        /* put keywords in category into keywordMap */
        keywords.clear();
        unsigned int length = 0;
        string noPunc;
        string input = logFile[sortedID[i]].cat;
        for (unsigned int j = 0; j < input.length(); j++){
            input[j] = (char)tolower(input[j]);
            if (isalnum(input[j])){
                length++;
            }
            else {
                if (length){
                    noPunc = input.substr(j - length, length);
                    keywords.emplace_back(noPunc);
                    length = 0;
                }
            }

            if ((j == (input.length() - 1)) && length){
                noPunc = input.substr(input.length() - length, length);
                keywords.emplace_back(noPunc);
            }
        }

        for (const auto &it : keywords){
            if (keywordMap[it].empty())
                keywordMap[it].emplace_back(i); // i: the keyword "it" show up in ... ith entry in the sortedID vector
            else if (keywordMap[it].back() != i) // show up several times in one entry will only be counted once
                keywordMap[it].emplace_back(i);
        }



        /* put keywords in message into keywordMap */
        keywords.clear();
        length = 0;
        noPunc = "";
        input = logFile[sortedID[i]].msg;
        for (unsigned int j = 0; j < input.length(); j++){
            input[j] = (char)tolower(input[j]);
            if (isalnum(input[j])){
                length++;
            }
            else {
                if (length){
                    noPunc = input.substr(j - length, length);
                    keywords.emplace_back(noPunc);
                    length = 0;
                }
            }
            if ((j == (input.length() - 1)) && length){
                noPunc = input.substr(input.length() - length, length);
                keywords.emplace_back(noPunc);
            }
        }

        for (const auto &it : keywords){
            if (keywordMap[it].empty())
                keywordMap[it].emplace_back(i); // i: the keyword "it" show up in ... ith entry in the sortedID vector
            else if (keywordMap[it].back() != i) // show up several times in one entry will only be counted once
                keywordMap[it].emplace_back(i);
        }

    }
    keywords.clear();


    /* command file processing */
    bool isTs = false;
    bool isCat = false;
    bool isKwd = false;
    bool isMatch = false;

    vector<unsigned int>::iterator lowIt, upIt;
    string givenStr;



    char cmd;
    do{
        outfile << "% " ;
        cmdfile >> cmd;
        /* PROCESS COMMAND */
        switch(cmd){
            case 'a':
            {
                unsigned int appID;
                cmdfile.get();
                cmdfile >> appID;
                if (appID < sortedID.size()){
                    auto it = find(sortedID.begin(), sortedID.end(), appID);
                    auto target = (unsigned int)(it - sortedID.begin());
                    excerpt.emplace_back(target); // store "location in the sortedID vector" into excerpt list
                    outfile << "log entry " << appID << " appended\n";
                }
                else{
                    cerr << "Exceeded entryID size!" << "\n";
                }
                break;
            }
            case 't':
            {
                cmdfile.get();
                string lower_str, upper_str;
                getline(cmdfile, lower_str, '|');
                cmdfile >> upper_str;

                if (lower_str.length() == 14 && upper_str.length() == 14){
                    isTs = true;
                    isCat = false;
                    isKwd = false;
                    isMatch = false;

                    lower_str.erase(2, 1);
                    lower_str.erase(4, 1);
                    lower_str.erase(6, 1);
                    lower_str.erase(8, 1);

                    upper_str.erase(2, 1);
                    upper_str.erase(4, 1);
                    upper_str.erase(6, 1);
                    upper_str.erase(8, 1);

                    long long lower, upper;
                    lower = stoll(lower_str);
                    upper = stoll(upper_str); // lower, upper is the timestamp bound in long long int

                    tCompare tcom(&logFile);
                    lowIt = lower_bound (sortedID.begin(), sortedID.end(), lower, tcom);
                    upIt = upper_bound(sortedID.begin(), sortedID.end(), upper, tcom);


//                    int seelow = (lowIt - sortedID.begin());
//                    int seeup = (upIt - sortedID.begin());
//                    std::cout << "lower_bound at position " << seelow << '\n';
//                    std::cout << "upper_bound at position " << seeup << '\n';
                    outfile << "Timestamps search: " << upIt - lowIt << " entries found\n";
                }
                else{
                    cerr << "Invalid timestamp interval!\n";
                }
                break;
            }

            case 'm':
            {
                isMatch = true;
                isCat = false;
                isKwd = false;
                isTs = false;

                cmdfile.get();
                string givenTs_str;
                cmdfile >> givenTs_str;

                givenTs_str.erase(2, 1);
                givenTs_str.erase(4, 1);
                givenTs_str.erase(6, 1);
                givenTs_str.erase(8, 1);

                long long givenTs = stoll(givenTs_str);
//                vector<int>::iterator lowIt,upIt;
                tCompare tcom(&logFile);
                lowIt = lower_bound (sortedID.begin(), sortedID.end(), givenTs, tcom);
                upIt = upper_bound(sortedID.begin(), sortedID.end(), givenTs, tcom);
                outfile << "Timestamp search: " << upIt - lowIt << " entries found\n";
                break;
            }

            case 'c':
            {
                isCat = true;
                isTs = false;
                isKwd = false;
                isMatch = false;

                cmdfile.get();
                getline(cmdfile, givenStr);
                transform(givenStr.begin(), givenStr.end(), givenStr.begin(), ::tolower);
                outfile << "Category search: " << catMap[givenStr].size() << " entries found\n";
                break;
            }

            case 'k':
            {
                isKwd = true;
                isCat = false;
                isTs = false;
                isMatch = false;

                cmdfile.get();
                getline(cmdfile, givenStr);

                bool containAll = true;
                unsigned int length = 0;
                string s;

                for (unsigned int i = 0; i < givenStr.length(); i++){
                    givenStr[i] = (char)tolower(givenStr[i]);
                    if (isalnum(givenStr[i])){
                        length++;
                    }
                    else{
                        if(length){
                            s = givenStr.substr(i - length, length);
                            keywords.emplace_back(s);
                            length = 0;
                        }
                    }
                    if ((i == (givenStr.length() - 1)) && length){
                        keywords.emplace_back(givenStr.substr(givenStr.length() - length, length));
                    }
                }

                // for every word in the given "keywords" string, iterate through the keywordMap
                // if any one does not appear in the keywordMap, then 0 entries found.
                for (const auto &it : keywords) {
                    if (keywordMap.find(it) == keywordMap.end()) {
                        containAll = false;
                    }
                }

                if (!containAll){ // if any keyword has not appeared in the keywordMap
                    outfile << "Keyword search: 0 entries found\n";
                }
                else{ // all seperate keywords have appeared in the keywordMap
                    intersection = keywordMap[keywords[0]]; // all entries in the sortedID vector that contain keywords[0]
                    vector<unsigned int> result;
                    for(const auto &it : keywords){
                        set_intersection(intersection.begin(), intersection.end(), keywordMap[it].begin(), keywordMap[it].end(), back_inserter(result));
                        intersection = result;
                        result.clear();
                    }
                    outfile << "Keyword search: " << intersection.size() << " entries found\n";
                }

                keywords.clear();

                break;
            }

            case 'r':
            {
                if (isTs){
                    auto cur = (unsigned int)(lowIt - sortedID.begin());
                    auto end = (unsigned int)(upIt - sortedID.begin());
                    while(cur < end){
                        excerpt.emplace_back(cur);
                        cur++;
                    }
                    outfile << upIt - lowIt << " log entries appended\n";
                }
                else if (isMatch){
                    auto cur = (unsigned int)(lowIt - sortedID.begin());
                    auto end = (unsigned int)(upIt - sortedID.begin());
                    while(cur < end){
                        excerpt.emplace_back(cur);
                        cur++;
                    }
                    outfile << upIt - lowIt << " log entries appended\n";
                }
                else if (isCat){
                    excerpt.insert(excerpt.end(), catMap[givenStr].begin(), catMap[givenStr].end());
                    outfile << catMap[givenStr].size() << " log entries appended\n";
                }
                else if (isKwd){
                    excerpt.insert(excerpt.end(), intersection.begin(), intersection.end());
                    outfile << intersection.size() << " log entries appended\n";
                }

                else {
                    cerr << "Invalid command 'r'!\n";
                }
                break;
            }

            case 'p':
            {
//                int idx = 0;
                for (unsigned int i = 0; i < excerpt.size(); i++){
//                    idx = sortedID[excerpt[i]];
                    outfile << i << "|" << sortedID[excerpt[i]] << "|" << logFile[sortedID[excerpt[i]]].ts << "|" <<
                            logFile[sortedID[excerpt[i]]].cat << "|" << logFile[sortedID[excerpt[i]]].msg << "\n";
                }
                break;
            }

                // sort excerpt list
            case 's':
            {
                if (!excerpt.empty()){
                    auto first = sortedID[excerpt[0]]; // entryID
                    auto last = sortedID[excerpt[excerpt.size() - 1]];

                    outfile << "excerpt list sorted\n";
                    outfile << "previous ordering:\n";
                    outfile << 0 << "|" << first << "|" << logFile[first].ts << "|" <<
                            logFile[first].cat << "|" << logFile[first].msg << "\n";
                    outfile << "...\n";
                    outfile << excerpt.size() - 1 << "|" << last << "|" << logFile[last].ts << "|" <<
                            logFile[last].cat << "|" << logFile[last].msg << "\n";

                    sort(excerpt.begin(), excerpt.end());

                    first = sortedID[excerpt[0]]; // entryID
                    last = sortedID[excerpt[excerpt.size() - 1]];
                    outfile << "new ordering:\n";
                    outfile << 0 << "|" << first << "|" << logFile[first].ts << "|" <<
                            logFile[first].cat << "|" << logFile[first].msg << "\n";
                    outfile << "...\n";
                    outfile << excerpt.size() - 1 << "|" << last << "|" << logFile[last].ts << "|" <<
                            logFile[last].cat << "|" << logFile[last].msg << "\n";

                }
                else{
                    outfile << "excerpt list sorted\n";
                    outfile << "(previously empty)\n";
                }
                break;
            }

            case 'g':
            {
                if (isCat){
                    for (auto it : catMap[givenStr]){
                        unsigned int entryID = sortedID[it];
                        outfile << entryID << "|" << logFile[entryID].ts << "|" <<
                                logFile[entryID].cat << "|" << logFile[entryID].msg << "\n";
                    }
                }
                else if (isKwd){
                    for (auto it : intersection){ // TODO intersection
                        unsigned int entryID = sortedID[it];
                        outfile << entryID << "|" << logFile[entryID].ts << "|" <<
                                logFile[entryID].cat << "|" << logFile[entryID].msg << "\n";
                    }
                }
                else if (isMatch){
                    auto cur = lowIt;
                    while(cur != upIt){
                        outfile << *cur << "|" << logFile[*cur].ts << "|" <<
                                logFile[*cur].cat << "|" << logFile[*cur].msg << "\n";
                        ++cur;
                    }
                }
                else if (isTs){
                    auto cur = lowIt;
                    while(cur != upIt){
                        outfile << *cur << "|" << logFile[*cur].ts << "|" <<
                                logFile[*cur].cat << "|" << logFile[*cur].msg << "\n";
                        ++cur;
                    }
                }

                else{ // a previous search never occurred
                    cerr << "Invalid command 'g'!\n";
                }

                break;
            }

            case 'd':
            {
                cmdfile.get();
                unsigned int deleteIdx;
                cmdfile >> deleteIdx;
                if (deleteIdx < excerpt.size()){
                    excerpt.erase(excerpt.begin() + deleteIdx);
                    outfile << "Deleted excerpt list entry " << deleteIdx << "\n";
                }
                else{
                    cerr << "Invalid entryID " << deleteIdx << " to delete!\n";
                }
                break;
            }

            case 'b':
            {
                cmdfile.get();
                unsigned int moveIdx;
                cmdfile >> moveIdx;
                if (moveIdx < excerpt.size()){
                    excerpt.emplace_front(excerpt[moveIdx]);
                    excerpt.erase(excerpt.begin() + moveIdx + 1);
                    outfile << "Moved excerpt list entry " << moveIdx << "\n";
                }
                else {
                    cerr << "Invalid entryID " << moveIdx << " to move to beginning!\n";
                }
                break;
            }

            case 'e':
            {
                cmdfile.get();
                unsigned int moveIdx;
                cmdfile >> moveIdx;
                if (moveIdx < excerpt.size()){
                    excerpt.emplace_back(excerpt[moveIdx]);
                    excerpt.erase(excerpt.begin() + moveIdx);
                    outfile << "Moved excerpt list entry " << moveIdx << "\n";
                }
                else {
                    cerr << "Invalid entryID " << moveIdx << " to move to end!\n";
                }
                break;
            }

            case 'l':
            {
                outfile << "excerpt list cleared\n";
                if (!excerpt.empty()){
                    unsigned int first = sortedID[(excerpt[0])]; // entryID
                    unsigned int last = sortedID[excerpt[excerpt.size() - 1]];

                    outfile << "previous contents:\n";
                    outfile << 0 << "|" << first << "|" << logFile[first].ts << "|" <<
                            logFile[first].cat << "|" << logFile[first].msg << "\n";
                    outfile << "...\n";
                    outfile << excerpt.size() - 1 << "|" << last << "|" << logFile[last].ts << "|" <<
                            logFile[last].cat << "|" << logFile[last].msg << "\n";
                    excerpt.clear();
                }
                else{
                    outfile << "(previously empty)\n";
                }
                break;
            }

            case '#':
            {
                string comment;
                getline(cmdfile, comment);
                break;
            }

            default:
                string junk;
                if (cmd != 'q'){
                    cerr << "Invalid command!\n";
                    getline(cmdfile, junk);
                }
                break;
        }

    } while(cmd != 'q');

    return 0;
}
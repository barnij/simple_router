#include <string>

struct entry{
    std::string ip;
    std::string netmask;
    std::string broadcast;
    std::string via;
    bool direct;
    bool connected;
    int mask;
    int dist;
};
#ifndef SESSIONS_H
#define SESSIONS_H
#include <vector>
#include <string>

class Sessions
{
public:
    Sessions(std::vector<int>& s, std::string conpar="dbname=sqlturor ");
    void exec();

private:
    std::string db_connection;
    std::vector<int> session_ids;
};

#endif // SESSIONS_H

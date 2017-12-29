#include <iostream>
#include <string>
#include "sessions.h"

int main(int argc, char* argv[])
  try
    {
        std::vector<int> session_ids;
        for (int i=1; i<argc; i++) {
            int n = std::stoi(std::string(argv[i]));
            session_ids.push_back(n);
        }
        std::string conpar = "dbname=sqltutor ";

      Sessions sessions(session_ids, conpar);
      sessions.exec();
    }
  catch (const std::exception &e)
    {
      std::cerr << e.what() << std::endl;
    }


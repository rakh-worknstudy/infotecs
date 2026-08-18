#ifndef TEST_H_
#define TEST_H_

#include "lib/logger.h"

#include <functional>
#include <string>

class Test
{
public:
    Test();
    ~Test();

    int run();
private:
    void printToCerr( const std::string func, const std::string msg ) const;
    const std::string getAbsolutePath( const std::string& path ) const;

    int iterateJournals();
    int iterateLevels();
    int iterateWrite();

    Logger* logger;
    std::string journal;
    Logger::Level level;
    Logger::ReturnCode rc;

    int count;
};

#endif  // TEST_H_

#include "test.h"

#include "lib/logger.h"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <functional>
#include <thread>
#include <string>

Test::Test() : logger( nullptr ), journal(), level( Logger::Level::DEFAULT ), rc( Logger::ReturnCode::Ok), count() {}
Test::~Test()
{
    if( nullptr != logger )
    {
        delete logger;
    }
}

void Test::printToCerr( const std::string func, const std::string msg ) const
{
    std::cerr << "[TEST] " << func << ": " << msg << std::endl;
}
const std::string Test::getAbsolutePath( const std::string& path ) const 
{
    const std::string absolutePath( std::filesystem::absolute( std::filesystem::path( path.c_str() ) ) );
    return absolutePath;
}

int Test::run()
{
    count = 0;

    logger = new Logger;
    if( nullptr == logger )
    {
        printToCerr( __func__,  "Logger() is nullptr. Aborting" );
        return -1;
    }
    if( 0 != iterateJournals() )
    {
        printToCerr( __func__, "FAIL on iterateJournals() return. Aborting" );
        return -1;
    }
    std::cout << "*** FINISH TEST ***\nResult: " << count << " errors" << std::endl;
    return 0;
}

int Test::iterateJournals()
{
    static const std::string names[3]
    {
        "test_journal_01.txt",
        "test_journal_02.txt",
        "test_journal_03.txt"
    };

    for( int iter = 0; iter < 3; ++iter )
    {
        if( Logger::ReturnCode::Ok != logger->setJournal( names[iter] ) )
        {
            printToCerr( __func__, "setJournal() failed. Aborting" );
            return -1;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        if( logger->getJournal() != getAbsolutePath( names[iter] ) )
        {
            printToCerr( __func__, "getJournal() is not equal setJournal()" );
            ++count;
        }

        // Do iterateLevels() on journal iterations
        if( 0 != iterateLevels() )
        {
            printToCerr( __func__, "FAIL on iterateLevels() return. Aborting" );
            return -1;
        }
    }
    std::cout << "* Test::iterateJournal(): SUCCESS" << std::endl;
    return 0;
}

int Test::iterateLevels()
{
    for( int iter = Logger::Level::FIRST; iter <= Logger::Level::LAST; ++iter )
    {
        if( Logger::ReturnCode::Ok != logger->setLevel( static_cast< Logger::Level >( iter ) ) )
        {
            printToCerr( __func__, "setLevel() failed. Aborting" );
            return -1;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        if( logger->getLevel() != static_cast< Logger::Level >( iter ) )
        {
            printToCerr( __func__, "getLevel() is not equal setLevel()" );
            ++count;
        }

        // Do iterateWrite() on level iterations
        if( 0 != iterateWrite() )
        {
            printToCerr( __func__, "FAIL on iterateWrite() return. Aborting" );
            return -1;
        }
    }

    std::cout << "  * Test::iterateLevels(): SUCCESS" << std::endl;
    return 0;
}

int Test::iterateWrite()
{
    int retval = -1;
    do
    {
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::DEBUG, "Yeah, science") ) break;
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::INFO, "Roses are red") ) break;
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::NOTICE, "For your concern" ) ) break;
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::WARNING, "You better not do that" ) ) break;
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::ERROR, "Told you not to do that!" ) ) break;
        if( Logger::ReturnCode::Ok != logger->write( Logger::Level::CRITICAL, "It's over..." ) ) break;
        retval = 0;
    } while( false );
    if( 0 != retval )
    {
        printToCerr( __func__, "write() failed. Aborting" );
    }
    else
    {
        std::cout << "    * Test::iterateWrite(): SUCCESS" << std::endl;
    }
    return retval;
}


#include "main.h"

#include <map>
#include <memory>
#include <fstream>
#include <iostream>

namespace
{
    /// @brief Get return code name as a string
    /// @param[in] value Return code
    /// return Return code as a string
    static const std::string returnCodeToString( const Logger::ReturnCode value );
}  // namespace


namespace test
{
    int logLevel( const Logger* logger, const Logger::Level level, const char* msg );
    int logEachLevel( const Logger* logger );
    int iterateEachLevelAndLogEachLevel( Logger*& logger );
}  // namespace test


/// @brief Logger initialization function
/// @param[in] journal Journal path
/// @param[in] level Logging level
/// @param[out] desc Logger descriptor
/// @return ReturnCode::Ok if successful, else - error type
Logger::ReturnCode initLogger( const std::string journal, const Logger::Level level, Logger*& logger )
{
    Logger::ReturnCode retval( Logger::ReturnCode::Fatal );
    auto printToCerr = [ retval ]( void )
    {
        std::cerr << "Unable to init a logger: " << returnCodeToString( retval ) << std::endl;
    };

    if( journal.empty() )
    {
        // No journal path given
        retval = Logger::ReturnCode::JournalUnspecified;
    }
    else if( !Logger::isValidLevel( level ) )
    {
        // Invalid logging level
        retval = Logger::ReturnCode::LevelUnknown;
    }
    else
    {
        // Trying to initialize a journal
        std::string journal( "test.txt" );
        logger = new Logger( journal, level );
        if( nullptr != logger )
        {
            // No fail upon 'new Logger(..)'
            // Get journal opening status
            Logger::ReturnCode journalStatus{ logger->isJournalOpen() };
            if( Logger::ReturnCode::Ok == journalStatus )
            {
                // Successful initalization
                retval = Logger::ReturnCode::Ok;
            }
            else if( Logger::ReturnCode::JournalNoopen == journalStatus )
            {
                // Journal coudln't open
                retval = Logger::ReturnCode::JournalNoopen;
            }
            // else Logger(..) inner failure (ReturnCode::Fatal)
        }
        // else Logger(..) failure (ReturnCode::Fatal);
    }

    if( Logger::ReturnCode::Ok != retval )
    {
        printToCerr();
    }
    return retval;
}

/// @brief Logger closing function
/// Tries to 
/// @param[in] desc To-close logger descriptor
/// @return ReturnCode::Ok if successful, else - error type
Logger::ReturnCode closeLogger( Logger*& logger )
{
    Logger::ReturnCode retval( Logger::ReturnCode::Fatal );
    auto printToCerr = [ retval ]( void )
    {
        std::cerr << "Unable to close the logger: " << returnCodeToString( retval ) << std::endl;
    };

    if( nullptr == logger )
    {
        retval = Logger::ReturnCode::LoggerNullptr;
    }
    else
    {
        logger->write( Logger::Level::DEBUG, "closeLogger(): closing the logger" );
        delete( logger );
        logger = nullptr;
        retval = Logger::ReturnCode::Ok;
    }

    if( Logger::ReturnCode::Ok != retval )
    {
        printToCerr();
    }

    return retval;
}

/// @brief Log by logger descriptor
/// @param[in] desc Logger descriptor
/// @param[in] level Log level
/// @param[in] msg Log message
/// @return ReturnCode::ok if successful, else - error type
Logger::ReturnCode log( const Logger* logger, const Logger::Level level, const std::string msg )
{
    Logger::ReturnCode retval( Logger::ReturnCode::Fatal );
    auto printToCerr = [ retval ]( void )
    {
        std::cerr << "Unable to log: " << returnCodeToString( retval );
    };

    retval = logger->write( level, msg );

    if( Logger::ReturnCode::Ok != retval )
    {
        printToCerr();
    
    }
    return retval;
}


int main( void )
{
    do
    {
        Logger *logger = nullptr;
        if( Logger::ReturnCode::Ok != initLogger( "text.txt", Logger::Level::DEBUG, logger ) ) break;
        if( 0 != test::iterateEachLevelAndLogEachLevel( logger ) ) break;
        if( Logger::ReturnCode::Ok != closeLogger( logger ) ) break;
        return 0;
    } while( false );

    return -1;
}


namespace
{
    static const std::string returnCodeToString( const Logger::ReturnCode returnCode )
    {
        const std::map< Logger::ReturnCode, std::string > returnCodeToStringMap =
        {
            { Logger::ReturnCode::Ok, "Success" },
            { Logger::ReturnCode::JournalUnspecified, "No journal path given" },
            { Logger::ReturnCode::JournalNoopen, "Unable to open a journal" },
            { Logger::ReturnCode::LevelUnknown, "Unknown logging level given" },
            { Logger::ReturnCode::LoggerNullptr, "No logger ptr given" },
            { Logger::ReturnCode::Fatal, "Uknown fatal error" }
        };
        auto search = returnCodeToStringMap.find( returnCode );
        if( returnCodeToStringMap.end() != search )
        {
            return search->second;
        }

        const static std::string unknownReturnCodeString( "Unknown return code" );
        return unknownReturnCodeString;
    }
}


namespace test
{
    int logLevel( const Logger* logger, const Logger::Level level, const char* msg )
    {
        std::cout << "Logging " << Logger::levelToString( level ) << " message" << std::endl;
        if( Logger::ReturnCode::Ok != log( logger, level, msg ) )
        {
            std::cerr << "FAIL" << std::endl;
            return -1;
        }
        return 0;
    }
    int logEachLevel( const Logger* logger )
    {
        do
        {
            if( 0 != logLevel( logger, Logger::Level::DEBUG, "Yeah, science!" ) ) break;
            if( 0 != logLevel( logger, Logger::Level::INFO, "Roses are red" ) ) break;
            if( 0 != logLevel( logger, Logger::Level::NOTICE, "For your concern" ) ) break;
            if( 0 != logLevel( logger, Logger::Level::WARNING, "You better not do that" ) ) break;
            if( 0 != logLevel( logger, Logger::Level::ERROR, "Told you not to do that!" ) ) break;
            if( 0 != logLevel( logger, Logger::Level::CRITICAL, "It's over..." ) ) break;
            return 0;
        } while( 0 );
        return -1;
    }
    int iterateEachLevelAndLogEachLevel( Logger*& logger )
    {
        std::cout << "Trying to iterate all log levels..." << std::endl; 
        for( int level = Logger::Level::FIRST; level <= Logger::Level::LAST; ++level )
        {
            const Logger::Level levelCast{ static_cast< Logger::Level >( level ) };
            std::cout << "Setting log level to " << Logger::levelToString( levelCast ) << std::endl;
            Logger::ReturnCode retval = logger->setLevel( levelCast );
            if( Logger::ReturnCode::Ok != retval )
            {
                std::cerr << "FAIL: " << returnCodeToString( retval );
                return -1;
            }
            const Logger::Level levelGot( logger->getLevel() );
            std::cout << "Got log level: " << Logger::levelToString( levelGot ) << std::endl;
            if( levelCast != levelGot )
            {
                std::cerr << "FAIL: level to setLevel(level) and getLevel() differ" << std::endl;
                return -1;
            }
            if( 0 != logEachLevel( logger ) )
            {
                std::cerr << "FAIL: unable to logEachLevel()" << std::endl;
                return -1;
            }
        }
        std::cout << "Iterating is complete" << std::endl;
        return 0; 
    }
}  // namespace test


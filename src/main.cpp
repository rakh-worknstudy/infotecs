#include "main.h"

#include <map>
#include <memory>
#include <fstream>
#include <iostream>

namespace
{
    const static std::string returnCodeToString( const Logger::ReturnCode value );
}  // namespace


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

    // logger = nullptr;
    if( journal.empty() )
    {
        retval = Logger::ReturnCode::JournalUnspecified;
    }
    else if( !Logger::isValidLevel( level ) )
    {
        retval = Logger::ReturnCode::LevelUnknown;
    }
    else
    {
        std::string journal( "test.txt" );
        logger = new Logger( journal, level );
        if( nullptr != logger )
        {
            if( !logger->isJournalOpen() )
            {
                retval = Logger::ReturnCode::JournalNoopen;
                delete( logger );
            }
            else
            {
                retval = Logger::ReturnCode::Ok;
            }
        }
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
        logger->log( Logger::Level::DEBUG, "Closing the logger" );
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
Logger::ReturnCode log( Logger* logger, const Logger::Level level, const std::string msg )
{
    Logger::ReturnCode retval( Logger::ReturnCode::Fatal );
    auto printToCerr = [ retval ]( void )
    {
        std::cerr << "Unable to log: " << returnCodeToString( retval );
    };

    retval = logger->log( level, msg );

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
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::DEBUG, "Yeah, science!" ) ) break;
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::INFO, "Roses are red" ) ) break;
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::NOTICE, "For your concern" ) ) break;
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::WARNING, "You better not do that" ) ) break;
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::ERROR, "Told you not to do that!" ) ) break;
        if( Logger::ReturnCode::Ok != log( logger, Logger::Level::CRITICAL, "It's over..." ) ) break;
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


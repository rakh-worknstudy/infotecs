#include "main.h"

#include <map>
#include <memory>
#include <fstream>
#include <iostream>

#include "test.h"

namespace
{
    /// @brief Get return code name as a string
    /// @param[in] value Return code
    /// return Return code as a string
    static const std::string returnCodeToString( const Logger::ReturnCode value );
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
Logger::ReturnCode log( Logger* logger, const Logger::Level level, const std::string msg )
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
    Test test;
    return test.run();
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



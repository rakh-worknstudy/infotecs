#include "logger.h"

#include <fstream>
/* debug */ #include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>


Logger::Logger( const std::string journal, const Level level ) : _logFileOut( journal )
{
    _level = level;
    /* debug */ std::cout << "Logger() ok" << std::endl;
}
Logger::Logger() : _logFileOut()
{
    _level = Level::DEFAULT;
}
Logger::~Logger()
{
    if( _logFileOut.is_open() )
    {
        log( Level::INFO, "Closing logger" );
        _logFileOut.close();
    }
}

Logger::ReturnCode Logger::setLevel( const Level level )
{
    const std::string levelStr( levelToString( level ) );
    std::stringstream message;
    if( level == _level )
    {
        message << "Log level is already " << levelStr << "\n";
        log( Level::NOTICE, message.str() );
    }
    else
    {
        message << "Changing log level to " << levelStr << "\n";
        log( Level::NOTICE, message.str() );
        _level = level;
    }
    return ReturnCode::Ok;
}
Logger::Level Logger::getLevel() const
{
    return _level;
}

Logger::ReturnCode Logger::setJournal( const std::string journal )
{
    return ReturnCode::Ok;
}

bool Logger::isJournalOpen() const
{
    return _logFileOut.is_open();
}

Logger::ReturnCode Logger::log( const Level level, const std::string msg )
{
    /* debug */ std::cout << "_logFileOut.is_open() = " << _logFileOut.is_open() << std::endl;
    if( !_logFileOut.is_open() )
    {
        return ReturnCode::JournalUnspecified;
    }
    if( level >= _level )
    {
       std::cout << "trying to write\n";
       _logFileOut << msg << std::endl;
       std::cout <<"SUCCESS" << std::endl;
    }
    return ReturnCode::Ok;
}

/// @brief Check if logging level is valid
/// @param[in] level Logging level
/// @return true - valid, else - false
bool Logger::isValidLevel( const Logger::Level level )
{
    bool isValid = true;
    /* debug */ std::cout << "DEBUG = " << Level::DEBUG << "\nlevel = " << level << "\nCRITICAL = " << Level::CRITICAL << std::endl;
    if( Level::DEBUG > level || Level::CRITICAL < level )
    {
        isValid = false;
    }
    /* debug */ std::cout << "isValid = " << isValid << std::endl;
    return isValid;
}

const std::string& Logger::levelToString( const Level level )
{
    const std::map< Logger::Level, std::string > levelToStringMap =
    {
        { Level::DEBUG, "DEBUG" },
        { Level::INFO, "INFO" },
        { Level::NOTICE, "NOTICE" },
        { Level::WARNING, "WARNING" },
        { Level::ERROR, "ERROR" },
        { Level::CRITICAL, "CRITICAL" }
    };
    auto search = levelToStringMap.find( level );
    if( levelToStringMap.end() != search )
    {
        return search->second;
    }

    const static std::string unknownLevelString( "Unknown logging level" );
    return unknownLevelString;
}


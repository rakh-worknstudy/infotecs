#include "logger.h"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <sstream>

Logger::Logger( std::unique_ptr< std::ofstream > logFileOut , const Level level ) : _logFileOut( std::move( logFileOut) )
{
    _level = level;
}
Logger::Logger( const std::string journal, const Level level )
{
    _logFileOut = std::make_unique< std::ofstream >( journal );
    _level = level;
}
Logger::Logger() : _logFileOut()
{
    _level = Level::DEFAULT;
}
Logger::~Logger()
{
    if( _logFileOut->is_open() )
    {
        log( Level::INFO, "Closing the logger" );
        _logFileOut->close();
    }
}

Logger::ReturnCode Logger::setLevel( const Level level )
{
    log( Level::DEBUG, "setLevel() called" );
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
    log( Level::DEBUG, "getLevel() called" );
    return _level;
}

Logger::ReturnCode Logger::setJournal( const std::string journal )
{
    log( Level::DEBUG, "setJournal() called" );
    return ReturnCode::Ok;
}

bool Logger::isJournalOpen() const
{
    log( Level::DEBUG, "isJournalOpen() called" );
    return _logFileOut->is_open();
}

Logger::ReturnCode Logger::log( const Level level, const std::string msg ) const
{
    if( !_logFileOut->is_open() )
    {
        return ReturnCode::JournalUnspecified;
    }
    if( level >= _level )
    {
        std::stringstream logMsg;
        const std::string levelString( levelToString( level ) );
        logMsg << "< " << levelString << " > " << msg << "\n";
        _logFileOut->write( logMsg.str().c_str(), logMsg.str().length() );
        _logFileOut->flush();
    }
    return ReturnCode::Ok;
}

/// @brief Check if logging level is valid
/// @param[in] level Logging level
/// @return true - valid, else - false
bool Logger::isValidLevel( const Logger::Level level )
{
    bool isValid = true;
    if( Level::DEBUG > level || Level::CRITICAL < level )
    {
        isValid = false;
    }
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


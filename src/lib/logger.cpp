#include "logger.h"

#include <ctime>
#include <fstream>
#include <iomanip>
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
    const std::string levelStr( levelToString( level ) );
    std::ostringstream msgOss;
    msgOss << "setLevel(): changing log level to " << levelStr;
    log( Level::NOTICE, msgOss.str() );
    if( level == _level )
    {
        msgOss.str( std::string() );
        msgOss << "setLevel(): log level is already " << levelStr;
        log( Level::NOTICE, msgOss.str() );
    }
    else
    {
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
        std::ostringstream fullMsgOss;
        const std::string dateTimeString( getDateTimeString() );
        const std::string levelString( levelToString( level ) );
        fullMsgOss << std::left;
        fullMsgOss << dateTimeString;
        fullMsgOss << " [ " << std::setw( levelStringMaxLength() ) << levelString << " ] ";
        fullMsgOss << msg << "\n";
        _logFileOut->write( fullMsgOss.str().c_str(), fullMsgOss.str().length() );
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
    if( Level::BEGIN > level || Level::END < level )
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

    const static std::string unknownLevelString( "UNKNOWN" );
    return unknownLevelString;
}

/// @brief Max possible length for a level string
/// return Max length for a level string
inline constexpr int Logger::levelStringMaxLength()
{
    std::size_t maxLength = 0;
    for( int level = Level::BEGIN; level <= Level::END + 1; ++level )
    {
        const std::size_t currentStringLength{ levelToString( static_cast< Level >( level ) ).length() };
        if( currentStringLength > maxLength )
        {
            maxLength = currentStringLength;
        }
    }
    return static_cast< int >( maxLength );
}

/// @brief Get current date and time string
/// param[out] dateTimeStr Resulting date and time string
const std::string Logger::getDateTimeString()
{
    std::time_t _time;
    std::time( &_time );

    std::ostringstream dateTimeStringOss;
    dateTimeStringOss << std::put_time( std::localtime( &_time ), "%F %T" );
    return dateTimeStringOss.str();
}


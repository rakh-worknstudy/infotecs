#include "logger.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <system_error>
#include <string>
#include <sstream>

/// @brief Main logger constructor
Logger::Logger( const std::string filePath, const Level level ) : _fileOut(), _filePath(), _level( level )
{
    ReturnCode isOk = getAbsolutePath( filePath, _filePath );
    if( ReturnCode::Ok == isOk )
    {
        _fileOut = std::make_unique< std::ofstream >( _filePath.c_str() );
    }
}
Logger::Logger() : _fileOut(), _filePath(), _level( Level::DEFAULT ) {}
Logger::~Logger()
{
    if( ReturnCode::Ok == isJournalOpen() && _level <= Level::NOTICE )
    {
        std::stringstream closeMsgOff;
        closeMsgOff << "~Logger(): closing the journal file " << _filePath;
        write( Level::NOTICE, closeMsgOff.str() );
        _fileOut->close();
    }
}

/// @brief Set new logging level
/// param[in] level New logging level
/// return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setLevel( const Level level )
{
    const std::string levelStr( levelToString( level ) );
    std::ostringstream msgOss;
    msgOss << "setLevel(): changing log level to " << levelStr;
    write( Level::NOTICE, msgOss.str() );
    if( level == _level )
    {
        msgOss.str( std::string() );
        msgOss << "setLevel(): log level is already " << levelStr;
        write( Level::NOTICE, msgOss.str() );
    }
    else
    {
        _level = level;
    }

    return ReturnCode::Ok;
}

/// @brief Get current logging level
/// return Logging level
Logger::Level Logger::getLevel() const
{
    return _level;
}

/// @brief Sets a new journal for the logger
/// param[in] filePath File path to the journal
/// return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setJournal( const std::string filePath )
{
    ReturnCode retval = ReturnCode::Fatal;
    if( filePath.empty() )
    {

    }
    return ReturnCode::Ok;
}

/// @brief Check if journal is currently open
/// return true if file is open, else - false
Logger::ReturnCode Logger::isJournalOpen() const
{
    ReturnCode retval = ReturnCode::Fatal;
    if( !_fileOut )
    {
        retval = ReturnCode::JournalUnspecified;
    }
    else if( !_fileOut->is_open() )
    {
        retval = ReturnCode::JournalNoopen;
    }
    else
    {
        retval = ReturnCode::Ok;
    }
    return retval;
}

/// @brief Main logging function
/// Attempts to log a message of a specified level in format:
/// YYYY-MM-DD HH:MM:SS [ %level%  ] %message%\n
/// param[in] level Level of the logged message
/// param[in] msg The message itself
/// return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::write( const Level level, const std::string msg ) const
{
    ReturnCode retval{ isJournalOpen() };
    if( ReturnCode::Ok == retval )
    {
        if( level >= _level )
        {
            std::ostringstream fullMsgOss;
            const std::string dateTimeString( getDateTimeString() );
            const std::string levelString( levelToString( level ) );
            fullMsgOss << std::left;
            fullMsgOss << dateTimeString;
            fullMsgOss << " [ " << std::setw( levelStringMaxLength() ) << levelString << " ] ";
            fullMsgOss << msg << "\n";
            _fileOut->write( fullMsgOss.str().c_str(), fullMsgOss.str().length() );
            _fileOut->flush();
        }
    }
    return retval;
}

/// @brief Check if logging level is valid
/// @param[in] level Logging level
/// @return true - valid, else - false
bool Logger::isValidLevel( const Logger::Level level )
{
    bool isValid = true;
    if( Level::FIRST > level || Level::LAST < level )
    {
        isValid = false;
    }
    return isValid;
}

/// @brief \"Convert\" logging level to a string
/// @param[in] level Logging level
/// @return Logging level text string
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

/// @brief Get an absolute path of a given path
/// param[in] path Given path
/// param[out] absolutePath Absolute path from given
/// return ReturnCode::Ok if successful, else - error type
Logger::ReturnCode Logger::getAbsolutePath( const std::string& path, std::string& absolutePath )
{
    ReturnCode retval{ ReturnCode::Fatal };
    if( path.empty() )
    {
        absolutePath = "";
        retval = ReturnCode::JournalUnspecified;
    }
    else
    {
        std::error_code error;
        const std::string temp( std::filesystem::absolute( std::filesystem::path( path ).c_str(), error ) );
        if( error.value() )
        {
            absolutePath = "";
            // ReturnCode::Fatal
        }
        else
        {
            absolutePath = temp;
            retval = ReturnCode::Ok;
        }
    }
    return retval;
}
/// @brief Max possible length for a level string
/// return Max length for a level string
inline constexpr int Logger::levelStringMaxLength()
{
    std::size_t maxLength{ 0 };
    for( int level = Level::FIRST; level <= Level::LAST + 1; ++level )
    {
        const std::size_t currentStringLength{ levelToString( static_cast< Logger::Level >( level ) ).length() };
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


#include "logger.h"

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <system_error>
#include <string>
#include <sstream>
#include <thread>
#include <utility>


/// @brief Main logger constructor
Logger::Logger( const std::string filePath, const Level level ) : \
    _fileOut(), _filePath(), _level( level ), _actionQueue(),     \
    _actionQueueMutex(), _fileOutMutex(), _actionCV(),            \
    _actionThread(), _activeJobMutex(), _isThreadActive( false )
{
    ReturnCode isOk = getAbsolutePath( filePath, _filePath );
    if( ReturnCode::Ok == isOk )
    {
        tryOpenJournalImpl();
    }
    setupThread();
}
Logger::Logger() :                                                   \
    _fileOut(), _filePath(),_level( Level::DEFAULT ),_actionQueue(), \
    _actionQueueMutex(), _fileOutMutex(), _actionCV(),               \
    _actionThread(), _isThreadActive( false )
{
    setupThread();
}
Logger::~Logger()
{
    _preventDestructMutex.lock();
    _isThreadActive = false;
    tryCloseJournalImpl();
    _preventDestructMutex.unlock();
    _actionCV.notify_one();
}

/// @brief Set new logging level
/// @param[in] level New logging level
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setLevel( const Level level )
{
    Action levelAction;
    levelAction.type = Action::Type::changeLevel;
    levelAction.data.integer = level;
    return addAction( levelAction );
}

/// @brief Get current logging level
/// @return Logging level
Logger::Level Logger::getLevel() const
{
    return _level;
}

/// @brief Sets a new journal for the logger
/// @param[in] filePath File path to the journal
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setJournal( const std::string filePath )
{
    Action pathAction;
    pathAction.type = Action::Type::changePath;
    pathAction.data.str = filePath;
    return addAction( pathAction );
}
/// @brief Get current journal path
/// @return Journal path
const std::string Logger::getJournal( void )
{
    std::lock_guard< std::mutex > noChangePath( _filePathMutex );
    return _filePath;
}

/// @brief Queue opening a journal
/// @note Is 'public' for cases of manual fix on bad std::ofstream::open()
/// @note Doesn't reopen if already open
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::tryOpenJournal( void )
{
    Action openAction;
    openAction.type = Action::Type::open;
    return addAction( openAction );
}
/// @brief Check if journal is currently open
/// @note Locks _fileOutMutex, avoid deadlocks
/// @return true if file is open, else - false
Logger::ReturnCode Logger::isJournalOpen()
{
    // lock so we don't change the journal state while trying to get ->is_open
    std::lock_guard< std::mutex > noCloseJournal( _fileOutMutex );
    return isJournalOpenImpl();
}

/// @brief Main logging function
/// Attempts to log a message of a specified level in format:
/// YYYY-MM-DD HH:MM:SS [ %level%  ] %message%\n
/// @param[in] level Level of the logged message
/// @param[in] msg The message itself
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::write( const Level level, const std::string msg )
{
    Action writeAction;
    writeAction.type = Action::Type::write;
    writeAction.data.integer = level;
    writeAction.data.str = msg;
    return addAction( writeAction );
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

/// @brief Mutex-less isJournalOpen(), for inner use
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::isJournalOpenImpl( void )
{
    ReturnCode retval{ ReturnCode::Fatal };
    if( !_fileOut )
    {
        if( _filePath.empty() )
        {
            retval = ReturnCode::JournalUnspecified;
        }
        // else ReturnCode::Fatal
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
/// @brief Function to close the journal, for inner use
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::tryOpenJournalImpl( void )
{
    std::lock_guard< std::mutex > openLock( _fileOutMutex );
    ReturnCode retval{ ReturnCode::Fatal };

    ReturnCode journalStatus{ isJournalOpenImpl() };
    if( ReturnCode::Ok == journalStatus )
    {
        writeImpl( NOTICE, "tryOpenJournalImpl(): journal is already open" );
        retval = ReturnCode::Ok;
    }
    else if( ReturnCode::JournalUnspecified == journalStatus )
    {
        writeImpl( WARNING, "tryOpenJournalImpl(): journal path is empty" );
        retval = ReturnCode::JournalUnspecified;
    }
    else
    {
        // Not an error: ofstream wasn't set yet
        if( ReturnCode::Fatal == journalStatus )
        {
            _fileOut = std::make_unique< std::ofstream >( _filePath.c_str() );
        }
        else if( ReturnCode::JournalNoopen == journalStatus )
        {
            _fileOut->open( _filePath.c_str() );
        }

        if( ReturnCode::Ok == isJournalOpenImpl() )
        {
            std::ostringstream openJournalOss;
            openJournalOss << "tryOpenJournalImpl(): opened a journal with level " << levelToString( _level );
            writeImpl( NOTICE, openJournalOss.str() );
            retval = ReturnCode::Ok;
        }
        else
        {
            retval = ReturnCode::JournalNoopen;
        }
    }

    return retval;
}
/// @brief Function to close the journal, for inner use
/// @note Does nothing with _filePath
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::tryCloseJournalImpl( void )
{
    std::lock_guard< std::mutex > closeLock( _fileOutMutex );
    ReturnCode retval{ ReturnCode::Ok };
    if( !_fileOut || !_fileOut->is_open() )
    {
        retval = ReturnCode::JournalNoopen;
    }
    else
    {
        writeImpl( NOTICE, "tryCloseJournalImpl(): closing the journal" );
        _fileOut->close();
    }
    return retval;
}

/// @brief Get an absolute path of a given path
/// @param[in] path Given path
/// @param[out] absolutePath Absolute path from given
/// @return ReturnCode::Ok if successful, else - error type
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

/// @brief Check if it's a filename
/// @return ReturnCode::Ok if it is, else - JournalUnspecified
Logger::ReturnCode Logger::checkPath( const std::string& path )
{
    ReturnCode retval{ ReturnCode::Fatal };
    std::filesystem::path fsPath( path );
    if( !fsPath.has_filename() )
    {
        retval = ReturnCode::JournalUnspecified;
    }
    else
    {
        static const std::string dotAt{ "." };
        static const std::string dotUp{ ".." };
        std::filesystem::path fileName = fsPath.filename();
        if( dotAt == fileName || dotUp == fileName )
        {
            retval = ReturnCode::JournalUnspecified;
        }
        else
        {
            retval = ReturnCode::Ok;
        }
    }
    return retval;
}

/// @brief Create directories
/// @param[in] path Path
void Logger::createDirectories( const std::string& path )
{
    std::filesystem::path fspath( std::filesystem::path( path ).remove_filename() );
    std::filesystem::create_directories( fspath );
}

/// @brief Max possible length for a level string
/// @return Max length for a level string
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
/// @param[out] dateTimeStr Resulting date and time string
const std::string Logger::getDateTimeString()
{
    std::time_t _time;
    std::time( &_time );

    std::ostringstream dateTimeStringOss;
    dateTimeStringOss << std::put_time( std::localtime( &_time ), "%F %T" );
    return dateTimeStringOss.str();
}

/// @brief Queue logging level change function
/// @param[in] level New logging level
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setLevelImpl( const Level level )
{
    ReturnCode retval{ ReturnCode::Fatal };

    const std::string levelStr( levelToString( level ) );
    if( level == _level )
    {
        std::ostringstream sameLevelOss;
        sameLevelOss  << "setLevelImpl(): log level is already " << levelStr;
        retval = writeImpl( Level::NOTICE, sameLevelOss.str() );
    }
    else
    {
        _level = level;
        std::ostringstream changeLevelOss;
        changeLevelOss << "setLevelImpl(): changing log level to " << levelStr;
        retval = writeImpl( Level::NOTICE, changeLevelOss.str() );
    }

    return retval;
}
/// @brief Queue journal path change function
/// @param[in] newPath New journal path
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::setJournalImpl( const std::string& filePath )
{
    ReturnCode retval{ ReturnCode::Fatal };
    std::string temp;
    retval = getAbsolutePath( filePath, temp );
    if( ReturnCode::JournalUnspecified == retval )
    {
        writeImpl( WARNING, "setJournal(): no journal path given" );
    }
    else if( ReturnCode::Ok == retval )
    {
        if( temp == _filePath )
        {
            std::ostringstream samePathMsg;
            samePathMsg << "setJournal(): file path is already " << _filePath;
            writeImpl( INFO, samePathMsg.str() );
            retval = ReturnCode::Ok;
        }
        else if( ReturnCode::Ok != checkPath( temp ) )
        {
            std::ostringstream badPathMsg;
            // log given string, not post-processed
            badPathMsg << "setJournal(): bad path " << filePath;
            writeImpl( WARNING, badPathMsg.str() );
            retval = ReturnCode::JournalUnspecified;
        }
        else
        {
            tryCloseJournalImpl();
            _filePathMutex.lock();
            _filePath = temp;
            _filePathMutex.unlock();
            createDirectories( temp );
            retval = tryOpenJournalImpl();
        }
    }
    else
    {
        writeImpl( ERROR, "setJournal(): bad path" );
        retval = ReturnCode::JournalUnspecified;
    }
    return retval;
}
/// @brief Queue write funcion
/// @param[in] level Level of the logged message
/// @param[in] msg The message itself
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::writeImpl( const Level level, const std::string& msg )
{
    ReturnCode retval{ isJournalOpenImpl() };
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

/// @brief Do single action
/// @param[in] action Action to do
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::doAction( const Action& action )
{
    ReturnCode retval{ ReturnCode::Fatal };
    switch( action.type )
    {
        case Action::Type::write:
        {
            const Level level{ action.data.integer };
            const std::string& msg = action.data.str;
            retval = writeImpl( level, msg );
            break;
        }
        case Action::Type::changeLevel:
        {
            const Level level{ static_cast< Level >( action.data.integer ) };
            retval = setLevelImpl( level );
            break;
        }
        case Action::Type::changePath:
        {
            const std::string filePath = action.data.str;
            retval = setJournalImpl( filePath );
            break;
        }
        case Action::Type::open:
        {
            retval = tryOpenJournalImpl();
            break;
        }
        case Action::Type::nothing:
        {
            retval = ReturnCode::Ok;
            break;
        }
        default:
            break;
    }
    return retval;
}
/// @brief Add action to actions queue
/// @param[in] action Action of type and data
/// @return ReturnCode::Ok if successful, else - error code
Logger::ReturnCode Logger::addAction( const Action& action )
{
    _actionQueueMutex.lock();
    _actionQueue.push( action );
    _actionQueueMutex.unlock();
    _actionCV.notify_one();
    return ReturnCode::Ok;
}
/// @todo Chunk unload (getActions()) for less _actionQueueMutex use
/// @brief Get action from actions queue
/// @return Action
const Logger::Action Logger::getAction( void )
{
    _actionQueueMutex.lock();
    Action action;
    if( !_actionQueue.empty() )
    {
        Action popAction{ _actionQueue.front() };
        _actionQueue.pop();
        action.type = popAction.type;
        switch( action.type )
        {
            case Action::Type::changeLevel:
                action.data.integer = popAction.data.integer;
                break;
            case Action::Type::changePath:
                action.data.str = popAction.data.str;
                break;
            case Action::Type::write:
                action.data.str = popAction.data.str;
                action.data.integer = popAction.data.integer;
                break;
            default:
                break;
        }
    }

    _actionQueueMutex.unlock();

    return action;
}

/// @brief Thread function for queue job
void Logger::actionQueueJob( void )
{
    do
    {
        std::unique_lock< std::mutex > jobLock( _activeJobMutex );
        _actionCV.wait( jobLock );
        jobLock.unlock();

        bool haveAction{ true };
        do
        {
            const Action action = getAction();
            if( action.type == Action::Type::nothing )
            {
                haveAction = false;
            }
            else
            {
                if( _preventDestructMutex.try_lock() )
                {
                    ReturnCode retval{ doAction( action ) };
                    if( ReturnCode::Ok != retval )
                    {
                        writeImpl( Level::ERROR, "actionQueueJob(): something has happened" );
                    }
                    _preventDestructMutex.unlock();
                }
            }
        } while( _isThreadActive && haveAction );
    } while( _isThreadActive );
}
/// @brief Prepare thread on constructor
void Logger::setupThread( void )
{
    _isThreadActive = true;
    _actionThread = std::thread( &Logger::actionQueueJob, this );
    _actionThread.detach();
}

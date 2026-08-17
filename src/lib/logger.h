#ifndef LOGGER_H_
#define LOGGER_H_

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <variant>

// @brief Logger main class
class Logger
{
public:
    /// @brief Return codes for Logger class
    enum class ReturnCode
    {
        Ok,                 // Success
        JournalUnspecified, // No journal path given
        JournalNoopen,      // Unable to open a journal
        LevelUnknown,       // Unknown logging level given
        LoggerNullptr,      // No logger ptr given
        Fatal               // Unknown fatal error
    };

    /// @brief Logging levels
    enum Level : int
    {
        FIRST       = 0,  // First element of Logger::Level (for iteration)
        DEBUG   = FIRST,  // Debug level    (0)
        INFO        = 1,  // Info level     (1)
        NOTICE      = 2,  // Notice level   (2)
        WARNING     = 3,  // Warning level  (3)
        ERROR       = 4,  // Error level    (4)
        CRITICAL    = 5,  // Critical level (5)
        LAST = CRITICAL,  // Last elelemt of Logger::Level (for iteration)
        DEFAULT  = INFO   // Default level (=1)
    };
private:
    /// @brief Action class for queueing
    class Action
    {
    public:
        /// @brief Action types
        enum Type : int
        {
            nothing = 0,
            changeLevel,
            changePath,
            write,
            open
        };
        /// @brief Action data
        struct Data
        {
            std::string str;
            int integer;
        };

    public:
        Type type;
        Data data;

        Action() : type( nothing ) {}
        Action( const Action& action ) : type( action.type )
        {
            switch( type )
            {
                case changeLevel:
                    data.integer = action.data.integer;
                    break;
                case changePath:
                    data.str = action.data.str;
                    break;
                case write:
                    data.integer = action.data.integer;
                    data.str = action.data.str;
                    break;
                default:
                    break;
            }
        }
        ~Action()
        {
        }
    };

public:
    Logger( const std::string filePath, const Level level );
    Logger( void );
    /// @brief Logger destructor
    /// @note Do NOT call delete unless finished
    ~Logger();

    /// @brief Queue setting new logging level
    /// @param[in] level New logging level
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode setLevel( const Level level );
    /// @brief Get current logging level
    /// @return Logging level
    Level getLevel( void ) const;
    /// @brief Sets a new journal for the logger
    /// @param[in] filePath File path to the journal
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode setJournal( const std::string filePath );
    /// @brief Queue opening a journal
    /// @note Is 'public' for cases of manual fix on bad std::ofstream::open()
    /// @note Doesn't reopen if already open
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode tryOpenJournal( void );
    /// @brief Check if journal is currently open
    /// @note Utilizes _fileOutMutex, avoid deadlocks
    /// @return ReturnCode::Ok if file is open, else - status code
    ReturnCode isJournalOpen( void );
    /// @brief Queue message logging
    /// @note Utilizes _fileOutMutex, avoid deadlocks
    /// Attempts to log a message of a specified level in format:
    /// YYYY-MM-DD HH:MM:SS [ %level%  ] %message%\n
    /// @param[in] level Level of the logged message
    /// @param[in] msg The message itself
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode write( const Level level, const std::string msg );
public:
    /// @brief Check if logging level is valid
    /// @param[in] level Logging  level
    /// @return true - valid, else - false
    static bool isValidLevel( const Level level );
    /// @brief \"Convert\" logging level to a string
    /// @param[in] level Logging level
    /// @return Logging level text string
    static const std::string& levelToString( const Level level );

private:
    std::unique_ptr< std::ofstream > _fileOut;
    std::string _filePath;
    std::atomic< Level > _level;
    std::queue< Action > _actionQueue;    // Queue of actions

    std::mutex _actionQueueMutex;         // Mutex exceptional to _actionQueue
    std::mutex _fileOutMutex;             // Mutex exceptional to _fileOut
    std::condition_variable _actionCV;    // To notify on addAction()

    std::thread _actionThread;            // actionQueueJob() thread
    std::mutex _activeJobMutex;           // Unfinished job mutex (to prevent destruction)
    std::mutex _preventDestructMutex;
    std::atomic< bool > _isThreadActive;  // End thread flag

private:
    /// @brief Mutex-less isJournalOpen(), for inner use
    /// @return ReturnCode::Ok if successful, else - status code
    ReturnCode isJournalOpenImpl( void );
    /// @brief Function to close the journal, for inner use
    /// @note Locks _fileOutMutex, avoid deadlocks
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode tryOpenJournalImpl( void );
    /// @brief Function to close the journal, for inner use
    /// @note Does nothing with _filePath
    /// @note Locks _fileOutMutex, avoid deadlocks
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode tryCloseJournalImpl( void );
    /// @brief Get an absolute path of a given path
    /// @param[in] path Given path
    /// @param[out] absolutePath Absolute path from given
    /// @return ReturnCode::Ok if successful, else - error code
    static ReturnCode getAbsolutePath( const std::string& path, std::string& absolutePath );
    /// @brief Get current date and time string
    /// @return Resulting date and time string
    static const std::string getDateTimeString( void );
    /// @brief Max possible length for a level string
    /// @return Max length for a level string
    static constexpr int levelStringMaxLength( void );

    /// @brief Queue logging level change function
    /// @param[in] level New logging level
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode setLevelImpl( const Level level );
    /// @brief Queue journal path change function
    /// @param[in] newPath New journal path
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode setJournalImpl( const std::string& filePath );
    /// @brief Queue write funcion
    /// @param[in] level Level of the logged message
    /// @param[in] msg The message itself
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode writeImpl( const Level level, const std::string& msg );

    /// @brief Do single action
    /// @param[in] action Action to do
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode doAction( const Action& action );
    /// @brief Add action to actions queue
    /// @param[in] action Action of type and data
    /// @return ReturnCode::Ok if successful, else - error code
    ReturnCode addAction( const Action& action );
    /// @todo Chunk unloack (getActions()) for less _actionQueueMutex use
    /// @brief Get action from actions queue
    /// @return Action
    const Action getAction( void );

    /// @brief Thread function for queue job
    void actionQueueJob( void );
    /// @brief Prepare thread on constructor
    void setupThread( void );
};

#endif  // LOGGER_H_


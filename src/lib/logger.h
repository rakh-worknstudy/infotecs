#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <memory>
#include <string>

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

    Logger( const std::string filePath, const Level level );
    Logger( void );
    ~Logger();

    /// @brief Set new logging level
    /// param[in] level New logging level
    /// return ReturnCode::Ok if successful, else - error code
    ReturnCode setLevel( const Level level );
    /// @brief Get current logging level
    /// return Logging level
    Level getLevel( void ) const;
    /// @brief Sets a new journal for the logger
    /// param[in] filePath File path to the journal
    /// return ReturnCode::Ok if successful, else - error code
    ReturnCode setJournal( const std::string filePath );
    /// @brief Function to open a journal
    /// @note Is 'public' for cases of manual fix on bad std::ofstream::open()
    /// @note Doesn't reopen if already open
    /// return ReturnCode::Ok if successful, else - error code
    ReturnCode tryOpenJournal( void );
    /// @brief Check if journal is currently open
    /// return ReturnCode::Ok if file is open, else - error code
    ReturnCode isJournalOpen( void ) const;
    /// @brief Main logging function
    /// Attempts to log a message of a specified level in format:
    /// YYYY-MM-DD HH:MM:SS [ %level%  ] %message%\n
    /// param[in] level Level of the logged message
    /// param[in] msg The message itself
    /// return ReturnCode::Ok if successful, else - error code
    ReturnCode write( const Level level, const std::string msg ) const;
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
    Level _level;

    /// @brief Function to close the journal
    /// @note Does nothing with _filePath
    /// return ReturnCode::Ok if successful, else - error code
    ReturnCode tryCloseJournal( void );
private:
    
    /// @brief \"Convert\" logging level to a string
    /// @param[in] level Logging level
    /// @return Logging level text string
    /// @brief Get an absolute path of a given path
    /// param[in] path Given path
    /// param[out] absolutePath Absolute path from given
    /// return ReturnCode::Ok if successful, else - error type
    static ReturnCode getAbsolutePath( const std::string& path, std::string& absolutePath );
    /// @brief Get current date and time string
    /// return Resulting date and time string
    static const std::string getDateTimeString( void );
    /// @brief Max possible length for a level string
    /// return Max length for a level string
    static constexpr int levelStringMaxLength( void );
};

#endif  // LOGGER_H_


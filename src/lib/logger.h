#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <memory>
#include <string>

// @brief Logger main class
class Logger
{
public:
    /// @brief Return codes for working with logger class 
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
        BEGIN = 0,
        DEBUG = BEGIN,
        INFO,
        NOTICE,
        WARNING,
        ERROR,
        CRITICAL,
        END = CRITICAL,
        DEFAULT = INFO
    };

    Logger( const std::string filePath, const Level level );
    Logger( void );
    ~Logger();

    ReturnCode setLevel( const Level level );
    Level getLevel( void ) const;
    ReturnCode setJournal( const std::string filePath );
    bool isJournalOpen( void ) const;
    ReturnCode log( const Level level, const std::string msg ) const;
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
private:
    /// @brief Get absolute path of path
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


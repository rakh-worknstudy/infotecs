#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>


namespace logger
{
    enum class ReturnCode
    {
        Ok,                 // Successful initialization
        JournalUnspecified, // No journal path given
        JournalNoopen,      // Unable to open a journal
        LevelUnknown,       // Unknown logging level given
        DescUnknown,        // Bad logger descriptor given
        DescExcessive,      // Too many loggers opened/Out of maxval
        Fatal               // Unspecified fatal error (Unknown)
    };

    enum Level : int
    {
        DEBUG = 0,
        INFO,
        NOTICE,
        WARNING,
        ERROR,
        CRITICAL
    };

    /// @brief Logger initialization function
    /// @param[in] journal Journal path
    /// @param[in] level Logging level
    /// @param[out] desc Logger descriptor
    /// @return ReturnCode::Ok if successful, else - error type
    ReturnCode init( const std::string journal, const logger::Level level, int& desc );
    /// @brief Logger closing function
    /// @param[in] desc To-close logger descriptor
    /// @return ReturnCode::Ok if successful, else - error type
    ReturnCode close( const int desc );
    /// @brief Log by logger descriptor
    /// @param[in] desc Logger descriptor
    /// @param[in] level Log level
    /// @param[in] msg Log message
    /// @return ReturnCode::ok if successful, else - error type
    ReturnCode log( const int desc, const logger::Level level, const std::string msg );
}


#endif  // LOGGER_H_


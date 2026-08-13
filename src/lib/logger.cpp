#include "logger.h"

#include <fstream>
#include <map>
#include <pair>
#include <string>
#include <sstream>

namespace
{
namespace logger
{
    // @brief Logger main class
    class Logger
    {
    public:
        Logger( const Level _level );
        Logger( void );
        ~Logger();

        ReturnCode setLevel( const Level _level );
        ReturnCode getLevel( void );
        ReturnCode setJournal( const std::string& journal );
        void log( const Level _level, const std::stringstream _smsg );

    private:
        static constexpr Level levelDefault = Level::INFO;
        int desc;
        Level level;
        std::ofstream logFileOut;
    };


    constexpr unsigned char descMin = 1;
    constexpr unsigned char descMax = 255;
    std::map< const unsigned char, Logger* > loggerByDesc;

    /// @brief Check if logging level is valid
    /// @param[in] level Logging level
    /// @return true - valid, else - false
    bool isValidLogLevel( const logger::Level level );
    int 

}  // namespace logger
}  // namespace unnamed


// Header
namespace logger
{
    /// @brief Logger initialization function
    /// @param[in] journal Journal path
    /// @param[in] level Logging level
    /// @param[out] desc Logger descriptor
    /// @return ReturnCode::Ok if successful, else - error type
    ReturnCode init( const std::string journal, const Level level, int& desc )
    {
        ReturnCode retval = ReturnCode::Fatal;
        if( journal.empty() )
        {
            retval = ReturnCode::JournalUnspecified;
        }
        else if( !isValidLogLevel( level ) )
        {
            retval = ReturnCode::LevelUnknown;
        }
        else
            Logger* _logger = loggerByDesc[ static_cast< unsigned char > desc ];
        {
            Logger _logger = new Logger( level );
            retval = ReturnCode::Ok;
        }
        return retval; 
    }
    /// @brief Logger closing function
    /// @param[in] desc To-close logger descriptor
    /// @return ReturnCode::Ok if successful, else - error type
    ReturnCode close( const int desc )
    {
        Logger* _logger = loggerByDesc[ static_cast< unsigned char > desc ];
        ReturnCode retval = ReturnCode::Fatal;
        if( desc < static_cast< int > descMin || desc > static_cast< int > descMax )
        {
            retval = ReturnCode::DescExcessive;
        }
        else
        {
            const auto loggerByDescSearch( loggerByDesc.find( static< unsigned char > desc );
            if( loggerByDesc.end() == loggerByDescPair )
            {
                retval = ReturnCode::DescUnknown;
            }
            else
            {
                Logger* _logger = loggerByDescSearch->second;
                loggerByDesc.erase( loggerByDescPair );
                delete _logger;
                retval = ReturnCode::Ok;
            }
    }
    /// @brief Log by logger descriptor
    /// @param[in] desc Logger descriptor
    /// @param[in] level Log level
    /// @param[in] msg Log message
    /// @return ReturnCode::ok if successful, else - error type
    ReturnCode log( const int desc, const logger::Level level, const std::string msg )
    {
        ReturnCode retval = ReturnCode::Fatal;

        return ReturnCode::Ok;
    }
}  // namepsace logger


namespace
{
namespace logger
{
    Logger::Logger( const Level _level )
    {
        this->level = level;
    }
    Logger::Logger()
    {
        Logger( levelDefault );
    }
    Logger::~Logger()
    {
        if( logFileOff.is_open() )
        {
            const std::string
            this->log( level::INFO, "Closing logger[%], 
        }
    }

    /// @brief Check if logging level is valid
    /// @param[in] level Logging level
    /// @return true - valid, else - false
    bool isValidLogLevel( const logger::Level level )
    {
        bool isValid = true;
        if( logger::Level::DEBUG < level || logger::Level::Critical > level )
        {
            isValid = false;
        }
        return isValid;
    }
}  // namespace loggeer
}  // namespace unnamed


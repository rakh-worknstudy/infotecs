#include "main.h"

#ifdef _BUILD_TEST
#   include "test.h"

#else  // _BUILD_TEST
#   include <cstdlib>
#   include <ctime>
#   include <iostream>
#   include <map>
#   include <thread>

#   include "lib/logger.h"
#endif  // ndef _BUILD_TEST


#ifndef _BUILD_TEST
namespace
{
    /// @brief Print help to std::cout
    static void printHelp( void );
    /// @brief Print error msg to std::cerr
    /// @param[in] func __func__
    /// @param[in] msg Message
    /// @param[in] code Error code
    static void printToCerr( const char* func, const std::string msg, const Logger::ReturnCode returnCode );
    /// @brief Parse envrionemt arguments
    /// @param[in] argc Number of args
    /// @param[in] argv Array of char string args
    /// @param[out] journal Resulting journal path
    /// @param[out] level Resulting level
    /// @return 0 if successful, else -1
    static int parseArgs( int argc, char** argv, std::string& journal, Logger::Level& level );
    /// @brief Get Logger::Level from string
    /// @note Lowercase
    /// @param[in] str Level string
    /// @param[out] level Level value
    /// @return 0 if successful, else -1
    static int stringToLevel( const std::string str, Logger::Level& level );
    /// @brief Get return code name as a string
    /// @param[in] value Return code
    /// @return Return code as a string
    static const std::string returnCodeToString( const Logger::ReturnCode value );

    /// @brief Main menu
    /// @param[in] logger Logger
    void funMenu( Logger* logger );
    /// @brief setLevel menu
    /// @param[in] logger Logger
    int funMenuSetLevel( Logger* logger );
    /// @brief setJournal menu
    /// @param[in] logger Logger
    int funMenuSetJournal( Logger* logger );
    /// @brief write menu
    /// @param[in] logger Logger
    int funMenuWrite( Logger* logger );
    /// @brief getLevel menu
    /// @param[in] logger Logger
    int funMenuGetLevel( Logger* logger );
    /// @brief getJournal menu
    /// @param[in] logger Logger
    int funMenuGetJournal( Logger* logger );
}  // namespace

/// @brief Initialize logger
/// @param[in] journal Journal path
/// @param[in] level Logging level
/// @param[out] logger Logger pointer
/// @return Logger::ReturnCode::Ok if successful, else - error code
Logger::ReturnCode init( const std::string journal, const Logger::Level level, Logger*& logger )
{
    Logger::ReturnCode retval{ Logger::ReturnCode::Fatal };
    if( journal.empty() )
    {
        retval = Logger::ReturnCode::JournalUnspecified;
    }
    else if( !Logger::isValidLevel( static_cast< Logger::Level >( level ) ) )
    {
        retval = Logger::ReturnCode::LevelUnknown;
    }
    else
    {
        logger = new Logger( journal, level );
        if( nullptr != logger )
        {
            // No fail upon 'new' Logger(..)
            // Get journal opening status
            Logger::ReturnCode journalStatus{ logger->isJournalOpen() };
            if( Logger::ReturnCode::Ok == journalStatus )
            {
                // Success
                retval = Logger::ReturnCode::Ok;
            }
            else if( Logger::ReturnCode::JournalNoopen == journalStatus )
            {
                // Cannot open the journal
                retval = Logger::ReturnCode::JournalNoopen;
            }
            // else Logger(..) inner failure (ReturnCode::Fatal)
        }
        // else new Logger(..) failure (ReturnCode::Fatal)
    }
    if( Logger::ReturnCode::Ok != retval )
    {
        printToCerr( __func__, "", retval );
    }
    return retval;
}
/// @brief Close the logger
/// @param[in] logger Logger to close
/// @return Logger::ReturnCode::Ok if successful, else - error code
Logger::ReturnCode close( Logger*& logger )
{
    Logger::ReturnCode retval{ Logger::ReturnCode::Fatal };
    if( nullptr == logger )
    {
        retval = Logger::ReturnCode::LoggerNullptr;
    }
    else
    {
        delete logger;
        logger = nullptr;
        retval = Logger::ReturnCode::Ok;
    }
    if( Logger::ReturnCode::Ok != retval )
    {
        printToCerr( __func__, "", retval );
    }
    return retval;
}

/// @brief User-unfriendly interface
/// @param[in] logger Logger
/// @return Logger::ReturnCode::Ok if successful, else - error code
Logger::ReturnCode fun( Logger* logger )
{
    Logger::ReturnCode retval{ Logger::ReturnCode::Fatal };
    if( nullptr == logger )
    {
        retval = Logger::ReturnCode::LoggerNullptr;
    }
    else
    {
        funMenu( logger );
        retval = Logger::ReturnCode::Ok;
    }
    return retval;
}
#endif  // ndef _BUILD_TEST

#ifdef _BUILD_TEST
int main( void )
{
    Test test;
    return test.run();
}
#else  // _BUILD_TEST
int main( int argc, char** argv )
{
    int retval{ -1 };

    Logger* logger;
    std::string journal;
    Logger::Level level;

    if( 0 != parseArgs( argc, argv, journal, level ) )
    {
        printHelp();
    }
    else if( Logger::ReturnCode::Ok == init( journal, level, logger ) )
    {
        fun( logger );
        if( Logger::ReturnCode::Ok == close( logger ) )
        {
            retval = 0;
        }
    }

    return retval;
}
#endif  // ndef _BUILD_TEST

#ifndef _BUILD_TEST
namespace
{
    /// @brief Print help to std::cout
    static void printHelp( void )
    {
        static const std::string helpString
        (
            "Usage: %program% [%journalPath%] [%level%]\n"
            "  %journalPath%: relative path to journal file\n"
            "  %level%: logging level, can be emppty or in:\n"
            "    [ debug, info, notice, warning, error, critical ]"
        );
        std::cout << helpString << std::endl;
    }

    /// @brief Print error msg to std::cerr
    /// @param[in] func __func__
    /// @param[in] msg Message
    /// @param[in] code Error code
    static void printToCerr( const char* func, const std::string msg, const Logger::ReturnCode returnCode )
    {
        std::cerr << func << ": ";
        if( !msg.empty() )
        {
            std::cerr << msg << ": ";
        }
        std::cerr << returnCodeToString( returnCode );
    }

    /// @brief Parse envrionemt arguments
    /// @param[in] argc Number of args
    /// @param[in] argv Array of char string args
    /// @param[out] journal Resulting journal path
    /// @param[out] level Resulting level
    /// @return Return 0 if successful, else -1
    static int parseArgs( int argc, char** argv, std::string& journal, Logger::Level& level )
    {
        if( argc < 2 || argc > 3 )
        {
            return -1;
        }

        if( argc == 3 )
        {
            const std::string levelChar( argv[2] );
            if( 0 != stringToLevel( levelChar, level ) )
            {
                return -1;
            }
        }
        else
        {
            level = Logger::Level::DEFAULT;
        }

        journal = argv[1];
        return 0;
    }

    /// @brief Get Logger::Level from string
    /// @note Lowercase
    /// @param[in] str Level string
    /// @param[out] level Level value
    /// @return 0 if successful, else -1
    static int stringToLevel( const std::string str, Logger::Level& level )
    {
        const std::map< std::string, Logger::Level > stringToLevelMap =
        {
            { "debug", Logger::Level::DEBUG },
            { "info", Logger::Level::INFO },
            { "notice", Logger::Level::NOTICE },
            { "warning", Logger::Level::WARNING },
            { "error", Logger::Level::ERROR },
            { "critical", Logger::Level::CRITICAL }
        };

        auto search = stringToLevelMap.find( str );
        if( stringToLevelMap.end() != search )
        {
            level = search->second;
            return 0;
        }
        return -1;
    }

    /// @brief Get return code name as a string
    /// @param[in] value Return code
    /// return Return code as a string
    static const std::string returnCodeToString( const Logger::ReturnCode returnCode )
    {
        static const std::map< Logger::ReturnCode, std::string > returnCodeToStringMap =
        {
            { Logger::ReturnCode::Ok, "Success" },
            { Logger::ReturnCode::JournalUnspecified, "No journal path given" },
            { Logger::ReturnCode::JournalNoopen, "Unable to open a journal" },
            { Logger::ReturnCode::LevelUnknown, "Unknown logging level given" },
            { Logger::ReturnCode::LoggerNullptr, "No logger ptr given" },
            { Logger::ReturnCode::Fatal, "Uknown fatal error" }
        };
        static const std::string unknownReturnCodeString( "Unknown return code" );

        auto search = returnCodeToStringMap.find( returnCode );
        if( returnCodeToStringMap.end() != search )
        {
            return search->second;
        }
        return unknownReturnCodeString;
    }

    /// @brief Main menu
    /// @param[in] logger Logger
    void funMenu( Logger* logger )
    {
        char ch;

        do
        {
            std::cout << "\033[2J\033[1;1H";
            std::cout << "1 - setLevel\n2 - setJournal\n3 - write\n4 - getLevel\n5 - getJournal\n0 - Exit" << std::endl;
            ch = std::cin.get();
            switch( ch )
            {
                case '1':
                    funMenuSetLevel( logger );
                    break;
                case '2':
                    funMenuSetJournal( logger );
                    break;
                case '3':
                    funMenuWrite( logger );
                    break;
                case '4':
                    funMenuGetLevel( logger );
                    break;
                case '5':
                    funMenuGetJournal( logger );
                    break;
                default:
                    break;
            }
        } while( ch != '0' );
    }
    /// @brief setLevel menu
    /// @param[in] logger Logger
    int funMenuSetLevel( Logger* logger )
    {
        std::cout << "\033[2J\033[1;1H";
        Logger::Level level;

        std::string levelStr;
        std::cout << "Enter level [debug, info, notice, warning, error, critical]: ";
        std::cin >> levelStr;
        if( 0 != stringToLevel( levelStr, level ) )
        {
            std::cout << "Bad input" << std::endl;
        }
        else
        {
            std::cout << "Trying to setLevel()" << std::endl;
            logger->setLevel( level );
        }

        std::cin.get();
        std::cin.get();

        return 0;
    }
    /// @brief setJournal menu
    /// @param[in] logger Logger
    int funMenuSetJournal( Logger* logger )
    {
        std::cout << "\033[2J\033[1;1H";
        std::string journal;

        std::cout << "Enter journal path: ";
        std::cin >> journal;
        std::cout << "Trying to setJournal(" << journal << ")" << std::endl;
        logger->setJournal( journal );

        std::cin.get();
        std::cin.get();

        return 0;
    }
    /// @brief write menu
    /// @param[in] logger Logger
    int funMenuWrite( Logger* logger )
    {
        std::cout << "\033[2J\033[1;1H";
        Logger::Level level;
        std::string msg;

        std::string levelStr;
        std::cout << "Enter level [debug, info, notice, warning, error, critical]: ";
        std::cin >> levelStr;
        if( 0 != stringToLevel( levelStr, level ) )
        {
            std::cout << "Bad input" << std::endl;
        }
        else
        {
            std::cout << "Enter message: ";
            std::cin >> msg;
            std::cout << "Trying to write()" << std::endl;
            logger->write( level, msg );
        }

        std::cin.get();
        std::cin.get();

        return 0;
    }
    /// @brief getLevel menu
    /// @param[in] logger Logger
    int funMenuGetLevel( Logger* logger )
    {
        std::cout << "Current level is: " << logger->levelToString( logger->getLevel() ) << std::endl;
        std::cin.get();
        std::cin.get();

        return 0;
    }
    /// @brief getJournal menu
    /// @param[in] logger Logger
    int funMenuGetJournal( Logger* logger )
    {
        std::cout << "Current journal is: " << logger->getJournal() << std::endl;

        std::cin.get();
        std::cin.get();

        return 0;
    }
}  // namespace

#endif  // ndef _BUILD_TEST


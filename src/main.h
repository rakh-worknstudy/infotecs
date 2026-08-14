#ifndef MAIN_H_
#define MAIN_H_

#include "lib/logger.h"


/// @brief Logger initialization function
/// @param[in] journal Journal path
/// @param[in] level Logging level
/// @param[out] logger Logger class reference
/// @return ReturnCode::Ok if successful, else - error type
Logger::ReturnCode initLogger( const std::string journal, const Logger::Level level, Logger*& logger );
/// @brief Logger closing function
/// @param[in] logger To-close logger reference
/// @return ReturnCode::Ok if successful, else - error type
Logger::ReturnCode closeLogger( Logger*& logger );
/// @brief Log by logger descriptor
/// @param[in] desc Logger descriptor
/// @param[in] level Log level
/// @param[in] msg Log message
/// @return ReturnCode::ok if successful, else - error type
Logger::ReturnCode log( Logger* logger, const Logger::Level level, const std::string msg );

int main( void );

#endif  // MAIN_H_


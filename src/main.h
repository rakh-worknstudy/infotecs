#ifndef MAIN_H_
#define MAIN_H_

#ifndef _BUILD_TEST
#   include "lib/logger.h"

/// @brief Initialize a logger
/// @param[in] journal Journal path
/// @param[in] level Logging level
/// @param[out] logger Logger pointer
/// @return Logger::ReturnCode::Ok if successful, else - error code
Logger::ReturnCode init( const std::string journal, const Logger::Level level, Logger*& logger );
/// @brief Close the logger
/// @param[in] logger Logger to close
/// @return Logger::ReturnCode::Ok if successful, else - error code
Logger::ReturnCode close( Logger*& logger );

int main( int argc, char** argv );
#else  // _BUILD_TEST
int main( void );
#endif  // ndef _BUILD_TEST

#endif  // MAIN_H_


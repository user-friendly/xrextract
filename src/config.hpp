/**
* @file
* Application compile time configurations.
*/

#ifndef __APPLICATION_CONFIG_HPP
#define __APPLICATION_CONFIG_HPP

# if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin
#   define WINDOWS_API
# else
#   define POSIX_API 
# endif

#endif // __APPLICATION_CONFIG_HPP
/**
 * @file
 * Header file for all libraries used.
 */

#ifndef __STD_HPP
#define __STD_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// C++ STD headers.
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
// C++ STD C headers.
#include <cstdlib>
#include <cstdint>

// Boost headers.
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// GSL
#include <gsl/gsl>

#endif // __STD_HPP

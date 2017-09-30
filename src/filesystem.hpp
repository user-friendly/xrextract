/**
 * @file
 * Declarations of file system related utilities.
 */

#ifndef __FILESYSTEM_HPP
#define __FILESYSTEM_HPP

#include "extlibs.hpp"
#include "assets.hpp"

namespace fs = boost::filesystem;
using namespace std;

namespace xrextract {
  /**
   * Retrieve data files from a list of files.
   */
  data_file_entries get_data_files_from_filenames(const vector<string>& cat_files);
  
  /**
   * Retrieve data files from a directory.
   */
  data_file_entries get_data_files_from_directory(const fs::path& data_dir);
}

#endif // __FILESYSTEM_HPP

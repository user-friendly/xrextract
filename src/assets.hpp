/**
 * @file
 * Assets declarations.
 */

#include "extlibs.hpp"

namespace fs = boost::filesystem;
using namespace std;

namespace xrextract {
  typedef fs::path::string_type path_string;

  /**
  * An md5 checksum.
  *
  * C-style string.
  */
  typedef array<char, 33> md5sum;

  /**
  * An asset entry found in .cat files.
  */
  struct asset_entry {
    fs::path filename;
    /**
    * Size in bytes.
    *
    * @TODO Maximum size of an 32 bit int is ~ 4 GB.
    *       Figure out of this member can be reduced to 32 bit int.
    */
    uint64_t size;
    /**
    * Unix time stamp of the file.
    *
    * Needs to be at least 45 bits. See std::chrono.
    */
    uint64_t ts;
    // @TODO Maybe a string would be better here?

    md5sum checksum;
  };

  /**
  * A container for asset entries.
  */
  typedef vector<asset_entry> asset_entries;

  /**
  * Simple data structure for data files.
  */
  struct data_file {
    path_string name;
    fs::path cat;
    fs::path dat;

    asset_entries assets;
  };

  /**
  * A container for data file entries.
  */
  typedef vector<data_file> data_file_entries;

  /**
   * Retrieve asset entries from a data file.
   */
  asset_entries get_assets(const data_file& df);

  /**
   * Retrieve data files from a directory.
   */
  data_file_entries get_data_files_from_directory(const fs::path& data_dir);
}

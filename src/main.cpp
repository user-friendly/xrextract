/**
 * @file
 * This the program's main entry file.
 */

// @TODO The data flow can be considered a stream.
// There might be no need to back reference any cat entries.
// Parse one line at a time and move the .dat "pointer" forward.

#include "std.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace al = boost::algorithm;
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
  
  asset_entries get_assets(const data_file& df) {
    asset_entries entries {};
    ifstream in(df.cat.native());
    // Enable stream exception throws for non-recoverable error states.
    in.exceptions(ios_base::badbit);

    // Format of .cat files is as follows:
    // [relative asset file name] [size in bytes] [timestamp] [md5 checksum]

    // @TODO Consider using Regular Expressions... might be easier in the end.
    
    using sst = string::size_type;
    string line {}, md5(32, '\0'), number (128, '\0'), rel_path {};
    uint64_t sz, ts;
    sst lpos;

    // Line number counter.
    uint32_t ln {0};
    while(true) {
      try {
        getline(in, line);
      }
      catch (std::ios_base::failure &fail) {
        cerr << "error: failed to read entry from: " << df.cat << "\n";
        cerr << "\twhat: " << fail.what() << ", code: " << fail.code() << "\n";
        break;
      }

      if (in.eof()) {
        break;
      }

      ln++;
      md5.clear();
      rel_path.clear();
      ts = sz = 0;

      string::reverse_iterator rbegin {}, rend {};

      // Parse md5 checksum.
      rbegin = find(line.rbegin(), line.rend(), ' ');
      md5.append(rbegin.base(), line.end());

      // Parse timestamp.
      try {
        number.clear();
        rend = find(++rbegin, line.rend(), ' ');
        number.append(rend.base(), rbegin.base());
        ts = stoul(number);
      } catch (const std::logic_error& e) {
        cerr << "error: failed to parse timestamp: " << e.what() << "\n";
        cerr << "\t\tline " << ln << ": " << line << "\n";
        break;
      }

      // Parse size.
      try {
        number.clear();
        rbegin = ++rend;
        rend = find(rbegin, line.rend(), ' ');
        number.append(rend.base(), rbegin.base());
        sz = stoul(number);
      } catch (const std::logic_error& e) {
        cerr << "error: failed to parse size: " << e.what() << "\n";
        cerr << "\t\tline " << ln << ": " << line << "\n";
        break;
      }

      // Get relative asset filename.
      rel_path.append(line.begin(), (++rend).base());
      al::trim(rel_path);

      // Construct a new asset entry.
      asset_entry ae { rel_path, sz, ts, {} };
      
      if (md5.length() == 32) {
        // Bad.. bad, bad, bad.
        strncpy(ae.checksum.data(), md5.c_str(), 32);
        ae.checksum[33] = '\0';
      }
      else {
        cerr << "error: invalid checksum string for asset `" << rel_path << "`\n";
      }

      entries.push_back(ae);
    }

    return entries;
  };
}

namespace xr = xrextract;

/**
 * Print legal information.
 */
void print_legal();

/**
 * Program's main entry point.
 * 
 * @param int argc
 *   The number of arguments passed to the program.
 * 
 * @param char* argv[]
 *   The arguments passed to the program.
 * 
 * @return int
 *   Returns UNIX style exit status.
 */
int main(int argc, char* argv[]) {
  try {
    fs::path data_dir {};
    
    po::options_description desc("Available options are");
    desc.add_options()
      ("help,h", "produce help message")
      ("data-dir,d", po::value<string>(), "directory where the .dat files reside")
      ("version,v", "print program information")
      ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
      print_legal();
      return EXIT_SUCCESS;
    }

    if (vm.count("data-dir")) {
      data_dir = vm["data-dir"].as<string>();
    }
    else {
      cout << "Data directory not set. Using current working directory.\n";
      data_dir = fs::current_path();
    }

    if (fs::is_directory(data_dir)) {
      cout << "Data directory is: " << data_dir.string() << "\n";
    }
    else {
      throw std::runtime_error::runtime_error("Data directory either does not exist or is not a directory.");
    }
    
    // Iterate over directory files.
    std::vector<fs::path> dir_files;
    // @TODO Only look for .cat files and when assembling the xr::data_file objects look for .dat files.
    std::regex pattern("(?:ext_|subst_)?\\d+\\.(?:c|d)at$");
    for (const fs::directory_entry& file : fs::directory_iterator(data_dir)) {
      if (fs::is_regular_file(file.status())) {
        if (std::regex_search(file.path().filename().c_str(), pattern)) {
          if (!fs::is_symlink(file.path())) {
            dir_files.push_back(file.path());
          }
          else {
            // @TODO Check the return value of read_symlink().
            dir_files.push_back(fs::read_symlink(file.path()));
          }
        }
      }
    }

    if (!dir_files.empty()) {
      cout << "Found " << dir_files.size() << " data files.\n";
      
      // Compare operator is not efficient - uses copies of the strings.
      std::sort(dir_files.begin(), dir_files.end(), [](const fs::path& a, const fs::path& b) {
          return a.filename().string() < b.filename().string();
        });

      vector<xr::data_file> data_files;

      // Do not modify the container being iterated over!
      cout << "Found data files: " << "\n";
      for (const fs::path& file : dir_files) {
        cout << "\t" << file << "\n";
        
        if (file.extension().string() == ".cat") {
          fs::path dat_file { file };
          dat_file.replace_extension( { ".dat" } );
          auto found = std::find(dir_files.begin(), dir_files.end(), dat_file);
          if (found != dir_files.end()) {
            xr::data_file df {
              file.stem().string(),
              file,
              dat_file,
            };
            data_files.push_back(df);
          }
          else {
            cout << "\t\tCat file does not have equivalent dat file!\n";
          }
        }
      }

      if (!data_files.empty()) {
        cout << "Found cat & dat file pairs:\n";
        for (xr::data_file& entry : data_files) {
          cout << "\t" << entry.cat.filename().native() << ": ";

          entry.assets = move(xr::get_assets(entry));
          cout << entry.assets.size() << " assets\n";
        }
      }
      else {
        cout << "Cat files found, but no dat files in directory: " << data_dir << "\n";
      }
    }
    else {
      cout << "No data files found in directory: " << data_dir << "\n";
    }
    
  }
  catch(exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch(...) {
    cerr << "Exception of unknown type!\n";
  }
  
  return EXIT_SUCCESS;
}

void print_legal()
{
#if defined PACKAGE_NAME && defined PACKAGE_VERSION
  std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << ' ';
#endif
  std::cout << "Copyright (C) 2017 Plamen Ivanov\n\
This program comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome to redistribute it\n\
funder certain conditions; see the license for details.\n\n";
}

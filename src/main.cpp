/**
 * @file
 * This the program's main entry file.
 */

// @TODO The data flow can be considered a stream.
// There might be no need to back reference any cat entries.
// Parse one line at a time and move the .dat "pointer" forward.

#include "extlibs.hpp"
#include "assets.hpp"

# if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin
#   define WINDOWS_API
# else
#   define POSIX_API 
# endif

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

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
      cout << desc << endl;
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
      cout << "Data directory not set. Using current working directory." << endl;
      data_dir = fs::current_path();
    }

    if (fs::is_directory(data_dir)) {
      cout << "Data directory is: " << data_dir.string() << endl;
    }
    else {
      runtime_error e {"Data directory either does not exist or is not a directory."};
      throw e;
    }
    
    // Iterate over directory files.
    vector<fs::path> dir_files;
    // @TODO Only look for .cat files and when assembling the xr::data_file objects look for .dat files.
    
# if defined(WINDOWS_API)
    basic_regex<xr::path_string::value_type> pattern(L"(?:ext_|subst_)?\\d+\\.(?:c|d)at$");
# else
    regex pattern("(?:ext_|subst_)?\\d+\\.(?:c|d)at$");
# endif
    
    for (const fs::directory_entry& file : fs::directory_iterator(data_dir)) {
      if (fs::is_regular_file(file.status())) {
        if (regex_search(file.path().filename().c_str(), pattern)) {
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
      cout << "Found " << dir_files.size() << " data files." << endl;
      
      // Compare operator is not efficient - uses copies of the strings.
      sort(dir_files.begin(), dir_files.end(), [](const fs::path& a, const fs::path& b) {
          return a.filename().string() < b.filename().string();
        });

      vector<xr::data_file> data_files;

      // Do not modify the container being iterated over!
      cout << "Found data files: " << endl;
      for (const fs::path& file : dir_files) {
        cout << "\t" << file << endl;
        
        if (file.extension().string() == ".cat") {
          fs::path dat_file { file };
          dat_file.replace_extension( { ".dat" } );
          auto found = find(dir_files.begin(), dir_files.end(), dat_file);
          if (found != dir_files.end()) {
            xr::data_file df {
              file.stem().native(),
              file,
              dat_file,
            };
            data_files.push_back(df);
          }
          else {
            cout << "\t\tCat file does not have equivalent dat file!" << endl;
          }
        }
      }

      if (!data_files.empty()) {
        cout << "Found cat & dat file pairs:" << endl;
        for (xr::data_file& entry : data_files) {
          // On Win32, fs::path::value_type is a wide character.
          // Can this conversion poitentially lead to loss of / invalid data being outputted?
          cout << "\t" << entry.cat.filename().string() << ": " << flush;

          entry.assets = move(xr::get_assets(entry));
          cout << entry.assets.size() << " assets" << endl;
        }
      }
      else {
        cout << "Cat files found, but no dat files in directory: " << data_dir << endl;
      }
    }
    else {
      cout << "No data files found in directory: " << data_dir << endl;
    }
    
  }
  catch(exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }
  catch(...) {
    cerr << "Exception of unknown type!" << endl;
  }
  
  return EXIT_SUCCESS;
}

void print_legal()
{
#if defined PACKAGE_NAME && defined PACKAGE_VERSION
  cout << PACKAGE_NAME << " " << PACKAGE_VERSION << ' ';
#endif
  cout << "Copyright (C) 2017 Plamen Ivanov\n\
This program comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome to redistribute it\n\
funder certain conditions; see the license for details.\n" << endl;
}

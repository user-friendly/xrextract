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
    po::options_description desc("Available options are");
    desc.add_options()
      ("help,h", "produce help message")
      ("data-dir,d", po::value<string>(), "directory where the .dat files reside; ignored if --data-file option is used")
      ("data-file,f", po::value< vector<string> >(), "provide a list of .dat files")
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

    fs::path data_dir {};
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
    
    xr::data_file_entries dfs { xr::get_data_files_from_directory(data_dir) };
    if (dfs.empty() == false) {
      cout << "\n";
      for (const xr::data_file& df : dfs) {
        cout << "data file [" << df.dat.string() << "] has " << df.assets.size() << " assets" << endl;
      }
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

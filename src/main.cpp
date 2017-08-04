/**
 * @file
 * This the program's main entry file.
 */

#include "std.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using namespace std;

namespace xrextract {
  typedef fs::path::string_type path_string;
  
  struct data_file {
    path_string name;
    fs::path cat;
    fs::path dat;
  };

  
}

namespace xr = xrextract;

/**
 * Print legal information.
 */
void printLegal();

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
  printLegal();

  try {
    fs::path data_dir {};
    
    po::options_description desc("Available options are");
    desc.add_options()
      ("help,h", "produce help message")
      ("data-dir,d", po::value<string>(), "directory where the .dat files reside")
      ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << desc << "\n";
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
    std::regex pattern("(?:ext_|subst_)?\\d+\\.(?:c|d)at");
    for (const fs::directory_entry& file : fs::directory_iterator(data_dir)) {
      if (fs::is_regular_file(file.status())) {
        //cout << "\t" << file.path();
        if (std::regex_search(file.path().filename().c_str(), pattern)) {
          // cout << " - file is a data file";
          if (!fs::is_symlink(file.path())) {
            dir_files.push_back(file.path());
          }
          else {
            // @TODO Check the return value of read_symlink().
            dir_files.push_back(fs::read_symlink(file.path()));
          }
        }
        // cout << "\n";
      }
    }

    if (!dir_files.empty()) {
      cout << "Found " << dir_files.size() << " data files.\n";
      
      // Compare operator is not efficient - uses copies of the strings.
      std::sort(dir_files.begin(), dir_files.end(), [](const fs::path& a, const fs::path& b) {
          return a.filename().string() < b.filename().string();
        });

      std::vector<xr::data_file> data_files;

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
        for (const xr::data_file& entry : data_files) {
          cout << "\t" << entry.name << "\n";
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

void printLegal()
{
#if defined PACKAGE_NAME && defined PACKAGE_VERSION
  std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << ' ';
#endif
  std::cout << "Copyright (C) 2017 Plamen Ivanov\n\
This program comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome to redistribute it\n\
funder certain conditions; see the license for details.\n\n";
}

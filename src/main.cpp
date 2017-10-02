/**
 * @file
 * This the program's main entry file.
 */

// @TODO The data flow can be considered a stream.
// There might be no need to back reference any cat entries.
// Parse one line at a time and move the .dat "pointer" forward.

#include "extlibs.hpp"
#include "assets.hpp"
#include "filesystem.hpp"

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
      ("data-dir,d", po::value<string>(), "directory where the cat/dat files reside; if ommitted, current directory is used")
      ("data-file,f", po::value< vector<string> >(), "provide a list of .cat file; can be used multiple times")
      ("list-assets,l", "list assets for each data file")
      ("filter-assets,F", po::value< string >(), "only extract given assets")
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

    regex filter_pattern {};
    if (vm.count("filter-assets")) {
      filter_pattern = vm["filter-assets"].as<string>();
    }

    xr::data_file_entries dfs {};
    
    if (vm.count("data-file")) {
      vector<string> catfiles = vm["data-file"].as< vector<string> >();
      xr::data_file_entries catdfs = xr::get_data_files_from_filenames(catfiles);
      if (!catdfs.empty()) {
        move(catdfs.begin(), catdfs.end(), back_inserter(dfs));
      }
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
      xr::data_file_entries dirdfs = xr::get_data_files_from_directory(data_dir);
      if (!dirdfs.empty()) {
        move(dirdfs.begin(), dirdfs.end(), back_inserter(dfs));
      }
    }
    else {
      cerr << "error: data directory either does not exist or is not a directory." << endl;
    }

    if (dfs.empty() == false) {
      cout << "\n";
      for (const xr::data_file& df : dfs) {
        cout << "data file [" << df.dat.string() << "] has " << df.assets.size() << " assets" << endl;

        if (vm.count("list-assets")) {
          if (vm.count("filter-assets")) {
            for (const xr::asset_entry& ae : df.assets) {
              if (regex_search(ae.filename.string(), filter_pattern)) {
                cout << "F";
              }
              cout << "\t" << ae.filename.string() << endl;
            }
          }
          else {
            for (const xr::asset_entry& ae : df.assets) {
              cout << "\t" << ae.filename.string() << endl;
            }
          }
        }
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

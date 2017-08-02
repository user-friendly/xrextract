/**
 * @file
 * This the program's main entry file.
 */

#include "std.hpp"

namespace po = boost::program_options;
using namespace std;

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

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("compression", po::value<double>(), "set compression level")
        ;

        po::variables_map vm;        
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }

        if (vm.count("compression")) {
            cout << "Compression level was set to " 
                 << vm["compression"].as<double>() << ".\n";
        } else {
            cout << "Compression level was not set.\n";
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

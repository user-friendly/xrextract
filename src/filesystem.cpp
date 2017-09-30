/**
 * @file
 * Filesystem definitions file.
 */

#include "filesystem.hpp"
#include "assets.hpp"

namespace fs = boost::filesystem;
using namespace std;

namespace xrextract {
  data_file_entries get_data_files_from_filenames(const vector<string>& cat_files) {
    data_file_entries data_files;
    
    for (const string& cat_file_str : cat_files) {
      fs::path cat_file {cat_file_str};
      fs::path dat_file {cat_file};      
      dat_file.replace_extension( { ".dat" } );

      if (!fs::is_regular_file(cat_file)) {
        cerr << "error: cat file not found, skipping " << cat_file_str << endl;
        continue;
      }
      if (!fs::is_regular_file(dat_file)) {
        cerr << "error: dat file not found, skipping " << cat_file_str << endl;
        continue;
      }

      data_file df {
        cat_file.stem().native(),
        cat_file,
        dat_file
      };
      // See the todo in the data_file declaration.
      df.assets = move(get_assets(df));
      data_files.push_back(df);
    }
    
    return data_files;
  }
  
  data_file_entries get_data_files_from_directory(const fs::path& data_dir) {
    data_file_entries data_files;
    // Iterate over directory files.
    vector<fs::path> dir_files;
    // @TODO Only look for .cat files and when assembling the data_file objects look for .dat files.
    
# if defined(WINDOWS_API)
    basic_regex<path_string::value_type> pattern(L"(?:ext_|subst_)?\\d+\\.(?:c|d)at$");
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
      // Compare operator is not efficient - uses copies of the strings.
      sort(dir_files.begin(), dir_files.end(), [](const fs::path& a, const fs::path& b) {
          return a.filename().string() < b.filename().string();
        });

      // TODO Re-factor. I did this for the funzies, didn't I?
      // Do not modify the container being iterated over!
      for (const fs::path& file : dir_files) {
        
        if (file.extension().string() == ".cat") {
          fs::path dat_file { file };
          dat_file.replace_extension( { ".dat" } );
          auto found = find(dir_files.begin(), dir_files.end(), dat_file);
          if (found != dir_files.end()) {
            data_file df {
              file.stem().native(),
              file,
              dat_file,
            };
            data_files.push_back(df);
          }
          else {
            cerr << "error: cat file does not have equivalent dat file, " << file.filename().string() << endl;
          }
        }
      }

      if (!data_files.empty()) {
        for (data_file& entry : data_files) {
          // Leaving this comment block as a reference:
          // On Win32, fs::path::value_type is a wide character.
          // Can this conversion poitentially lead to loss of / invalid data being outputted?
          // cout << "\t" << entry.cat.filename().string() << ": " << flush;

          entry.assets = move(get_assets(entry));

# if defined(VERBOSE)
          cout << "\n\n" << entry.cat.filename().string() << ": " << flush;
          for (const asset_entry& ae : entry.assets) {
            cout << ae.filename.string() << ", sz: " << ae.size << endl;
          }
          cout << endl;
# endif
        }
      }
      else {
        cerr << "error: cat files found, but no dat files in directory: " << data_dir << endl;
      }
    }
    else {
      cerr << "error: no data files found in directory: " << data_dir << endl;
    }

    return data_files;
  }
}

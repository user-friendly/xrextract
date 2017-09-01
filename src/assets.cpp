/**
 * @file
 * Assets definitions file.
 */

#include "assets.hpp"

namespace fs = boost::filesystem;
namespace al = boost::algorithm;
using namespace std;

namespace xrextract {
  asset_entries get_assets(const data_file& df) {
    asset_entries entries{};
    ifstream in(df.cat.native());
    // Enable stream exception throws for non-recoverable error states.
    in.exceptions(ios_base::badbit);

    // Format of .cat files is as follows:
    // [relative asset file name] [size in bytes] [timestamp] [md5 checksum]

    // @TODO Consider using Regular Expressions... might be easier in the end.

    using sst = string::size_type;
    string line{}, md5(32, '\0'), number(128, '\0'), rel_path{};
    uint64_t sz, ts;
    sst lpos;

    // Line number counter.
    uint32_t ln{ 0 };
    while (true) {
      try {
        getline(in, line);
      }
      catch (ios_base::failure &fail) {
        cerr << "error: failed to read entry from: " << df.cat << endl;
        cerr << "\twhat: " << fail.what() << ", code: " << fail.code() << endl;
        break;
      }

      if (in.eof()) {
        break;
      }

      ln++;
      md5.clear();
      rel_path.clear();
      ts = sz = 0;

      string::reverse_iterator rbegin{}, rend{};

      // Parse md5 checksum.
      rbegin = find(line.rbegin(), line.rend(), ' ');
      md5.append(rbegin.base(), line.end());

      // Parse timestamp.
      try {
        number.clear();
        rend = find(++rbegin, line.rend(), ' ');
        number.append(rend.base(), rbegin.base());
        ts = stoul(number);
      }
      catch (const logic_error& e) {
        cerr << "error: failed to parse timestamp: " << e.what() << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        break;
      }

      // Parse size.
      try {
        number.clear();
        rbegin = ++rend;
        rend = find(rbegin, line.rend(), ' ');
        number.append(rend.base(), rbegin.base());
        sz = stoul(number);
      }
      catch (const logic_error& e) {
        cerr << "error: failed to parse size: " << e.what() << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        break;
      }

      // Get relative asset filename.
      rel_path.append(line.begin(), (++rend).base());
      al::trim(rel_path);

      // Construct a new asset entry.
      asset_entry ae{ rel_path, sz, ts,{} };

      if (md5.length() == 32) {
        // Bad.. bad, bad, bad.
        strncpy(ae.checksum.data(), md5.c_str(), 32);
        ae.checksum.at(32) = '\0';
      }
      else {
        cerr << "error: invalid checksum string for asset `" << rel_path << "`" << endl;
      }

      entries.push_back(ae);
    }

    return entries;
  };

  data_file_entries get_data_files_from_directory(const fs::path& data_dir) {
    // Define an empty container.
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

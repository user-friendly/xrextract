/**
 * @file
 * Assets definitions file.
 */

// Visual studio mandates the the PCH source header file is included.
#include "extlibs.hpp"
#include "assets.hpp"

namespace fs = boost::filesystem;
namespace al = boost::algorithm;
using namespace std;

namespace xrextract {
  void extract_assets(const data_file& df) {
    throw runtime_error("TODO Implement void extract_assets(const data_file& df)");
  ;}
  
  void extract_assets(const data_file& df, const regex& filter) {
    array<char, 4096> rdbuff;
    ifstream dat_in {df.dat.string()};
    uint64_t read_bytes {0}, written_bytes {0};
    streamsize read_chunk_size {0};
    
    for (const asset_entry& ae : df.assets) {
      if (!regex_search(ae.filename.string(), filter)) {
        dat_in.seekg(ae.size, ios_base::cur);
        continue;
      }
      cout << "extract " << ae.filename.string() << " ... " << flush;

      // cout << "root_name: " << ae.filename.root_name().string() << endl;
      // cout << "root_directory: " << ae.filename.root_directory().string() << endl;
      // cout << "root_path: " << ae.filename.root_path().string() << endl;
      // cout << "relative_path: " << ae.filename.relative_path().string() << endl;
      //cout << "parent_path: " << ae.filename.parent_path().string() << endl;
      fs::create_directories(ae.filename.parent_path());
      
      ofstream asset_out {ae.filename.string(), ios_base::out | ios_base::binary | ios_base::trunc};

      written_bytes = 0;
      
      // Asset entry size is in bytes.
      while (written_bytes < ae.size) {
        if (!dat_in.good()) {
          break;
        }
        fill(rdbuff.begin(), rdbuff.end(), 0);

        read_chunk_size = rdbuff.size();
        if (written_bytes + read_chunk_size > ae.size) {
          read_chunk_size = written_bytes % rdbuff.size();
        }
        dat_in.read(rdbuff.data(), read_chunk_size);
        if (dat_in.gcount() > 0) {
          // Bytes were read.
          if (rdbuff.size() >= dat_in.gcount()) {
            
            // Write read buffer to asset file.
            asset_out.write(rdbuff.data(), dat_in.gcount());
            if (asset_out.good()) {
              written_bytes += read_bytes;
            }
            else {
              cerr << "error: failed to write to asset file" << endl;
              return;
            }
          }
          else {
            cerr << "error: read more bytes than buffer can hold" << endl;
            return;
          }
        }
        else {
          cerr << "error: failed to read from dat file" << endl;
          return;
        }
      }

      cout << "done" << endl;
    };
  };
  
  asset_entries get_assets(const data_file& df) {
    asset_entries entries {};
    basic_ifstream<string::value_type> in(df.cat.native(), ios_base::in | ios_base::binary);
    // Enable stream exception throws for non-recoverable error states.
    in.exceptions(ios_base::badbit);

    // Format of .cat files is as follows:
    // [relative asset file name] [size in bytes] [timestamp] [md5 checksum]

    // @TODO Consider using Regular Expressions capturing.
    
    string line {}, md5(32, '\0'), number(128, '\0'), rel_path{};
    uint64_t sz, ts;
    typedef array<string::value_type, 256> readbuffer;
    readbuffer rdbuff;
    double readbytes {0}, filebytes {static_cast<double>(fs::file_size(df.cat))}, readprog {0};

    auto cout_precision = cout.precision();
    auto cout_flags = cout.flags();
    cout << fixed;
    cout.precision(2);
    chrono::time_point<chrono::steady_clock>
      start_time = chrono::steady_clock::now(),
      current_time = chrono::steady_clock::now();
    
    cout << "info: get assets list from file " << df.cat.native() <<  "... " << flush;

    // Line number counter.
    uint32_t ln { 0 };
    while (true) {
      if (in.eof()) {
        break;
      }

      // Clear read buffer.
      line.clear();
      string::value_type ch {'\0'};
      readbuffer::iterator it, end;
      while (!(ch == '\n' || in.eof())) {
        rdbuff.fill('\0');
        it = rdbuff.begin();
        end = rdbuff.end();
        end--;
        
        while (it != end) {
          in.get(ch);
          readbytes++;
          if (ch == '\n' || in.eof()) {
            // Make sure stats are accurate.
            if (in.eof()) {
              readbytes--;
            }
            break;
          }
          *it = ch;
          it++;
        }
        
        line.append(rdbuff.data());
      }

      readprog = ((readbytes / filebytes) * 100);
      if (1 <= chrono::duration_cast<chrono::seconds>(current_time - start_time).count()) {
        cout << readprog << "% " << flush;
        
        start_time = chrono::steady_clock::now();
        current_time = chrono::steady_clock::now();
      }
      else {
        current_time = chrono::steady_clock::now();
      }
      
      al::trim(line);
      if (line.empty()) {
        continue;
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
      if (rbegin == line.rend()) {
        cerr << "error: no timestamp part" << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        continue;
      }
      try {
        number.clear();
        rend = find(++rbegin, line.rend(), ' ');
        number.append(rend.base(), rbegin.base());
        ts = stoul(number);
      }
      catch (const logic_error& e) {
        cerr << "error: failed to parse timestamp: " << e.what() << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        continue;
      }

      // Parse size.
      if (rbegin == line.rend()) {
        cerr << "error: no file size part" << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        continue;
      }
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
        continue;
      }

      // Get relative asset filename.
      if (rbegin == line.rend()) {
        cerr << "error: no asset file name part" << endl;
        cerr << "\t\tline " << ln << ": " << line << endl;
        continue;
      }
      rel_path.append(line.begin(), (++rend).base());
      al::trim(rel_path);

      // Construct a new asset entry.
      asset_entry ae{ rel_path, sz, ts,{} };

      if (md5.length() == 32) {
        // @TODO Consider using another approach.
        strncpy(ae.checksum.data(), md5.c_str(), 32);
        ae.checksum.at(32) = '\0';
      }
      else {
        cerr << "error: invalid checksum string for asset `" << rel_path << "`" << endl;
        continue;
      }

      entries.push_back(ae);
    }

    cout << readprog << "% " << flush;
    cout << "done" << endl;

    cout.precision(cout_precision);
    cout.flags(cout_flags);
    
    return entries;
  };  
}

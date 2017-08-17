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
}

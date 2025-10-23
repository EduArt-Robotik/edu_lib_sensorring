#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace eduart {

struct Version {
  union {
    struct {
      unsigned int major;
      unsigned int minor;
      unsigned int patch;
    }v;
    unsigned int version[3];
  };

  std::string toString() const {
    std::ostringstream oss;
    oss << v.major << "." << v.minor << "." << v.patch;
    return oss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const Version& ver) { return os << ver.toString(); }
};

struct CommitHash {
  union {
    struct {
      std::uint8_t a;
      std::uint8_t b;
      std::uint8_t c;
      std::uint8_t d;
    }h;
    std::uint32_t hash;
  };

  std::string toString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(h.a) << std::setw(2) << static_cast<int>(h.b)
        << std::setw(2) << static_cast<int>(h.c) << std::setw(2) << static_cast<int>(h.d);
    return oss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const CommitHash& ch) { return os << ch.toString(); }
};

} // namespace eduart
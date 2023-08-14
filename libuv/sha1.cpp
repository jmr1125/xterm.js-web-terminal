#include "sha1.h"
// #include <iomanip>
// #include <iostream>
#include <ostream>
// using namespace std;
bool isbigendian() {
  static bool first = true;
  static bool isbig = false;
  if (!first) {
    first = false;
  } else {
    first = false;
    union {
      WORD a;
      BYTE b;
    } tester;
    tester.a = 0x1234;
    if (tester.b == 0x12) {
      isbig = true;
    } else {
      isbig = false;
    }
  }
  return isbig;
}
WORD reverse(WORD x) { return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8); };
DWORD reverse(DWORD x) {
  return ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
         ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24);
};
QWORD reverse(QWORD x) {
  return ((x & 0x00000000000000ff) << 56) | ((x & 0x000000000000ff00) << 40) |
         ((x & 0x0000000000ff0000) << 24) | ((x & 0x00000000ff000000) << 8) |
         ((x & 0x000000ff00000000) >> 8) | (x & 0x0000ff0000000000) >> 24 |
         ((x & 0x00ff000000000000) >> 40) | ((x & 0xff00000000000000) >> 56);
}
WORD to_big(WORD x) { return isbigendian() ? x : reverse(x); }
DWORD to_big(DWORD x) { return isbigendian() ? x : reverse(x); }
QWORD to_big(QWORD x) { return isbigendian() ? x : reverse(x); }
vector<bool> pad(vector<bool> s) {
  QWORD size = s.size();
  s.insert(s.end(), {1, 0, 0, 0, 0, 0, 0, 0});
  size = size + 8;
  if (size % 512 != 448) {
    s.reserve(size + 448 - (size % 512) + (size < 448 ? 0 : 512) + 64);
    for (int i = size; i % 512 != 448; ++i) {
      if (i == size) {
        // s.push_back(1);
        s.push_back(0);
      } else {
        s.push_back(0);
      }
    }
    s.shrink_to_fit();
  } else {
    s.reserve(size + 64);
  }
  size = size - 8;
  for (int i = 0; i < 64 / 8; ++i) {
    // s.push_back(to_big(size) & ((QWORD)1 << i));
    QWORD tmp = to_big(size);
    // cout << hex << tmp << endl;
    // cout << hex << setw(16) << ((QWORD)0xff << (8 * i)) << endl;
    tmp = ((tmp & ((QWORD)0xff << (8 * i))) >> (8 * i));
    s.push_back(tmp & 0x80);
    s.push_back(tmp & 0x40);
    s.push_back(tmp & 0x20);
    s.push_back(tmp & 0x10);
    s.push_back(tmp & 0x08);
    s.push_back(tmp & 0x04);
    s.push_back(tmp & 0x02);
    s.push_back(tmp & 0x01);
  }
  return s;
}

vector<vector<bool>> blocks(vector<bool> s) {
  assert(s.size() % 512 == 0);
  vector<vector<bool>> res;
  res.resize(s.size() / 512);
  for (int i = 0; i < s.size(); ++i) {
    res.at(i / 512).resize(512);
    res.at(i / 512).at(i % 512) = s[i];
  }
  return res;
}
vector<DWORD> blocksGetw(vector<bool> s) {
  vector<DWORD> w;
  w.resize(80);
  for (int i = 32 * 0; i < 32 * 16; ++i) {
    w.at(i / 32) |= (s[i] << (31 - i % 32));
  }
  for (int i = 16; i < 80; ++i) {
    // DWORD tmp = to_big(w.at(i - 3) ^ w.at(i - 8) ^ w.at(i - 14) ^ w.at(i -
    // 16));
    DWORD tmp = (w.at(i - 3) ^ w.at(i - 8) ^ w.at(i - 14) ^ w.at(i - 16));
    // printf("%d : %08x %08x %08x %08x\n%08x %08x\n%08x,%08x\n", i, w.at(i -
    // 3),
    //        w.at(i - 8), w.at(i - 14), w.at(i - 16), tmp, (tmp << 1 | tmp >>
    //        31), (tmp << 1), (tmp >> 31));
    w.at(i) = (tmp << 1 | tmp >> 31);
  }
  return w;
}
int F(int i, vector<DWORD> abcde) {
  //  DWORD a = abcde[0];
  DWORD b = abcde[1];
  DWORD c = abcde[2];
  DWORD d = abcde[3];
  // DWORD e = abcde[4];
  if (i < 20) {
    return (b & c) | ((~b) & d);
  } else if (i < 40) {
    return b ^ c ^ d;
  } else if (i < 60) {
    return (b & c) | (b & d) | (c & d);
  } else {
    assert(i < 80);
    return b ^ c ^ d;
  }
}
DWORD K(int i) {
  if (i < 20) {
    return 0x5a827999;
  } else if (i < 40) {
    return 0x6ed9eba1;
  } else if (i < 60) {
    return 0x8f1bbcdc;
  } else {
    assert(i < 80);
    return 0xca62c1d6;
  }
}
vector<DWORD> loop80(vector<DWORD> w, vector<DWORD> abcde) {
  assert(abcde.size() == 5);
  DWORD a = abcde[0];
  DWORD b = abcde[1];
  DWORD c = abcde[2];
  DWORD d = abcde[3];
  DWORD e = abcde[4];
  for (int i = 0; i < 80; ++i) {
    int tmp =
        e + F(i, {a, b, c, d, e}) + ((a << 5) | (a >> 27)) + w.at(i) + K(i);
    e = d;
    d = c;
    c = ((b << 30) | (b >> 2));
    b = a;
    a = tmp;
  }
  return {a + abcde[0], b + abcde[1], c + abcde[2], d + abcde[3], e + abcde[4]};
}
vector<bool> to_vector(string s) {
  vector<bool> res;
  res.reserve(s.size() * 8);
  for (char c : s) {
    res.push_back(c & 0x80);
    res.push_back(c & 0x40);
    res.push_back(c & 0x20);
    res.push_back(c & 0x10);
    res.push_back(c & 0x08);
    res.push_back(c & 0x04);
    res.push_back(c & 0x02);
    res.push_back(c & 0x01);
  }
  return res;
}
vector<bool> to_vector(DWORD x) {
  return {static_cast<bool>(x & 0x80000000), static_cast<bool>(x & 0x40000000),
          static_cast<bool>(x & 0x20000000), static_cast<bool>(x & 0x10000000),
          static_cast<bool>(x & 0x08000000), static_cast<bool>(x & 0x04000000),
          static_cast<bool>(x & 0x02000000), static_cast<bool>(x & 0x01000000),
          static_cast<bool>(x & 0x00800000), static_cast<bool>(x & 0x00400000),
          static_cast<bool>(x & 0x00200000), static_cast<bool>(x & 0x00100000),
          static_cast<bool>(x & 0x00080000), static_cast<bool>(x & 0x00040000),
          static_cast<bool>(x & 0x00020000), static_cast<bool>(x & 0x00010000),
          static_cast<bool>(x & 0x00008000), static_cast<bool>(x & 0x00004000),
          static_cast<bool>(x & 0x00002000), static_cast<bool>(x & 0x00001000),
          static_cast<bool>(x & 0x00000800), static_cast<bool>(x & 0x00000400),
          static_cast<bool>(x & 0x00000200), static_cast<bool>(x & 0x00000100),
          static_cast<bool>(x & 0x00000080), static_cast<bool>(x & 0x00000040),
          static_cast<bool>(x & 0x00000020), static_cast<bool>(x & 0x00000010),
          static_cast<bool>(x & 0x00000008), static_cast<bool>(x & 0x00000004),
          static_cast<bool>(x & 0x00000002), static_cast<bool>(x & 0x00000001)};
}

std::ostream &operator<<(std::ostream &ost, vector<bool> x) {
  ost << "[";
  for (int i = 0; i < x.size(); i += 4) {
    int v = x[i] * 8 + x[i + 1] * 4 + x[i + 2] * 2 + x[i + 3] * 1;
    if (i % (8 * 4) == 0) {
      ost << ' ';
    }
    if (i % (32 * 4) == 0) {
      ost << std::endl;
    }
    ost << ((string) "0123456789abcdef").at(v);
  }
  ost << "\n]";
  return ost;
}

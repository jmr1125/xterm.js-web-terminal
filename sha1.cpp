#include "sha1.h"
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
    tmp = ((tmp & (0xff << (8 * i))) >> (8 * i));
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
#define loop(tmp1, tmp2)                                                       \
  DWORD temp = tmp1;                                                           \
  DWORD temp2 = tmp2;                                                          \
  e = d;                                                                       \
  d = c;                                                                       \
  c = to_big((to_big(b) << 30) | (to_big(b) >> 2));                            \
  b = a;                                                                       \
  a = temp + temp2 + e + w.at(i);
// a = to_big(to_big(temp) + to_big(temp2) + to_big(e) + to_big(w.at(i)));
vector<DWORD> loop80(vector<DWORD> w, vector<DWORD> abcde) {
  assert(abcde.size() == 5);
  DWORD a = abcde[0];
  DWORD b = abcde[1];
  DWORD c = abcde[2];
  DWORD d = abcde[3];
  DWORD e = abcde[4];
  for (int i = 0; i < 20; ++i) {
    // loop(to_big(to_big((b & c) | ((~b) & d)) + to_big((DWORD)0x5a827999)),
    //      to_big((to_big(a) << 5) | (to_big(a) >> 27)));
    loop(((b & c) | ((~b) & d)) + (DWORD)0x5a827999, (a << 5) | (a >> 27));
  }
  for (int i = 20; i < 40; ++i) {
    // loop(to_big(to_big(b ^ c ^ d) + to_big((DWORD)0x6ed9eba1)),
    //      to_big((to_big(a) << 5) | (to_big(a) >> 27)));
    loop((b ^ c ^ d) + (DWORD)0x6ed9eba1, (a << 5) | (a >> 27));
  }
  for (int i = 40; i < 60; ++i) {
    // loop(
    //     to_big(to_big((b & c) | (b & d) | (c & d)) +
    //     to_big((DWORD)0x8f1bbcdc)), to_big(to_big(a << 5) | to_big(a >>
    //     27)));
    loop(((b & c) | (b & d) | (c & d)) + (DWORD)0x8f1bbcdc,
         (a << 5) | (a >> 27));
  }
  for (int i = 60; i < 80; ++i) {
    // loop(to_big(to_big(b ^ c ^ d) + to_big((DWORD)0xca62c1d6)),
    //      to_big((to_big(a) << 5) | (to_big(a) >> 27)));
    loop((b ^ c ^ d) + (DWORD)0xca62c1d6, (a << 5) | (a >> 27));
  }
  return {a, b, c, d, e};
}

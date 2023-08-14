#include "base64.h"
struct base64code {
  base64code() {
    //======ENCODE======
    int id = 0;
    for (char i = 'A'; i <= 'Z'; ++i) {
      base64en[id++] = i;
    }
    for (int i = 'a'; i <= 'z'; ++i) {
      base64en[id++] = i;
    }
    for (int i = '0'; i <= '9'; ++i) {
      base64en[id++] = i;
    }
    base64en[id++] = '+';
    base64en[id++] = '/';
    //======DECODE=======
    for (int i = 0; i < 64; ++i) {
      base64de[base64en[i]] = i;
    }
  }
  char base64en[64];
  int base64de[256];
} base;
vector<bool> decode(string s) {
  vector<bool> res;
  assert(s.size() % 4 == 0);
  int numequ = 0;
  for (int i = 0; i < s.size(); ++i) {
    int x = base.base64de[s[i]];
    if (s[i] == '=') {
      ++numequ;
      continue;
    }
    res.push_back(x & 0x20);
    res.push_back(x & 0x10);
    res.push_back(x & 0x08);
    res.push_back(x & 0x04);
    res.push_back(x & 0x02);
    res.push_back(x & 0x01);
  }
  res.resize(res.size() - (res.size() % 24) + (8 * (3 - numequ)) % 24);
  return res;
}
string encode(vector<bool> v) {
  string res = "";
  int i;
  assert(v.size() % 8 == 0);
  int size = v.size();
  // v.resize(v.size() + (6 - (v.size() % 6)) % 6);
  for (i = 0; i < size; i += 6) {
    int x = v[i] * 32 + v[i + 1] * 16 + v[i + 2] * 8 + v[i + 3] * 4 +
            v[i + 4] * 2 + v[i + 5];
    res += base.base64en[x];
  }
  while (res.size() % 4 != 0)
    res += '=';
  return res;
}

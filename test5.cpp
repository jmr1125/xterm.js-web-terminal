#include "base64.h"
#include "sha1.h"
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::ostream;
ostream &operator<<(ostream &ost, vector<bool> x) {
  ost << "[";
  for (int i = 0; i < x.size(); i += 4) {
    int v = x[i] * 8 + x[i + 1] * 4 + x[i + 2] * 2 + x[i + 3] * 1;
    if (i % (8 * 4) == 0) {
      ost << ' ';
    }
    if (i % (32 * 4) == 0) {
      ost << endl;
    }
    ost << ((string) "0123456789abcdef").at(v);
  }
  ost << "\n]";
  return ost;
}
int main() {
  string s;
  while (cin >> s) {
    vector<bool> vect = to_vector(s);
    string base64 = encode(vect);
    cout << "base64: " << base64 << endl;
    cout << "decode: " << decode(base64) << endl;
    cout << "expect: " << vect << endl;
  }
}

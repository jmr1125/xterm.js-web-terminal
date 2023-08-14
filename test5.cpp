#include "base64.h"
#include "sha1.h"
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::ostream;
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

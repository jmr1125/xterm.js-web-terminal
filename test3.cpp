#include "sha1.h"
#include <ios>
#include <iostream>
#include <sys/stat.h>
using namespace std;
ostream &operator<<(ostream &ost, vector<bool> x) {
  ost << "[";
  // int id = 0;
  // for (bool i : x) {
  //   if (id % 64 == 0) {
  //     ost << endl;
  //   }
  //   ost << i << ' ';
  //   ++id;
  // }
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
  vector<bool> input{
      0, 1, 0, 0, 1, 0, 0, 0, /*H*/
      0, 1, 0, 0, 0, 1, 0, 1, /*E*/
      0, 1, 0, 0, 1, 1, 0, 0, /*L*/
      0, 1, 0, 0, 1, 1, 0, 0, /*L*/
      0, 1, 0, 0, 1, 1, 1, 1  /*O*/
  };
  // cout << "input: " << input << endl;
  // auto resized = resize512(input);
  // cout << "resized: " << resized << endl;
  // cout << "blocks" << endl;
  // auto block = blocks(resized);
  // for (auto block : block) {
  //   cout << block << endl;
  // }
  // auto abcde = loop80(blocksGetw(block[0]), Initabcde);
  // cout << hex << abcde[0] << ' ' << abcde[1] << ' ' << abcde[2] << ' '
  //      << abcde[3] << ' ' << abcde[4] << endl;
  auto paddata = pad(input);
  cout << paddata << endl;
  auto w = blocksGetw(blocks(paddata)[0]);
  // cout << hex;
  // for (int i = 0; i < 80; ++i) {
  //   if (i % 16 == 0) {
  //     cout << endl;
  //   }
  //   cout << w[i] << ' ';
  // }
  // cout << endl;
  auto abcde = loop80(w, Initabcde);
  // cout << hex << to_big(to_big(abcde[0]) + to_big(Initabcde[0])) << ' '
  //      << to_big(to_big(abcde[1]) + to_big(Initabcde[1])) << ' '
  //      << to_big(to_big(abcde[2]) + to_big(Initabcde[2])) << ' '
  //      << to_big(to_big(abcde[3]) + to_big(Initabcde[3])) << ' '
  //      << to_big(to_big(abcde[4]) + to_big(Initabcde[4])) << endl;
  cout << hex << abcde[0] + Initabcde[0] << ' ' << abcde[1] + Initabcde[1]
       << ' ' << abcde[2] + Initabcde[2] << ' ' << abcde[3] + Initabcde[3]
       << ' ' << abcde[4] + Initabcde[4] << ' ';
}

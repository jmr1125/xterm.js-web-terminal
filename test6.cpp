#include "base64.h"
#include "sha1.h"
#include <iostream>
using std::cout;
using std::endl;
std::ostream &operator<<(std::ostream &ost, vector<bool> x) {
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
  vector<bool> result;
  const string magickey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  string key = "dGhlIHNhbXBsZSBub25jZQ==";
  key += magickey;
  vector<vector<bool>> Blocks = blocks(pad(to_vector(key)));
  vector<DWORD> res = Initabcde;
  for (vector<bool> b : Blocks) {
    cout << b << endl << "====";
    res = loop80(blocksGetw(b), res);
  }
  for (int i = 0; i < 5; ++i) {
    vector<bool> tmp;
    tmp = to_vector(res[i]);
    cout << std::hex << res[i] << ' ';
    result.insert(result.end(), tmp.begin(), tmp.end());
  }
  cout << endl;
  cout << "result: " << encode(result) << endl;
  cout << "expect: "
       << "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=" << endl;
}

/*
dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11

GET /chat HTTP/1.1
Host: example.com:8000
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 13

HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
*/

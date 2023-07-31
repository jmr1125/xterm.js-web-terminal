#include "base64.h"
#include "sha1.h"
#include "ws.h"
#include <iostream>
#include <ostream>
using std::cout;
using std::endl;
void test(string s1, string expect) {
  vector<bool> result;
  const string magickey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  string key = s1;
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
  cout << "expect: " << expect << endl;
}
int main() {
  test("dGhlIHNhbXBsZSBub25jZQ==", "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
  test("0WwsB8bLMlLJ9x9/V7s9rQ==", "b3xeeAGFIMKaSYqvbLdYmC+LaDA=");
  cout << "      : " << get_Sec_ws_Accept("0WwsB8bLMlLJ9x9/V7s9rQ==") << endl;
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

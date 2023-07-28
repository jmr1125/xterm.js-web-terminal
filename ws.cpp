#include "ws.h"
const static inline string magickey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B1";
string get_Sec_ws_Accept(string key) {
  auto Blocks = blocks(pad(to_vector(key + magickey)));
  vector<DWORD> res = Initabcde;
  for (const auto b : Blocks) {
    res = loop80(blocksGetw(b), res);
  }
  vector<bool> Res;
  for (DWORD x : res) {
    auto tmp = to_vector(x);
    Res.insert(Res.end(), tmp.begin(), tmp.end());
  }
  return encode(Res);
}

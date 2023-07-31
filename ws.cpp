#include "ws.h"
#include "sha1.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <ios>
// #include <iostream>
// using namespace std;

const static inline string magickey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
string get_Sec_ws_Accept(string key) {
  auto Blocks = blocks(pad(to_vector(key + magickey)));
  vector<DWORD> res = Initabcde;
  for (const auto b : Blocks) {
    // cout << b << endl << "===";
    res = loop80(blocksGetw(b), res);
  }
  vector<bool> Res;
  for (DWORD x : res) {
    auto tmp = to_vector(x);
    Res.insert(Res.end(), tmp.begin(), tmp.end());
  }
  return encode(Res);
}

QWORD get_data(const vector<bool> data, const int i, const int j) {
  if (i == j) {
    return data.at(i);
  }
  QWORD res = 0;
  for (int x = i; x <= j; ++x) {
    res |= (data.at(x) << (j - x));
  }
  return res;
}
wsFrame to_frame(string s) {
  vector<bool> data = to_vector(s);
  wsFrame res;
  // res.FIN = data[0];
  // res.RSV1 = data[1];
  // res.RSV2 = data[2];
  // res.RSV3 = data[3];
  // res.opcode = ((data[4] << 3) | (data[5] << 2) | (data[6] << 1) |
  // (data[7])); res.MASK = data[8]; res.payloadLen =
  //     ((data[9] << 6) | (data[10] << 5) | (data[11] << 4) | (data[12] << 3) |
  //      (data[13] << 2) | (data[14] << 1) | (data[15]));
  res.FIN = get_data(data, 0, 0);
  res.RSV1 = get_data(data, 1, 1);
  res.RSV2 = get_data(data, 2, 2);
  res.RSV3 = get_data(data, 3, 3);
  res.opcode = get_data(data, 4, 7);
  res.MASK = get_data(data, 8, 8);
  res.PayloadLen = get_data(data, 9, 15);
  int id;
  if (res.PayloadLen == 126) { // 16
    res.extpayloadLen = get_data(data, 16, 31);
    id = 32;
  } else if (res.PayloadLen == 127) { // 64
    res.extpayloadLen = get_data(data, 16, 79);
    id = 80;
  } else {
    id = 16;
  }
  if (res.MASK) {
    res.maskingKey = get_data(data, id, id + 31);
    id = id + 32;
  }
  /*read everything except data*/
  if (res.PayloadLen == 126) { // 16
    res.len = res.extpayloadLen;
  } else if (res.PayloadLen == 127) { // 64
    res.len = res.extpayloadLen;
  } else {
    res.len = res.PayloadLen;
  }
  res.PayloadData.resize(res.len * 8);
  std::copy(data.begin() + id, data.end(), res.PayloadData.begin());
  assert(res.PayloadData.size() % 8 == 0);
  res.data.resize(res.PayloadData.size() / 8);
  for (int i = 0; i < res.PayloadData.size(); i += 8) {
    bool x1 = res.PayloadData[i], x2 = res.PayloadData[i + 1],
         x3 = res.PayloadData[i + 2], x4 = res.PayloadData[i + 3],
         x5 = res.PayloadData[i + 4], x6 = res.PayloadData[i + 5],
         x7 = res.PayloadData[i + 6], x8 = res.PayloadData[i + 7];
    BYTE x = ((x1 << 7) | (x2 << 6) | (x3 << 5) | (x4 << 4) | (x5 << 3) |
              (x6 << 2) | (x7 << 1) | (x8));
    if (res.MASK) {
      BYTE y = ((to_big(res.maskingKey) & (0xff << i)) >> i);
      // std::printf("==mask: %x origin: %x\n", y, x);
      x ^= y;
    }
    res.data[i / 8] = x;
  }
  return res;
}

string from_frame(wsFrame x) {
  x.MASK = false;
  vector<bool> v(
      {x.FIN, x.RSV1, x.RSV2, x.RSV3, static_cast<bool>(x.opcode & 0x8),
       static_cast<bool>(x.opcode & 0x4), static_cast<bool>(x.opcode & 0x2),
       static_cast<bool>(x.opcode & 0x1), x.MASK,
       static_cast<bool>(x.PayloadLen & 0x40),
       static_cast<bool>(x.PayloadLen & 0x20),
       static_cast<bool>(x.PayloadLen & 0x10),
       static_cast<bool>(x.PayloadLen & 0x8),
       static_cast<bool>(x.PayloadLen & 0x4),
       static_cast<bool>(x.PayloadLen & 0x2),
       static_cast<bool>(x.PayloadLen & 0x1)});
  if (x.len < 126) {
    x.PayloadLen = x.len;
  } else {
    if (x.len < 0xffff) {
      x.PayloadLen = 126;
      x.extpayloadLen = x.len;
    } else {
      x.PayloadLen = 127;
      x.extpayloadLen = x.len;
    }
  }
  if (x.PayloadLen == 126 || x.extpayloadLen == 127) {
    for (int i = ((x.PayloadLen == 126) ? 15 : 63); i >= 0; --i) {
      v.insert(v.end(), x.extpayloadLen & (1 << i));
    }
  }
  auto tmp = x.data;
  x.PayloadData.resize(x.len * 8);
  for (int i = 0; i < x.len; ++i) {
    x.PayloadData[i * 8 + 0] = tmp[i] & 0x80;
    x.PayloadData[i * 8 + 1] = tmp[i] & 0x40;
    x.PayloadData[i * 8 + 2] = tmp[i] & 0x20;
    x.PayloadData[i * 8 + 3] = tmp[i] & 0x10;
    x.PayloadData[i * 8 + 4] = tmp[i] & 0x8;
    x.PayloadData[i * 8 + 5] = tmp[i] & 0x4;
    x.PayloadData[i * 8 + 6] = tmp[i] & 0x2;
    x.PayloadData[i * 8 + 7] = tmp[i] & 0x1;
  }
  v.reserve(v.size() + x.len * 8);
  v.insert(v.end(), x.PayloadData.begin(), x.PayloadData.end());
  string res = "";
  for (int i = 0; i < v.size(); i += 8) {
    res += (char)((v[i + 0] << 7) | (v[i + 1] << 6) | (v[i + 2] << 5) |
                  (v[i + 3] << 4) | (v[i + 4] << 3) | (v[i + 5] << 2) |
                  (v[i + 6] << 1) | (v[i + 7] << 0));
  }
  return res;
}

ostream &operator<<(ostream &ost, const wsFrame &x) {
  ost << "FIN: " << x.FIN << "\n";
  ost << "RSV123: " << x.RSV1 << x.RSV2 << x.RSV3 << "\n";
  ost << "opcode: " << std::hex << (int)x.opcode << "\n";
  ost << "MASK: " << x.MASK << "\n";
  ost << "Payload len: " << std::dec << (int)x.PayloadLen << "\n";
  if (x.PayloadLen == 126 | x.PayloadLen == 127) {
    ost << "Extended payload len: " << x.extpayloadLen << "\n";
  }
  if (x.MASK) {
    ost << "Masking key: " << std::hex << x.maskingKey << "\n";
  }
  ost << "data: " << x.PayloadData << "\n";
  if (x.MASK) {
    for (int i = 0; i < x.data.size(); ++i) {
      auto c = x.data[i];
      if (std::isprint(c)) {
        ost << c;
      } else {
        ost << ".";
      }
      if (i % 8 == 0 && i) {
        ost << "\n";
      }
    }
  }
  return ost;
}

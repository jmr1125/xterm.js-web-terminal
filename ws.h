#include "base64.h"
#include "sha1.h"
#include <ostream>
using std::ostream;

string get_Sec_ws_Accept(string);

struct wsFrame {
  bool FIN;  // 1
  bool RSV1; // 1
  bool RSV2;
  bool RSV3;
  BYTE opcode;         // 7
  bool MASK;           // 1
  BYTE PayloadLen;     // 7
  QWORD extpayloadLen; // 16/64
  DWORD maskingKey;       // 32
  vector<bool> PayloadData;
  string data;
  size_t len;
};
wsFrame to_frame(string);
string from_frame(wsFrame);
ostream &operator<<(ostream &, const wsFrame &);

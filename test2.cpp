#include "socket.h"
#include "uv.h"
#include "ws.h"
#include <iostream>
#include <regex>
#error do not use it
using namespace std;
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
  Socket sock("127.0.0.1", 9000);
  sock.listen_accept();
  while (!sock.client)
    ;
  string s;
  // for (; !s.size();) {
  //   if (!sock.read(s)) {
  //     cout << s << endl;
  //   }
  // }
  /*======MAIN========*/
  const regex line("(.+)\r\n(.*)");
  while (!s.size()) {
    if (!sock.read(s)) {
      cout << s << endl;
    }
  }
  const regex secwskey("Sec-WebSocket-Key: (.+)");
  smatch res;
  if (regex_search(s, res, secwskey)) {
    cout << "get key";
    cout << res[1] << endl;
  }
  string handshake =
      "HTTP/1.1 101 Switching Protocols\r\nUpgrade: "
      "websocket\r\nConnection: \r\nUpgrade\r\nSec-WebSocket-Accept: " +
      get_Sec_ws_Accept(res[1]);
  cout << "handshake: " << handshake << endl;
  sock.write(handshake);
  for (; getchar() != 'q';) {
    if (!sock.read(s)) {
      cout << "=====" << endl;
      cout << s << endl;
      cout << to_vector(s) << endl;
    }
  }
  return 0;
}

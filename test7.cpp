#include "socket.h"
#include "uv.h"
#include "ws.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <regex>
using namespace std;
Socket sock("127.0.0.1", 8080);
int server(int i) {
  message_t s;
  {
    s = sock.waitread(i);
    if (s.first < 0) {
      cout << uv_strerror(s.first) << endl;
      return 1;
    }
  }
  {
    const regex r("Sec-WebSocket-Key: (.+)");
    smatch sm;
    if (!regex_search(s.second, sm, r)) {
      cout << "no key found in \n" << s.second << endl;
      return 1;
    }
    // cout << "key = " << sm[1] << endl;
    string handshake;
    handshake.append("HTTP/1.1 101 Switching Protocols\r\n");
    handshake.append("Upgrade: websocket\r\n");
    handshake.append("Connection: Upgrade\r\n");
    handshake.append("Sec-WebSocket-Accept: ");
    handshake.append(get_Sec_ws_Accept(sm[1]));
    handshake.append("\r\n\r\n");
    sock.write(handshake, i);
  }
  // cout << "vvvvvvvv" << s.second << endl << "^^^^^^^^" << endl;
  while (true) {
    s = sock.waitread(i);
    if (s.first < 0) {
      cout << uv_strerror(s.first) << endl;
      return 1;
    }
    wsFrame frame = to_frame(s.second);
    // cout << "vvvvvvv#FRAME#vvvvvvv" << endl
    //      << frame << endl
    //      << "^^^^^^^#FRAME#^^^^^^^" << endl;
    // cout << "data = " << frame.data << endl;
    if (frame.opcode == 8) {
      // cout << "=======DISCONNECT========" << endl;
      break;
    }
    {
      static int ID = 0;
      vector<string> v{"hi", "bonjour", "你好"};
      string data = (v[ID++]);
      // cin >> data;
      wsFrame tmp = get_frame(data);
      // cout << "vvvvvvv#WRITE#vvvvvvv" << endl
      //      << tmp << endl
      //      << "^^^^^^^#WRITE#^^^^^^^" << endl;
      sock.write(from_frame(tmp), i);
    }
  }
  sock.close(i);
  return 0;
}
#include <ctime>
int main() {
  thread *threadpool[MAX_CLIENT] = {NULL};
  bool used[MAX_CLIENT];
  sock.listen();
  bool first = true;
  while ( // getchar() != 'q'
      first || [used]() -> bool {
        for (int i = 0; i < MAX_CLIENT; ++i) {
          if (used[i]) {
            // cout << "there's still client!!" << endl;
            return true;
          }
        }
        return false;
      }()) {
    cout << "refresh..." << endl;
    for (int i = 0; i < MAX_CLIENT; ++i) {
      cout << i << ": ";
      if (Pool_used(i) && !used[i]) {
        cout << "new!";
        assert(threadpool[i] == NULL);
        threadpool[i] = new (thread)(server, i);
        used[i] = true;
        first = false;
      } else if (!Pool_used(i) && used[i]) {
        cout << "delete!";
        threadpool[i]->join();
        delete threadpool[i];
        threadpool[i] = NULL;
        used[i] = false;
      } else {
        cout << "nothing..";
      }
      cout << "   ";
      cout << endl;
    }
    cout << "\033[17A";
    cout.flush();
    {
      int now = clock();
      while (clock() - now <= 2 * CLOCKS_PER_SEC)
        ;
    }
  }
}

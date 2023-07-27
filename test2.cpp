#include "socket.h"
#include "uv.h"
#include <iostream>
#include <regex>
using namespace std;
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
  regex line("(.+)\r\n(.*)");
  return 0;
}

/* https://www.cnblogs.com/xumaojun/p/8526488.html */

#include "ws.h"
void parse_str(string request, string secwskey) {
  request += "HTTP/1.1 101 Switching Protocols\r\n";
  request += "Connection: upgrade\r\n";
  request += "Sec-WebSocket-Accept: ";
  const string MAGIC_KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  string key = secwskey + MAGIC_KEY;
  SHA1 sha;
  sha.update(key);
  uint32_t msg_dgt[5];
  for (int i = 0; i < 5; ++i) {
    msg_dgt[i] = htonl(sha.digest[i]);
  }
  string server_key;
  unsigned char result_sha[20];
  memcpy(result_sha, msg_dgt, sizeof result_sha);
  server_key = base64_encode(result_sha, 20, false);
  server_key += "\r\n";
  request += server_key;
  request += "Upgrade: websocket\r\n\r\n";
}

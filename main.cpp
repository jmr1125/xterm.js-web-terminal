#include "shell_runner.h"
#include "socket.h"
#include "uv.h"
#include "ws.h"
#include <cstddef>
#include <cstdio>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <sys/signal.h>
#include <unistd.h>
using namespace std;
Socket sock("127.0.0.1", 8080);
void httpserver(int i, string req) {
  string file;
  {
    const regex head0(R"(GET ([a-zA-Z0-9\.\/_]+) HTTP/[0-9]\.[0-9])");
    smatch sm;
    if (regex_search(req, sm, head0)) {
      file = sm[1];
    } else {
      cout << i << "->"
           << "don't match" << endl;
      sock.write("HTTP/1.1 500 Internal Server Error", i);
      return;
    }
  }
  file = '.' + file;
  cout << i << "->"
       << "file: " << file << endl;
  ifstream ist(file.c_str(), ios_base::in);
  string response;
  if (!ist) { // R404
    response.append("HTTP/1.1 404 Not Found\r\n");
  } else {
    response.append("HTTP/1.1 200 OK\r\n");
  }
  string Date;
  string MIMEtype;
  long long len = 0;
  {
    time_t raw;
    tm *gmt;
    time(&raw);
    gmt = gmtime(&raw);
    string week[] = {"Sun", "Mon", "Tues", "Wed", "Thrs", "Fri", "Sat"};
    string month[] = {"Jan", "Feb", "Mar",  "Apr", "May", "Jun",
                      "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
    Date += week[gmt->tm_wday];
    Date += ", ";
    Date += to_string(gmt->tm_mday);
    Date += ' ';
    Date += month[gmt->tm_mon];
    Date += ' ';
    Date += to_string(gmt->tm_year + 1900);
    Date += ' ';
    if (gmt->tm_hour < 10) {
      Date += '0';
    }
    Date += to_string(gmt->tm_hour);
    Date += ':';
    if (gmt->tm_min < 10) {
      Date += '0';
    }
    Date += to_string(gmt->tm_min);
    Date += ':';
    if (gmt->tm_sec < 10) {
      Date += '0';
    }
    Date += to_string(gmt->tm_sec);
  }
  {
    int i = file.find_last_of('.');
    string type;
    if (i == -1) {
      type = "";
    } else
      type = file.substr(i);
    if (type == ".html") {
      MIMEtype = "text/html";
    } else if (type == ".css") {
      MIMEtype = "text/css";
    } else if (type == ".js") {
      MIMEtype = "text/javascript";
    } else
      MIMEtype = "text/plain";
  }
  {
    ist.seekg(0, ios_base::end);
    len = ist.tellg();
    ist.seekg(0, ios_base::beg);
    // cout << "now at " << ist.tellg() << endl;
  }
  response.append((string) "Date: " + Date + "\r\n");
  response.append("contect-Type: " + MIMEtype + "\r\n");
  response.append("contect-Length: " + to_string(len) + "\r\n");
  response.append("\r\n");
  int t = 0;
  while (ist.good()) {
    char c = ist.get();
    response += c;
    ++t;
  }
  cout << "t = " << t << endl;
  cout << i << "->"
       << "fail" << (ist.rdstate() & ifstream::failbit) << endl;
  cout << i << "->"
       << "eof " << (ist.rdstate() & ifstream::eofbit) << endl;
  cout << i << "->"
       << "bad " << (ist.rdstate() & ifstream::badbit) << endl;

  cout << i << "-> vvvv" << endl
       << i << "->" << response.substr(0, 500) << "\n..." << endl
       << i << "-> ^^^^" << endl;
  sock.write(response, i);
}
void wsserver(int i, string key) {
  string handshake;
  handshake.append("HTTP/1.1 101 Switching Protocols\r\n");
  handshake.append("Upgrade: websocket\r\n");
  handshake.append("Connection: Upgrade\r\n");
  handshake.append("Sec-WebSocket-Accept: ");
  handshake.append(get_Sec_ws_Accept(key));
  handshake.append("\r\n\r\n");
  sock.write(handshake, i);
  bool quit = false;
  pty term = start("/bin/sh");
  thread tmp([&]() {
    while (!quit) {
      fd_set SET;
      FD_ZERO(&SET);
      FD_SET(term.x, &SET);
      timeval timeout;
      timeout.tv_usec = 500;
      int num = select(term.x + 1, &SET, NULL, NULL, &timeout);
      if (FD_ISSET(term.x, &SET)) {
        char data[256]{0};
        int ret = read(term.x, data, sizeof data);
        if (ret <= 0) {
          continue;
        }
        wsFrame tmp = get_frame(data);
        sock.write(from_frame(tmp), i);
      }
    }
  });
  while (true) {
    wsFrame frame;
    bool ok = true;
    message_t s;
    try {
      s = sock.waitread(i);
      if (s.first < 0) {
        cout << i << ">>" << uv_strerror(s.first) << endl;
        quit = true;
        tmp.join();
        cout << i << "->"
             << "kill: " << kill(term, SIGKILL) << endl;
        cout << i << "->"
             << "status: " << get_status(term) << endl;
        return;
      }
      frame = to_frame(s.second);
    } catch (const out_of_range &ex) {
      ok = false;
      cout << i << ">>"
           << "error on read frame" << endl;
    }
    if (ok) {
      write(term.x, frame.data.c_str(), frame.data.length());
    } else {
      cout << i << "->"
           << "not OK" << endl
           << frame.data << endl;
    }
    if (frame.opcode == 8) {
      cout << i << "->"
           << "=======DISCONNECT========" << endl;
      cout << i << "->"
           << "kill: " << kill(term, SIGKILL) << endl;
      cout << i << "->"
           << "status: " << get_status(term) << endl;
      break;
    }
  }
  quit = true;
  tmp.join();
}
int server(int i) {
  message_t s;
  {
    s = sock.waitread(i);
    if (s.first < 0) {
      cout << uv_strerror(s.first) << endl;
      return 1;
    }
    cout << i << "=>"
         << "read" << endl;
  }
  {
    const regex r("Sec-WebSocket-Key: (.+)");
    smatch sm;
    if (!regex_search(s.second, sm, r)) {
      cout << i << "=>"
           << "no key found" << endl;
      /*HTTP here*/
      cout << i << "=>"
           << "HTTP" << endl;
      httpserver(i, s.second);
    } else {
      cout << i << "=>"
           << "key = " << sm[1] << endl;
      cout << i << "=>"
           << "ws" << endl;
      wsserver(i, sm[1]);
    }
  }
  cout << i << "=>close" << endl;
  sock.close(i);
  cout << Pool_used(i) << endl;
  cout << Pool_used(i) << endl;
  cout << Pool_used(i) << endl;
  return 0;
}
#include <ctime>
int main() {
  thread *threadpool[MAX_CLIENT] = {NULL};
  sock.listen();
  bool quit = false;
  thread t([&quit]() {
    char c;
    do {
      cout << "Quit/Dump" << endl;
      if (c == 'd') {
        string res = "aaa";
        for (int i = 0; i < MAX_CLIENT; ++i) {
          // cout << i << " ";
          switch (get_pool(i).status) {
          case client_status::NON:
            res += "\033[0m";
            break;
          case client_status::NEW:
            res += "\033[32m";
            break;
          case client_status::RUNNING:
            res += "\033[31m";
            break;
          case client_status::DONE:
            res += "\033[34m";
            break;
          }
          res += to_string(i);
          res += "\033[0m";
        }
        cout << res << endl;
      }
    } while ((c = getchar()) != 'q');
    quit = true;
  });
  while (!quit) {
    for (int i = 0; i < MAX_CLIENT; ++i) {
      if (get_pool(i).status == client_status::NEW) {
        assert(threadpool[i] == NULL);
        threadpool[i] = new (thread)(server, i);
        get_pool(i).status = client_status::RUNNING;
      } else if (get_pool(i).status == client_status::DONE) {
        cout << "delete!";
        threadpool[i]->join();
        delete threadpool[i];
        threadpool[i] = NULL;
        get_pool(i).status = client_status::NON;
      }
    }
  }
  t.join();
}

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "util.h"
using namespace std;
#define _FD_SETSIZE 1024
int main() {
    tk_conf_t conf;
    cout<<read_conf("./conf.txt",&conf)<<endl;
    cout<<conf.root<<endl;
    cout<<conf.port<<conf.thread_num<<endl;
}
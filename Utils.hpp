#ifndef CONSTS_H
#define CONSTS_H

#include <string>

namespace Const {
    static const std::string SERVER_IP = "127.0.0.1";
    static const int SERVER_IP_BYTE1 = 127;
    static const int SERVER_IP_BYTE2 = 0;
    static const int SERVER_IP_BYTE3 = 0;
    static const int SERVER_IP_BYTE4 = 1;

    static constexpr short COMM_PORT = 3773;


    /////////////////////
    //// ERROR CODES ////
    /////////////////////
    static constexpr int ERR_NONE = 0;
    static constexpr int ERR_SERVER_INIT = 1;
    // ...
}


#endif
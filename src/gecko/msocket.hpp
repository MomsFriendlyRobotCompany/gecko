/**************************************************\
* The MIT License (MIT)
* Copyright (c) 2014 Kevin Walchko
* see LICENSE for full details
\**************************************************/
#pragma once

#include <arpa/inet.h>  // for sockaddr_in
#include <stdint.h>
#include <string>
#include <tuple>
#include <exception>

struct MulticastError : public std::exception {
    MulticastError(const std::string &s): msg("Multicast Error: " + s) {}
    MulticastError(): msg("Multicast Error") {}
    const char * what () const throw () {return msg.c_str();}
protected:
    std::string msg;
};

// http://www.kohala.com/start/mcast.api.txt
// http://www.kohala.com/start/
// https://rg3.name/201504241907.html


using MsgAddr = std::tuple<std::string, struct sockaddr_in>;

/*
 * Multicast socket for a beacon
 *
 */
class SSocket{
public:
    void init(
        std::string mc_addr_str={"224.3.29.110"}, // FIXME: address
        uint16_t mc_port=11311,
        uint8_t mc_ttl=1,
        bool reuse=false
    );
    bool ready(long msec=500);
    MsgAddr recv();
    MsgAddr recv_nb(long msec=500);
    bool send(const std::string& msg); // remove?
    bool send(const std::string& msg, struct sockaddr_in& addr); // sendto?

// protected:
    int sock;                   // socket descriptor
    struct sockaddr_in mc_addr; // socket address structure
    struct sockaddr_in from_addr;

};

#include "gecko/zmq_sub_pub.hpp"
#include "gecko/time.hpp"
#include "gecko/color.hpp"
#include "zmq.hpp"
#include <iostream>


using namespace std;

Publisher::Publisher(): zmqBase(ZMQ_PUB){}

void Publisher::publish(zmq::message_t& msg){
    sock.send(msg);
}

///////////////////////////////////////////////////

Subscriber::Subscriber(const string& topic): zmqBase(ZMQ_SUB){}

zmq::message_t Subscriber::recv(int flags){
    zmq::message_t msg;
    // if(check()) sock.recv(&msg, flags);
    sock.recv(&msg, flags);
    return msg;
}

////////////////////////////////////////////////////////
/*
Will bind or connect to an address (tcp://x.x.x.x:*, where * can be replacced
with a port number if desired)
https://stackoverflow.com/questions/16699890/connect-to-first-free-port-with-tcp-using-0mq
*/
// Publisher::Publisher(string addr, bool bind): zmqBase(ZMQ_PUB)
// {
//     if (bind) {
//         this->bind = true;
//         // https://github.com/pi-hole/FTL/blob/master/socket.c#L63
//         // setsockopt(int option_, const void *optval_, size_t optvallen_)
//         // sock.setsockopt(SO_REUSEADDR, &(int){ 1 }, sizeof(int));
//         sock.bind(addr);
//     }
//     else sock.connect(addr);
//
//     setEndPt();
//     printf(">> Publisher[%s] %s\n",endpoint.c_str(), bind ? "bind" : "connect");
// }

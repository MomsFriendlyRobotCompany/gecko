#include <gecko/zmq_rep_req.hpp>
#include <gecko/time.hpp>
#include <gecko/color.hpp>
#include <zmq.hpp>
#include <iostream>

using namespace std;


Reply::Reply(std::string addr): zmqBase(ZMQ_REP){
    bind(addr);
    setEndPt();
}

void Reply::listen(zmq::message_t (*callback)(zmq::message_t&), zmq::recv_flags flags){
    zmq::message_t request;

    // if (zmqBase::check(1) == false) return;

    zmq::detail::recv_result_t rr = sock.recv (request, flags);

    // if (request.size() == 0) return;

    //  create the reply
    zmq::message_t reply = callback(request);
    // printf(">> sending reply\n");
    cout << ">> reply::listen(): " << reply << endl;
    // sock.send(reply, zmq::send_flags::none);
    cout << ">> reply::listen() sent message" << endl;
    // if (reply.size() > 0) sock.send(reply, zmq::send_flags::none);
}


////////////////////////////////////////////////////

Request::Request(std::string addr): zmqBase(ZMQ_REQ){
    connect(addr);
    setEndPt();
}

zmq::message_t Request::get(zmq::message_t& msg, zmq::recv_flags flags){
    // sock.send(msg);

    // bool msg_ready = zmqBase::check(1);

    zmq::message_t rep;
    // if (msg_ready)
    zmq::detail::recv_result_t rr = sock.recv(rep, flags);
    cout << ">> request::get(): " << rep << endl;

    return rep;
}

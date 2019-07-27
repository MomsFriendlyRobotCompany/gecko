#pragma once

#include <ssocket.hpp>
#include <string>

class MCSocket: public SSocket {
public:
    MCSocket(){}
    ~MCSocket(){
        // struct sockaddr_in addr = make(port, "224.3.29.110");
        // sockopt(IPPROTO_IP, IP_DROP_MEMBERSHIP, addr); // FIXME
    }

    bool broadcast(const std::string& msg){
        // cout << "=> send: " << msg << " to " << inet_ntoa(mc_addr.sin_addr) << ":" << ntohs(mc_addr.sin_port) << endl;

        // send string to multicast address
        if ((sendto(sock, msg.c_str(), msg.size(), 0,
                (struct sockaddr *) &mc_addr, sizeof(mc_addr))) != msg.size()) {
            throw MulticastError("sendto() sent incorrect number of bytes");
        }
        return true;
    }
    void multicastGroup(const std::string& group, int port){
        mc_addr = make(port, INADDR_ANY);

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(group.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        // string s = ip();
        // cout << ">> ip: " << s << " " << inet_addr(s.c_str()) << endl;
        // mreq.imr_interface.s_addr = inet_addr(s.c_str());
        int err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
        if (err < 0){
            throw MulticastError("SSock::setsockopt IP_ADD_MEMBERSHIP failed");
        }

        // std::string s = ip();
        // // cout << ">> ip: " << s << " " << inet_addr(s.c_str()) << endl;
        // struct in_addr addr;
        // addr.s_addr = inet_addr(s.c_str());
        // setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addr, sizeof(addr));
    }

    void timeToLive(int ttl){
        sockopt(IPPROTO_IP, IP_MULTICAST_TTL, ttl);
    }

    // If you plan to have more than one process or user "listening",
    // loopback must be enabled.
    // 0-disable  1-enable
    void multicastLoop(){
        sockopt(IPPROTO_IP, IP_MULTICAST_LOOP, 0);
    }

protected:
    struct sockaddr_in mc_addr; // socket address structure -- why?
};
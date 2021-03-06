#include <gecko/ps.hpp>
#include <gecko/core.hpp>
#include <gecko/defaults.hpp>
#include <thread>
#include <map>
#include <unistd.h>     // getpid
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <exception>

using namespace std;

///////////////////////////////////////////

/*
 Binders [6]
  [PID]   Topic                  CPU   MEM   EndPoint
  [65993] hello................. 0.0%  0.0%  tcp://10.0.1.57:65303
  [65994] hey there............. 0.0%  0.0%  tcp://10.0.1.57:65303
  [66008] ryan.................. 0.1%  0.0%  uds:///var/run/ryan-0
*/

// using namespace gecko;

BeaconCoreServer::BeaconCoreServer(const string& key): port(GECKOCORE_PORT), delay(3) {
    HostInfo hi;

    // if (key.size() > 0) this->key = key;
    // else this->key = hi.cleanHostname();
    this->key = key;
    hostname = hi.hostname;
    address = hi.address;

    pid = getpid();

    datum = time_date();
}

BeaconCoreServer::~BeaconCoreServer(){
    prnt.join();
}

void BeaconCoreServer::stop(){
    ok = false;
}

void BeaconCoreServer::set_broadcast(int p){
    port = p;
}

void BeaconCoreServer::set_broadcast(const std::string& addr, int port){}

string BeaconCoreServer::handle_bind(ascii_t& data){
    // PublishTopic [4]: {key,topic,pid,endpt}
    string msg;
    string topic = data[1];
    string pid = data[2];
    string endpt = data[3];

    if (endpt == "ok"){
        // this is an echo
        cout << "** crap echo: " << endl;
        return msg;
    }
    // {key,topic,pid,endpt/fail,ok/fail}
    // services.push(topic, endpt);
    // bind.push(topic, pid);
    // data.push_back("ok");

    services.pushbind(topic, pid, endpt);

    data[3] = "ok";

    printf(">> BIND[%s]: %s: %s\n", pid.c_str(), topic.c_str(), endpt.c_str());

    Ascii a;
    msg = a.pack(data);

    return msg;
}

string BeaconCoreServer::handle_conn(ascii_t& data){
    // FindTopic [3]: {key,topic,pid}
    string msg;
    string topic = data[1];
    string pid = data[2];
    try {
        string endpt = services.get(topic);
        // conn.push(topic, pid);
        // services.push(topic, endpt);
        services.pushconn(topic, pid, endpt);

        // {key,topic,pid,endpt/fail,ok/fail}
        printf(">> CONN[%s]: %s: %s\n", pid.c_str(), topic.c_str(), endpt.c_str());
        // data.push_back(endpt);
        data[2] = endpt;
        data.push_back("ok");

        Ascii a;
        msg = a.pack(data);
        return msg;
    }
    catch (InvalidKey e){
        printf("** Invalid Topic: %s **\n", topic.c_str());
    }
    return "";
}


void BeaconCoreServer::listen(bool verbose){
    // setup multicast
    BCSocket ss(port);
    ss.bind();

    // setup printing loop in another thread
    if (verbose){
        printf("===============================\n");
        printf("   Starting Print Loop\n");
        printf("===============================\n");
        prnt = thread(&BeaconCoreServer::printLoop, this);
    }
    else
        this->print();

    Ascii a;
    while(ok){
        string ans;
        struct sockaddr_in addr;

        tie(ans, addr) = ss.recv_nb(500);

        if(!ans.empty()){
            cout << "remote: " << print_addr(addr) << endl;

            ascii_t t = a.unpack(ans);
            string msg;

            if (t.size() == 3) msg = handle_conn(t);
            else if (t.size() == 4) msg = handle_bind(t);

            if (msg.size() > 0){
                // ss.send(msg, addr);
                ss.cast(msg);
            }
        }
    }
    if(verbose) prnt.join();
}

void BeaconCoreServer::printLoop(){
    // cout << "printLoop()" << endl;
    while(ok){
        this->print();
        gecko::sleep(delay);
    }
}

void BeaconCoreServer::print(){
    PS ps;
    ps.get(to_string(pid));

    printf("========================================\n");
    printf(" Geckocore [%d]\n", pid);
    printf("-------------\n");
    printf(" Start: %s\n", datum.c_str());
    printf(" Key: %s\n", key.c_str());
    printf(" Host name[IP]: %s[%s]\n", hostname.c_str(), address.c_str());
    printf(" Listening on: %d\n", port);
    printf(" CPU: %s   Memory: %s\n", ps.cpu.c_str(), ps.mem.c_str());
    printf("-------------\n");
    services.print();
}

#include <gecko/core.hpp>
#include <thread>
#include <map>
#include <unistd.h>     // getpid
#include <stdlib.h>
// #include <tuple>s
// #include <map>
#include <iostream>
#include <vector>
#include <exception>

using namespace std;

///////////////////////////////////////////

// FIXME: move else where
static string mc_addr = {"224.3.29.110"};
static int mc_port = 11311;

/*

*/

BeaconCoreServer::BeaconCoreServer(const string& key, int ttl, int delay):
    exit(false), pid(getpid()), delay(delay) {
    // HostInfo hi = HostInfo();
    HostInfo hi;

    if (key.size() > 0) this->key = key;
    else this->key = hi.hostname;

    host = hi.address;

    datum = time_date();
}

void BeaconCoreServer::start(){
    // start thread
    // thread(this->printLoop());
}

void BeaconCoreServer::stop(){
    exit = true;
}

string BeaconCoreServer::handle_bind(ascii_t& data){
    // PublishTopic [4]: {key,topic,pid,endpt}
    string msg;

    // if(data.size() == 4){
        string topic = data[1];
        string pid = data[2];
        string endpt = data[3];

        if (endpt == "ok"){
            // this is an echo
            cout << "** crap echo: " << endl;
            return msg;
        }
        // {key,topic,pid,endpt/fail,ok/fail}
        services.push(topic, endpt);
        bind.push(topic, pid);
        // data.push_back("ok");
        data[3] = "ok";

        printf(">> BIND[%s]: %s: %s\n", pid.c_str(), topic.c_str(), endpt.c_str());

        Ascii a;
        msg = a.pack(data);
        cout << "\nbind send: " << msg << "\n" << endl;
        // ss.send(msg);
    // }

    return msg;
}

string BeaconCoreServer::handle_conn(ascii_t& data){
    // FindTopic [3]: {key,topic,pid}
    string msg;
    try {
        string topic = data[1];
        string pid = data[2];
        string endpt = services.get(topic);
        conn.push(topic, pid);
        services.push(topic, endpt);

        // {key,topic,pid,endpt/fail,ok/fail}
        printf(">> CONN[%s]: %s: %s\n", pid.c_str(), topic.c_str(), endpt.c_str());
        // data.push_back(endpt);
        data[2] = endpt;
        data.push_back("ok");

        Ascii a;
        msg = a.pack(data);
        cout << "\nconn send: " << msg << "\n" << endl;
        // ss.send(msg);
        // return msg;
    }
    catch (InvalidKey e){
        printf("** Invalid Key **\n");
        // data.push_back("fail");

        // Ascii a;
        // string msg = a.pack(data);
        // ss.send(msg);
    }
    return msg;
}

void BeaconCoreServer::run(){}

void BeaconCoreServer::listen(){
    // setup multicast
    SSocket ss;
    ss.init(mc_addr, mc_port, 1, true);

    // setup printing loop in another thread
    thread prnt(&BeaconCoreServer::printLoop, this);

    Ascii a;
    while(ok){
        // string ans = ss.recv_nb();
        // MsgAddr ma = ss.recv2();
        // string ans = ma.msg;
        string ans;
        struct sockaddr_in addr;

        tie(ans, addr) = ss.recv();

        if(!ans.empty()){
            ascii_t t = a.unpack(ans);

            // cout << "Msg: ";
            // for (const auto& s: t) cout << s << " ";
            // cout << endl;
            string msg;

            if (t.size() == 3) msg = handle_conn(t);
            else if (t.size() == 4) msg = handle_bind(t);
            ss.send(msg, addr);
        }
        else cout << "** nothing **" << endl;
    }
}

void BeaconCoreServer::printLoop(){
    while(ok){
        print();
        sleep(delay);
    }
}

void BeaconCoreServer::print(){
    printf("========================================\n");
    printf(" Geckocore [%d]\n", pid);
    printf("-------------\n");
    printf(" Start: %s\n", datum.c_str());
    printf(" Key: %s\n", key.c_str());
    printf(" Host IP: %s\n", host.c_str());
    printf(" Listening on: %s:%d\n", mc_addr.c_str(), mc_port);
    printf("-------------\n");
    printf("Known Services [%d]\n", services.size());
    services.print();
    printf("Binders [%d]\n", bind.size());
    bind.print();
    printf("Connections [%d]\n", conn.size());
    conn.print();
}
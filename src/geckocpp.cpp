#include "geckocpp.hpp"
#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include <unistd.h>  // getopt
#include "transport.hpp"
#include "network.hpp"
#include "signals.hpp"
#include "time.hpp"
#include "helpers.hpp"

#include <thread>
#include "mbeacon.hpp"

using namespace std;

void beacon(bool& ok){
    Rate rate(10);
    while (ok){
        rate.sleep();
    }
}


/*
This holds the static info for this thread/process. User will never call this
but just access it via the functions below.
*/
class Singleton: public SigCapture {
public:
    static Singleton& get() {
        // Since it's a static variable, if the class has already been created,
        // it won't be created again.
        // And it **is** thread-safe in C++11.
        static Singleton myInstance;

        // Return a reference to our instance.
        return myInstance;
    }

    // delete copy and move constructors and assign operators
    Singleton(Singleton const&) = delete;             // Copy construct
    Singleton(Singleton&&) = delete;                  // Move construct
    Singleton& operator=(Singleton const&) = delete;  // Copy assign
    Singleton& operator=(Singleton &&) = delete;      // Move assign

    // Any other public methods.
    void print(){
        cout << "[GeckoCore]--------------------------" << endl;
        // cout << "  Status: " << ((core_found) ? "Core Found" : "Core Unknown") << endl;
        // cout << "  Core: " << core_addr << endl;
        cout << "  Host: " << host_addr << endl;
    }
    // string core_addr;  // address geckocore runs at
    string host_addr;  // address of the system this process/thread runs at
    vector<Subscriber*> subs;
    bool initialized;  // has geckocore been initialized yet?
    // bool core_found;
    map<string,string> pubs;  // topics, endpoints

protected:
    Singleton(): initialized(false) //, core_found(false)
    {
        cout << "GeckoCore: constructor" << endl;
    }

    ~Singleton() {
        cout << "GeckoCore: destructor" << endl;
        for (auto const& p: subs) delete p;
        subs.clear();
    }

    // void thread_loop(){
    //     Listener listen(key,ip,port);
    //     bool err = listen.init();
    //     if(err){
    //         printf("Listener error\n");
    //         return;
    //     }
    //
    //     //  in: key|topic
    //     // out: key|topic|tcp://hostname:port
    //     while(ok){
    //         string s = listen.listen();
    //         cout << ">> listen: "<< s << endl;
    //         stringstream ss(s);
    //         string token;
    //         vector<string> toks;
    //         while(getline(ss,token,'|')) toks.push_back(token);
    //
    //         if (toks[0] == key) {
    //             stringstream ans;
    //             auto search = db.find(toks[1]);
    //             if(search != db.end()){
    //                 ans << key << "|" << topic << "|" << search->second;
    //                 listen.send(ans.str());
    //             }
    //             else
    //                 printf("valid key, but no topic supported\n");
    //             break;
    //         }
    //     }
    // }
};

// args --------------------------
#include <map>

typedef struct {
    std::string program;
    std::map<char, std::string> args;
} args_t;

args_t processArgs(int argc, char* argv[]){
    args_t args;
    args.program = argv[0];  // program name
    for (int i=1; i < argc; i+=2) {
        if (strlen(argv[i]) == 2){
            char key = argv[i][1];  // -key
            if (i+i < argc || key == 'h'){
                string value(argv[i+1]);
                args.args.insert_or_assign(key, value);
            }
            else
                args.args.insert_or_assign(key, "");
        }
        else {
            printf("*** invalid args: %s %s ***\n",argv[i],argv[i+1]);
        }
    }
    return args;
}
//---------------------------------

void gecko::init(int argc, char* argv[]){
    printf("\n*** FIXME ***\n");


    if (Singleton::get().initialized) return;

    string help("\n"
    "./program -h -m name \n"
    "  h: help \n"
    "  m: machine name for publisher \n");

    string machine;

    args_t args = processArgs(argc, argv);

    for( auto const& [key, val] : args.args ) {
        switch(key){
        case 'd':
            printf("d => %d\n", stoi(val));
            break;
        case 'f':
            printf("f => %s\n", val.c_str());
            break;
        case 'h':
            printf("%s\n", help.c_str());
        case 'm':
            machine = val;
            break;
        default:
            printf("*** Unknown args: %c %s ***\n",key,val.c_str());
        }
    }

    if (machine.empty()) machine = "localhost";

    // cout << "Machine: " << machine << endl;

    // exit(0);

    // string s;
    // init(s);
    // if (Singleton::get().initialized)
    HostInfo h = HostInfo();
    Singleton::get().host_addr = h.addr;

    // if (c.empty()) Singleton::get().core_addr = zmqTCP(h.addr, ":11311");
    // else Singleton::get().core_addr = zmqTCP(c, ":11311");

    // bool found = pingCore(Singleton::get().core_addr);

    // if (found) Singleton::get().core_found = true;
    // else {
    //     cout << "*** Couldn't find GeckoCore [" << Singleton::get().core_addr << "] ***" << endl;
    //     // exit(1);
    // }

    Singleton::get().print();

    exit(0);
}


// void gecko::init(const string& c){
//     // if (Singleton::get().core_found) return;
//     if (Singleton::get().core_found)
//     HostInfo h = HostInfo();
//     Singleton::get().host_addr = h.addr;
//
//     if (c.empty()) Singleton::get().core_addr = zmqTCP(h.addr, ":11311");
//     else Singleton::get().core_addr = zmqTCP(c, ":11311");
//
//     // bool found = pingCore(Singleton::get().core_addr);
//
//     // if (found) Singleton::get().core_found = true;
//     // else {
//     //     cout << "*** Couldn't find GeckoCore [" << Singleton::get().core_addr << "] ***" << endl;
//     //     // exit(1);
//     // }
//
//     Singleton::get().print();
// }

bool gecko::ok(){
    return Singleton::get().isOk();
}

Publisher* gecko::advertise(string topic, int queue, bool bind){
    string addr = zmqTCP(Singleton::get().host_addr );  // bind to next available port

    Publisher *p = new Publisher(addr, true);
    return p;
}

Subscriber* gecko::subscribe(string topic, void(*cb)(zmq::message_t&), int queue, bool bind){
    // Singleton gc = Singleton::Instance();
    // Request req(Singleton::get().core_addr);
    // zmq::message_t reqt(topic.c_str(), topic.size());
    // zmq::message_t respt = req.get(reqt);
    //
    // string m(static_cast<char*>(respt.data()), respt.size());
    // vector<string> toks = split(m, ':');  // host:topic:addr:port
    //
    // string addr = zmqTCP(toks[2], toks[3]);

    // string addr = getTopic(Singleton::get().core_addr, topic);
    string addr;

    Subscriber *s = new Subscriber(addr, topic, bind);
    if(cb != nullptr) s->setCallback(cb);
    Singleton::get().subs.push_back(s);
    return s;
}

void gecko::spin(int hertz){
    Rate rate(hertz);
    while(gecko::ok()){
        // for(int i=0; i < Singleton::get().subs.size(); ++i){
        //     zmq::message_t msg = Singleton::get().subs[i]->recv();
        // }
        for (auto const& p: Singleton::get().subs){
            zmq::message_t msg = p->recv_nb();
            p->callback(msg);
        }
        rate.sleep();
    }

    // clean up
    // for(int i=0; i < Singleton::get().subs.size(); ++i) delete Singleton::get().subs[i];
    for (auto const& p: Singleton::get().subs) delete p;
    Singleton::get().subs.clear();
}

void gecko::loginfo(std::string msg){
    cout << "[INFO]: " << msg << endl;
}



//////////////////////////////////////////////////////////////////////////

// /*
//     core: tcp://x.x.x.x:port
//     return: found core true/false
// */
// bool pingCore(string& core){
//     Request req(core);
//     zmq::message_t msg("ping", 4);
//     zmq::message_t resp = req.get(msg);
//     string s(static_cast<char*>(resp.data()), resp.size());
//     // bool ret = false;
//     // if (s == "ok") ret = true;
//     // return ret;
//     return (s == "ok");
// }
//
// /*
//     core: x.x.x.x:port
//     topic: topic name
//     return: tcp://1.2.3.4:1234
// */
// string getTopic(string& core, string& topic){
//     Request req(core);
//     string msg = string("get:") + topic;
//     zmq::message_t m((void*)msg.data(), msg.size());
//
//     // resp = tcp://1.2.3.4:1234
//     zmq::message_t resp = req.get(m);
//     string addr(static_cast<char*>(resp.data()), resp.size());
//
//     cout << "getTopic: " << addr << endl;
//     return addr;
// }
//
// /**
//     core: tcp://1.2.3.4:1234
//     topic: topic name
//     return: success true/false
// */
// bool setTopic(const string& core, const string& topic){
//     Request req(core);
//
//     string msg = string("set:") + topic;
//     zmq::message_t m((void*)msg.data(), msg.size());
//
//     // resp = ok/fail
//     zmq::message_t resp = req.get(m);
//     string status(static_cast<char*>(resp.data()), resp.size());
//
//     return (status == "ok");
// }
// #include <type_traits>
// #include <typeinfo>
// #include <memory>
// #include <string>
// #include <cstdlib>
// template <typename T>
// std::string type_name(){
//     typedef typename std::remove_reference<T>::type TR;
//     std::unique_ptr<char, void(*)(void*)> own
//            (
//                 nullptr,
//                 std::free
//            );
//     std::string r = own != nullptr ? own.get() : typeid(TR).name();
//     if (std::is_const<TR>::value)
//         r += " const";
//     if (std::is_volatile<TR>::value)
//         r += " volatile";
//     if (std::is_lvalue_reference<T>::value)
//         r += "&";
//     else if (std::is_rvalue_reference<T>::value)
//         r += "&&";
//     return r;
// }

// #include <typeinfo>

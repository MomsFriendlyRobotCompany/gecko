#include <gecko/ps.hpp>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <algorithm>


using namespace std;

bool isHidden(unsigned char c) {
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f');
}

string clean(string& s){
    s.erase(std::remove_if(s.begin(), s.end(), isHidden), s.end());
    return std::move(s);
}


int PS::get(const string& pid){

// I don't support windows, but I threw this in for fun
#if defined(_WIN32)
    this->pid = "0";
    this->mem = "0";
    this->cpu = "0";

#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    string cmd = "/bin/ps -o pid,%mem,%cpu -p " + pid + " | sed -n '1!p'";

    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    stringstream ss(result);
    string token;
    char sep = ' ';
    vector<string> toks;
    while(getline(ss,token,sep)) {
        if (token.size() > 0) toks.push_back(token);
    }

    if (toks.size() < 3) return 1;

    this->pid = clean(toks[0]);
    this->mem = clean(toks[1]);
    this->cpu = clean(toks[2]);


#endif

    return 0;
}

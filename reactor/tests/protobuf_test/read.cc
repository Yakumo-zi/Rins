#include "msg.pb.h"
#include <fstream>
#include <iostream>
using namespace std;

void ListMsg(const protobuf_test::hellworld &msg) {
    cout << msg.id() << endl;
    cout << msg.str() << endl;
}
int main() {
    protobuf_test::hellworld msg1;
    std::fstream input("./log", std::ios::in | std::ios::binary);
    if (!msg1.ParseFromIstream(&input)) {
        cerr << "Failed to parse address book." << endl;
        return -1;
    }
    ListMsg(msg1);
}
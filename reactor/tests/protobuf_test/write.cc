#include "msg.pb.h"
#include <fstream>
#include <iostream>

using namespace std;

int main() {
    protobuf_test::hellworld msg1;
    msg1.set_id(101);
    msg1.set_str("hello");
    fstream output("./log", ios::out | ios::binary | ios::trunc);
    if (!msg1.SerializeToOstream(&output)) {
        cerr << "Failed to write msg." << endl;
        return -1;
    }
    return 0;
}
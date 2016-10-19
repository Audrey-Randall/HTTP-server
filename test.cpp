//C++
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct Test {
    string x;
};

int main(int argc, char* argv[]) {
    Test mytest = {};
    mytest.x = "tadaa";
    cout<<mytest.x<<endl;
    string what("what");
    mytest.x = what;
    cout<<mytest.x<<endl;
    const char* next = "next";
    mytest.x = next;
    cout<<mytest.x<<endl;
    return 0;
}

#include <iostream>

class A;
class B {
    A *a;
public:
    B(const A *arg_a) : a(arg_a) {}
    void printA() {
        std::cout << a->getContents() << std::endl;
    }
};

class A {
    std::string contents;
public:
    A(const char* cc) : contents(cc) {};
    const std::string getContents() const { return contents; }
};


int main(int argc, char** argv) {
    A a(argv[0]);
    B b(&a);
    b.printA();
    return 0;
};
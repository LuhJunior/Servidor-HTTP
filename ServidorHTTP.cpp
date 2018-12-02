#include <iostream>
#include "ServerHTTP.hpp"

int main(){
    http HTTP;
    HTTP.Listen(PORT);
    HTTP.end();
    return 0;
}
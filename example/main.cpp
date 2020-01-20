/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "libnullnexus/nullnexus.hpp"

void msg(std::string username, std::string msg)
{
    std::cout << username << " " << msg << std::endl;
}

int main()
{
    NullNexus online;
    online.changeSettings();
    online.setHandlerChat(msg);
    online.connect();
    online.sendChat("BRUH");
    std::this_thread::sleep_for(std::chrono_literals::operator""s(5));
}

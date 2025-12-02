#include "Client.hpp"
#include <iostream>

int main() {
    try {
        rtype::client::Client client("127.0.0.1", 4242); // IP and port
        client.connect();

        std::cout << "Press Enter to disconnect...\n";
        std::cin.get(); //Call render function

        client.disconnect();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

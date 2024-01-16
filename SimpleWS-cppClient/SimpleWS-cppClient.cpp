// SimpleWS-cppClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "WebSocketClient.h"

#include <boost/asio.hpp>

#include <iostream>

namespace net = boost::asio;            // from <boost/asio.hpp>

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 3)
    {
        std::cerr <<
            "Usage: " << argv[0] << " <host> <port>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " echo.websocket.org 80\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];

    // The io_context is required for all I/O
    net::io_context ioc;
    const auto shared_ioc = std::make_shared<WebSocketClient>(ioc);

    // Launch the asynchronous operation
    shared_ioc->run(host, port);
    shared_ioc->write("Mouep");

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

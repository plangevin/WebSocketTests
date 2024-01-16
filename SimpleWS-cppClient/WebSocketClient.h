#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{
private:
   tcp::resolver _resolver;
   websocket::stream<beast::tcp_stream> _ws;
   beast::flat_buffer _buffer;
   std::string _host;

public:
   // Resolver and socket require an io_context
   explicit WebSocketClient(net::io_context& ioc);

   // Start the asynchronous operation
   void run(char const* host, char const* port);
   void write(const std::string &iText);

   void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
   void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
   void on_handshake(beast::error_code ec);
   void on_write(beast::error_code ec, std::size_t bytes_transferred);
   void on_read(beast::error_code ec, std::size_t bytes_transferred);
   void on_close(beast::error_code ec);
};


#include "WebSocketClient.h"
#include <iostream>

// Report a failure
void fail(beast::error_code ec, char const* what)
{
   std::cerr << what << ": " << ec.message() << "\n";
}

WebSocketClient::WebSocketClient(net::io_context& ioc)
   : _resolver(net::make_strand(ioc))
   , _ws(net::make_strand(ioc))
{
}

void WebSocketClient::run(char const* host, char const* port)
{
   // Save these for later
   _host = host;

   try
   {
      const tcp::resolver::results_type results = _resolver.resolve(host, port);

      // Set the timeout for the operation
      beast::get_lowest_layer(_ws).expires_after(std::chrono::seconds(30));

      // Make the connection on the IP address we get from a lookup
      beast::get_lowest_layer(_ws).connect(results);

      // Turn off the timeout on the tcp_stream, because
      // the websocket stream has its own timeout system.
      beast::get_lowest_layer(_ws).expires_never();

      // Set suggested timeout settings for the websocket
      _ws.set_option(
         websocket::stream_base::timeout::suggested(
            beast::role_type::client));

      // Set a decorator to change the User-Agent of the handshake
      _ws.set_option(websocket::stream_base::decorator(
         [](websocket::request_type& req)
         {
            req.set(http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) +
            " websocket-client-async");
         }));

      // Perform the websocket handshake
      _ws.handshake(_host, "/ws?name=MEEEEE");

      // And start the reader
      _ws.async_read(
         _buffer,
         beast::bind_front_handler(
            &WebSocketClient::on_read,
            shared_from_this()));
   }
   catch (boost::system::system_error &se)
   {
      fail(se.code(), se.what());
   }


   //// Look up the domain name
   //_resolver.async_resolve(
   //   host,
   //   port,
   //   beast::bind_front_handler(
   //      &WebSocketClient::on_resolve,
   //      shared_from_this()));
}

void WebSocketClient::on_resolve(
   beast::error_code ec,
   tcp::resolver::results_type results)
{
   if (ec)
      return fail(ec, "resolve");

   // Set the timeout for the operation
   beast::get_lowest_layer(_ws).expires_after(std::chrono::seconds(30));

   // Make the connection on the IP address we get from a lookup
   beast::get_lowest_layer(_ws).async_connect(
      results,
      beast::bind_front_handler(
         &WebSocketClient::on_connect,
         shared_from_this()));
}

void WebSocketClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
   if (ec)
      return fail(ec, "connect");

   // Turn off the timeout on the tcp_stream, because
   // the websocket stream has its own timeout system.
   beast::get_lowest_layer(_ws).expires_never();

   // Set suggested timeout settings for the websocket
   _ws.set_option(
      websocket::stream_base::timeout::suggested(
         beast::role_type::client));

   // Set a decorator to change the User-Agent of the handshake
   _ws.set_option(websocket::stream_base::decorator(
      [](websocket::request_type& req)
      {
         req.set(http::field::user_agent,
         std::string(BOOST_BEAST_VERSION_STRING) +
         " websocket-client-async");
      }));

   // Perform the websocket handshake
   _ws.async_handshake(_host, "/",
      beast::bind_front_handler(
         &WebSocketClient::on_handshake,
         shared_from_this()));
}

void WebSocketClient::on_handshake(beast::error_code ec)
{
   if (ec)
      return fail(ec, "handshake");

   // Send the message
   _ws.async_write(
      net::buffer(""),
      beast::bind_front_handler(
         &WebSocketClient::on_write,
         shared_from_this()));
}

void WebSocketClient::write(const std::string &iText)
{
   _ws.write(net::buffer(iText));
}

void WebSocketClient::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
   boost::ignore_unused(bytes_transferred);

   if (ec)
      return fail(ec, "write");

   // Read a message into our buffer
   _ws.async_read(
      _buffer,
      beast::bind_front_handler(
         &WebSocketClient::on_read,
         shared_from_this()));
}

void WebSocketClient::on_read(
   beast::error_code ec,
   std::size_t bytes_transferred)
{
   boost::ignore_unused(bytes_transferred);

   if (ec)
      return fail(ec, "read");

   // Close the WebSocket connection
   _ws.async_close(websocket::close_code::normal,
      beast::bind_front_handler(
         &WebSocketClient::on_close,
         shared_from_this()));
}

void WebSocketClient::on_close(beast::error_code ec)
{
   if (ec)
      return fail(ec, "close");

   // If we get here then the connection is closed gracefully

   // The make_printable() function helps print a ConstBufferSequence
   std::cout << beast::make_printable(_buffer.data()) << std::endl;
}



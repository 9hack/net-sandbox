#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>

#define PORT 9000

using boost::asio::ip::tcp;

void session(tcp::socket sock)
{
  try
  {
    for (;;)
    {
      char data[BUFSIZ];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      std::cerr << "received: " << data << "\n";
      boost::asio::write(sock, boost::asio::buffer(data, length));
      boost::asio::write(sock, boost::asio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

int main()
{
  try
  {
    boost::asio::io_service io_service;

    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), PORT));

    for (;;)
    {
      std::cerr << "Waiting to accept...\n";
      tcp::socket socket(io_service);
      acceptor.accept(socket);
      std::cerr << "Connected!\n";
      
      std::thread(session, std::move(socket)).detach();


      // std::string message = "what's up";
      // 
      // boost::system::error_code ignored_error;
      // boost::asio::write(socket, boost::asio::buffer(message),
      //     boost::asio::transfer_all(), ignored_error);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

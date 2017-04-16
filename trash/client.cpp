#include <iostream>
#include <chrono>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#define PORT "9000"

using boost::asio::ip::tcp;

class Client : public boost::enable_shared_from_this<Client> {
public:
  typedef boost::shared_ptr<Client> pointer;

  static pointer create(boost::asio::io_service& io_service, char* host) {
    return pointer(new Client(io_service, host));
  }
  
  void start() {
    for (;;) {
      std::cerr << "Looping...\n";
      
      socket.async_read_some(boost::asio::buffer(rcvbuf, BUFSIZ),
        boost::bind(&Client::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
            
      std::string data = "Hello from the client!";
      size_t wlen = boost::asio::write(socket, boost::asio::buffer(data));
      
      std::cerr << "Wrote " << wlen << "\n";
      
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // socket.async_read_some(boost::asio::buffer(rcvbuf, BUFSIZ),
    //   boost::bind(&Client::handle_read, shared_from_this(),
    //     boost::asio::placeholders::error,
    //     boost::asio::placeholders::bytes_transferred));
    //     
    // std::string data = "Hello from the client!";
    // boost::asio::async_write(socket, boost::asio::buffer(data),
    //     boost::bind(&Client::handle_write, shared_from_this(),
    //       boost::asio::placeholders::error,
    //       boost::asio::placeholders::bytes_transferred));
  }

private:
  Client(boost::asio::io_service& io_service, char* host) :
    socket(io_service) {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, PORT);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::connect(socket, endpoint_iterator);
  }
  
  void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred) {
    std::cerr << "handle read:" << bytes_transferred << "\n";
    
    if (!error) {
      std::cerr << "  MSG: " << std::string(rcvbuf);
    }
    else if (error == boost::asio::error::eof) {
      std::cerr << "  no new data.\n";
    }
    else {
      std::cerr << "  handle_read error: " << error << "\n";
    }
  }
  
  void handle_write(const boost::system::error_code&, size_t bytes_transferred) {
    std::cerr << "handle write: " << bytes_transferred << "\n";
  }
  
  tcp::socket socket;
  char rcvbuf[BUFSIZ];
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: client <host>" << std::endl;
    return 1;
  }

  boost::asio::io_service io_service;
  Client::pointer client = Client::create(io_service, argv[1]);
  
  io_service.run();
  client->start();

  return 0;
}

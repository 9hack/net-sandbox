/**
 * Source: http://codereview.stackexchange.com/questions/51235/udp-network-server-client-for-gaming-using-boost-asio
 * License: MIT
 */

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "threadsafe_queue.h"

#include <memory>
#include <array>

#define PORT "9000"

using boost::asio::ip::tcp;


class NetworkClient 
{
public:
    NetworkClient(std::string host) :
      socket(io_service), 
      service_thread(std::bind(&NetworkClient::run_service, this)) 
    {
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(tcp::v4(), host, PORT);      
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
      boost::asio::connect(socket, endpoint_iterator);      
    }
    
    ~NetworkClient()
    {
        io_service.stop();
        service_thread.join();
    }
    
    void Send(std::string message)
    {
      boost::asio::write(socket, boost::asio::buffer(message));
    }

    bool HasMessages() 
    {
      return !message_queue.empty();
    }
    
    std::string PopMessage() 
    { 
      if (!HasMessages()) 
        throw std::logic_error("No messages"); 
      return message_queue.pop(); 
    };

private:
    boost::asio::io_service io_service;
    tcp::socket socket;
    std::array<char, BUFSIZ> recv_buffer;
    boost::thread service_thread;

    // Thread safe message queue
    ThreadSafeQueue<std::string> message_queue;

    void start_receive() 
    {      
      socket.async_receive(boost::asio::buffer(recv_buffer),
        boost::bind(&NetworkClient::handle_receive, this, 
          boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
      if (!error)
      {
          std::string message(recv_buffer.data(), recv_buffer.data() + bytes_transferred);
          std::cerr << "received: " << message << std::endl;
          message_queue.push(message);
      }

      start_receive();
    }
    
    void run_service()
    {
      std::cerr << "Client network thread started\n";
      start_receive();
      std::cerr << "Client started receiving\n";
      while (!io_service.stopped()) {
          try {
              io_service.run();
          }
          catch (const std::exception& e) {
              std::cerr << "Client network exception" << e.what();
          }
          catch (...) {
              std::cerr << "Unknown exception in client network";
          }
      }
      std::cerr << "Client network thread stopped";
    }
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: game_client <host> <msg>" << std::endl;
    return 1;
  }

  NetworkClient client(argv[1]);
  
  for (;;) {
    // Check for any messages from server.
    if (client.HasMessages())
      while (client.HasMessages()) {
        std::cerr << "MSG: " << client.PopMessage() << "\n";
      }
    else
      std::cerr << "no messages\n";
    
    // TODO Client-side updates and rendering.
    
    // Send input updates to server.
    std::cerr << "Sending...\n";
    client.Send(argv[2]);
    
    sleep(1);
  }
  
  return 0;
}

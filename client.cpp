/**
 * Source: http://codereview.stackexchange.com/questions/51235/udp-network-server-client-for-gaming-using-boost-asio
 * License: MIT
 */

#include "threadsafe_queue.h"
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <array>

#define PORT "9000"

using boost::asio::ip::tcp;

/**
 * Represents a single client on the network.
 */
class NetworkClient {
public:
  NetworkClient(std::string host) :
    socket(io_service), 
    service_thread(std::bind(&NetworkClient::run_service, this)) {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), host, PORT);      
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket, endpoint_iterator);      
  }
  
  // Destructor.
  ~NetworkClient() {
      io_service.stop();
      service_thread.join();
  }
  
  // Synchronously sends a message to the server. 
  void send(std::string message) {
    boost::asio::write(socket, boost::asio::buffer(message));
  }

  // Returns true if there are message(s) in the queue.
  bool has_messages() {
    return !message_queue.empty();
  }
  
  // Removes and returns a message from the FIFO queue.
  std::string pop_message() { 
    if (!has_messages()) 
      throw std::logic_error("No messages"); 
    return message_queue.pop(); 
  }

private:
  // Begin receiving messages by adding an async receive task.
  void start_receive() {      
    socket.async_receive(boost::asio::buffer(recv_buffer),
      boost::bind(&NetworkClient::handle_receive, this, 
        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
  }
  
  // Callback for when receive is completed. Adds to message queue, continue reading.
  void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        std::string message(recv_buffer.data(), recv_buffer.data() + bytes_transferred);
        message_queue.push(message);
    }

    start_receive();
  }
  
  // Service thread for receiving messages.
  void run_service() {
    start_receive();
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
  }
  
  boost::asio::io_service io_service;
  tcp::socket socket;
  std::array<char, BUFSIZ> recv_buffer;
  boost::thread service_thread;
  ThreadSafeQueue<std::string> message_queue;
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: client <host> <msg>" << std::endl;
    return 1;
  }
  
  std::string server_hostname = argv[1];
  std::string message = argv[2];
  message += "\n";

  NetworkClient client(server_hostname);
  
  for (;;) {
    // Check for any messages from server.
    if (client.has_messages())
      while (client.has_messages()) {
        std::cerr << "↘ " << client.pop_message();
      }
    else
      std::cerr << "no messages\n";
    
    // TODO Client-side updates and rendering.
    
    // Send input updates to server.
    std::cerr << "↗ " << message;
    client.send(message);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  return 0;
}

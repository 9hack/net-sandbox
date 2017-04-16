#include "threadsafe_queue.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>

#define PORT 9000

using boost::asio::ip::tcp;

/**
 * Represents one TCP connection to a client.
 */
class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
public:
  typedef boost::shared_ptr<TcpConnection> pointer;

  // Create a shared pointer to this TCP connection.
  static pointer create(boost::asio::io_service& io_service) {
    return pointer(new TcpConnection(io_service));
  }

  // Returns this connection's socket.
  tcp::socket& get_socket() {
    return socket;
  }

  // When connection starts, begin reading.
  void start() {
    socket.async_read_some(boost::asio::buffer(rcvbuf),
        boost::bind(&TcpConnection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  
  // Sends a message to this client. Returns true if write was successful.
  bool send(std::string message) {
    if (!socket.is_open()) {
      return false;
    }
    
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer(message), error);
    
    // If we get these errors, it's likely a clean disconnect.
    if ((error == boost::asio::error::eof) ||
        (error == boost::asio::error::connection_reset) ||
        (error == boost::asio::error::broken_pipe)) {
      std::cerr << "[send] client disconnected.\n";
      return false;
    } else if (error) {
      std::cerr << "[send] some other error: " << error << "\n";
      return false;
    }
    
    return true;
  }
  
  // Returns true if there are messages in the queue.
  bool has_messages() {
    return !message_queue.empty();
  }
  
  // Removes a message and returns it.
  std::string pop_message() { 
    if (!has_messages()) 
      throw std::logic_error("No messages"); 
    return message_queue.pop(); 
  }

private:
  // Initializes the socket.
  TcpConnection(boost::asio::io_service& io_service)
    : socket(io_service) {}
  
  // Callback for when an asynchronous  read completes. 
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {    
    if (!error) {
      message_queue.push(rcvbuf);
    }
    else if (error != boost::asio::error::eof) {
      std::cerr << "FATAL handle_read error: " << error << "\n";
      return;
    }

    // Wait for and read the next message.
    socket.async_read_some(boost::asio::buffer(rcvbuf),
        boost::bind(&TcpConnection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  tcp::socket socket;
  char rcvbuf[BUFSIZ];
  ThreadSafeQueue<std::string> message_queue;
};

/**
 * Represents the single TCP server, managing many client connections.
 */
class TcpServer {
public:
  // Initializes this server with the given port.
  TcpServer(unsigned int port) :
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), next_id(0) {
    start_accept();
    
    // Run the io_service in a separate thread so it's non-blocking.
    std::thread(TcpServer::run, std::ref(io_service)).detach();
  }
  
  // Sends a message to all clients.
  void send_to_all(std::string message) {
    // No clients connected.
    if (client_list.size() == 0) {
      return;
    }
    
    std::cerr << "[" << client_list.size() << "]\n";
    for (auto const &c : client_list) {
      std::cerr << "↗ (" << c.first << ") " << message;
      bool success = c.second->send(message);
      if (!success) {
        std::cerr << "Write failed!\n";
        client_list.erase(c.first);
      }
    }
  }
  
  // Read all messages from all clients.
  std::vector<std::string> read_all_messages() {
    std::vector<std::string> messages;
    
    // No clients connected.
    if (client_list.size() == 0) {
      return messages;
    }
    
    for (auto const &c : client_list) {
      while (c.second->has_messages()) {
        auto message = c.second->pop_message();
        messages.push_back(message);
      }
    }
    
    return messages;
  }

private:
  // Begin accepting new clients.
  void start_accept() {
    TcpConnection::pointer new_connection =
      TcpConnection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&TcpServer::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  // Callback for when a client is connected.
  void handle_accept(TcpConnection::pointer new_connection,
      const boost::system::error_code& error) {
    if (!error) {
      std::cerr << "Accepted new connection." << std::endl;
      client_list[++next_id] = new_connection;
      new_connection->start();
    }

    // Accept the next client.
    start_accept();
  }

  // Run the io_service. Is run on a separate thread to avoid blocking.
  static void run(boost::asio::io_service& io_service) {
    io_service.run();
  }
  
  boost::asio::io_service io_service;
  tcp::acceptor acceptor_;
  std::map<int, TcpConnection::pointer> client_list;
  int next_id;
};

int main() {
  try {
    TcpServer server(PORT);
    std::cerr << "Running server on port " << PORT << std::endl;
    for (;;) {
      auto messages = server.read_all_messages();
      
      if (messages.size() > 0) {
        std::cerr << "Read (" << messages.size() << ") messages:\n";
        for (auto const &message : messages) {
          std::cerr << "↘ " << message;
        }
      }
      
      // TODO update stuff
      
      server.send_to_all("Hello from server!\n");
      
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

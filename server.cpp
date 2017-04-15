#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#define PORT 9000

using boost::asio::ip::tcp;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
public:
  typedef boost::shared_ptr<TcpConnection> pointer;

  static pointer create(boost::asio::io_service& io_service) {
    return pointer(new TcpConnection(io_service));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void start() {
    // for (;;) {
    //   std::cerr << "Looping...\n";
    //   
    //   socket_.async_read_some(boost::asio::buffer(rcvbuf, BUFSIZ),
    //     boost::bind(&TcpConnection::handle_read, shared_from_this(),
    //       boost::asio::placeholders::error,
    //       boost::asio::placeholders::bytes_transferred));
    //         
    //   std::string data = "Hello from the server, " + std::to_string(seq++) + "\n";
    //   size_t wlen = boost::asio::write(socket_, boost::asio::buffer(data));
    //   
    //   std::cerr << "Wrote " << wlen << "\n";
    //   
    //   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // }
    
    message_ = "Hello from server, ";
    
    socket_.async_read_some(boost::asio::buffer(rcvbuf),
        boost::bind(&TcpConnection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    
    boost::asio::async_write(socket_, boost::asio::buffer(message_ + std::to_string(seq++) + "\n"),
        boost::bind(&TcpConnection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  TcpConnection(boost::asio::io_service& io_service)
    : socket_(io_service) {}
    
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    // std::cerr << "handle read:" << bytes_transferred << "\n";
    
    if (!error) {
      std::cerr << "  MSG: " << std::string(rcvbuf);
    }
    else if (error != boost::asio::error::eof) {
      std::cerr << "  handle_read error: " << error << "\n";
      return;
    }
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // 
    socket_.async_read_some(boost::asio::buffer(rcvbuf),
        boost::bind(&TcpConnection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    std::cerr << "Wrote bytes: " << bytes_transferred << "\n";
    
    if (error == boost::asio::error::broken_pipe) {
      std::cerr << "Connection closed by peer.\n";
      return;
    }
    else if (error) {
      std::cerr << "Write err: " << error << "\n";
      return;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    boost::asio::async_write(socket_, boost::asio::buffer(message_ + std::to_string(seq++) + "\n"),
        boost::bind(&TcpConnection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  tcp::socket socket_;
  std::string message_;
  int seq;
  char rcvbuf[BUFSIZ];
};

class TcpServer {
public:
  TcpServer(boost::asio::io_service& io_service) : 
    acceptor_(io_service, tcp::endpoint(tcp::v4(), PORT)),
    next_id(0)
  {
    // start_accept();
    
    std::thread(&TcpServer::start_accept, this).join();
    // io_service.run();
  }

private:
  void start_accept() {
    std::cerr << "start_accept\n";
    TcpConnection::pointer new_connection =
      TcpConnection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&TcpServer::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(TcpConnection::pointer new_connection,
      const boost::system::error_code& error) {
    if (!error) {
      std::cerr << "Accepted new connection." << std::endl;
      client_list[++next_id] = new_connection;
        std::cerr << "hi. #conns: " << client_list.size() << "\n";

      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
  std::map<int, TcpConnection::pointer> client_list;
  int next_id;
};

int main() {
  try {
    boost::asio::io_service io_service;
    TcpServer server(io_service);
    std::cerr << "Running server on port " << PORT << std::endl;
    for (;;) {
      // io_service.poll();
      std::cerr << "here?\n";
      sleep(1);
    }
    // io_service.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

/**
main thread:
  create server
  - run on new thread
  
  for ever:
    server.read_from_all_clients()
    
    // process data 
    
    server.send_to_all_clients(msg)
    
    sleep(300 ms)

server class:
  
*/

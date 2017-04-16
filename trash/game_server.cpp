#include "threadsafe_queue.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>
#include <boost/thread.hpp>

#include <string>
#include <array>

#define PORT 9000

using boost::asio::ip::tcp;

typedef boost::bimap<int, tcp::endpoint> ClientList;
typedef ClientList::value_type Client;
typedef std::pair<std::string, int> ClientMessage;

class NetworkServer {
public:
    NetworkServer(unsigned short port) :
      socket(io_service, tcp::endpoint(tcp::v4(), port)),
      service_thread(std::bind(&NetworkServer::run_service, this)),
      nextClientID(0)
    {
      std::cerr << "Starting server on port " << port << "\n";
    }
    
    ~NetworkServer() 
    {
      io_service.stop();
      service_thread.join();
    }

    // bool HasMessages();
    // ClientMessage PopMessage();
    // 
    // void SendToClient(std::string message, unsigned __int64 clientID, bool guaranteed = false);
    // void SendToAllExcept(std::string message, unsigned __int64 clientID, bool guaranteed = false);
    // void SendToAll(std::string message, bool guaranteed = false);

private:
    // Network send/receive stuff
    boost::asio::io_service io_service;
    tcp::socket socket;
    // tcp::endpoint server_endpoint;
    // tcp::endpoint remote_endpoint;
    std::array<char, BUFSIZ> recv_buffer;
    boost::thread service_thread;

    void start_receive()
    {
      socket.async_receive(boost::asio::buffer(recv_buffer),
        boost::bind(&NetworkServer::handle_receive, this, 
          boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
      if (!error)
      {
          try {
              auto message = ClientMessage(
                std::string(recv_buffer.data(), recv_buffer.data() + bytes_transferred), 
                1); //get_client_id(remote_endpoint));
              if (!message.first.empty())
                  message_queue.push(message);
          }
          catch (std::exception ex) {
              std::cerr << "handle_receive: Error parsing incoming message:" << ex.what();
          }
          catch (...) {
              std::cerr << "handle_receive: Unknown error while parsing incoming message";
          }
      }
      else
      {
          std::cerr << "handle_receive: error: " << error.message();
      }

      start_receive();
    }
    
    // void handle_send(std::string /*message*/, const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/)    {}
    
    void run_service() 
    {
      start_receive();
      while (!io_service.stopped()){
          try {
              io_service.run();
          } catch( const std::exception& e ) {
              std::cerr << "Server network exception: " << e.what();
          }
          catch(...) {
              std::cerr << "Unknown exception in server network thread";
          }
      }
      std::cerr << "Server network thread stopped";
    }
    
    // unsigned int get_client_id(tcp::endpoint endpoint)
    // {
    //   auto cit = clients.right.find(endpoint);
    //   if (cit != clients.right.end())
    //       return (*cit).second;
    // 
    //   nextClientID++;
    //   clients.insert(Client(nextClientID, endpoint));
    //   return nextClientID;
    // }

    // void send(std::string pmessage, tcp::endpoint target_endpoint);

    // Incoming messages queue
    // locked_queue<ClientMessage> incomingMessages;
    ThreadSafeQueue<ClientMessage> message_queue;

    // Clients of the server
    ClientList clients;
    unsigned int nextClientID;
};

int main(int argc, char* argv[]) {
  NetworkServer server(PORT);
  
  for (;;) {
    sleep(1);
  }
}

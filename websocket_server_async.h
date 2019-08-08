#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include <algorithm>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>


// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<tcp::socket> ws_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::beast::multi_buffer buffer_;
    std::atomic<int> mConnectStatus; 
public:
    // Take ownership of the socket
    explicit
    session(tcp::socket socket);

    // Start the asynchronous operation
    void
    run();

    void
    on_accept(boost::system::error_code ec);

    void
    do_read();

    void
    do_write(std::shared_ptr<std::string> data);

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred);

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred);

    bool
    sessionValid();
};


class sessionManager 
{
public:
    static sessionManager& getInstance(); 

private:

    sessionManager(){}
public:
    void pushSession(std::shared_ptr<session> pSession); 

    void notify(std::shared_ptr<std::string> data); 

private:
    std::vector<std::shared_ptr<session>> mSession;
};


// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
    listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint);

    // Start accepting incoming connections
    void
    run();

    void
    do_accept();

    void
    on_accept(boost::system::error_code ec);
};


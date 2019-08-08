//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

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
    session(tcp::socket socket)
        : ws_(std::move(socket))
        , strand_(ws_.get_executor())
    {
        boost::beast::ostream(buffer_) << "Hello world";
        mConnectStatus.store(-1);
    }

    // Start the asynchronous operation
    void
    run()
    {
        // Accept the websocket handshake
        ws_.async_accept(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_accept,
                    shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_accept(boost::system::error_code ec)
    {
        if(ec){
            //mHandshake = false;
            mConnectStatus = 1; // invalid status
            return fail(ec, "accept");
        }
        mConnectStatus = 0; // valid status;
        //mHandshake = true;
        //mConnection = true;
        // Read a message
        //do_read();
    }

    void
    do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_read,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    do_write(std::shared_ptr<std::string> data) 
    {
        if(mConnectStatus != 0) {
            return;
        }
        ws_.text(true);
        //boost::beast::multi_buffer b;
        //boost::beast::ostream(b) << (*data);
        //std::cout <<"start to write" << std::endl;
        auto b = boost::asio::buffer(*data);
        ws_.async_write(
            //buffer_.data(),
            //b.data(),
            boost::asio::buffer((*data)),
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_write,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
        //std::cout <<"end to write" << std::endl;
    }

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            fail(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(
            buffer_.data(),
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_write,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        //if(ec) {
       // //    mConnectStatus = 2;
       //     return fail(ec, "write");
       // }

        // Clear the buffer
        //buffer_.consume(buffer_.size());

        // Do another read
        //do_read();
    }

    bool
    sessionValid()
    {
        if(mConnectStatus == 1 || mConnectStatus == 2)
            return false;
        return true;
    }
};


class sessionManager 
{
public:
    static sessionManager& getInstance() {
        static sessionManager mSessionManager;
        return mSessionManager;
    }
private:

    sessionManager(){}
public:
    void pushSession(std::shared_ptr<session> pSession) {
       // mSession.erase(std::remove_if(mSession.begin(), mSession.end(), [](std::shared_ptr<session>& it){
       //     return !(it->sessionValid());
       // }));
        mSession.push_back(pSession);
        std::cout <<"session length: " << mSession.size() << std::endl;
    }

    void notify(std::shared_ptr<std::string> data) {
        //std::cout << "call notify" << std::endl;
        for(auto it : mSession) {
            it->do_write(data);
        }
        //std::cout<<"notify end" <<std::endl;
    }

private:
    std::vector<std::shared_ptr<session>> mSession;
};
//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
    listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint)
        : acceptor_(ioc)
        , socket_(ioc)
    {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            boost::asio::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        if(! acceptor_.is_open())
            return;
        do_accept();
    }

    void
    do_accept()
    {
        acceptor_.async_accept(
            socket_,
            std::bind(
                &listener::on_accept,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_accept(boost::system::error_code ec)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            auto ptr = std::make_shared<session>(std::move(socket_));
            ptr->run();
            sessionManager::getInstance().pushSession(ptr);
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
            "Usage: websocket-server-async <address> <port> <threads>\n" <<
            "Example:\n" <<
            "    websocket-server-async 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    auto iocThread = std::thread([&ioc](){
        ioc.run();
    });
    // Run the I/O service on the requested number of threads
//    std::vector<std::thread> v;
//    v.reserve(threads - 1);
//    for(auto i = threads - 1; i > 0; --i)
//        v.emplace_back(
//        [&ioc]
//        {
//            ioc.run();
//        });
//    ioc.run();
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sessionManager::getInstance().notify(std::make_shared<std::string>("Hello seat"));
    }
    return EXIT_SUCCESS;
}

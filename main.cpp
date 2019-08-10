#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <chrono>
#include <functional>
#include "serialPortChannel.h"
#include "parserManager.h"
#include "hardwareChannelManager.h"
#include "websocket_server_async.h"
#include "util.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <thread>

class syncSession : public std::enable_shared_from_this<syncSession> 
{
	websocket::stream<tcp::socket> ws;
	std::atomic<bool> mValid;
public:
	syncSession(tcp::socket& socket): ws{std::move(socket)} {
		mValid.store(true);
	}

	void run() {
		std::thread(&syncSession::work, shared_from_this()).detach();
	}

	bool write(std::shared_ptr<std::string>& data) {
		try {
			ws.text(true);
		//	ws.write(boost::asio::buffer("Hello world"));
			ws.write(boost::asio::buffer((*data)));
			return true;
		} catch (boost::system::system_error const& se) {
			mValid.store(false);
		} catch (std::exception const& e) {
			mValid.store(false);
		}
		return false;
	}
	
	bool accept() {
		try {
			ws.accept();
			return true;
		} catch(boost::system::system_error const& se) {
			return false;
		} catch (std::exception const& e) {
			return false;
		}
	}

	bool validSession() {
		return mValid.load();
	}

private:
	void work() {
		try {
			ws.accept();
			for(;;){
				ws.text(true);
				ws.write(boost::asio::buffer("hello world"));
			}
		} catch(boost::system::system_error const& se) {

		} catch (std::exception const& e) {

		}
	}
};

class syncSessionManager {
	syncSessionManager() {
		mExitUnvalidSession.store(false);
	}
public:
	static syncSessionManager& getInstance() {
		static syncSessionManager sSyncSessionManager;
		return sSyncSessionManager;
	}

public:
	void notify(std::shared_ptr<std::string>& data)
	{
		mExitUnvalidSession.store(false);
		std::lock_guard<std::mutex> lk(mtx);
		for(auto it = mSession.begin(); it != mSession.end();){
			auto ret = (*it)->write(data);
			++it;
			if(!ret)
				mExitUnvalidSession.store(true);
		}
		if(!mExitUnvalidSession.load())
			return;
		for(auto it = mSession.begin(); it != mSession.end(); ) {
			if(false == (*it)->validSession()) {
				it = mSession.erase(it);
			} else {
				++it;
			}
		}
		mExitUnvalidSession.store(false);
	}

	void registerSession(std::shared_ptr<syncSession>& pSession) 
	{
		{
			std::lock_guard<std::mutex> lk(mtx);
			mSession.push_back(pSession);
		}
		std::cout << "Session Length: "<< mSession.size() << std::endl; 
	}

	void removeSession(std::shared_ptr<syncSession>& pSession)
	{
		std::lock_guard<std::mutex> lk(mtx);
		for(auto it = mSession.begin(); it != mSession.end(); ) {
			if((*it) == pSession) {
				it = mSession.erase(it);
			} else {
				++it;
			}
		}
	}
private:
	std::mutex mtx;
	std::shared_ptr<syncSession> gSession;
	std::vector<std::shared_ptr<syncSession>> mSession;
	std::atomic<bool> mExitUnvalidSession;
};

class emgDataListener : public parserListener {

	void onParserData(parserDataType type, void* pData)
	{
		//static long length = 0;
		if( parserDataType::EMG_DATA != type) {
			return;
		}
		emgData* pEmgData = static_cast<emgData*>(pData);
		auto pstr = util::emgData2JsonString((*pEmgData));
	//	std::cout << ++length << std::endl;
		syncSessionManager::getInstance().notify(pstr);
		//sessionManager::getInstance().notify(std::make_shared<std::string>("hello world"));
		//std::cout << (*pstr) << std::endl;
	//	std::cout   << "ch1 average:" <<  pEmgData->ch1Average <<", " 
	//				<< "ch1 value:" << pEmgData->ch1Value <<", "
	//				<< "ch1 power:" << pEmgData->ch1Power <<", "
	//				<< "ch1 strength:" << pEmgData->ch1Strength <<", "
	//				<< "ch2 average:" << pEmgData->ch2Average <<", "
	//				<< "ch2 value:" << pEmgData->ch2Value <<", "
	//				<< "ch2 power:" << pEmgData->ch2Power<<", "
	//				<< "ch2 strength:" << pEmgData->ch2Strength << std::endl;
		delete pEmgData;
	}

};

class emgListener : public hardwareChannelListener {
public:
	void onOpen() override
	{
		std::cout << "serial port is opened" << std::endl;
	}

	void startParser() 
	{
		//mParser = std::make_shared<emgParser>();
		mParser = parserManager::createParser(parserDataType::EMG_DATA);
		auto listener = std::make_shared<emgDataListener>();
		mParser->registerListener(listener);
	}

	void onClose() override
	{
		std::cout << "serial port is closed" << std::endl;
	}

	void onData(std::shared_ptr<hardwareChannelData> dataPtr) override
	{
		auto tmpEmgData = dataPtr->rawData;
		mParser->push(tmpEmgData);
	//	std::cout <<"onData called  " << (*tmpEmgData).size() << std::endl;
	//	for (unsigned int i = 0; i < (*tmpEmgData).size(); ++i) {
	//		std::cout << std::hex << (*tmpEmgData)[i] << " ";
	//		if (i % 10 == 0) {
	//			std::cout << std::endl;
	//		}
	//	}
	}

	~emgListener() override {

	}
private:
	std::shared_ptr<parser> mParser;
	int i = 0;
};

static int fd = 0;

int uart_opens(int fd, const char* pathname)
{
	fd = open(pathname, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		perror("Open UART failed");
		return -1;
	}
	return fd;
}

int uart_sets(int fd, int baud, int c_flow, int bits, char parity, int stop)
{
	struct termios options;
	if (tcgetattr(fd, &options) < 0) {
		perror("tcgetattr error");
		return -1;
	}

	cfsetspeed(&options, B115200); //baudrate
	options.c_cflag |= CLOCAL; // make compatible to original
	options.c_cflag |= CREAD;  // make compatible to origianl
	options.c_cflag &= ~CRTSCTS; // no flow control
	options.c_cflag &= ~CSIZE; // set 8 date bit
	options.c_cflag |= CS8;  
	options.c_cflag &= ~CSTOPB; // one stop bit
	options.c_cflag &= ~PARENB; // no check bit
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_iflag &= ~(ICRNL | IXON);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;
	tcflush(fd, TCIFLUSH);

	if (tcsetattr(fd, TCSANOW, &options) < 0) {
		perror("tcsetattr failed");
		return -1;
	}
	return 0;

}

int uart_closes(int fd)
{
	close(fd);
	return 0;
}

ssize_t safe_reads(int fd, void *vptr, size_t count)
{
	size_t nleft = count;
	ssize_t nread = 0;
	char *ptr = static_cast<char*>(vptr);

	while (nleft > 0)
	{
		if ((nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)//���ź��ж�
				nread = 0;
			else
				return -1;
		}
		else
			if (nread == 0)
				break;
		nleft -= nread;
		ptr += nread;
	}
	return (count - nleft);
}

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;


int main(int argc, char* argv[])
{
//        if (argc != 4)
//        {
//            std::cerr <<
//                "Usage: websocket-server-async <address> <port> <threads>\n" <<
//                "Example:\n" <<
//                "    websocket-server-async 0.0.0.0 8080 1\n";
//            return EXIT_FAILURE;
//        }
	//serialPortChannel serial;
	auto serialChannel = hardwareChannelManager::createHardwareChannel(hardwareChannelType::EMG_UART);
	auto listen = std::make_shared<emgListener>();
	listen->startParser();
	//serial.registerListener(listen);
	serialChannel->registerListener(listen);
	std::string path("/dev/ttyUSB0");
//	serial.open(path);
	serialChannel->open(path);
	//serial.run();
	serialChannel->run();
	try {

		if(argc != 3) {
			std::cerr <<
				"Usage: websocket-server-sync <address> <port>\n" <<
				"Example: \n" <<
				"    websocket-server-sync 0.0.0.0 8080\n";
				return EXIT_FAILURE;
		}

		auto const address = boost::asio::ip::make_address(argv[1]);
		auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
		boost::asio::io_context ioc{1};
		tcp::acceptor acceptor{ioc, {address, port}};
		for(;;) {
			tcp::socket socket{ioc};
			acceptor.accept(socket);
			auto pSession = std::make_shared<syncSession>(socket);
			auto ret = pSession->accept();
			if(ret) {
				syncSessionManager::getInstance().registerSession(pSession);
			}
		}
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
//        auto const address = boost::asio::ip::make_address(argv[1]);
//        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
//        auto const threads = std::max<int>(1, std::atoi(argv[3]));
//    
//        // The io_context is required for all I/O
//        boost::asio::io_context ioc{threads};
//    
//        // Create and launch a listening port
//        std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();
//    
//        auto iocThread = std::thread([&ioc](){
//            ioc.run();
//        });
//        while(true) {
//            std::this_thread::sleep_for(std::chrono::microseconds(1));
//            sessionManager::getInstance().notify(std::make_shared<std::string>("Hello seat"));
//        }
//      return EXIT_SUCCESS;


	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

	std::cout <<"Start to open ttyUSB0" << std::endl;
	fd = uart_opens(fd, "/dev/ttyUSB0");
	std::cout <<"port " << fd << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (fd) {
		std::cout << "open uart success" << std::endl;
	}
	int ret = uart_sets(fd, 0, 0, 0, 0, 0);
	if (ret == 0) {
		std::cout << "set parameters success" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::microseconds(1));
	unsigned char rvalue[50];

	while (true) {
		ssize_t re = safe_reads(fd, rvalue, 50);
		for (unsigned int i = 0; i < re; ++i) {
			std::cout << std::hex << (unsigned int)rvalue[i] << " ";
			if (i % 10 == 0) {
				std::cout << std::endl;
			}
		}
		//std::cout << std::endl;
	}
    return 0;
}

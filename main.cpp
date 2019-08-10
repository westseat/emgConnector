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

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;


int main(int argc, char* argv[])
{
	auto serialChannel = hardwareChannelManager::createHardwareChannel(hardwareChannelType::EMG_UART);
	auto listen = std::make_shared<emgListener>();
	listen->startParser();
	serialChannel->registerListener(listen);
	std::string path("/dev/ttyUSB0");
	serialChannel->open(path);
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
}

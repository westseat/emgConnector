#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <iostream>
#include "serialPortChannel.h"
#include <chrono>
#include <functional>
#include "hardwareChannel.h"
#include "emgParser.h"
#include <string>

class emgDataListener : public parserListener {

void onParserData(DataType type, void* pData)
{
	if( DataType::EMG_DATA != type) {
		return;
	}
	emgData* pEmgData = static_cast<emgData*>(pData);
	std::cout << pEmgData->ch1Average <<" " 
				<< pEmgData->ch1Value <<" "
				<< pEmgData->ch1Power <<" "
				<< pEmgData->ch1Strength <<" "
				<< pEmgData->ch2Average <<" "
				<< pEmgData->ch2Value <<" "
				<< pEmgData->ch2Power<<" "
				<< pEmgData->ch2Strength << std::endl;
	delete pEmgData;
}

};

class emgListener : public HardwareChannelListener {
public:
	void onOpen() override
	{
		std::cout << "serial port is opened" << std::endl;
	}

	void startParser() 
	{
		mParser = std::make_shared<emgParser>();
		auto listener = std::make_shared<emgDataListener>();
		mParser->registerListener(listener);
	}

	void onClose() override
	{
		std::cout << "serial port is closed" << std::endl;
	}

	void onData(std::shared_ptr<HardwareChannelData> dataPtr) override
	{
		auto tmpEmgData = dataPtr->rawData;
		//mParser->push(tmpEmgData);
		std::cout <<"onData called  " << (*tmpEmgData).size() << std::endl;
		for (unsigned int i = 0; i < (*tmpEmgData).size(); ++i) {
			std::cout << std::hex << (*tmpEmgData)[i] << " ";
			if (i % 10 == 0) {
				std::cout << std::endl;
			}
		}
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

int main()
{
	serialPortChannel serial;
	auto listen = std::make_shared<emgListener>();
	serial.registerListener(listen);
	std::string path("/dev/ttyUSB0");
	serial.open(path);
	serial.run();
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
	std::this_thread::sleep_for(std::chrono::seconds(1));
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
#include "serialPortChannel.h"
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <thread>
#include <array>
#include <algorithm>
#include <functional>
#include <iostream>

int uart_open(const char* pathname)
{
	int fd = open(pathname, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		perror("Open UART failed");
		return -1;
	}
	return fd;
}

int uart_set(int fd, int baud, int c_flow, int bits, char parity, int stop)
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

int uart_close(int fd)
{
	close(fd);
	return 0;
}

ssize_t safe_read(int fd, void *vptr, size_t count)
{
	size_t nleft = count;
	ssize_t nread = 0;
	unsigned char *ptr = static_cast<unsigned char*>(vptr);
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

serialPortChannel::serialPortChannel() {
	mFd = 0;
	mQuit.store(false);
}

serialPortChannel::~serialPortChannel() {

}

int serialPortChannel::open(std::string& pathname) {
//	int	fd = uart_open("/dev/ttyUSB0");
//	std::cout <<"port " << fd << std::endl;
//	std::this_thread::sleep_for(std::chrono::seconds(5));
//	if (fd) {
//		std::cout << "open uart success" << std::endl;
//	}
//	std::this_thread::sleep_for(std::chrono::seconds(5));
//	int ret = uart_set(fd, 0, 0, 0, 0, 0);
//	if (ret == 0) {
//		std::cout << "set parameters success" << std::endl;
//	}
//	std::this_thread::sleep_for(std::chrono::seconds(5));
	mFd = uart_open(pathname.c_str());
	std::cout <<"port " << mFd << std::endl;
	if(mFd <= 0) {
		return mFd;
	}
	std::cout <<"open uart success" << std::endl;
	int ret = uart_set(mFd, 0, 0, 0, 0, 0);
	if (ret == 0) {
		std::cout << "set parameters success" << std::endl;
	}
//	if (mFd) {
//		if( 0 == uart_set(mFd, 0, 0, 0, 0, 0)){
//			std::cout <<"set uart parameters success" << std::endl;
//		}
//	}
	//mFd = fd;
	return mFd;
}

int serialPortChannel::close() {
	if (mThread.joinable()) {
		mQuit.store(true);
		mThread.join();
	}
	return uart_close(mFd);
}

bool serialPortChannel::registerListener(std::shared_ptr<HardwareChannelListener> listener) {
	auto ret = mListenSet.insert(listener);
	std::cout << "registerListener " << mListenSet.size() << std::endl;
	return ret.second;
}

bool serialPortChannel::removeListener(std::shared_ptr<HardwareChannelListener> listener) {
	return mListenSet.erase(listener);
}



void serialPortChannel::readData() {

	std::array<unsigned char, 50> readValue;
	unsigned char rvalue[50];
	while (true) {
		if (mQuit.load()) {
			return;
		}
		ssize_t re = safe_read(mFd, rvalue, 50);
	//	for (unsigned int i = 0; i < re; ++i) {
	//		std::cout << std::hex << (unsigned int)rvalue[i] << " ";
	//		if (i % 10 == 0) {
	//			std::cout << std::endl;
	//		}
	//	}
		if (re == 0) {
			continue;
		}
		auto data = std::make_shared<HardwareChannelData>();
		data->type = HardwareInface::EMG_UART;
		auto emgData = std::make_shared<std::vector<unsigned char>>();
		for (unsigned i = 0; i < re; ++i) {
			emgData->push_back(rvalue[i]);
		}
		data->rawData = emgData;
		for (auto it = mListenSet.begin(); it != mListenSet.end(); ++it) {
			(*it)->onData(data);
		}
	}
}

void serialPortChannel::run() {
	std::cout <<"start to invoke run function" << std::endl;
	mThread = std::thread(std::mem_fn(&serialPortChannel::readData), this);
}

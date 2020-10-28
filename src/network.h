/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <vector>
#include <list>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "logger.h"
#include "mthread.h"
#include "rsa.h"

#define NETWORKMESSAGE_SIZE 65536

class Client {
private:
	static boost::asio::io_service	io_service;

	boost::asio::ip::tcp::socket	socket;
	boost::asio::deadline_timer		deadline;

	std::list<std::pair<char*, unsigned int> >	buffer;
	bool										writing;

	bool	connected;
	bool	connectionProcess;

	static int		clients;

public:
	Client();
	~Client();

	static void RUN_THREAD();

	bool checkError(const boost::system::error_code& error);

	bool Connect(const char* host, const char* port);
	void onConnect(const boost::system::error_code& error);
	void onTimeout(const boost::system::error_code& error);

	bool Connected();
	size_t ReadyToRead();

	unsigned int syncRead(char* msg, unsigned int size, bool resetOnError = true);
	unsigned int syncWrite(const char* msg, unsigned int size, bool resetOnError = true);
	void asyncWrite(const char* msg, unsigned int size);
	void close();

	void doAsyncWrite(char* msg, unsigned int size);
	void doClose();

	void onWrite(const boost::system::error_code& error);
};


class NetworkMessage {
private:
	unsigned short	version;

	char	buffer[NETWORKMESSAGE_SIZE];
	size_t	buffSize;
	size_t	readPos;
	size_t	startPos;

	RSA*			rsa;
	unsigned int	XTEAkey[4];

	bool	cryptoEnable;

	unsigned int adlerChecksum(unsigned char* data, int len);

public:
	NetworkMessage();
	NetworkMessage(unsigned short version);
	NetworkMessage(unsigned short version, unsigned int k[4]);
	~NetworkMessage();

	NetworkMessage* Clone(unsigned short size);

	void Reset();
	void SetRSA(RSA* rsa);
	void SetCrypto(bool state, unsigned int k[4]);

	void SendMessage(Client* client, bool login = false);
	void ReceiveMessage(Client* client);

	void AddU8(unsigned char data);
	void AddU16(unsigned short data);
	void AddU32(unsigned int data);
	void AddU64(unsigned long data);
	void AddString(std::string data);

	void AddHeaderU16(unsigned short data);
	void AddHeaderU32(unsigned int data);

	unsigned char GetU8();
	unsigned short GetU16();
	unsigned int GetU32();
	unsigned long GetU64();
	std::string GetString();
	void Skip(unsigned short size);

	unsigned short GetReadPos();
	unsigned short GetSize();
	bool EndOfPacket();

	bool RSA_encrypt(size_t pos);
	bool RSA_decrypt();
	void XTEA_encrypt();
	void XTEA_decrypt();

	friend class Logger;
};

#endif //__NETWORK_H_

/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "network.h"

#include "allocator.h"
#include "tools.h"


// ---- Client ---- //

boost::asio::io_service		Client::io_service;
int							Client::clients = 0;


Client::Client() : socket(io_service), deadline(io_service) {
	connected = false;
	writing = false;
	connectionProcess = false;

	clients++;
	if (clients == 1)
		THREAD thread(boost::bind(&Client::RUN_THREAD));
}

Client::~Client() {
	doClose();

	clients--;
	if (clients <= 0)
		io_service.stop();
}


void Client::RUN_THREAD() {
	while(clients) {
		io_service.reset();
		io_service.run();

		Sleep(50);
	}
}


bool Client::checkError(const boost::system::error_code& error) {
	if (error) {
		doClose();
		Logger::AddLog("Client::checkError()", error.message(), LOG_WARNING);
		return true;
	}

	return false;
}


bool Client::Connect(const char* host, const char* port) {
	boost::system::error_code error = boost::asio::error::host_not_found;

	boost::asio::ip::tcp::resolver 			resolver(io_service);
	boost::asio::ip::tcp::resolver::query	query(host, port);
	boost::asio::ip::tcp::resolver::iterator	endpoint_it = resolver.resolve(query, error);
	//boost::asio::ip::tcp::resolver::iterator	end;
	if (checkError(error)) return false;

	//while(!connected && endpoint_it != end) {
		doClose();

		connectionProcess = true;
		socket.async_connect(*endpoint_it, boost::bind(&Client::onConnect, this, boost::asio::placeholders::error));

		deadline.expires_from_now(boost::posix_time::seconds(3));
		deadline.async_wait(boost::bind(&Client::onTimeout, this, boost::asio::placeholders::error));

		while(connectionProcess)
			Sleep(50);
	//}

	return connected;
}

void Client::onConnect(const boost::system::error_code& error) {
	deadline.cancel();

	if (connected)
		return;

	if (!connectionProcess) {
		doClose();
		return;
	}

	if (!socket.is_open() || error) {
		std::stringstream ss;
		ss << "Error while connecting! (" << error.message() << ")";
		Logger::AddLog("Client::onConnect()", ss.str(), LOG_WARNING);
		doClose();
	}
	else
		connected = true;

	connectionProcess = false;
}

void Client::onTimeout(const boost::system::error_code& error) {
	if (error) {
		std::stringstream ss;
		ss << "Error while connecting! (" << error.message() << ")";
		Logger::AddLog("Client::onTimeout()", ss.str(), LOG_WARNING);
	}
	else
		doClose();
}


bool Client::Connected() {
	return connected;
}

size_t Client::ReadyToRead() {
	boost::system::error_code error;
	size_t size = socket.available(error);

	if (error && connected) {
		Logger::AddLog("Client::ReadyToRead()", "Error while checking read buffer!", LOG_ERROR);
		doClose();
		size = 0;
	}

	return size;
}

unsigned int Client::syncRead(char* msg, unsigned int size, bool resetOnError) {
	boost::system::error_code error;
	unsigned int rsize = boost::asio::read(socket, boost::asio::buffer(msg, size), boost::asio::transfer_all(), error);

	if (error && connected && resetOnError) {
		//Logger::AddLog("Client::syncRead()", "Error while reading from socket!", LOG_ERROR);
		doClose();
		rsize = 0;
	}

	return rsize;
}

unsigned int Client::syncWrite(const char* msg, unsigned int size, bool resetOnError) {
	boost::system::error_code error;
	unsigned int wsize = boost::asio::write(socket, boost::asio::buffer(msg, size), boost::asio::transfer_all(), error);

	if (error && connected && resetOnError) {
		//Logger::AddLog("Client::syncWrite()", "Error while writing to socket!", LOG_ERROR);
		doClose();
		wsize = 0;
	}

	return wsize;
}

void Client::asyncWrite(const char* msg, unsigned int size) {
	char* buf = new(M_PLACE) char[size];
	memcpy(buf, msg, size);
	buffer.push_back(std::pair<char*, unsigned int>(buf, size));

	io_service.post(boost::bind(&Client::doAsyncWrite, this, buf, size));
}

void Client::close() {
	io_service.post(boost::bind(&Client::doClose, this));
}

void Client::doAsyncWrite(char* msg, unsigned int size) {
	if (!writing) {
		writing = true;
		boost::asio::async_write(socket, boost::asio::buffer(msg, size), boost::asio::transfer_all(), boost::bind(&Client::onWrite, this, boost::asio::placeholders::error));
	}
}

void Client::doClose() {
	connected = false;
	connectionProcess = false;
	writing = false;
	deadline.cancel();
	socket.close();
}

void Client::onWrite(const boost::system::error_code& error) {
	if (!error) {
		char* 			buf = buffer.front().first;
		unsigned int	size = buffer.front().second;

		delete_debug_array(buf, M_PLACE);
		buffer.pop_front();

		if (buffer.empty())
			writing = false;
		else
			boost::asio::async_write(socket, boost::asio::buffer(buffer.front().first, buffer.front().second), boost::bind(&Client::onWrite, this, boost::asio::placeholders::error));
	}
	else {
		Logger::AddLog("Client::onWrite()", "Error while writing to socket!", LOG_ERROR);
		socket.close();
		connected = false;
	}
}


// ---- NetworkMessage ---- //

NetworkMessage::NetworkMessage() { }

NetworkMessage::NetworkMessage(unsigned short version) {
	this->version = version;
	Reset();

	rsa = NULL;
	cryptoEnable = false;
}

NetworkMessage::NetworkMessage(unsigned short version, unsigned int k[4]) {
	this->version = version;
	Reset();

	rsa = NULL;
	SetCrypto(true, k);
}

NetworkMessage::~NetworkMessage() { }


NetworkMessage* NetworkMessage::Clone(unsigned short size) {
	NetworkMessage* msg = new(M_PLACE) NetworkMessage;

	msg->version = this->version;
	msg->rsa = this->rsa;
	msg->cryptoEnable = this->cryptoEnable;

	msg->buffSize = size;
	msg->readPos = 0;
	msg->startPos = 0;
	memcpy(msg->buffer, this->buffer + this->startPos + this->readPos, size);

	memcpy(msg->XTEAkey, this->XTEAkey, 4 * sizeof(unsigned int));

	return msg;
}


void NetworkMessage::Reset() {
	memset(buffer, 0, NETWORKMESSAGE_SIZE);
	buffSize = 0;
	readPos = 0;

	if (version <= 822)
		startPos = 4;
	else
		startPos = 8;
}

void NetworkMessage::SetRSA(RSA* rsa) {
	this->rsa = rsa;
}

void NetworkMessage::SetCrypto(bool state, unsigned int k[4]) {
	cryptoEnable = state;
	memcpy(XTEAkey, k, sizeof(unsigned int) * 4);
}

void NetworkMessage::SendMessage(Client* client, bool login) {
	if (!login)
		AddHeaderU16(buffSize);

	if (cryptoEnable)
		XTEA_encrypt();

	if (version > 822)
		AddHeaderU32(adlerChecksum((unsigned char*)(buffer + startPos), buffSize));
	AddHeaderU16(buffSize);

	if (client->Connected())
		client->syncWrite(buffer + startPos, buffSize);
}

void NetworkMessage::ReceiveMessage(Client* client) {
	readPos = 0;
	startPos = 0;

	size_t packet = 0;

	packet = client->syncRead(buffer, 2);
	buffSize += packet;
	if (packet == 0 || buffSize != 2 || buffSize > NETWORKMESSAGE_SIZE - 2) {
		Logger::AddLog("NetworkMessage::ReceiveMessage()", "Wrong buffer size (1)!", LOG_WARNING);
		Reset();
		return;
	}

	unsigned short size = GetU16();

	packet = client->syncRead(buffer + 2, size);
	buffSize += packet;
	if (packet == 0 || buffSize <= 2 || buffSize != size + 2) {
		Logger::AddLog("NetworkMessage::ReceiveMessage()", "Wrong buffer size (2)!", LOG_WARNING);
		Reset();
		return;
	}

	if (version > 822) {
		unsigned int checksumA = GetU32();
		unsigned int checksumB = adlerChecksum((unsigned char*)(buffer + readPos), buffSize - readPos);
	}

	if (cryptoEnable)
		XTEA_decrypt();
}

void NetworkMessage::AddU8(unsigned char data) {
	*((unsigned char*)(buffer + startPos + buffSize)) = data;
	buffSize += 1;
}

void NetworkMessage::AddU16(unsigned short data) {
	*((unsigned short*)(buffer + startPos + buffSize)) = data;
	buffSize += 2;
}

void NetworkMessage::AddU32(unsigned int data) {
	*((unsigned int*)(buffer + startPos + buffSize)) = data;
	buffSize += 4;
}

void NetworkMessage::AddU64(unsigned long data) {
	*((unsigned long*)(buffer + startPos + buffSize)) = data;
	buffSize += 8;
}

void NetworkMessage::AddHeaderU16(unsigned short data) {
	if (startPos - 2 >= 0) startPos -= 2;
	else return;

	*((unsigned short*)(buffer + startPos)) = data;
	buffSize += 2;
}

void NetworkMessage::AddHeaderU32(unsigned int data) {
	if (startPos - 4 >= 0) startPos -= 4;
	else return;

	*((unsigned int*)(buffer + startPos)) = data;
	buffSize += 4;
}

void NetworkMessage::AddString(std::string data) {
	unsigned short length = data.length();

	*((unsigned short*)(buffer + startPos + buffSize)) = length;
	buffSize += 2;

	memcpy(buffer + startPos + buffSize, data.c_str(), length);
	buffSize += length;
}

unsigned char NetworkMessage::GetU8() {
	if (buffSize < readPos + 1) {
		Logger::AddLog("NetworkMessage::GetU8()", "End of buffer!", LOG_ERROR);
		return 0;
	}

	unsigned char ret = *((unsigned char*)(buffer + startPos + readPos));
	readPos += 1;

	return ret;
}

unsigned short NetworkMessage::GetU16() {
	if (buffSize < readPos + 2) {
		Logger::AddLog("NetworkMessage::GetU16()", "End of buffer!", LOG_ERROR);
		return 0;
	}

	unsigned short ret = *((unsigned short*)(buffer + startPos + readPos));
	readPos += 2;

	return ret;
}

unsigned int NetworkMessage::GetU32() {
	if (buffSize < readPos + 4) {
		Logger::AddLog("NetworkMessage::GetU32()", "End of buffer!", LOG_ERROR);
		return 0;
	}

	unsigned int ret = *((unsigned int*)(buffer + startPos + readPos));
	readPos += 4;

	return ret;
}

unsigned long NetworkMessage::GetU64() {
	if (buffSize < readPos + 8) {
		Logger::AddLog("NetworkMessage::GetU64()", "End of buffer!", LOG_ERROR);
		return 0;
	}

	unsigned long ret = *((unsigned long*)(buffer + startPos + readPos));
	readPos += 8;

	return ret;
}

std::string NetworkMessage::GetString() {
	if (buffSize < readPos + 2) {
		Logger::AddLog("NetworkMessage::GetString()", "End of buffer (1)!", LOG_ERROR);
		return "";
	}

	unsigned short length = *((unsigned short*)(buffer + startPos + readPos));
	readPos += 2;

	if (buffSize < readPos + length) {
		Logger::AddLog("NetworkMessage::GetString()", "End of buffer (2)!", LOG_ERROR);
		return "";
	}

	std::string ret;
	ret.insert(0, buffer + startPos + readPos, length);
	readPos += length;

	return ret;
}

void NetworkMessage::Skip(unsigned short size) {
	if (buffSize < readPos + size) {
		Logger::AddLog("NetworkMessage::Skip()", "End of buffer!", LOG_ERROR);
		return;
	}

	readPos += size;
}

unsigned short NetworkMessage::GetReadPos() {
	return readPos;
}

unsigned short NetworkMessage::GetSize() {
	return buffSize;
}

bool NetworkMessage::EndOfPacket() {
	if (readPos + 1 > buffSize)
		return true;

	return false;
}

bool NetworkMessage::RSA_encrypt(size_t pos) {
	if (!rsa) {
		Logger::AddLog("NetworkMessage::RSA_encrypt()", "No RSA key!", LOG_ERROR);
		return false;
	}

	char* data = new(M_PLACE) char[128];
	memset(data, 0, 128);
	memcpy(data + 1, buffer + startPos + pos, buffSize - pos);

	if (!rsa->encrypt(data, 128)) {
		Logger::AddLog("NetworkMessage::RSA_encrypt()", "Error while encrypting!", LOG_ERROR);
		return false;
	}

	memcpy(buffer + startPos + pos, data, 128);
	buffSize = 128 + pos;

	return true;
}

bool NetworkMessage::RSA_decrypt() {
	if (!rsa) {
		Logger::AddLog("NetworkMessage::RSA_decrypt()", "No RSA key!", LOG_ERROR);
		return false;
	}

	if (buffSize - readPos != 128) {
		Logger::AddLog("NetworkMessage::RSA_decrypt()", "Wrong buffer size!", LOG_ERROR);
		return false;
	}

	if (!rsa->decrypt((char*)(buffer + readPos), 128)) {
		Logger::AddLog("NetworkMessage::RSA_decrypt()", "Error while decrypting!", LOG_ERROR);
		return false;
	}

	if (GetU8() != 0) {
		Logger::AddLog("NetworkMessage::RSA_decrypt()", "Wrong first byte!", LOG_ERROR);
		return false;
	}

	return true;
}

void NetworkMessage::XTEA_encrypt() {
	unsigned int k[4];
	k[0] = XTEAkey[0];
	k[1] = XTEAkey[1];
	k[2] = XTEAkey[2];
	k[3] = XTEAkey[3];

	int messageLength = buffSize;

	//add bytes until reach 8 multiple
	unsigned int n;
	if((messageLength % 8) != 0){
		n = 8 - (messageLength % 8);
		for (int i = 0; i < n; i++)
			AddU8(0x33);
		messageLength = messageLength + n;
	}

	int read_pos = 0;
	unsigned int* buffer = (unsigned int*)(this->buffer + startPos);
	while(read_pos < messageLength/4){
		unsigned int v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		unsigned int delta = 0x61C88647;
		unsigned int sum = 0;

		for(int i = 0; i < 32; i++) {
			v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
			sum -= delta;
			v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
		}
		if (read_pos + 1 < NETWORKMESSAGE_SIZE) {
			buffer[read_pos] = v0;
			buffer[read_pos + 1] = v1;
		}
		read_pos = read_pos + 2;
	}
}

void NetworkMessage::XTEA_decrypt() {
	uint32_t k[4];
	k[0] = XTEAkey[0];
	k[1] = XTEAkey[1];
	k[2] = XTEAkey[2];
	k[3] = XTEAkey[3];

	unsigned int* buffer = (unsigned int*)(this->buffer + startPos + readPos);
	int read_pos = 0;
	int messageLength = buffSize + 2 - startPos;
	while(read_pos < messageLength/4){
		unsigned int v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		unsigned int delta = 0x61C88647;
		unsigned int sum = 0xC6EF3720;

		for(int i = 0; i < 32; i++) {
			v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
			sum += delta;
			v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
		}
		if (read_pos + 1 < NETWORKMESSAGE_SIZE) {
			buffer[read_pos] = v0;
			buffer[read_pos + 1] = v1;
		}
		read_pos = read_pos + 2;
	}

	buffSize = GetU16();

	startPos += readPos;
	readPos = 0;
}

unsigned int NetworkMessage::adlerChecksum(unsigned char* data, int len) {
	if(len < 0)
		return 0;

    unsigned int a = 1, b = 0;
    while(len > 0)
    {
        size_t tlen = len > 5552 ? 5552 : len;
        len -= tlen;
        do
        {
            a += *data++;
            b += a;
        } while (--tlen);

        a %= 65521;
        b %= 65521;
    }
    unsigned int ret = (b << 16) | a;

    return ret;
}

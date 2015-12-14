extern "C" {
  #include "wl_definitions.h"
  #include "wl_types.h"
  #include "string.h"
}

#include "WiFi.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "server_drv.h"

WiFiClient::WiFiClient() : _sock(MAX_SOCK_NUM) {
}

WiFiClient::WiFiClient(uint8_t sock) {
	_sock = sock;
	if(sock != 255)
		_is_connected = true;
}

uint8_t WiFiClient::connected() {
  	if (_sock == 255) {
    		return 0;
  	}
	else {
		if(_is_connected)
			return 1;
		else
			return 0;
	}
}

int WiFiClient::available() {
  	if (_sock != 255)
  	{	
      		return clientdrv.availData(_sock);
  	}
   
  	return 0;
}

int WiFiClient::read() {
  	uint8_t b[1];
	
  	if (!available())
    		return -1;

  	clientdrv.getData(_sock, b);
  	return b[1];
}


int WiFiClient::read(uint8_t* buf, size_t size) {

  	uint16_t _size = size;
	int ret;
	ret = clientdrv.getDataBuf(_sock, buf, _size);
  	if (ret <= 0){
		_is_connected = false;
  	}
  	return ret;
}

void WiFiClient::stop() {

  	if (_sock == 255)
    		return;

  	clientdrv.stopClient(_sock);
  	WiFiClass::_state[_sock] = NA_STATE;
	_is_connected = false;
	
  	_sock = 255;
}

size_t WiFiClient::write(uint8_t b) {
	  return write(&b, 1);
}

size_t WiFiClient::write(const uint8_t *buf, size_t size) {
  	if (_sock >= MAX_SOCK_NUM)
  	{
	  	setWriteError();
	  	return 0;
  	}
  	if (size==0)
  	{
	  	setWriteError();
      		return 0;
  	}

  	if (!clientdrv.sendData(_sock, buf, size))
  	{
	  	setWriteError();
		_is_connected = false;
      		return 0;
  	}
	
  	return size;
}

WiFiClient::operator bool() {
  	return _sock != 255;
}

int WiFiClient::connect(const char* host, uint16_t port) {
	IPAddress remote_addr;
	
	if (WiFi.hostByName(host, remote_addr))
	{
		return connect(remote_addr, port);
	}
	return 0;
}

int WiFiClient::connect(IPAddress ip, uint16_t port) {
	_sock = getFirstSocket();
	if (_sock != NO_SOCKET_AVAIL)
    	{
    		int ret = clientdrv.startClient(uint32_t(ip), port, _sock);
    		WiFiClass::_state[_sock] = _sock;
		if(ret == 0)
			_is_connected = true;
		else
			return 0;

    	}else{
    		printf("No Socket available");
    		return 0;
    	}
    	return 1;
}


// Private Methods
uint8_t WiFiClient::getFirstSocket()
{
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
      if (WiFiClass::_state[i] == NA_STATE)
      {
          return i;
      }
    }
    return SOCK_NOT_AVAIL;
}

int WiFiClient::peek() {
	uint8_t b;
	if (!available())
		return -1;

	clientdrv.getData(_sock, &b, 1);
	return b;
}

void WiFiClient::flush() {
	while (available())
		read();
}
// Author: Daniil Chizhevskij <daniilchizhevskij@gmail.com>
// License: MIT

#include <openssl/ripemd.h>
#include <boost/asio.hpp>
#include <variant>
#include <sstream>

class AerospikeClient {
public:
  bool reconnect = false;
  std::map<std::string, std::variant<std::string, long long, double>> result;
  AerospikeClient(const std::string &host, const int &port)
      : endpoint(boost::asio::ip::address::from_string(host), port) {
    socket = new boost::asio::ip::tcp::socket(ios);
    socket->connect(endpoint);
  }

  ~AerospikeClient() { delete socket; }

  void get(std::string space, std::string key) {
    try {
      decltype(result)(std::move(result));
      sendMessage(space, key);
      unsigned long long length = getLength();
      if (length == 0)
        reconnect = true;
      else {
        reconnect = false;
        getMessage(length);
      }
    } catch (...) {
      reconnect = true;
    }
  }

private:
  boost::asio::io_service ios;
  boost::asio::ip::tcp::socket *socket;
  boost::asio::ip::tcp::endpoint endpoint;

  void sendMessage(std::string space, std::string key) {
    char request[] = "\x02\x03\x00\x00\x00\x00\x00\x00\x16\x03\x00\x00\x00\x00"
                     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02"
                     "\x00\x00\x00\x00\x00\x00";
    char body[] = "\x00\x00\x00\x15\x04";
    int lengthValue = 52 + space.size();
    for (unsigned short int i = 7; i >= 2 && lengthValue > 0; i--) {
      request[i] = request[i] + lengthValue % 256;
      lengthValue /= 256;
    }
    lengthValue = space.size() + 1;
    for (unsigned short int i = 33; i >= 28 && lengthValue > 0; i--) {
      request[i] = request[i] + lengthValue % 256;
      lengthValue /= 256;
    }
    char outputBuffer[40], result[20];
    uint8_t bytes[20];
    ripemdEncode(key, outputBuffer);
    hexToBinary(outputBuffer, bytes, 20);
    for (unsigned short int i = 0; i < 20; i++) {
      result[i] = bytes[i];
    }
    std::string message{request, sizeof(request)};
    std::string digest{result, sizeof(result)};
    std::string str{"\x00\x00\x00\x15\x04", 5};
    std::stringstream buf;
    buf << message << space << str << digest;
    std::string data = buf.str();
    socket->write_some(boost::asio::buffer(data, data.size()));
  }

  unsigned long long getLength() {
    std::vector<char> buf(8);
    boost::system::error_code error;
    size_t len = socket->read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof)
      return 0;
    else if (error)
      throw boost::system::system_error(error);
    unsigned long long length = 0;
    for (unsigned short int i = 2; i < 8; i++) {
      length *= 256;
      length += int(buf.data()[i]);
    }
    return length;
  }

  void getMessage(const unsigned long long length) {
    std::map<std::string, std::variant<std::string, long long, double>> temp;
    result = temp;
    std::vector<char> buf(length + 1);
    boost::system::error_code error;
    size_t len = socket->read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof)
      return;
    else if (error)
      throw boost::system::system_error(error);
    std::string data;
    std::copy(buf.begin(), buf.begin() + len + 1, std::back_inserter(data));
    unsigned short int keys = (int)data[21], position = 22;
    for (int i = 0; i < keys; i++) {
      unsigned short int keyLength =
          ((int)data[position + 6]) * 256 + ((int)data[position + 7]);
      unsigned long long dataLength = 0;
      for (int i = position; i < position + 4; i++) {
        dataLength *= 256;
        dataLength += (int)data[i];
      }
      std::string key = data.substr(position + 8, keyLength);
      if (data[position + 5] == '\x01') {
        long long x = 0;
        std::reverse_copy(&data[position + 7 + keyLength],
                          &data[position + 16 + keyLength],
                          reinterpret_cast<char *>(&x));
        result[key] = x;
      } else if (data[position + 5] == '\x02') {
        double x = 0;
        std::reverse_copy(&data[position + 7 + keyLength],
                          &data[position + 16 + keyLength],
                          reinterpret_cast<char *>(&x));
        result[key] = x;
      } else if (data[position + 5] == '\x03') {
        std::string x =
            data.substr(position + 8 + keyLength, dataLength - keyLength);
        result[key] = x;
      }
      position += 4 + dataLength;
    }
  }

  void hexToBinary(const char *str, uint8_t *bytes, size_t blen) {
    uint8_t pos;
    uint8_t idx0;
    uint8_t idx1;
    const uint8_t hashmap[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memset(bytes, 0, blen);
    for (pos = 0; ((pos < (blen * 2)) && (pos < strlen(str))); pos += 2) {
      idx0 = ((uint8_t)str[pos + 0] & 0x1F) ^ 0x10;
      idx1 = ((uint8_t)str[pos + 1] & 0x1F) ^ 0x10;
      bytes[pos / 2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
    };
  }

  void ripemdEncode(std::string data, char outputBuffer[40]) {
    char *string = (char *)data.c_str();
    for (short int i = strlen(string) - 1; i >= 0; i--) {
      string[i + 1] = string[i];
    }
    string[0] = '\x03';
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160_CTX ripemd160;
    RIPEMD160_Init(&ripemd160);
    RIPEMD160_Update(&ripemd160, string, data.size() + 1);
    RIPEMD160_Final(hash, &ripemd160);
    for (short int i = 0; i < RIPEMD160_DIGEST_LENGTH; i++) {
      snprintf(outputBuffer + (i * 2), sizeof(outputBuffer + (i * 2)), "%02x",
               hash[i]);
    }
    outputBuffer[40] = 0;
  }
};

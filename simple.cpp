// Author: Daniil Chizhevskij <daniilchizhevskij@gmail.com>
// License: MIT

#include "AerospikeClient.h"
#include <iostream>
#include <variant>
#include <map>

int main() {
  AerospikeClient client("127.0.0.1", 3000);
  client.get("namespace", "key");
  std::map<std::string, std::variant<std::string, long long, double>> result = client.result;
  bool isEmpty = true;
  for (auto const &iterator : result) {
    isEmpty = false;
    std::cout << iterator.first << ": \"";
    std::visit([](const auto &elem) {
      std::cout << elem << "\"." << std::endl;
    }, iterator.second);
  }
  if (isEmpty) {
    std::cout << "Empty." << std::endl;
  }
  return 0;
}

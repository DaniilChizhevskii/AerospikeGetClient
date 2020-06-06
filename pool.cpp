// Author: Daniil Chizhevskij <daniilchizhevskij@gmail.com>
// License: MIT

#include "AerospikePool.h"
#include <iostream>
#include <variant>
#include <vector>
#include <map>

int main() {
  AerospikePool pool("127.0.0.1", 3000, 10);
  for (int i = 0; i < 10; i++) {
    std::cout << "Request #" << i + 1 << "." << std::endl;
    std::map<std::string, std::variant<std::string, long long, double>> result;
    result = pool.get("namespace", "key", 20);
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
  }
  std::vector<std::tuple<int, double, double>> status = pool.status();
  for (int i = 0; i < 10; i++) {
    std::cout << "Statistic #" << i + 1 << "." << std::endl
              << std::fixed << std::setprecision(3)
              << "Creation time: " << std::get<1>(status[i])
              << ". Last using time: " << std::get<2>(status[i]) << "."
              << std::endl;
  }
  return 0;
}

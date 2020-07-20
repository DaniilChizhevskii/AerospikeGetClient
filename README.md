# Aerospike Get Client
A simple [**Aerospike**](https://www.aerospike.com) client that allows you to receive binary data by namespace and key

![Build](https://img.shields.io/github/workflow/status/DaniilChizhevskii/AerospikeGetClient/Build) ![Top Language](https://img.shields.io/github/languages/top/DaniilChizhevskii/AerospikeGetClient) ![License](https://img.shields.io/github/license/DaniilChizhevskii/AerospikeGetClient) ![Repo size](https://img.shields.io/github/repo-size/DaniilChizhevskii/AerospikeGetClient)

# Used libraries
I tried to use standard libraries - **std** and **Boost**. However, when sending a request to the server, the key is encrypted with *RIPEMD-160*, so I connected the **OpenSSL** library.

# Possibilities
This project can be used to connect to an existing **Aerospike** server for subsequent receipt of binary data from it (strings, as well as integer and fractional numbers). You can also use connection pooling to maximize performance.

# Advantages
1. Speed of work.
2. Connections pooling.

# Building
1. Install [**Boost**](https://github.com/boostorg/boost) (tested on version ≥ 1.50)
2. Install [**OpenSSL**](https://github.com/openssl/openssl) (tested on version ≥ 1.1)
3. Clone project: `git clone https://github.com/DaniilChizhevskii/AerospikeGetClient`
4. Set your folder to project folder: `cd AerospikeGetClient`
5. Make examples: `make`

# Usage
### Simple Usage (one connection)
```cpp
#include "AerospikeClient.h"
#include <iostream>
#include <variant>
#include <map>

...

AerospikeClient client("127.0.0.1", 3000); // Host & Port
client.get("namespace", "key"); // Namespace & Key
for (auto const &iterator : client.result) {
    std::cout << iterator.first << ": \"";
    std::visit([](const auto &elem) {
      std::cout << elem << "\"." << std::endl;
    }, iterator.second);
}
```
### Using connection pooling (async, up to 256 connections, with error handling)
```cpp
#include "AerospikePool.h"
#include <iostream>
#include <variant>
#include <map>

...

AerospikePool pool("127.0.0.1", 3000, 10); // Host & Port & Number of connections in pool
std::map<std::string, std::variant<std::string, long long, double>> result;
result = pool.get("namespace", "key", 20); // Namespace & Key & Timeout in milliseconds
for (auto const &iterator : result) {
    std::cout << iterator.first << ": \"";
    std::visit([](const auto &elem) {
      std::cout << elem << "\"." << std::endl;
    }, iterator.second);
}
```
#### Getting pool usage stats
```cpp
#include "AerospikePool.h"
#include <iostream>
#include <vector>

...

AerospikePool pool("127.0.0.1", 3000, 10); // Host & Port & Number of connections in pool

...

std::vector<std::tuple<int, double, double>> status = pool.status();
for (int i = 0; i < 10; i++) {
std::cout << "Statistic #" << i + 1 << "." << std::endl
          << std::fixed << std::setprecision(3)
          << "Creation time: " << std::get<1>(status[i])
          << ". Last using time: " << std::get<2>(status[i]) << "."
          << std::endl;
}
```

# Aerospike Get Client
A simple **Aerospike** client that allows you to receive binary data by namespace and key

# Used libraries
I tried to use standard libraries - **std** and **Boost**. However, when sending a request to the server, the key is encrypted with *RIPEMD-160*, so I connected the **OpenSSL** library.

# Possibilities
This project can be used to connect to an existing **Aerospike** server for subsequent receipt of binary data from it (strings, as well as integer and fractional numbers). You can also use connection pooling to maximize performance.

# Usage
### Simple Usage (one connection)
```cpp
#include "AerospikeClient.h"
#include <iostream>
#include <variant>
#include <map>

...

AerospikeClient client("127.0.0.1", 3000);
client.get("namespace", "key");
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

AerospikePool pool("127.0.0.1", 3000, 10);
std::map<std::string, std::variant<std::string, long long, double>> result;
result = pool.get("namespace", "key");
for (auto const &iterator : client.result) {
    std::cout << iterator.first << ": \"";
    std::visit([](const auto &elem) {
      std::cout << elem << "\"." << std::endl;
    }, iterator.second);
}
```

#include "AerospikeClient.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <string>
#include <vector>
#include <map>

#define MAX_RECONNECT_ATTEMPTS 1

class AerospikePool {
public:
  AerospikePool(const std::string &_host, const int &_port, const int &poolSize)
      : pool(new AerospikeClient *[poolSize]), business(new bool[poolSize]),
        statistic(new std::pair<time_t, time_t>[poolSize]) {
    size = poolSize;
    host = _host;
    port = _port;
    for (int i = 0; i < size; i++) {
      pool[i] = new AerospikeClient(host, port);
      business[i] = false;
      statistic[i].first =
          boost::chrono::duration_cast<boost::chrono::milliseconds>(
              boost::chrono::system_clock::now().time_since_epoch())
              .count();
      statistic[i].second = 0;
    }
  }

  ~AerospikePool() {
    boost::this_thread::sleep_for(boost::chrono::milliseconds{50});
    for (int i = 0; i < size; i++) {
      delete pool[i];
    }
    delete[] pool;
  }

  std::map<std::string, std::variant<std::string, long long, double>>
  get(std::string space, std::string key, const int timeOutMs) {
    int index;
    bool isFree = false;
    for (int i = 0; i < size; i++) {
      if (!business[i]) {
        index = i;
        business[i] = true;
        isFree = true;
        break;
      }
    }
    if (isFree) {
      int attempts = 0;
      while (attempts++ < MAX_RECONNECT_ATTEMPTS &&
             (attempts == 1 || pool[index]->reconnect)) {
        boost::thread *thr = new boost::thread(
            boost::bind(&AerospikeClient::get, pool[index], space, key));
        statistic[index].second =
            boost::chrono::duration_cast<boost::chrono::milliseconds>(
                boost::chrono::system_clock::now().time_since_epoch())
                .count();
        boost::this_thread::sleep_for(boost::chrono::milliseconds{timeOutMs});
        thr->interrupt();
        if (pool[index]->reconnect) {
          while (attempts++ < MAX_RECONNECT_ATTEMPTS + 1) {
            try {
              boost::this_thread::sleep_for(
                  boost::chrono::milliseconds{timeOutMs});
              reconnect(index);
              break;
            } catch (...) {
              continue;
            }
          }
        }
      }
      business[index] = false;
      return pool[index]->result;
    } else {
      std::map<std::string, std::variant<std::string, long long, double>>
          result;
      return result;
    }
  }

  void reconnect(int index) {
    delete pool[index];
    pool[index] = new AerospikeClient(host, port);
    statistic[index].first =
        boost::chrono::duration_cast<boost::chrono::milliseconds>(
            boost::chrono::system_clock::now().time_since_epoch())
            .count();
    pool[index]->reconnect = true;
  }

  std::vector<std::tuple<int, double, double>> status() {
    std::vector<std::tuple<int, double, double>> result(size);
    for (int i = 0; i < size; i++) {
      result[i] = std::make_tuple(i + 1, statistic[i].first / 1000.0,
                                  statistic[i].second / 1000.0);
    }
    return result;
  }

private:
  int size, port;
  bool *business;
  std::pair<time_t, time_t> *statistic;
  std::string host;
  AerospikeClient **pool;
};
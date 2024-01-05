#include "rins_reactor.h"
#include <bits/stdint-uintn.h>
#include <string>

class rins_client {
  public:
    rins_client();
    ~rins_client();
    int get_host(int modid, int cmdid, std::string &ip, int port);

  private:
    int _sockfd[3];
    uint32_t _seqid;
};
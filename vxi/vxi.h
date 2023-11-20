#ifndef VXI_H
#define VXI_H

#include <string>

class VXI{
  void* client;
  long lid;  // link ID
  long max_recv_size;
  u_short abort_port;
  unsigned long io_timeout;   // ms
  unsigned long lock_timeout; // ms

  // We want to have one copy of the RPC client and VXI link
  VXI(const VXI &) = delete;
  VXI& operator=(const VXI &) = delete;

public:
  VXI(const char *host, const char *dev, const double rpc_timeout=2000);


  ~VXI();
  void clear();
  void write(const char *msg);
  std::string read();

};


#endif
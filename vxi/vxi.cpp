#include <memory>
#include <iostream>
#include "err/err.h"
#include "vxi.h"
#include "vxi11.h"
#include <rpc/rpc.h>

#define OP_FLAG_WAIT_BLOCK  1
#define OP_FLAG_END  8
#define OP_FLAG_TERMCHAR_SET  128

#define RX_REQCNT  1
#define RX_CHR  2
#define RX_END  4

const char *
vxi11_strerr(int v){
  switch (v){
  case 0:  return "No error";
  case 1:  return "Syntax error";
  case 3:  return "Device not accessible";
  case 4:  return "Invalid link identifier";
  case 5:  return "Parameter error";
  case 6:  return "Channel not established";
  case 8:  return "Operation not supported";
  case 9:  return "Out of resources";
  case 11: return "Device locked by another link";
  case 12: return "No lock held by this link";
  case 15: return "IO timeout";
  case 17: return "IO error";
  case 21: return "Invalid address";
  case 23: return "Abort";
  case 29: return "Channel already established";
  }
  return "Unknown error";
}

VXI::VXI(const char *host, const char *dev, const double rpc_timeout):
    lid(0), max_recv_size(0), abort_port(0), io_timeout(5000), lock_timeout(2000){

  // create RPC connection
  client = clnt_create(host,
      DEVICE_CORE, DEVICE_CORE_VERSION, "tcp");
  if (!client) throw Err() << "RPC: can't create client";

  int s  = int(rpc_timeout);
  int us = int((rpc_timeout - s)*1e6);
  struct timeval timeout = {s, us};
  if (!clnt_control((CLIENT*)client, CLSET_TIMEOUT, &timeout))
    throw Err() << "RPC: can't set timeout";

  // Create VXI11 link
  Create_LinkParms p;
  p.clientId = 1;
  p.lockDevice = false;
  p.lock_timeout = 0;
  p.device = (char *)dev;

  Create_LinkResp *r = create_link_1(&p, (CLIENT*)client);
  if (r==0) throw Err() << "RPC: create_link";
  if (r->error!=0) throw Err() << "VXI11: " << vxi11_strerr(r->error);

  lid = r->lid;
  abort_port = r->abortPort;
  max_recv_size = r->maxRecvSize;
}

VXI::~VXI(){
  Device_Error *r = destroy_link_1(&lid, (CLIENT*)client);
  //if (r==0) throw Err() << "RPC: destroy_link";
  //if (r->error!=0) throw Err() << "VXI11: " << vxi11_strerr(r->error);
  clnt_destroy((CLIENT*)client);
}

void
VXI::clear(){
  Device_GenericParms p;
  p.lid = lid;
  p.flags = 0;
  p.io_timeout = io_timeout;
  p.lock_timeout = lock_timeout;
  Device_Error * r = device_clear_1(&p, (CLIENT*)client);
  if (r==0) throw Err() << "RPC: device_clear";
  if (r->error!=0) throw Err() << "VXI11: " << vxi11_strerr(r->error);
}

void
VXI::write(const char *msg){
  Device_WriteParms p;
  p.lid = lid;
  p.io_timeout = io_timeout;
  p.lock_timeout = lock_timeout;


  int n = strlen(msg);
  int offs = 0;
  while (n>0){
    if (n<=max_recv_size){
      p.flags = OP_FLAG_END;
      p.data.data_len = n;
    }
    else {
      p.flags = 0;
      p.data.data_len = max_recv_size;
    }
    p.data.data_val = (char *)msg + offs;
    Device_WriteResp *r = device_write_1(&p, (CLIENT*)client);
    if (r==0) throw Err() << "RPC: device_write";
    if (r->error!=0) throw Err() << "VXI11: " << vxi11_strerr(r->error);
    offs+=r->size;
    n-=r->size;
  }
}

std::string
VXI::read(){
  Device_ReadParms p;
  p.lid = lid;
  p.io_timeout = io_timeout;
  p.lock_timeout = lock_timeout;

  ///////////////////
  // TODO: we can stop on max size or
  // termination character (flags should be set!)
  int max_read_len=128*1024*1024;
  p.requestSize = max_read_len;
  p.termChar = 0;
  p.flags = 0;

  long reason = 0;
  std::string ret;
  while ((reason & (RX_END | RX_CHR | RX_REQCNT)) == 0) {
    Device_ReadResp *r = device_read_1(&p, (CLIENT*)client);
      if (r==0) throw Err() << "RPC: device_read";
      if (r->error!=0) throw Err() << "VXI11: " << vxi11_strerr(r->error);
      ret += std::string(r->data.data_val, r->data.data_len);
      reason = r->reason;
    }
  return ret;
}


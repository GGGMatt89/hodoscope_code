//------------------------------------------------------------------------------
//
// gh_packetize.h : packet tool functions
//
//------------------------------------------------------------------------------
//
// AUTHOR      : B.CARLUS (b.carlus@ipnl.in2p3.fr)
// PROJECT     : Daq GamHadron
//
//------------------------------------------------------------------------------

#ifndef GH_PACKETIZE_
#define GH_PACKETIZE_

#include <stdint.h>

namespace GhDetector
{
  
/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/ 
inline void packi16(unsigned char *buf, uint16_t i)
{
    *buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/ 
inline void packi32(unsigned char *buf, uint32_t i)
{
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/ 
inline uint16_t unpacki16(unsigned char *buf)
{
    return (buf[0]<<8) | buf[1];
}
inline uint16_t unpacky16(unsigned char *buf)
{
    return (buf[1]<<8) | buf[0];
}
/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
inline uint32_t unpacki32(unsigned char *buf)
{
    return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
}

/*
** unpacky32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
inline uint32_t unpacky32(unsigned char *buf)
{
    return (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
}
/*
** unpacki24() -- unpack a 24-bit int from a char buffer (like ntohl())
*/ 
inline uint32_t unpacki24(unsigned char *buf)
{
    return 0x00ffffff & ((buf[0]<<16) | (buf[1]<<8) | buf[2]);
}

}
#endif
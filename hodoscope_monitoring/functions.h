#ifndef FUNCTIONS_
#define FUNCTIONS_

#include <stdint.h>
#include <vector>

//VARIABLES
#define buffer_size 1472
#define BUFLEN 2048


//STRUCTURES
typedef struct hit_fiber hit_fiber;
struct hit_fiber{
  uint32_t N_fiber;
  uint8_t hit_time[4];
};

typedef struct hit_fiber_send hit_fiber_send;
struct hit_fiber_send{
  uint8_t N_fiber;
  uint8_t hit_time[4];
};

typedef struct hit_fiber_receive hit_fiber_rec;
struct hit_fiber_receive{
  uint32_t N_fiber_rec;
  uint32_t hit_time_rec;
};

typedef struct hit_fiber_receive1 hit_fiber_rec1;
struct hit_fiber_receive1{
  uint32_t N_fiber_rec;
  uint32_t hit_time_rec;
  uint16_t hit_charge_rec;

};

typedef struct header_hodo header_hodo;
struct header_hodo{
  uint8_t Front_end_N;
  uint8_t Trigger_N[3];
  uint8_t Mode_N;
  uint8_t N_hit_fibers; // ici 1
  hit_fiber * fibers = new hit_fiber[N_hit_fibers];//
};

typedef struct header_hodo_send header_hodo_send;
struct header_hodo_send{
  uint8_t Front_end_N;
  uint8_t Trigger_N[3];
  uint8_t Mode_N;
  uint8_t N_hit_fibers; // ici 1
  //hit_fiber * fibers = new hit_fiber[N_hit_fibers];//
};

typedef struct _fileHeader {
  uint16_t file_beg_id;
  uint32_t run_number;
  uint32_t Ntot_events;
}FILEHEADER;

typedef struct _fileEnd {
  uint16_t file_end_id;
  uint32_t evnt_number;
  uint32_t Ntot_oct;
}FILEEND;

typedef struct _eventHeader {
  uint16_t event_beg_id;
  uint32_t event_number;
  uint32_t trigger_number;
  uint16_t hit_in_trig;
}EVENTHEADER;

typedef struct _dataMain {
  uint8_t data_beg_id;
  uint32_t fe_number;
  uint32_t trigger_number;
  uint32_t mode_num;
  uint32_t modules_num;
}DATAMAIN;
//FUNCTIONS

inline void packi8(unsigned char *buf, uint8_t i)
{
   *buf++ = i&0x0FF;
}

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
inline uint32_t unpacki8(unsigned char *buf)
{
  return buf[0];
}

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
/*inline uint32_t unpacki24(unsigned char *buf)
{
    return 0x00ffffff & ((buf[0]<<16) | (buf[1]<<8) | buf[2]);
}
*/
inline uint32_t unpacki24(unsigned char *buf)
{
    return  (00000000 | (buf[0]<<16) | (buf[1]<<8) | buf[2]);
}

inline void convert32to24(unsigned char *buf, uint32_t i)
{
    //*buf++ = i>>24;
    *buf++ = i>>16;
    *buf++ = i>>8;
    *buf++ = i;
}
inline void convert32to16(unsigned char *buf, uint32_t i)
{
    *buf++ = i>>8;
    *buf++ = i;
}


inline void adapt32(unsigned char *buf, uint32_t i)
{
    *buf++ = (i>>24)&0x0FF;
    *buf++ = (i>>16)&0x0FF;
    *buf++ = (i>>8)&0x0FF;
    *buf++ = i&0x0FF;
}//this function basically inverts the number i in groups of 8 bits -> so for example if I have 11111110 11111100 11111000 11110000 it becomes -> 11110000 11111000 11111100 11111110


inline void create_file_limit(unsigned char* buf, uint16_t id, uint32_t runOct_n, uint16_t evnt_n){
  packi16((unsigned char*)(buf), id);
  packi32((unsigned char*)(buf+2), runOct_n);
  packi16((unsigned char*)(buf+6), evnt_n);
}

inline void pack_header(unsigned char* buf, uint16_t id, uint32_t runOct_n, uint16_t evnt_n){

  packi16((unsigned char*)(buf), id);
  packi32((unsigned char*)(buf+2), runOct_n);
  packi16((unsigned char*)(buf+6), evnt_n);

}

inline void unpack_fileHead(unsigned char* buf, FILEHEADER &file_head){
  file_head.file_beg_id = unpacki16((unsigned char *)(buf));
  file_head.run_number = unpacki32((unsigned char *)(buf+2));
  file_head.Ntot_events = unpacki32((unsigned char *)(buf+6));
}

inline void pack_fileEnd(unsigned char* buf, uint16_t id, uint32_t evnt_n, uint32_t oct_n){

  packi16((unsigned char*)(buf), id);
  packi32((unsigned char*)(buf+2), evnt_n);
  packi32((unsigned char*)(buf+6), oct_n);

}

inline void unpack_fileend(unsigned char* buf, FILEEND &file_end){
  file_end.file_end_id = unpacki16((unsigned char *)(buf));
  file_end.evnt_number = unpacki32((unsigned char *)(buf+2));
  file_end.Ntot_oct = unpacki32((unsigned char *)(buf+6));
}

inline void pack_eventHeader(unsigned char* buf, uint16_t id, uint32_t runOct_n, uint32_t trig,  uint16_t evnt_n){

  packi16((unsigned char*)(buf), id);
  packi32((unsigned char*)(buf+2), runOct_n);
  convert32to24((unsigned char*)(buf+6), trig);
  packi16((unsigned char*)(buf+9), evnt_n);

}

inline void unpack_eventHead(unsigned char* buf, EVENTHEADER &event_head){
  event_head.event_beg_id = unpacki16((unsigned char *)(buf));
  event_head.event_number = unpacki32((unsigned char *)(buf+2));
  event_head.trigger_number = unpacki24((unsigned char *)(buf+6));
  event_head.hit_in_trig = unpacki16((unsigned char *)(buf+9));
}

inline void pack_dataMain(unsigned char* buf, uint16_t id, uint8_t fe, uint32_t trig,  uint8_t mode, uint8_t N_modules){

  packi8((unsigned char*)(buf), id);
  packi8((unsigned char*)(buf+1), fe);
  convert32to24((unsigned char*)(buf+2), trig);
  packi8((unsigned char*)(buf+5), mode);
  packi8((unsigned char*)(buf+6), N_modules);

}

inline void pack_dataMain_udp(unsigned char* buf, uint8_t fe, uint32_t trig,  uint8_t mode, uint8_t N_modules){

  packi8((unsigned char*)(buf+1), fe);
  convert32to24((unsigned char*)(buf+2), trig);
  packi8((unsigned char*)(buf+5), mode);
  packi8((unsigned char*)(buf+6), N_modules);

}

inline void unpack_dataMain(unsigned char* buf, DATAMAIN &data){
  data.data_beg_id = unpacki8((unsigned char *)(buf));
  data.fe_number = unpacki8((unsigned char *)(buf+1));
  data.trigger_number = unpacki24((unsigned char *)(buf+2));
  data.mode_num = unpacki8((unsigned char *)(buf+5));
  data.modules_num = unpacki8((unsigned char *)(buf+6));

}

inline void pack_data(unsigned char* buf, uint8_t fib, uint32_t tim){

  packi8((unsigned char*)(buf), fib);
  packi32((unsigned char*)(buf+1), tim);

}

inline void unpack_data(unsigned char* buf, hit_fiber_receive &data){
  data.N_fiber_rec = unpacki8((unsigned char *)(buf));
  data.hit_time_rec = unpacki32((unsigned char *)(buf+1));
}

inline void pack_data1(unsigned char* buf, uint8_t fib, uint32_t tim, uint16_t charge){

  packi8((unsigned char*)(buf), fib);
  packi32((unsigned char*)(buf+1), tim);
  packi16((unsigned char*)(buf+5), charge);

}

inline void unpack_data1(unsigned char* buf, hit_fiber_receive1 &data){
  data.N_fiber_rec = unpacki8((unsigned char *)(buf));
  data.hit_time_rec = unpacki32((unsigned char *)(buf+1));
  data.hit_charge_rec = unpacki16((unsigned char *)(buf+5));

}


//Decoding function for fiber position VS front end channel mapping

inline int convert_Xchannel(int fe_ch){

  if(fe_ch>=0 &&fe_ch < 8){
   return (2*fe_ch) + 1;
  }
  else if(fe_ch>=8 && fe_ch < 16){
    return 32 - (2*fe_ch);
  }
  else if(fe_ch>=16 && fe_ch < 24){
    return 64 - (2*fe_ch);
  }
  else if(fe_ch>=24 && fe_ch<32){
    return 79 - (2*fe_ch);
  }
  else{/*std::cout<<"**ERROR IN FIBRE NUMBER DECODING -> EXIT! **"std::endl;*/
    return 0;
  }
}

inline int convert_Ychannel(int fe_ch){
  if(fe_ch>=0 &&fe_ch < 8){
    return 31 - (2*fe_ch);
  }
  else if(fe_ch>=8 && fe_ch < 16){
    return 48 - (2*fe_ch);
  }
  else if(fe_ch>=16 && fe_ch < 24){
    return 2*(fe_ch - 15);
  }
  else if(fe_ch>=24 && fe_ch<32){
    return 2*(31-fe_ch)+1;
  }
  else{/*std::cout<<"**ERROR IN FIBRE NUMBER DECODING -> EXIT! **"std::endl;*/
    return 0;
  }
}

inline int convert_Xchannel_withCable(int fe_ch){

  if(fe_ch>=0 &&fe_ch < 8){
   return 32 - (2*fe_ch);
  }
  else if(fe_ch>=8 && fe_ch < 16){
    return 2*fe_ch - 15;
  }
  else if(fe_ch>=16 && fe_ch < 24){
    return 64 - (2*fe_ch) - 1;
  }
  else if(fe_ch>=24 && fe_ch<32){
    return 64 - (2*fe_ch);
  }
  else{/*std::cout<<"**ERROR IN FIBRE NUMBER DECODING -> EXIT! **"std::endl;*/
    return 0;
  }
}

inline int convert_Ychannel_withCable(int fe_ch){
  if(fe_ch>=0 &&fe_ch < 8){
    return 32 - (2*fe_ch);
  }
  else if(fe_ch>=8 && fe_ch < 16){
    return (2*fe_ch) + 1;
  }
  else if(fe_ch>=16 && fe_ch < 24){
    return  (2*fe_ch) - 31;
  }
  else if(fe_ch>=24 && fe_ch<32){
    return 2*fe_ch - 46;
  }
  else{/*std::cout<<"**ERROR IN FIBRE NUMBER DECODING -> EXIT! **"std::endl;*/
    return 0;
  }
}


inline int check_mult(std::vector <int> hits, int distance, double &final_pos){

  std::sort (hits.begin(), hits.end());
  int temp_pos = hits[0];
  final_pos = hits[0];
  int mult = 1;
  for(int i = 1; i<hits.size(); i++){
    if((hits[i]-temp_pos) <= distance){
      mult++;
      temp_pos = hits[i];
      final_pos+=hits[i];
    }
    else{
      if(mult==1){
        temp_pos = hits[i];
        final_pos = hits[i];
      }
      else{
        if(i == (hits.size()-1) && mult>1){
          break;
        }
        else{
          mult = 0;
        }
      }
    }
  }

  if(mult>1){
    final_pos /= mult;
  }

return mult;

}



#endif

//c++ classes
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <vector>
#pragma pack(1)
#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

#include "functions.h"

#define BUFLEN 2048
#define buffer_size 1472

int main (int argc,char ** argv)
{
  cout<<"STARTING data creation..."<<endl;

  struct sockaddr_in client_sin, remaddr;
  int client_sock, n;
  socklen_t slen=sizeof(remaddr);
  char buf[BUFLEN];
  int recvlen;
  const char *server = "192.168.1.1";//"127.0.0.1";	/*this is for the local machine - change this to use a different server */
  uint8_t buffer_data[buffer_size];
  uint8_t buffer_data1[buffer_size];
  uint8_t temp_buf[4];
  uint8_t temp_buf_16[2];
  uint8_t beginning_event[2];
  beginning_event[0] = 0xAB;
  beginning_event[1] = 0xCD;
  uint32_t event_N = 0;
  uint32_t hit_inTrig = 0;

  header_hodo_send HODO_DATA;
  hit_fiber_send *fibers;
  uint8_t nfe = 0;
  uint32_t ntrig = 0;
  uint8_t nmode = 0;
  int nfibers = 0;
  int Nfib = 0;
  int int_time = 0;
  uint8_t nfibers8 = 0.;
  uint8_t Nfib8 = 0.;
  uint16_t nbre_struct=0;
  uint32_t compte=1;
  uint8_t aux_buf[4];
  uint8_t help_buf[2];
  int indice=0;
  int sizestructure=0;


  if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	   perror("cannot create socket");
	   return 0;
  }
  else{cout<<"socket created for client"<<endl;}
  //Definition of the client socket
  memset((char *)&client_sin, 0, sizeof(client_sin));
  client_sin.sin_family = AF_INET;
  inet_aton(server,&client_sin.sin_addr);
  client_sin.sin_port = htons(60001);

  //definition of the server address
  memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(60001);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

  int rnd_num;
  indice = 5;
  enum {UNKNOWN , EXIT , START};
	int typcommand=UNKNOWN;
  int i = 1;
  char* command = new char[100];
  char* data_char = new char[buffer_size];
  srand((int)time(0));

  while (typcommand!=EXIT)
  {
    cout << "-> Enter a command : ";
    cin >> command;

    if (!strcmp(command,"exit")) typcommand=EXIT;
    else if (!strcmp(command,"start")) typcommand=START;
    else typcommand=UNKNOWN;
    switch (typcommand)
    {
      case EXIT:
        cout << "* exit : close the connection" << endl;
        break;

      case START:
	cout << "* info : starting creation of data for the hodoscope" << endl;
        nfe = 0;
        ntrig = 0;
        nmode = 0;
        nfibers = 0;
        Nfib = 0;
        int_time = 0;
        nbre_struct=0;
        compte=1;
        indice=6;//this represent the position in the buffer where we are filling - starting from 6 to leave space for buffer header
        sizestructure=0;

        while(true){
	  usleep(1000);
          event_N = i;
          nfe = 5;
          ntrig = i;
          nmode = 0;
          nfibers = (rand() % 20);
	  nfibers8 = (uint8_t)nfibers;
	  fibers = new hit_fiber_send[nfibers];
	  //cout<<"Simulated number of touched fibers for this event : "<<nfibers<<endl;
	 cout << "* info : creating data structure" << endl;
	 //creation of the data structure for a new event
	  HODO_DATA.Front_end_N=(uint8_t)nfe;
          convert32to24((unsigned char *)HODO_DATA.Trigger_N, ntrig);
          HODO_DATA.Mode_N=(uint8_t)nmode;
	  HODO_DATA.N_hit_fibers =  (uint8_t)nfibers8;
	  for(int fbr = 0; fbr<nfibers; fbr++){
	    Nfib = rand() % 63;
	    Nfib8 = (uint8_t)Nfib;
	    int_time = rand() % 6000;
	    fibers[fbr].N_fiber = (uint8_t)Nfib8;
	    adapt32(fibers[fbr].hit_time, int_time);
	  }

	  header_hodo *pb;
	  hit_fiber_send * ffbb[nfibers];
          sizestructure = 6 + (5*nfibers);
          nbre_struct++;

	  if(indice+sizestructure>buffer_size){
	    cout<<"Sending data and starting with a new buffer ... "<<" this buffer contained "<<indice<<" octets !"<<endl;
            adapt32(aux_buf, compte);
	    for(int pk = 0; pk<4; pk++){
	      buffer_data1[pk]=aux_buf[pk];
	    }
	    packi16(&buffer_data1[4], nbre_struct-1);
	    if( sendto( client_sock, buffer_data1, indice, 0, (struct sockaddr *)&client_sin, sizeof( client_sin ) ) <0 ) {
              perror( "sendto" );
              exit( -5 );
            }//sending the old buffer
            cout<<"Buffer of data sent! Buffer size : "<<indice<<" -ooo- Events in buffer : "<<nbre_struct<<" -ooo- Total events sent till here : "<<compte<<" -ooo-  Working ... "<<endl;
            nbre_struct = 1;
            bzero(buffer_data1,buffer_size);
            compte++;
            //start filling the new buffer
            indice=6;
	    pb=(header_hodo *) & buffer_data1[indice];
	    indice = indice + 6;
	    for(int fbstr = 0; fbstr<nfibers; fbstr++){
	      ffbb[fbstr]=(hit_fiber_send *) & buffer_data1[indice];
	      indice+=5;
	    }
            bzero(aux_buf, 4);
	  }
          //STILL IN THE PRESENT BUFFER LIMIT -> GO ON FILLING
	  else if(indice+sizestructure<=buffer_size){

	    pb=(header_hodo *) & buffer_data1[indice];
	    indice = indice + 6;
	    for(int fbstr = 0; fbstr<nfibers; fbstr++){
	      ffbb[fbstr]=(hit_fiber_send *) & buffer_data1[indice];
	      indice+=5;
	    }
	  }
          pb->Front_end_N=(uint8_t)HODO_DATA.Front_end_N;
	  pb->Trigger_N[0]=(uint8_t)HODO_DATA.Trigger_N[2];
	  pb->Trigger_N[1]=(uint8_t)HODO_DATA.Trigger_N[1];
	  pb->Trigger_N[2]=(uint8_t)HODO_DATA.Trigger_N[0];
	  pb->Mode_N=(uint8_t)HODO_DATA.Mode_N;
	  pb->N_hit_fibers=(uint8_t)HODO_DATA.N_hit_fibers;
	  for(int fbr2 = 0; fbr2<nfibers; fbr2++){
	    ffbb[fbr2]->N_fiber=(uint8_t)fibers[fbr2].N_fiber;
	    ffbb[fbr2]->hit_time[0]=(uint8_t)fibers[fbr2].hit_time[0];
	    ffbb[fbr2]->hit_time[1]=(uint8_t)fibers[fbr2].hit_time[1];
	    ffbb[fbr2]->hit_time[2]=(uint8_t)fibers[fbr2].hit_time[2];
	    ffbb[fbr2]->hit_time[3]=(uint8_t)fibers[fbr2].hit_time[3];
	  }
	  i++;//incrementing the number of events created
        }
      break;

      default:
        cout << "*! ERROR : unknown command ! " << endl;
        break;
    }
  }
  delete [] command;

  close(client_sock);
  cout << "* info : end of data creation program! Bye! "<< endl;

}

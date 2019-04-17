#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>

#include <TGraph.h>
#include <TH1F.h>
#include <TString.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TFile.h>
#include <TMath.h>
#include <TSystem.h>
#include "functions.h"

using namespace std;

// Paramètres d'automatisation du nom des fichiers
void gain_channel_max_config(int N_files = 15, bool PM_an = 0){//15 values of gain
TString path;
TString path2;
TString filename;
int nASIC;
int nChannel;
int Channel_keep=0;
double thr = 0.;
double enable_sig = 0.;
double short_sig = 0.;
double first = 0., last = 0.;
double Max_enable=0., Max_short=0., Max_enable_ref=0., Max_short_ref=0.; 
double Thr_keep=0., Thr_second_keep=0., Thr_third_keep=0.;
double Max_enable_inj=0., Max_short_inj=0.;
int check_even_number=0; //used to check if the thr value is a multiple of 100 ( !!! WARNING : the injected charge datas are acquired with a threshold step o 100 !!! if it changes : adapt the program)
float Gain_keep=0.;
float gain[N_files];
for(int s = 0; s<N_files; s++){
  gain[s] = 0.25 + 0.25*s;
}
int channels=32; // 32 channels per ASIC
int breaker; // variable used for determine the minimum gain set : not used currently
int thr_position; // used to determine the reference threshold position in the vector
int new_thr_position, new_thr_position2; // used to determine the reference threshold position in the vector in the second and third parts of the program
int no_null_enable; // variable used for the selection of gains kept set
int TOT_ARRAYS = N_files * channels;

vector <int> thr_vec, enable_vec, short_vec, threshold_vec;

cout<<"Tapez le chemin vers le fichier de bruit "<<endl;
cin>>path;
cout<<"Entrez le numéro ASIC "<<endl;
cin>>nASIC;

int overlap_temp, ovl,final_thr,final_channel;
ifstream input_file;

  int thr_low[32];
  int thr_up[32];
  int thr_low_over[32];
  int thr_up_over[32];
  int Thr_set[32];
  double gain_set_max[32], gain_set_second[32], gain_set_third[32];
  double gain_record[32];
  int overlap[32];
  for(int s = 0; s<32; s++){
    thr_low[s]=0;//
    thr_up[s]=0;//
    Thr_set[s]=0;
    gain_set_max[s]=0.;
    gain_set_second[s]=0.;
    gain_set_third[s]=0.;
    gain_record[s]=0.;
    overlap[s]=0;//
   }
bool flag = 0;
double max_abs = 0.;
double max_thr = 0.;

TFile *output = new TFile(path + Form("output_max_gain_config_ASIC%i.root", nASIC), "RECREATE");

int i = 0, ii=0, iii=0, iiii=0; // loop iterators

//Files reading
for (int nChannel = 1;nChannel<=32;nChannel++) {
  i=0;
  for(int l = 0; l<N_files; l++){
    Max_enable=0.,Max_short=0.;
    max_abs = 0.;
    max_thr = 0.;
    filename = " ";
    filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
    input_file.open(filename);

    if(input_file.is_open()){

      while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);
      
//Maximum value recording
        if (enable_sig>Max_enable) {
        Max_enable = enable_sig;
        }
        if (short_sig>Max_short) {
        Max_short = short_sig;
        }
      }
    
// Scan for research of max gain without noise
    for (int scan_vectors=0; scan_vectors < thr_vec.size(); scan_vectors++) {

      if (enable_vec.at(scan_vectors)>0.01*Max_enable && short_vec.at(scan_vectors)>0.01*Max_short && Max_short>1000) { //if max short signal < 1000 counts, it is not considered as noise
      gain_record[nChannel-1]=gain[l];
      Thr_set[nChannel-1]=thr_vec.at(scan_vectors);
      break;
      }
       else{ continue;}
      
    }
    input_file.close();
    }

    else{
    cout<<"Fichier pas trouvé -> verifier le nom du fichier"<<endl;
    }

  thr_vec.clear();
  enable_vec.clear();
  short_vec.clear();

  } // close l++
  } // close channel ++
// Comparison and research of max gain value
    for (int scan_kept_gain=0; scan_kept_gain<channels; scan_kept_gain++){
      if (gain_record[scan_kept_gain]>=Gain_keep && Thr_set[scan_kept_gain]>=Thr_keep) {
      Gain_keep = gain_record[scan_kept_gain];
      Thr_keep = Thr_set[scan_kept_gain];
      Channel_keep = scan_kept_gain+1;
      }

      else {continue;}
    }

check_even_number=0;
check_even_number= Thr_keep/50;
if (check_even_number%2 == 0) {
Thr_keep = Thr_keep +200; // increase of the reference threshold to reach the end of the slope
}
else {Thr_keep = Thr_keep +150;}
  cout << "--------------------------------------------------------"<<endl;
  cout << "seuil de reference : "<< Thr_keep <<endl;
  cout << "gain de reference : "<< Gain_keep <<endl;
  cout << "channel de reference : "<< Channel_keep <<endl;

// Opening charge injection or measurement data files
cout<<"Tapez le chemin vers les fichiers injection de charge ou de mesures "<<endl;
cin>>path2;
for (int nChannel2 = 1;nChannel2<=32;nChannel2++) {
  ii=0;
  breaker=0;
  thr_position=0;
  for(int ll = 0; ll<N_files; ll++){
  Max_enable_inj=0., Max_short_inj=0.;
  filename = Form(path2 + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel2, gain[ll]);
  input_file.open(filename);

    if(input_file.is_open()){

      while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);
      
//Maximum value recording
        if (enable_sig>Max_enable_inj) {
        Max_enable_inj = enable_sig;
        }
        if (short_sig>Max_short_inj) {
        Max_short_inj = short_sig;
        } 
      }
    
//Research of reference threshold position
    if (thr_position == 0) {
    vector <int>::iterator position_thr=find (thr_vec.begin(), thr_vec.end(), Thr_keep);
    thr_position = position_thr - thr_vec.begin() +1;
    }

// Gain of all channel setting for the reference threshold
      for (int scan_configuration = 0; scan_configuration < thr_vec.size(); scan_configuration++) {
        no_null_enable=0;
        for (int scan_enable = 0; scan_enable<thr_position; scan_enable++) { 
          if (enable_vec.at(scan_enable)> 0.035*Max_enable_inj) { no_null_enable +=1; // if it decrease under 3,5% of enable max : don't count
           }
        }
        if ((thr_vec.at(scan_configuration) == Thr_keep && no_null_enable == thr_position) || (thr_vec.at(scan_configuration) == Thr_keep && enable_vec.at(scan_configuration) > 0.3*Max_enable_inj && short_vec.at(scan_configuration)< 0.05*Max_short_inj) /*&& breaker==0*/) {
        gain_set_max[nChannel2-1] = gain[ll];
        //breaker=1; //breaker condition is used to keep only the minimum gain
        break;
        }
      }
    input_file.close();
    }
    else {
    cout<<"Fichier pas trouvé -> verifier le nom du fichier"<<endl;
    continue;
    }
  thr_vec.clear();
  enable_vec.clear();
  short_vec.clear();
  }
}   

cout << "--------------------------------------------------------"<<endl;
for(int sortie=0; sortie<32; sortie++){
cout<< Form("Channel %02i : ",sortie+1)<< gain_set_max[sortie]<<endl;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Second part of the program : screening of a FIRST lower gain value for the reference channel found in the first part of the program
// Opening file with reference channel and (reference gain - 1,25V)
filename = " ";
filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,Channel_keep, Gain_keep-1.25);
input_file.open(filename);
iii=0;
if(input_file.is_open()){
  while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);

      if (enable_sig>Max_enable_ref) {
      Max_enable_ref = enable_sig;
      }
      if (short_sig>Max_short_ref) {
      Max_short_ref = short_sig;
      } 
      iii++;
  }
// Research new reference threshold for the new selected gain
  for (int second_scan=0; second_scan < thr_vec.size(); second_scan++) {
      if (enable_vec.at(second_scan)>0.01*Max_enable_ref && short_vec.at(second_scan)>0.01*Max_short_ref) {
      Thr_second_keep = thr_vec.at(second_scan);
      break;
      }
  }
check_even_number=0;
check_even_number= Thr_second_keep/50;
if (check_even_number%2 == 0) {
Thr_second_keep = Thr_second_keep +200; // increase of the reference threshold to reach the end of the slope
}
else {Thr_second_keep = Thr_second_keep +150;}
cout << "--------------------------------------------------------"<<endl;
cout << "Seuil de reference 2eme partie : " << Thr_second_keep<<endl;
input_file.close();
thr_vec.clear();
enable_vec.clear();
short_vec.clear();
}
else {
cout<<"Seconde partie du programme : Fichier pas trouvé -> verifier le nom du fichier"<<endl;
}

// Opening charge injection or measurements files 
filename = " ";
for (int nChannel3 = 1;nChannel3<=32;nChannel3++) {//
  iii=0;
  breaker=0.;
  new_thr_position=0;
  for(int lll = 0; lll<N_files; lll++){
  Max_enable_inj=0., Max_short_inj=0.;
  filename = Form(path2 + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel3, gain[lll]);
  input_file.open(filename);

  if(input_file.is_open()){
      while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);

//Maximum value recording
        if (enable_sig>Max_enable_inj) {
        Max_enable_inj = enable_sig;
        }
        if (short_sig>Max_short_inj) {
        Max_short_inj = short_sig;
        }
      iii++; 
      }
    
//Research of reference threshold position
    if (new_thr_position == 0) {
    vector <int>::iterator position_thr2=find (thr_vec.begin(), thr_vec.end(), Thr_second_keep);
    new_thr_position = position_thr2 - thr_vec.begin() +1;
    }

// Gain of all channel setting for the reference threshold
      for (int scan_configuration2 = 0; scan_configuration2 < thr_vec.size(); scan_configuration2++) {
        no_null_enable=0;
        for (int scan_enable2 = 0; scan_enable2<new_thr_position; scan_enable2++) { 
          if (enable_vec.at(scan_enable2)> 0.035*Max_enable_inj) { no_null_enable +=1; // if it decrease under 3,5% of enable max : don't count
          }
        }
        if ((thr_vec.at(scan_configuration2) == Thr_second_keep && no_null_enable == new_thr_position /*&& breaker==0*/) || (thr_vec.at(scan_configuration2) == Thr_second_keep && enable_vec.at(scan_configuration2) > 0.3*Max_enable_inj && short_vec.at(scan_configuration2)< 0.05*Max_short_inj /*&& breaker==0 && gain[lll]> (Gain_keep-1.25)-0.75*/)) {
          if (gain[lll] < (Gain_keep-1.25)+0.50){ // boundary condition to have a range of gains close to the reference gain and avoid the highest gains per channel which always meet the conditions
          gain_set_second[nChannel3-1] = gain[lll];
          //breaker=1;
          break;
          }
          else {
          gain_set_second[nChannel3-1] = (Gain_keep-1.25)+0.50;
          //breaker=1;
          break;
          }
       }
       
     }
  thr_vec.clear();
  enable_vec.clear();
  short_vec.clear();
  input_file.close();
  }
  else {
  cout<<"Seconde partie du programme : Fichier pas trouvé -> verifier le nom du fichier"<<endl;
  continue;
  } 
  }
} 
cout << "--------------------------------------------------------"<<endl;
for(int sortie2=0; sortie2<32; sortie2++){
cout<< Form("Channel %02i : ",sortie2+1)<< gain_set_second[sortie2]<<endl;
} 
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Third part of the program : screening of a SECOND lower gain value for the reference channel found in the first part of the program
// Opening file with reference channel and (reference gain - 2,50V)
filename = " ";
filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,Channel_keep, Gain_keep-2.50);
input_file.open(filename);
iiii=0;
Max_enable_ref=0., Max_short_ref=0.; //Reinitialization of max enable and short values
if(input_file.is_open()){
  while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);

      if (enable_sig>Max_enable_ref) {
      Max_enable_ref = enable_sig;
      }
      if (short_sig>Max_short_ref) {
      Max_short_ref = short_sig;
      } 
      iiii++;
  }
// Research new reference threshold for the new selected gain
  for (int third_scan=0; third_scan < thr_vec.size(); third_scan++) {
      if (enable_vec.at(third_scan)>0.01*Max_enable_ref && short_vec.at(third_scan)>0.01*Max_short_ref) {
      Thr_third_keep = thr_vec.at(third_scan);
      break;
      }
  }
check_even_number=0;
check_even_number= Thr_third_keep/50;
if (check_even_number%2 == 0) {
Thr_third_keep = Thr_third_keep +200; // increase of the reference threshold to reach the end of the slope
}
else {Thr_third_keep = Thr_third_keep +150;}
cout << "--------------------------------------------------------"<<endl;
cout << "Seuil de reference 3eme partie : " << Thr_third_keep<<endl;
input_file.close();
thr_vec.clear();
enable_vec.clear();
short_vec.clear();
}
else {
cout<<"Troisième partie du programme : Fichier pas trouvé -> verifier le nom du fichier"<<endl;
}

// Opening charge injection or measurements files 
filename = " ";
for (int nChannel4 = 1;nChannel4<=32;nChannel4++) {//
  iiii=0;
  breaker=0.;
  new_thr_position2=0;
  for(int llll = 0; llll<N_files; llll++){
  Max_enable_inj=0., Max_short_inj=0.;
  filename = Form(path2 + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel4, gain[llll]);
  input_file.open(filename);

  if(input_file.is_open()){
      while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      thr_vec.push_back(thr);
      enable_vec.push_back(enable_sig);
      short_vec.push_back(short_sig);

//Maximum value recording
        if (enable_sig>Max_enable_inj) {
        Max_enable_inj = enable_sig;
        }
        if (short_sig>Max_short_inj) {
        Max_short_inj = short_sig;
        }
      iiii++; 
      }
//Research of reference threshold position
    if (new_thr_position2 == 0) {
    vector <int>::iterator position_thr3=find (thr_vec.begin(), thr_vec.end(), Thr_third_keep);
    new_thr_position2 = position_thr3 - thr_vec.begin() +1;
    }
// Gain of all channel setting for the reference threshold
      for (int scan_configuration3 = 0; scan_configuration3 < thr_vec.size(); scan_configuration3++) {
        no_null_enable=0;
        for (int scan_enable3 = 0; scan_enable3<new_thr_position2; scan_enable3++) { 
          if (enable_vec.at(scan_enable3)> 0.035*Max_enable_inj) { no_null_enable +=1;
          }
        }
        if ((thr_vec.at(scan_configuration3) == Thr_third_keep && no_null_enable == new_thr_position2 /*&& breaker==0*/) || (thr_vec.at(scan_configuration3) == Thr_third_keep && enable_vec.at(scan_configuration3) > 0.3*Max_enable_inj && short_vec.at(scan_configuration3)< 0.05*Max_short_inj /*&& breaker==0 && gain[lll]> (Gain_keep-1.25)-0.75*/)) {
          if (gain[llll] < (Gain_keep-2.50)+0.50){ // boundary condition to have a range of gains close to the reference gain and avoid the highest gains per channel which always meet the conditions
          gain_set_third[nChannel4-1] = gain[llll];
          //breaker=1;
          break;
          }
          else {
          gain_set_third[nChannel4-1] = (Gain_keep-2.50)+0.50;
          //breaker=1;
          break;
          }
       }
       //else { cout<<"Ne rentre pas dans les conditions"<<endl;}
     }
  thr_vec.clear();
  enable_vec.clear();
  short_vec.clear();
  input_file.close();
  }
  else {
  cout<<"Seconde partie du programme : Fichier pas trouvé -> verifier le nom du fichier"<<endl;
  continue;
  } 
  }
} 
cout << "--------------------------------------------------------"<<endl;
for(int sortie3=0; sortie3<32; sortie3++){
cout<< Form("Channel %02i : %.02f",sortie3+1, gain_set_third[sortie3])<<endl;
} 

// Output file writing
ofstream out_txt(path2 + Form("Setup_gain-thr_ch_asic%i.txt", nASIC));
for(int loop = 0; loop<32; loop++){
	if(nASIC==2){out_txt<<loop+33<<"\t"<<gain_set_max[loop]<<"\t"<<gain_set_second[loop]<<"\t"<<gain_set_third[loop]<<endl;}
	if(nASIC==1){out_txt<<loop+1<<"\t"<<gain_set_max[loop]<<"\t"<<gain_set_second[loop]<<"\t"<<gain_set_third[loop]<<endl;}
}
/*int_min=Thr_min_ref;
int_max=Thr_max_ref;
nChannel=1;
for (int nChannel = 1;nChannel<=32;nChannel++) {
  if(!PM_an){
    if(nChannel != channel_ref){find_gain(path, nASIC, int_min, int_max, nChannel, thr_low[nChannel-1], thr_up[nChannel-1], thr_low_over[nChannel-1], thr_up_over[nChannel-1], gain_set[nChannel-1], overlap[nChannel-1]);
    cout<<"The identified gain for channel "<<nChannel<<" is "<<gain_set[nChannel-1]<<" with an overlap of "<<overlap[nChannel-1]<<" thr values!"<<endl;}}
  else{find_thr_PM(path, nASIC, nChannel);}
} // ferme channel ++

int min_overlap = 20;
int min_over_thrMin;
int min_over_thrMax;
int overlap_all[32];
int min_over_ch;
double gain_set_temp = 0.;

for(int x = 0; x < 32; x++){
  overlap_all[x] = 0;
  if(overlap[x]<=min_overlap){
    min_overlap = overlap[x];
    if(gain_set[x] > gain_set_temp){
      gain_set_temp = gain_set[x];
      min_over_thrMin = thr_low_over[x];
      min_over_thrMax = thr_up_over[x];
      min_over_ch = x + 1;
    }
  }
}

cout<<"Minimum overlap with reference values for channel "<<min_over_ch<<" with overlap of "<<min_overlap<<" in range "<<min_over_thrMin<<" - "<<min_over_thrMax<<endl;

int value_over[min_overlap];
for(int xc = 0; xc < min_overlap; xc++){
  value_over[xc] = 0;
}

for(int z = 0; z < 32; z++){
  for(int ss = 0; ss < min_overlap; ss++){
    for(int xs = 0; xs < overlap[z]; xs++){
      if((min_over_thrMin + (100*ss)) == (thr_low_over[z] + (xs*100))){
        overlap_all[z]++;
        value_over[ss]++;
        break;
      }
    }
  }
}
cout<< channel_ref << "  seuil bas : "<< Thr_min_ref<<"  seuil haut : "<<Thr_max_ref<<endl;
cout<<"Verified overlap for all channels "<<endl;
for(int za = 0; za <32; za++){
  cout<<"CH "<<za + 1<<" found overlap "<<overlap_all[za]<<endl;
}

int thr_to_find = 0;
int max_vote_temp = 0;
for(int fgh = 0; fgh<min_overlap; fgh++){
  if(max_vote_temp<value_over[fgh]){
    thr_to_find = min_over_thrMin + (100*fgh);
  }
}


for(int h = 0; h < min_overlap; h++){
  cout<<value_over[h]<<endl;
  if(value_over[h]==32){
    cout<<"Possible thr value = "<<min_over_thrMin + 100*h<<endl;
  }
}

bool OK = 0;
int count = 0;
for (int jh = 0; jh< 32; jh++){
  OK = 0;
  count = 0;
  if(overlap_all[jh] == 0){
    cout<<"Checking channel "<<jh<<endl;
    while((!OK)&&(count<14)){
      OK = check_gain(path, nASIC, int_min, int_max, jh+1, thr_to_find, thr_low[jh], thr_up[jh], thr_low_over[jh], thr_up_over[jh], gain_set[jh], overlap[jh]);
      if(!OK){gain_set[jh] = gain_set[jh]+0.25;}
      count++;
    }
  }
}

ofstream out_txt(path + Form("results_gain-thr_ch_asic%i.txt", nASIC));
for(int loop = 0; loop<32; loop++){
	if(nASIC==2){out_txt<<loop+33<<"\t"<<gain_set[loop]<<"\t"<<thr_low[loop]<<"\t"<<thr_up[loop]<<"\t"<<thr_low_over[loop]<<"\t"<<thr_up_over[loop]<<"\t"<<overlap[loop]<<endl;}
	if(nASIC==1){out_txt<<loop+1<<"\t"<<gain_set[loop]<<"\t"<<thr_low[loop]<<"\t"<<thr_up[loop]<<"\t"<<thr_low_over[loop]<<"\t"<<thr_up_over[loop]<<"\t"<<overlap[loop]<<endl;}
}
int cnl=1;
ovl=overlap_temp;
for (cnl=1;cnl<=32;cnl++) {
    if (overlap[cnl-1]<ovl) {
        ovl=overlap[cnl-1];
        final_channel=cnl;
    }
}
cout<<thr_low[final_channel-1]<<endl;
i=0;

threshold_vec.clear();
output->cd();*/

output->Close();

return;

} // ferme fonction main

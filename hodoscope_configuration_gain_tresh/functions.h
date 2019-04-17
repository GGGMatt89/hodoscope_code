#include <iostream>
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

using namespace std;

void find_gain(TString path, int nASIC, double int_min, double int_max, int nChannel, int &thr_low, int &thr_up, int &thr_low_over, int &thr_up_over, double &gain_set, int &overlap_temp){
  int N_files = 15;
  float gain[N_files];
  for(int s = 0; s<N_files; s++){
    gain[s] = 0.25 + 0.25*s;
  }
  double thr = 0.;
  double enable_sig = 0.;
  double short_sig = 0.;
  double first = 0., last = 0.;
  int thr_min[15];
  int thr_max[15];
  double short_rel_max = 0.;
  int flag = 0;
  TString filename;
  int overlap[15];
  overlap_temp = 0;
  // Courbes
  vector <int> thr_vec, enable_vec, short_vec;
  ifstream input_file;

for(int l = 0; l<N_files; l++){
  overlap[l]=0;
	thr_min[l]=0;
	thr_max[l]=0;

  filename = " ";
  filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
  //cout<<filename<<endl;
  input_file.open(filename);
  short_rel_max = 0.;
  //thr_min = 0;
  //thr_max = 0;
  flag = 0;
  //overlap = 0;
  if(input_file.is_open()){
    //cout<<"Reading file "<<filename<<endl;
    while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      //cout<<thr<<"\t"<<short_sig<<endl;
      if(short_sig>short_rel_max){
        short_rel_max = short_sig;
        //cout<<"relative maximum is "<<short_rel_max<<endl;
      }
      else if(short_rel_max>10 && short_sig<=short_rel_max/20 && flag == 0){
        thr_max[l] = thr;
        flag = 1;
      }
      else if(short_rel_max>10 && short_sig>=short_rel_max/20 && flag == 1){
        thr_min[l] = thr+100;
        break;
      }
    }
    input_file.close();
    cout<<"Gain "<<gain[l]<<" -> relative max "<<short_rel_max<<endl;
    cout<<"thr between "<<thr_min[l]<<" - "<<thr_max[l]<<endl;
    if(thr_min[l]>=int_min && thr_min[l]<=int_max && thr_max[l] >= int_max){
      overlap[l] = (int_max - thr_min[l])/100 + 1;
      cout<<"Overlap of "<<overlap[l]<<" values "<<endl;
    }
    else if(thr_min[l]<=int_min && thr_max[l] >= int_max){
      overlap[l] = (int_max - int_min)/100 + 1;
      cout<<"Overlap of "<<overlap[l]<<" values "<<endl;
    }
    else if(thr_min[l]>=int_min && thr_max[l] <= int_max){
      overlap[l] = (thr_max[l] - thr_min[l])/100 + 1;
      cout<<"Overlap of "<<overlap[l]<<" values "<<endl;
    }
    else if(thr_min[l]<=int_min && thr_max[l]>=int_min && thr_max[l] <= int_max){
      overlap[l] = (thr_max[l] - int_min)/100 + 1;
      cout<<"Overlap of "<<overlap[l]<<" values "<<endl;
    }
    else{cout<<"Interval mis-identification"<<endl;}
}
else{
    cout<<"File not found -> skipping gain "<<gain[l]<<endl;
    continue;
  }
}

for(int y = 0; y < 15; y++){
  if(overlap[y] > overlap_temp){
    overlap_temp = overlap[y];
    gain_set = gain[y];
	  thr_low = thr_min[y];
	  thr_up = thr_max[y];
    break;
  }
}

if(thr_low>=int_min && thr_low<=int_max && thr_up >= int_max){
  thr_up_over = int_max;
  thr_low_over = thr_low;
}
else if(thr_low<=int_min && thr_up >= int_max){
  thr_up_over = int_max;
  thr_low_over = int_min;
}
else if(thr_low>=int_min && thr_up <= int_max){
  thr_up_over = thr_up;
  thr_low_over = thr_low;
}
else if(thr_low<=int_min && thr_up>=int_min && thr_up <= int_max){
  thr_up_over = thr_up;
  thr_low_over = int_min;
}

cout<<"----------------------------------"<<endl;
}

bool check_gain(TString path, int nASIC, double int_min, double int_max, int nChannel, int thr_to_find, int &thr_low, int &thr_up, int &thr_low_over, int &thr_up_over, double &gain_set, int &overlap_temp){

  double thr = 0.;
  double enable_sig = 0.;
  double short_sig = 0.;
  double first = 0., last = 0.;
  int thr_min;
  int thr_max;
  double short_rel_max = 0.;
  int flag = 0;
  TString filename;
  int overlap;
  overlap_temp = 0;
  // Courbes
  vector <int> thr_vec, enable_vec, short_vec;
  ifstream input_file;

  overlap=0;
	thr_min=0;
	thr_max=0;

  filename = " ";
  filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain_set + 0.25);
  //cout<<filename<<endl;
  input_file.open(filename);
  short_rel_max = 0.;
  //thr_min = 0;
  //thr_max = 0;
  flag = 0;
  //overlap = 0;
  if(input_file.is_open()){
    //cout<<"Reading file "<<filename<<endl;
    while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      //cout<<thr<<"\t"<<short_sig<<endl;
      if(short_sig>short_rel_max){
        short_rel_max = short_sig;
        //cout<<"relative maximum is "<<short_rel_max<<endl;
      }
      else if(short_rel_max>10 && short_sig<=short_rel_max/20 && flag == 0){
        thr_max = thr;
        flag = 1;
      }
      else if(short_rel_max>10 && short_sig>=short_rel_max/20 && flag == 1){
        thr_min = thr+100;
        break;
      }
    }
    input_file.close();

    if(thr_min>=int_min && thr_min<=int_max && thr_max >= int_max){
      overlap = (int_max - thr_min)/100 + 1;
      cout<<"Overlap of "<<overlap<<" values "<<endl;
    }
    else if(thr_min<=int_min && thr_max >= int_max){
      overlap = (int_max - int_min)/100 + 1;
      cout<<"Overlap of "<<overlap<<" values "<<endl;
    }
    else if(thr_min>=int_min && thr_max <= int_max){
      overlap = (thr_max - thr_min)/100 + 1;
      cout<<"Overlap of "<<overlap<<" values "<<endl;
    }
    else if(thr_min<=int_min && thr_max>=int_min && thr_max <= int_max){
      overlap = (thr_max - int_min)/100 + 1;
      cout<<"Overlap of "<<overlap<<" values "<<endl;
    }
    else{cout<<"Interval mis-identification"<<endl;
    return 0;}
}
else{
    return 0;
}

thr_low = thr_min;
thr_up = thr_max;

if(thr_low>=int_min && thr_low<=int_max && thr_up >= int_max){
  thr_up_over = int_max;
  thr_low_over = thr_low;
}
else if(thr_low<=int_min && thr_up >= int_max){
  thr_up_over = int_max;
  thr_low_over = int_min;
}
else if(thr_low>=int_min && thr_up <= int_max){
  thr_up_over = thr_up;
  thr_low_over = thr_low;
}
else if(thr_low<=int_min && thr_up>=int_min && thr_up <= int_max){
  thr_up_over = thr_up;
  thr_low_over = int_min;
}

for(int zxs = 0; zxs<overlap; zxs++){
  if((thr_low_over + zxs*100) == thr_to_find){
    gain_set = gain_set + 0.25;
    cout<<"GAIN FOUND"<<endl;
    return 1;
  }
}

return 0;

cout<<"----------------------------------"<<endl;
}


void find_thr_PM(TString path, int nASIC, int nChannel){
  int N_files = 15;
  float gain[N_files];
  for(int s = 0; s<N_files; s++){
    gain[s] = 0.25 + 0.25*s;
  }
  double thr = 0.;
  double enable_sig = 0.;
  double short_sig = 0.;
  double first = 0., last = 0.;
  int thr_min[15];//thr short
  int thr_max[15];//thr enable
	bool flag_min = 0;
	bool flag_max = 0;

  TString filename;
  // Courbes
  vector <int> thr_vec, enable_vec, short_vec;
  ifstream input_file;

for(int l = 0; l<N_files; l++){
thr_min[l]=0;
thr_max[l]=0;
flag_min = 0;
flag_max = 0;
  filename = " ";
  filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
  //cout<<filename<<endl;
  input_file.open(filename);
  if(input_file.is_open()){
    //cout<<"Reading file "<<filename<<endl;
    while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
      //cout<<thr<<"\t"<<short_sig<<endl;
      if(short_sig>10 && flag_min==0){
        thr_min[l] = thr;
		flag_min = 1;
      }
      if(enable_sig>10 && flag_max==0){
        thr_max[l] = thr;
		flag_max = 1;
    	}
	if(flag_min == 1 && flag_max == 1)
		break;
	}
    input_file.close();

}else{cout<<"File not found"<<endl; continue;}

}

ofstream out_txt(path + Form("THR_PM_Asic%i_CH(%02i).txt",nASIC ,nChannel));
for(int y = 0; y < 15; y++){
	out_txt<<gain[y]<<"\t"<<thr_min[y]<<"\t"<<thr_max[y]<<endl;

}

out_txt.close();
cout<<"----------------------------------"<<endl;
}

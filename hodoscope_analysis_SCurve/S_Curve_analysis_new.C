#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <TGraph.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TFile.h>
#include <TMath.h>
#include <TSystem.h>
#include <TPad.h>

using namespace std;
// Reading files and saving data in vectors function
void txt_to_vec_max(ifstream& file, vector <int> &enable_vec, vector <int> &short_vec, vector <int> &threshold_vec, int &max){
  double enable_sig=0., short_sig=0., thr=0., first=0., last=0.;
  while(file>>thr>>first>>enable_sig>>short_sig>>last){
    threshold_vec.push_back(thr);
    enable_vec.push_back(enable_sig);
    short_vec.push_back(short_sig);
    if (enable_sig > max) {max = enable_sig;}
    if (short_sig > max) {max = short_sig;}
  }
}


void S_Curve_analysis (int nFiles=15){

// variable used
TString filename_injection, filename_measurement;
TString path_injection;
TString path_measurement;
ifstream input_file, input_file2;
int nASIC;
int npC;
int nRUN;
/*cout << "Tapez le chemin vers le fichier data - injection de charge"<<endl;
cin >> path_injection;
cout << "Tapez le chemin vers le fichier data - mesure sur faisceau"<<endl;
cin >> path_measurement;*/
cout << "Entrez le numero ASIC"<<endl;
cin >> nASIC;
cout << "Entrez le numero du RUN : format 00XXX"<<endl;
cin >> nRUN;
cout << "Entrez la charge injectee etudiee"<<endl;
cin >> npC;

path_injection=Form("/media/oreste/DATA/S_CURVE/%ipC/",npC);
path_measurement=Form("/media/oreste/DATA/S_CURVE/NICE/RUN%05i/",nRUN);
//path_injection=Form("/Users/fontana/Dropbox/ASIC_Characterization_Hodoscope/RUN_S_CURVE/%ipC/",npC);
//path_measurement=Form("/Users/fontana/Dropbox/ASIC_Characterization_Hodoscope/NICE/RUN%05i/",nRUN);

int channels=32;	// 32 channels per ASIC
int tot_arrays=nFiles*channels;
TCanvas *c1[tot_arrays];
TPad *enable_pad[tot_arrays];
TPad *short_pad[tot_arrays];
TGraph *enable_injection[tot_arrays];
TGraph *short_injection[tot_arrays];
TGraph *enable_measurement[tot_arrays];
TGraph *short_measurement[tot_arrays];
TLegend *leg_enable[tot_arrays];
TLegend *leg_short[tot_arrays];
TH1F *frame_enable[tot_arrays];
TH1F *frame_short[tot_arrays];

TFile *analysis_output = new TFile(path_measurement+Form("analysis_ASIC%i_%ipC.root",nASIC,npC),"RECREATE");
int nChannel;
double enable_sig=0., short_sig=0., thr=0., first=0., last=0.;
int Max_abscisse, Max_ordonnee; 			//abscissa & ordinate delimitation


float gain[nFiles];
  for(int s=0; s<nFiles; s++){
  gain[s]=0.25+0.25*s;
  }

vector <int> enable_vec_injected, short_vec_injected, threshold_vec;			//built vectors from S_Curve obtained by current injection
vector <int> meas_enable_vec, meas_short_vec, meas_threshold_vec;			//built vectors from S_Curve obtained by proton beam measurment




  for (int nChannel=1; nChannel<=channels; nChannel++) {
	int i=0,j=0;			//SetPoint iterator
    for (int l=0; l<nFiles; l++) {
		enable_vec_injected.clear();
		short_vec_injected.clear();
		threshold_vec.clear();
		meas_enable_vec.clear();
		meas_short_vec.clear();
		meas_threshold_vec.clear();
    // Canvas building
    c1[(nChannel-1)*nFiles+l]= new TCanvas (Form("Canvas_analysis_ch%i_gain%i",nChannel,l), Form("Canvas_analysis_ch%i_gain%i",nChannel,l),1200,1000);
    c1[(nChannel-1)*nFiles+l]->SetTitle(Form("Analyse ASIC%i Hodoscope ch %02i gain %f - overlap injection/measurement", nASIC, nChannel, gain[l]));
    enable_pad[(nChannel-1)*nFiles+l]= new TPad(Form("Pad enable ch%i gain%f",nChannel,gain[l]), Form("Pad enable ch%i gain%f",nChannel,gain[l]), 0.10,0.505,0.90,0.995, 0);
    short_pad[(nChannel-1)*nFiles+l]= new TPad(Form("Pad short ch%i gain%f",nChannel,gain[l]), Form("Pad short ch%i gain%f",nChannel,gain[l]), 0.10,0.005,0.90,0.50, 0);
    enable_pad[(nChannel-1)*nFiles+l] -> Draw();
    short_pad[(nChannel-1)*nFiles+l] -> Draw();
		Max_abscisse=0, Max_ordonnee=0;
    //Graphical settings
    enable_injection[(nChannel-1)*nFiles +l] = new TGraph();
    enable_injection[(nChannel-1)*nFiles +l]->SetMarkerSize(1);
    enable_injection[(nChannel-1)*nFiles +l]->SetMarkerStyle(21);
    enable_injection[(nChannel-1)*nFiles +l]->SetMarkerColor(kBlack);
    enable_injection[(nChannel-1)*nFiles +l]->SetLineColor(kBlack);


    short_injection[(nChannel-1)*nFiles +l] = new TGraph();
    short_injection[(nChannel-1)*nFiles +l]->SetMarkerSize(1);
    short_injection[(nChannel-1)*nFiles +l]->SetMarkerStyle(21);
    short_injection[(nChannel-1)*nFiles +l]->SetMarkerColor(kBlack);
    short_injection[(nChannel-1)*nFiles +l]->SetLineColor(kBlack);


    enable_measurement[(nChannel-1)*nFiles +l] = new TGraph();
    enable_measurement[(nChannel-1)*nFiles +l]->SetMarkerSize(1);
    enable_measurement[(nChannel-1)*nFiles +l]->SetMarkerStyle(8);
    enable_measurement[(nChannel-1)*nFiles +l]->SetMarkerColor(kRed);
    enable_measurement[(nChannel-1)*nFiles +l]->SetLineColor(kRed);


    short_measurement[(nChannel-1)*nFiles +l] = new TGraph();
    short_measurement[(nChannel-1)*nFiles +l]->SetMarkerSize(1);
    short_measurement[(nChannel-1)*nFiles +l]->SetMarkerStyle(8);
    short_measurement[(nChannel-1)*nFiles +l]->SetMarkerColor(kRed);
    short_measurement[(nChannel-1)*nFiles +l]->SetLineColor(kRed);




		// Files reading
    filename_injection = Form(path_injection + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
    input_file.open(filename_injection);
			if (input_file.is_open()) {
			txt_to_vec_max(input_file, enable_vec_injected, short_vec_injected, threshold_vec, Max_ordonnee);
				for(i = 0; i<enable_vec_injected.size(); i++){
        enable_injection[(nChannel-1)*nFiles +l]->SetPoint(i, threshold_vec.at(i), enable_vec_injected.at(i));
		    short_injection[(nChannel-1)*nFiles +l]->SetPoint(i, threshold_vec.at(i), short_vec_injected.at(i));
			}

			Max_abscisse = *max_element(threshold_vec.begin(), threshold_vec.end());

			input_file.close();
			}
			else {
			cout << "File not found : check file name"<<endl;
				if (c1[(nChannel-1)*nFiles +l]){c1[(nChannel-1)*nFiles +l]->Close();gSystem->ProcessEvents();delete c1[(nChannel-1)*nFiles +l];c1[(nChannel-1)*nFiles +l]=0;}
			continue;
				}

			filename_measurement = Form(path_measurement + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
				input_file2.open(filename_measurement);
			if (input_file2.is_open()) {
          txt_to_vec_max(input_file2, meas_enable_vec, meas_short_vec, meas_threshold_vec, Max_ordonnee);
          for(j = 0; j<meas_enable_vec.size(); j++){
          enable_measurement[(nChannel-1)*nFiles +l]->SetPoint(j, meas_threshold_vec.at(j), meas_enable_vec.at(j));
		      short_measurement[(nChannel-1)*nFiles +l]->SetPoint(j, meas_threshold_vec.at(j), meas_short_vec.at(j));
					}

					// Pads filling
			leg_enable[(nChannel-1)*nFiles + l] = new TLegend (.7,.4,.9,.6);
			leg_short[(nChannel-1)*nFiles + l] = new TLegend (.7,.4,.9,.6);
			leg_enable[(nChannel-1)*nFiles + l]->AddEntry(enable_injection[(nChannel-1)*nFiles + l],"Enable - injection de charge", "pl");
			leg_short[(nChannel-1)*nFiles + l]->AddEntry(short_injection[(nChannel-1)*nFiles + l],"Short pulse - injection de charge", "pl");
			leg_enable[(nChannel-1)*nFiles + l]->AddEntry(enable_measurement[(nChannel-1)*nFiles + l],"Enable - mesure faisceau", "pl");
			leg_short[(nChannel-1)*nFiles + l]->AddEntry(short_measurement[(nChannel-1)*nFiles + l],"Short pulse - mesure faisceau", "pl");

			enable_pad[(nChannel-1)*nFiles+l]	-> cd();
			frame_enable[(nChannel-1)*nFiles + l] = enable_pad[(nChannel-1)*nFiles + l]->DrawFrame(0,0,Max_abscisse+200,750/*Max_ordonnee+0.1*Max_ordonnee*/);
			frame_enable[(nChannel-1)*nFiles + l]->GetXaxis()->SetTitle("Threshold");
			frame_enable[(nChannel-1)*nFiles + l]->GetYaxis()->SetTitle("Entries");
			enable_measurement[(nChannel-1)*nFiles + l]->Draw("PL");
			enable_injection[(nChannel-1)*nFiles + l]->Draw("PL same"); //Plot Line
			leg_enable[(nChannel-1)*nFiles + l] -> Draw();


			short_pad[(nChannel-1)*nFiles+l]	-> cd();
			frame_short[(nChannel-1)*nFiles + l] = short_pad[(nChannel-1)*nFiles + l]->DrawFrame(0,0,Max_abscisse+200,750/*Max_ordonnee+0.1*Max_ordonnee*/);
			frame_short[(nChannel-1)*nFiles + l]->GetXaxis()->SetTitle("Threshold");
			frame_short[(nChannel-1)*nFiles + l]->GetYaxis()->SetTitle("Entries");
			short_measurement[(nChannel-1)*nFiles + l]->Draw("PL");
			short_injection[(nChannel-1)*nFiles + l]->Draw("PL same"); //Plot Line
			leg_short[(nChannel-1)*nFiles + l] -> Draw();


			c1[(nChannel-1)*nFiles + l]->cd();
			enable_pad[(nChannel-1)*nFiles + l]->cd();
			enable_pad[(nChannel-1)*nFiles + l]->Modified();
			enable_pad[(nChannel-1)*nFiles + l]->Update();
			short_pad[(nChannel-1)*nFiles + l]->cd();
			short_pad[(nChannel-1)*nFiles + l]->Modified();
			short_pad[(nChannel-1)*nFiles + l]->Update();

			input_file2.close();
			}
			else {
			cout << "File not found : check file name"<<endl;
				if (c1[(nChannel-1)*nFiles +l]){c1[(nChannel-1)*nFiles +l]->Close();gSystem->ProcessEvents();delete c1[(nChannel-1)*nFiles +l];c1[(nChannel-1)*nFiles +l]=0;}
			continue;
				}



		analysis_output -> cd();
		c1[(nChannel-1)*nFiles + l]->Write(Form("Canvas_analysis_channel%i_gain%.02f_-_overlap_injection/measurement", nChannel, gain[l]));
		enable_injection[(nChannel-1)*nFiles + l]->Write(Form("en_inj_graph_ch%i_gain%.02f",nChannel, gain[l]));
		short_injection[(nChannel-1)*nFiles + l]->Write(Form("short_inj_graph_ch%i_gain%.02f",nChannel, gain[l]));
		enable_measurement[(nChannel-1)*nFiles + l]->Write(Form("en_meas_graph_ch%i_gain%.02f",nChannel, gain[l]));
		short_measurement[(nChannel-1)*nFiles + l]->Write(Form("short_meas_graph_ch%i_gain%.02f",nChannel, gain[l]));
			if (c1[(nChannel-1)*nFiles +l]){c1[(nChannel-1)*nFiles +l]->Close();gSystem->ProcessEvents();delete c1[(nChannel-1)*nFiles +l];c1[(nChannel-1)*nFiles +l]=0;}
		analysis_output -> Close();
		}
	}
return;
}

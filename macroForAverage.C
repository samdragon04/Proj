
/*
basic macro to read tpcdigits and store read info in easily readable TTree
I store the q in a TVector, you could alternatively use TGraphs or TH1
it takes way too long to run for all sectors, you can consider splitting into 36 jobs, one for each sector (submitting the jobs in some farm)
you can draw the output file like this:

TFile * f = new TFile("...root")
TTree * tree = (TTree*) f->Get("signals");

 */
#if !defined(__CLING__) || defined(__ROOTCLING__)
#include "TChain.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH1.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include "TString.h"
#include "TFile.h"
#include "TVector.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "CommonUtils/TreeStreamRedirector.h"
#include "TPCBase/Sector.h"
#include "TPCBase/Mapper.h"
#include "DataFormatsTPC/Digit.h"
#include "TPCSimulation/CommonMode.h"
#include "TPCSimulation/DigitTime.h"
//#include "TPCSimulation/SAMPAProcessing.h"

#endif

using namespace o2::tpc;

//======================================================================================================================================================//
// some hardcoded values
// you could get these via the tpc Mapper, but I never looked up how to do this :)
const Int_t nRows      = 152;      //number of rows in one TPC iSector
const Int_t nPadsTotal = 14560;    //total number of pads in one iSector
const Int_t nSectors   = 36;       // how many sectors to analyze, set to 36 for entire TPC
const Int_t nCrus      = 10; 	   // number of CRUs in one sector
int nPadsPerCRU[10]    = {0};     // how many pads per CRU, will retrieved by data
int padsPerRow[nRows]  = {66,66,66,68,68,68,70,70,70,72,72,72,74,74,74,74,76,76,76,76,78,78,78,80,80,80,82,82,82,84,84,84,86,86,86,88,88,88,90,90,90,90,92,
92,92,94,94,94,92,92,92,94,94,94,96,96,96,98,98,98,100,100,100,76,76,76,76,78,78,78,80,80,80,80,82,82,82,84,84,84,84,86,86,86,88,88,88,90,90,
90,90,92,92,92,94,94,94,94,96,96,96,98,98,98,100,100,102,102,102,104,104,104,106,110,110,112,112,112,114,114,114,116,116,116,118,118,118,118,
118,120,120,122,122,124,124,124,126,126,128,128,128,130,130,132,132,132,134,134,136,136,138,138};
//======================================================================================================================================================//
const int nTimeBinsTotal = 1500; // total number of timebins for each pad
const float qThresh = 2.565; // charge threshold for occupancy estimation
int padLIDArray[nPadsTotal];
double qArray[nPadsTotal][nTimeBinsTotal];  // array holding the charge for all pads of one iSector
double cmArray[nTimeBinsTotal];
TTree *tree;
o2::utils::TreeStreamRedirector *pcstream;
const Mapper& mMapper    = Mapper::instance(); // to get coordinates
int getPadLID(int row, int pad); // function to give the local pad ID (unique pad number for one iSector)

void macroForAverage(){
	TFile *f = new TFile("tpcdigits.root");
	tree = (TTree*)f->Get("o2sim");
	pcstream = new o2::utils::TreeStreamRedirector("data/average.root","recreate");
	tree->SetCacheSize(100000000);
	vector<o2::tpc::Digit> *digitVector=0;
	vector<o2::tpc::CommonMode> *commonmodeVector=0;
	int nEvents = tree->GetEntries(); // number of events
	//======================================================================================================================================================//

	for (int iSector = 0; iSector < nSectors; iSector++){
		cout <<"Analyzing iSector " << iSector << endl;
		tree->SetBranchAddress(Form("TPCDigit_%d",iSector), &digitVector);
		tree->SetBranchAddress(Form("TPCCommonMode_%d",iSector), &commonmodeVector);
		for (int iEvent = 0; iEvent < nEvents; iEvent++){ // in your case only 1 event
			cout <<"Analyzing event " << iEvent << endl;
			tree->GetBranch(Form("TPCDigit_%d", iSector))->GetEntry(iEvent);
			tree->GetBranch(Form("TPCCommonMode_%d", iSector))->GetEntry(iEvent);
			//======================================================================================================================================================//
			// reset qArray, padLIDArray
			for (int iPad = 0; iPad < nPadsTotal; iPad++){
				padLIDArray[iPad] = -1;
				for (int iTime = 0; iTime < nTimeBinsTotal; iTime++){
					qArray[iPad][iTime] = 0.;
				}
			}
			
			// necessary arrays, restore after each event
			int rowArray[nPadsTotal]    = {0}; 					// row for every padLID
			int padArray[nPadsTotal]	= {0}; 					// pad for every padLID
			int cruArray[nPadsTotal]	= {0}; 					// cru for every padLID

		  //======================================================================================================================================================//
		  //
		  
		  for (unsigned long iEntry = 0; iEntry < (*digitVector).size(); iEntry++) {
		   	Int_t row 			= (*digitVector)[iEntry].getRow();
		   	Int_t pad 			= (*digitVector)[iEntry].getPad();
		   	Int_t cru 			= (*digitVector)[iEntry].getCRU();
		   	Int_t timeStamp     = (*digitVector)[iEntry].getTimeStamp();
		   	Float_t q  		    = (*digitVector)[iEntry].getChargeFloat();
		   	Int_t padLID 		= getPadLID(row,pad);
		   	if (padLIDArray[padLID]==-1) nPadsPerCRU[cru]++; // if it is the first time we loop over that specific pad, increment nPadsPerCRU
		   	padLIDArray[padLID] = padLID;
		   	//if (qArray[padLID][timeStamp]!=0) cout << "writing problem" << endl; // it means tpcdigits contains 2 times the same pad-time
		   	qArray[padLID][timeStamp] = q;
	  		rowArray[padLID] = row;
	  		padArray[padLID] = pad;
	  		cruArray[padLID] = cru;
	  	} //for (unsigned long iEntry = 0; iEntry < (*digitVector).size(); iEntry++) {
	  	//======================================================================================================================================================//
		


		Float_t cmArray[4][nTimeBinsTotal];

		  for (unsigned long iEntry = 0; iEntry < (*commonmodeVector).size(); iEntry++){
			cmArray[ (int)(*commonmodeVector)[iEntry].getGEMstack()][ (*commonmodeVector)[iEntry].getTimeBin()]= (*commonmodeVector)[iEntry].getCommonMode();
		  	 }//for (unsigned long iEntry = 0; iEntry < (*commonmodeVector).size(); iEntry++){  

		// info per CRU
		
	  	for (int iCru = 0; iCru < nCrus; iCru++){
			cout << "CRU " << iCru <<endl;
			
			int gemStack = (iCru<4)*0+(iCru>=4&&iCru<6)*1+(iCru>=6&&iCru<8)*2+(iCru>=9)*3;
	  		int gCru = iSector*4 + iCru; // unique cru identifier
	  		for (int iTime = 0; iTime < nTimeBinsTotal; iTime++){
				//float cmValue = cmArray[gemStack][iTime];
				TVectorD cmVector(nTimeBinsTotal);
				cmVector[iTime]	= cmArray[gemStack][iTime];
	  			float occupancy = 0; // calculate occupancy per timebin
  				TVectorD qVector(nPadsPerCRU[iCru]);
  				TVectorD padVector(nPadsPerCRU[iCru]);
  				TVectorD rowVector(nPadsPerCRU[iCru]);
				TVectorD qVector2(nPadsPerCRU[iCru]);
				vector<float> avg;
				vector<float> adc_filter;
				float mean = 0;
				float sum = 0;
				float err = 0;
				int cnt = 1;
				int iCounter = 0;
  				for (int iPad = 0; iPad < nPadsTotal; iPad++){
  					if (cruArray[iPad] == iCru){
						qVector[iCounter] = qArray[iPad][iTime];
						//cout << "qarray " << qVector[iCounter] << endl;  
						if (qVector[iCounter] > qThresh) occupancy++;
  						padVector[iCounter] = padArray[iPad];
  						rowVector[iCounter] = rowArray[iPad];
						iCounter++;
  					}
					 // cout << "iCru = " << iCru <<" " << "iTime = " << iTime <<endl;
					  //cout << "qvec1 = " << qArray[iPad][iTime] << "cm = " << cmVector[iTime] <<endl; 

		//=========================avg filter=================================================================================================================//
					   
					  //qVector2[cnt] = qArray[iPad][iTime];
        	          //if (abs(qVector2[cnt] - qVector2[cnt - 1]) < qThresh) {
						  
						  if((qArray[iPad][iTime] - qArray[iPad][iTime - 1]) < qThresh){
							  if(qArray[iPad][iTime] == 0 && qArray[iPad][iTime - 1] == 0){
							  break;
						  }
						  else;
						    //cout << " cnt = " << cnt <<endl;
							sum += qArray[iPad][iTime];
						}
							mean = sum/cnt;
							err = qArray[iPad][iTime] - mean;
							//cout << "sum = " << sum <<endl;
							//cout << "mean =" << mean <<endl;
							avg.push_back(mean);
							adc_filter.push_back(err);							
					  		cnt++;					   				    
				  }			
				  	
  				occupancy/=nPadsPerCRU[iCru]; // normalization
								
							
  				// here call CM function. input will be only the qVector (charge of all pads of cru for one timebin)
  				// cmFromFilter = commonModeFilter(qVector); //output is cm for given tb
                  
				
  			 	

	  			(*pcstream)         <<    "signals"  	<<
	  			"sector="	        <<    iSector    	<<
	  			"cru="	            <<    iCru 		    <<
			//"gcru="	            <<    gCru 		    <<
	  			"time="	            <<    iTime 		<<
	  			"occupancy="        <<    occupancy 	<<
	  			"q.="	            <<    qVector 	    <<
			//"pad.="	            <<    padVector 	<<
			//"row.="	            <<    rowVector 	<<
	  			"mean="      		<<    avg		    <<
				"cm="   		    <<  cmVector[iTime] <<
	  			"err="              <<    adc_filter    <<

	  			"\n";
	  		}
		 } //for (int iCru = 0; iCru < nCrus; iCru++){
		} //for (int iEvent = 0; iEvent < nEvents; iEvent++){
	} //for (int iSector = 0; iSector < nSectors; iSector++){
	delete pcstream;
}

int getPadLID(int row, int pad){
  int iRow = 0; int iPad = 0;
  int padLID = 0;
	while (!(iRow==row && iPad==pad)){
		if (iPad <=padsPerRow[iRow]-1) iPad++;
		else {iRow++; iPad=0; padLID--;}
		padLID++;
	}
	return padLID;
}

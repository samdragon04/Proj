#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <vector>
#include <algorithm>
#include <fstream>
#include <bitset>
#include <ctime>
#include <cstdlib>
#include "gsl/span"

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TString.h"

#include "TPCBase/Sector.h"
#include "TPCBase/Mapper.h"
#include "DataFormatsTPC/Digit.h"
#include "TPCSimulation/CommonMode.h"
#include "DataFormatsTPC/Defs.h"

#endif

//=================================================================================
void readDigitsAndCommonModetest(std::string_view fileName = "tpcdigits.root")
{
  using namespace o2::tpc;

  // ===| open file and get tree |==============================================
  std::unique_ptr<TFile> o2simDigits(TFile::Open(fileName.data()));
  auto treeSim = (TTree*)o2simDigits->Get("o2sim");

  gROOT->cd();

  // ===| set up branch addresses |=============================================
  std::vector<Digit>* vecDigitsPerSector[Sector::MAXSECTOR]; // container that keeps Digits per sector
  std::vector<CommonMode>* vecCommonModePerSector[Sector::MAXSECTOR]; // container that keeps common mode per sector

  for (int iSec=0; iSec<Sector::MAXSECTOR; ++iSec) {
    vecDigitsPerSector[iSec] = nullptr;
    treeSim->SetBranchAddress(TString::Format("TPCDigit_%d", iSec), &vecDigitsPerSector[iSec]);

    vecCommonModePerSector[iSec] = nullptr;
    treeSim->SetBranchAddress(TString::Format("TPCCommonMode_%d", iSec), &vecCommonModePerSector[iSec]);
  }
  // ===| sorted data formats |=================================================

  // sort the common mode as one value per GEM stack per time bin
  // this will make it simpler to access it

  std::vector<std::array<float, GEMSTACKSPERSECTOR * Sector::MAXSECTOR>> vecCommonModePerTimeBin(1000);
	int cnt = 0;
	std::string inputfile = "data/InputData.txt";
	std::string simresultfile = "data/O2simResult.txt";	
	
	std::ofstream ofsm(inputfile);
	std::ofstream ofss(simresultfile);
	if(!(ofsm && ofss)){
		std::cout << "Can't open file" << std::endl;
		return;
	}
	
	ofss << "  time cru sec Row      CM Pad  ANALOG ADC(signal+random noise) " << endl;

  // ===| event loop |==========================================================
  for (Long64_t ievent = 0; ievent < treeSim->GetEntries(); ++ievent) {
      treeSim->GetEntry(ievent);

    // sort common mode data to simpler structure
    for (int iSec = 0; iSec < Sector::MAXSECTOR; ++iSec) {
      //printf("sec %d\n", iSec);
      const auto& vecCM = *vecCommonModePerSector[iSec];
      //printf("cmptr: %p, size: %lu\n", &vecCM, vecCM.size());

      // loop over all common mode signal in the present sector
      // the common mode is stored as absolute value, so it has to be subtracted from
      //   the ADC signal
      for (const auto& cmSignal : vecCM) {
        const int gemStackInTPC = cmSignal.getGEMstack() + iSec * GEMSTACKSPERSECTOR;
        const size_t timeBin = size_t(cmSignal.getTimeBin());
        
        if (vecCommonModePerTimeBin.size() <= size_t(timeBin)) {
          vecCommonModePerTimeBin.resize(timeBin+100);
        }
        auto& arr = vecCommonModePerTimeBin[timeBin];
        arr[gemStackInTPC] = cmSignal.getCommonMode();
      }
    }

    // loop over digits
    // the digits are ordered increasing in time
    for (int iSec = 0; iSec < Sector::MAXSECTOR; ++iSec) {
      auto& vecDigits = *vecDigitsPerSector[iSec];
      
      // loop over all digits in the present sector
      for (const auto& digit : vecDigits) {
        const CRU cru(digit.getCRU());
        const int gemStackInTPC = cru.gemStack() + iSec * GEMSTACKSPERSECTOR;
        const float cm = vecCommonModePerTimeBin[digit.getTimeStamp()][gemStackInTPC];

        
		// (cru % 10 ← Number of CRU left after mod 10)
		  if(cru % 10 >= 0){       //Entire data
		
			ofss <<std::setw(6) << digit.getTimeStamp() << setw(4) << cru << std::setw(4) << iSec  << std::setw(4) << digit.getRow()  
				<< std::fixed << std::setprecision(3) << std::setw(8) << cm
					 << std::setw(4) << digit.getPad() << std::fixed << std::setprecision(2) << std::setw(8) << digit.getChargeFloat()   
						<< std::setw(4) << int(digit.getChargeFloat() - cm) << std::endl;

			//To decrease file size using hexa
			if(digit.getChargeFloat() < 0){
				ofsm << std::setw(3) << std::setfill('0') << std::hex << 0 << " ";
				//ofsp << std::setw(3) << std::setfill('0') << std::hex << int(digit.getChargeFloat()) << " ";
			}
			else{
				ofsm << std::setw(3) << std::setfill('0') << std::hex << int(digit.getChargeFloat()) << " ";
				//ofsp << std::setw(3) << std::setfill('0') << std::hex << int(digit.getChargeFloat() << " ";
			}
      if(++cnt == 40){
				ofsm << endl;
				cnt = 0;
			}
		}
	}
	std::cout << "sector:" << iSec << std::endl;
//return;			//end at sector:0
    }
  }
}



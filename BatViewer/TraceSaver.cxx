#include "TraceSaver.h"
#include "PulseTools.h"
#include "AdminData.h"
#include "PulseData.h"

#include "TFile.h"
#include "TDirectory.h"
#include "TMultiGraph.h"
#include "TGraph.h"
#include "TList.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

TraceSaver::TraceSaver(const string& filename) : 
  fFilename(filename), fOutFile(0)
{}

TraceSaver::~TraceSaver()
{
  if(fOutFile){
    fOutFile->Close();
    delete fOutFile;
  }
}

int TraceSaver::SavePulse(const PulseData* pulse, AdminData* admin)
{
   //make sure the file is open
  if(!fOutFile){
    //check for '.root' suffix at end of file name
    if(fFilename.substr(fFilename.length()-5) != ".root")
      fFilename.append(".root");
    fOutFile = new TFile(fFilename.c_str(), "UPDATE");
    if(!fOutFile || !fOutFile->IsOpen())
      return -1;
  }
  ////create the directory structure with series/event
  //create the series directory
  stringstream seriesstr;
  seriesstr<< "0" << admin->GetSeries()/10000 << "_" 
	   << std::setw(4)<<std::setfill('0')<<admin->GetSeries()%10000;
  TDirectory* seriesdir = fOutFile->GetDirectory(seriesstr.str().c_str());
  if(!seriesdir){
    seriesdir = fOutFile->mkdir(seriesstr.str().c_str());
    if(!seriesdir){
      cerr<<"ERROR TraceSaver::SavePulse: Unable to create series directory\n";
      return -2;
    }
  }
  //create the event directory
  TDirectory* eventdir = seriesdir->GetDirectory(Form("%u",admin->GetEvent()));
  if(!eventdir){
    eventdir = seriesdir->mkdir(Form("%u",admin->GetEvent()));
    if(!eventdir){
      cerr<<"ERROR TraceSaver::SavePulse: Unable to create event directory\n";
      return -3;
    }
  }
  //save the pulse as a TGraph
  eventdir->cd();
  TGraph* g = PulseTools::Vector2TGraph(pulse->GetRawPulse(),
					pulse->GetSampleDt()*0.001,
					pulse->GetTriggerT0()*0.001);
  g->SetName(Form("Detector%d_%s", pulse->GetDetectorNum(), 
		  pulse->GetChannelName().c_str()) );
  g->SetTitle(g->GetName());
  g->Write("", TObject::kOverwrite);
  delete g;
  
  return 1;
}



int TraceSaver::SaveDetector(const vector<PulseData>& pulses, 
			     AdminData* admin)
{
  for(size_t i=0; i<pulses.size(); ++i){
    int saved = SavePulse( &pulses[i], admin);
    if(saved != 1)
      return saved;
  }
  cout<<"Saved "<<pulses.size()<<" traces for detector "
      <<pulses[0].GetDetectorNum()<<" to file "<<fFilename<<endl;
  return (int)pulses.size();
}
  
int TraceSaver::SaveDetectorMap(const map<int, vector<PulseData> >& pulsemap,
				AdminData* admin)
{
  int totalsaved = 0;
  map<int, vector<PulseData> >::const_iterator mapit;
  for(mapit = pulsemap.begin(); mapit != pulsemap.end(); ++mapit){
    int saved = SaveDetector( mapit->second, admin);
    if(saved != (int)mapit->second.size())
      return saved;
    totalsaved += saved;
  }
  cout<<"Saved "<<pulsemap.size()<<" detectors to file "<<fFilename<<endl;
  return totalsaved;
}



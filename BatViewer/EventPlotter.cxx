// CDMS headers
#include "EventPlotter.h"
#include "AdminData.h"
#include "PadZoomer.h"
#include "RootGraphix.h"
#include "PulseTools.h"

//ROOT headers
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TH1F.h"
#include "TROOT.h"
#include "TLegend.h"
#include "TColor.h"
#include "TStyle.h"
#include "TPaveLabel.h"
#include "TPaveText.h"

//Stdlib headers
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

EventPlotter::EventPlotter(bool hardcodeplots) : 
  _hardcodeplots(hardcodeplots), fCanvas(0)
{

}

EventPlotter::~EventPlotter()
{
  delete fCanvas;
}

TCanvas* EventPlotter::GetCanvas(double scale)
{
  if(!fCanvas && scale>0)
    fCanvas = RootGraphix::GetCanvas(scale);
  return fCanvas;
}

TLegend* EventPlotter::AddLegend(TPad* pad, bool draw)
{
  //look for a multigraph to determine how many channels are in the pad
  int nchans = 4;
  TList* prims = pad->GetListOfPrimitives();
  for(int i=0; i<prims->GetEntries(); ++i){
    if(!strcmp(prims->At(i)->ClassName(),"TMultiGraph")){
      TMultiGraph* mg = (TMultiGraph*)(prims->At(i));
      nchans = mg->GetListOfGraphs()->GetSize();
      break;
    }
  }
  //TLegend* legend = pad->BuildLegend(0.13,0.6,0.29,0.9);
  TLegend* legend = pad->BuildLegend(0.93,0.9-0.1*nchans,1,0.9);
  legend->SetName("legend");
  //legend->SetNColumns(2);
  //legend->SetFillStyle(1001);
  //legend->SetBorderSize(1);
  legend->SetEntrySeparation(0);
  //legend->SetMargin(0.1);
  TLegend* l2 = legend;
  if(!draw){
    l2 = (TLegend*)(legend->Clone());
    delete legend;
  }
  return l2;
}

const int fillcolor = kBlue-10;

void DrawVerticalLine(double x, int color=kGray+2, int width=2, int style=3)
{
  TGraph* g = new TGraph(2);
  g->SetBit(TObject::kCanDelete);
  g->SetEditable(false);
  g->SetLineColor(color);
  g->SetLineWidth(width);
  g->SetLineStyle(style);
  g->SetPoint(0,x,-1.e300);
  g->SetPoint(1,x,1.e300);
  g->Draw("l");
}

std::string GetEventTypeString(uint32_t EventType)
{
  switch(EventType){
  case 0: return "Bg";
  case 3: return "Cf";
  case 4: return "Rand";
  case 5: return "Pulser";
  case 6: return "Test";
  case 7: return "Mon";
  case 9: return "Ba";
  case 10: return "Veto";
  case 11: return "DAQ";
  case 1: return "Co";
  case 2: return "Co LowR";
  }
  
  return Form("%d",EventType);

}

std::string GetEventCategoryString(uint32_t EventCategory)
{
  switch(EventCategory){
  case 0: return "Full Zip";
  case 1: return "Random";
  case 6: return "Selective Zip";
  }
  
  return Form("%d",EventCategory);

}

int EventPlotter::DrawOneEvent(const map<int,vector<PulseData> >& zips,
			       const vector<PulseData>& veto,
			       const vector<PulseData>& noisemons,
			       uint32_t EventType,
			       uint32_t EventCategory,
			       AdminData* admin,
			       bool interactive,
			       const std::string& trigtxt)
{
//declare some useful colors for drawing multiple plots
  //const int pcolors[] = {kBlack, kRed, kGreen, kCyan, kBlue, kMagenta, kYellow,
  ///		kGray+2, kOrange-3, kGreen+3, kCyan+3, kMagenta-5, 
  //		kRed-2};
  //const int pcolors[] = {kBlack, kRed+2, kGreen+2, kBlue+2,
  //		 kGray+2, kRed, kGreen, kBlue };
  
  const int hardcode_nzips=15;
  
  double zipxmax = 0.9;
  double zipymax = 0.87;
  double titlexmax = 0.7;
  //make a new canvas if we don't have one already
  double scale = 0.6;
  if(!interactive)
    scale = 1;
  GetCanvas(scale);

  RootGraphix::Lock glock = RootGraphix::AcquireLock();
  fCanvas->Clear();
  //determine how many pads we need
  int nplots = zips.size();
  if(!veto.empty()) nplots+=1;
  if(!noisemons.empty()) nplots+=1;
  fCanvas->SetFillColor(kWhite); // subpads inherit color
  std::vector<TPad*> pads;
  TPad* titlepad=0;
  if(_hardcodeplots){
    fCanvas->cd(0);
    for(int padn=0; padn<hardcode_nzips; ++padn){
      fCanvas->cd(0);
      double yhigh = zipymax-zipymax/3.*(padn%3);
      double ylow = yhigh - zipymax/3.;
      double xlow = zipxmax/5. * (padn/3);
      double xhigh = xlow + zipxmax/5.;
      char padname[20];
      sprintf(padname,"zpad%d",padn);
      TPad* p = new TPad(padname,"Zip pad",xlow,ylow,xhigh,yhigh);
      pads.push_back(p);
      p->Draw();
    }
    if(!veto.empty()){
      fCanvas->cd(0);
      TPad* p = new TPad("vetopad","Veto Pad",zipxmax,0,0.995,
			 noisemons.empty() ? zipymax : zipymax*0.66);
      pads.push_back(p);
      p->Draw();
    }
    if(!noisemons.empty()){
      fCanvas->cd(0);
      TPad* p = new TPad("noisepad","Noise Monitor Pad",zipxmax,
			 veto.empty() ? 0 : zipymax*0.66 ,0.995, zipymax);
      pads.push_back(p);
      p->Draw();
    }
    
    //Draw the title box
    //TPad* titlepad = 0;
    if(admin){
      zipymax *= 1.005;
      titlepad = new TPad("titlepad","Title pad",0.005,zipymax,0.995,0.995);
      titlepad->SetFillColor(kWhite);
      titlepad->SetBorderSize(0);
      titlepad->SetBorderMode(0);
      fCanvas->cd(0);
      titlepad->Draw();
      titlepad->cd();
      std::stringstream title;
      title<<"Series "<<admin->GetSeries()<<" Event "
	   <<admin->GetEvent();
      TPaveLabel* pl = new TPaveLabel(0,0.5,titlexmax/2.,1,
				      title.str().c_str());
      pl->SetBit(TObject::kCanDelete,true);
      pl->SetFillColor(kWhite);
      pl->SetFillStyle(1);
      pl->SetBorderSize(0);
      //pl->SetLineWidth(2);
      pl->SetTextColor(kBlue);
      pl->SetLineColor(kWhite);
      pl->SetTextSize(0.45);
      pl->Draw();
      
      TPaveText* pt = new TPaveText(0,0,titlexmax/2., 0.55);
      pt->SetBit(TObject::kCanDelete,true);
      pt->SetFillColor(kWhite);
      pt->SetFillStyle(1);
      pt->SetLineColor(kWhite);
      pt->SetBorderSize(0);

      time_t evtime = admin->GetEventTime();
      pt->AddText(std::ctime(&evtime));
            
      uint32_t tbetween = admin->GetTimeBetween();
      pt->AddText(Form("Time since last event: %.3f s",1.*tbetween/1000.) );
      
      pt->AddText(Form("EventType: %s  EventCategory: %s",
		       GetEventTypeString(EventType).c_str(), 
		       GetEventCategoryString(EventCategory).c_str()) );
      pt->Draw();
      
      if(trigtxt!=""){
	TPaveText* pt = new TPaveText(titlexmax/2.,0,titlexmax,1);
	pt->SetFillColor(kWhite);
	pt->SetLineColor(kWhite);
	pt->SetFillStyle(1);
	pt->SetBorderSize(0);
	size_t lastnewline = 0;
	int nlines=0;
	for(size_t c = 0 ; c<trigtxt.size(); ++c){
	  if(trigtxt[c]=='\n'){
	    if(nlines++>3)
	      pt->AddText(trigtxt.substr(lastnewline,c-lastnewline-9).c_str());
	    lastnewline = c;
	  }
	}
	
	pt->Draw();
      }
      /*TLine* l = new TLine(0,zipymax,1,zipymax);
      l->SetBit(TObject::kCanDelete, true);
      l->SetLineWidth(2);
      l->Draw();
      */
    }
  }
  else if(nplots>1){
    RootGraphix::DividePad(fCanvas,nplots,false);
    for(int i=0; i<nplots; ++i)
      pads.push_back((TPad*)(fCanvas->GetPad(i+1)));
  }
  //fCanvas->SetFillColor(kGray+1); // allow to see the different grids
  fCanvas->SetFillColor(fillcolor);
  //loop through the map of zips
  map<int,vector<PulseData> >::const_iterator mapit = zips.begin();
  int pad = 0;
  for( ; mapit != zips.end(); ++mapit){
    int id = mapit->first;
    if(_hardcodeplots && id > hardcode_nzips) 
      continue;
    const vector<PulseData>& zipvec = mapit->second;
    TVirtualPad* subpad;
    if(_hardcodeplots)
      subpad = pads[id-1];
    else
      subpad = pads[pad];
    if(!subpad){
      cerr<<"There was an error creating the drawing pads!"<<endl;
      continue;
    }
    subpad->cd();
    //need to release the graphics mutex to draw the detector
    glock.reset();
    DrawOneDetector(zipvec,id,
		    interactive ? admin : 0, 
		    interactive || !_hardcodeplots || 
		    (pad==0 && _hardcodeplots) );
    glock = RootGraphix::AcquireLock();
    
    if(pad==0 && _hardcodeplots){
      //get the two legends
      subpad->cd(1);
      TLegend* pleg = (TLegend*)(gPad->FindObject("legend"));
      if(!pleg)
	pleg = AddLegend((TPad*)gPad,false);
      subpad->cd(2);
      TLegend* cleg = (TLegend*)(gPad->FindObject("legend"));
      if(!cleg)
	cleg = AddLegend((TPad*)gPad,false);
      fCanvas->cd();
      if(titlepad)
	titlepad->cd();
      if(pleg){
	TLegend* plegclone = (TLegend*)(pleg->Clone());
	plegclone->SetBit(TObject::kCanDelete,true);
	plegclone->SetBorderSize(0);
	plegclone->SetX1(titlexmax);
	plegclone->SetY1(titlepad ? 1./3. : zipymax + 1./3.*(1.-zipymax));
	plegclone->SetX2(1);
	plegclone->SetY2(1);
	plegclone->SetNColumns(4);
	plegclone->Draw();
      }
      if(cleg){
	TLegend* clegclone = (TLegend*)(cleg->Clone());
	clegclone->SetBit(TObject::kCanDelete,true);
	clegclone->SetBorderSize(0);
	clegclone->SetX1(titlexmax);
	clegclone->SetY1(titlepad ? 0 : zipymax);
	clegclone->SetX2(1);
	clegclone->SetY2(titlepad ? 1./3. : zipymax + 1./3.*(1.-zipymax));
	clegclone->SetNColumns(4);
	clegclone->Draw();
      }
      if(!interactive && _hardcodeplots){
	delete pleg;
	delete cleg;
      }
    }
    
    pad++;
    
  } //end loop over zip channels
  if(_hardcodeplots)
    pad = hardcode_nzips;

  if(!veto.empty()){
    std::stringstream title;
    if(admin && interactive)
      title<<"Series "<<admin->GetSeries()<<" Event "<<admin->GetEvent()<<" ";
    title<<"Veto";
    pads[pad]->cd();
    ++pad;
    DrawGenericPulseVec(veto,false,title.str(),false/*no legend*/);
  }
  if(!noisemons.empty()){
    std::stringstream title;
    if(admin && interactive)
      title<<"Series "<<admin->GetSeries()<<" Event "<<admin->GetEvent()<<" ";
    title<<"Noise Monitors";
    pads[pad]->cd();
    ++pad;
    DrawGenericPulseVec(noisemons,true,title.str(),true/*add legend*/,0);
  }
    
  //RootGraphix::Lock glock = RootGraphix::AcquireLock();
  //delete other pads
  TList* l = fCanvas->GetListOfPrimitives();
  for(int i=l->GetSize()-1; i>=0; --i){
    TObject* o = l->At(i);
    TPad* p = dynamic_cast<TPad*>(o);
    if(p && p->GetListOfPrimitives()->GetSize()==0)
      delete p;
  }
  //if(!_hardcodeplots){
  //while(TVirtualPad * extrapad = fCanvas->GetPad(++pad))
  //  delete extrapad;
  //}
  fCanvas->cd(0);
  fCanvas->Modified();
  fCanvas->Update();  
  new PadZoomer(fCanvas);
  return nplots;
}

int EventPlotter::DrawOneDetector(const vector<PulseData>& zipvec,
				  int detector_num,
				  AdminData* admin,
				  bool drawlegend)
{
  const int pcolors[] = {kGreen,   kBlue,   kCyan, kMagenta, 
			 kGreen+2, kBlue+2, kCyan+2, kMagenta+2 };
  const int npcolors = sizeof(pcolors)/sizeof(int); 
  const int ccolors[] = {kRed, kBlack, kRed+2, kGray+2};
  const int nccolors = sizeof(ccolors)/sizeof(int);
  
  //use a multigraph to hold all the pulses and set the axes accordingly
  TMultiGraph* mgphonon = new TMultiGraph;
  mgphonon->SetBit(TObject::kCanDelete,true); //deletes when canvas clears
  std::stringstream title;
  if(admin)
    title<<"Series "<<admin->GetSeries()<<" Event "<<admin->GetEvent()<<" ";
  title<<"Zip "<<detector_num<<" Phonon";
  title<<" ; sample time [#mus] ; ";
  mgphonon->SetTitle(title.str().c_str());
  TMultiGraph* mgcharge = new TMultiGraph;
  mgcharge->SetBit(TObject::kCanDelete,true); //deletes when canvas clears
  title.str("");
  if(admin)
    title<<"Series "<<admin->GetSeries()<<" event "<<admin->GetEvent()<<" ";
  title<<"Zip "<<detector_num<<" Charge";
  title<<" ; sample time [#mus] ; ";
  mgcharge->SetTitle(title.str().c_str());
  //loop over all pulses in the channel
  vector<PulseData>::const_iterator pulse = zipvec.begin();
  int nc=0, np=0;
  for( ; pulse != zipvec.end(); ++pulse){
    TGraph* g = PulseTools::Vector2TGraph(pulse->GetRawPulse(),
					  pulse->GetSampleDt()*0.001,
					  pulse->GetTriggerT0()*0.001);
    g->SetName(pulse->GetChannelName().c_str());
    g->SetTitle(pulse->GetChannelName().c_str());
    g->GetXaxis()->SetTitle("sample time [#mus]");
    int color=pcolors[0];
    if(pulse->IsPhononPulse()){
      mgphonon->Add(g);
      color = pcolors[np%npcolors];
      ++np;
    }
    else if(pulse->IsChargePulse()){
      mgcharge->Add(g);
      color = ccolors[nc%nccolors];
      ++nc;
    }
    else{
      std::cerr<<"ERROR: zip pulse is neither charge nor phonon!\n";
      delete g;
    }
    g->SetLineColor(color);
    g->SetMarkerColor(color);
    g->SetFillColor(color);
    
  }
  if(!gPad)
    GetCanvas(0.6);
  TVirtualPad* subpad = gPad;
  RootGraphix::Lock glock = RootGraphix::AcquireLock();
  gPad->Clear();
  //subpad->SetBorderSize(1);
  subpad->SetFillColor(kWhite);
  subpad->Divide(1,2);
  //subpad->SetFillColor(TColor::GetColor(222,222,222));
  subpad->SetFillColor(fillcolor);
  subpad->SetBorderSize(0);
  subpad->SetBorderMode(0);
  subpad->cd(1);
  //gPad->SetMargin(0.1,0.02,0.05,0.1);
  gPad->SetMargin(0.07,0.07,0.095,0.1);
  gPad->SetPad(0.02,0.50,0.98,0.98);
  mgphonon->Draw("al");
  if(drawlegend)
    AddLegend((TPad*)gPad);
  DrawVerticalLine(0);
  subpad->cd(2);
  //gPad->SetMargin(0.1,0.02,0.05,0.1);
  gPad->SetMargin(0.07,0.07,0.095,0.1);
  gPad->SetPad(0.02,0.02,0.98,0.5);
  mgcharge->Draw("al");
  if(drawlegend)
    AddLegend((TPad*)gPad);
  DrawVerticalLine(0);
  //gPad->Modified();
  subpad->cd(0);
  new PadZoomer(subpad);
  return 0;
}

int EventPlotter::DrawGenericPulseVec(const vector<PulseData>& pulses,
				      bool splitpad,
				      const std::string& title,
				      bool legend_and_color,
				      double offsetincrement)
{
  const int colors[] = {kBlack, kRed, kGreen,   kBlue,   kCyan, kMagenta, 
			kRed+2, kGray, kGreen+2, kBlue+2, kCyan+2,kMagenta+2 };
  const int ncolors = sizeof(colors)/sizeof(int); 
  
  TPad* toppad = (TPad*)gPad;
  if(!toppad)
    toppad = TCanvas::MakeDefCanvas();
  toppad->Clear();
  TMultiGraph* mg=0;
  if(!splitpad){
    mg = new TMultiGraph;
    mg->SetTitle((title+" ; sample time [#mus] ; ").c_str());
    mg->SetBit(TObject::kCanDelete,true); //deletes when canvas clears
  }
  else{
    gPad->Divide(1,pulses.size(),0.01,0.01);
  }
  vector<PulseData>::const_iterator pulse = pulses.begin();
  double offset=0;
  int pad=1;
  for( ; pulse != pulses.end(); ++pulse){
    TGraph* g = PulseTools::Vector2TGraph(pulse->GetRawPulse(),
					  pulse->GetSampleDt()*0.001,//microsec
					  pulse->GetTriggerT0()*0.001);
    if(offset>0){
      for(int samp=0; samp<g->GetN(); ++samp){
	(g->GetY())[samp] += offset;
      }
    }
    offset += offsetincrement;
    if(legend_and_color){
      int color = colors[(pulse-pulses.begin())%ncolors];
      g->SetLineColor(color);
      g->SetMarkerColor(color);
      g->SetFillColor(color);
    }
    stringstream name;
    name<<pulse->GetChannelName()
	<<" "<<pulse->GetDetectorNum();
    g->SetName(name.str().c_str());
    g->SetTitle(name.str().c_str());
    g->GetXaxis()->SetTitle("sample time [#mus]");
    if(splitpad){
      toppad->cd(pad);
      gPad->SetMargin(0.1,0.01,0.095,0.095);
      g->Draw("al");
      DrawVerticalLine(0);
      ++pad;
    }
    else
      mg->Add(g);
  }
  toppad->cd();
  if(!splitpad){
    gPad->SetMargin(0.1,0.01,0.095,0.095);
    mg->Draw("al");
    DrawVerticalLine(0);
  }
  if(!splitpad && legend_and_color)
    ((TPad*)(gPad))->BuildLegend(0.93,0.9-0.1*pulses.size(),1,0.9);
  return (int)pulses.size();
}

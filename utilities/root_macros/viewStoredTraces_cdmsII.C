// This is a simple macro that will display raw traces that have been 
// stored in the filter file after running NoiseGen.  One may opt to 
// store traces in the filter file by setting the processing config 
// flag "WRITE_NOISE_PULSES" to 1.  
//
// Run this script on the resulting noise file with the command: 
// root 'viewStoredTraces_cdmsII.C("mypath/myFilerFile.root", fileNum, detNum, eventNum(optional)'
//
// OR, if you just type: root viewStoredTraces_cdmsII.C
// it will run the script with the defaults specified below
//
// If you specify an event number for skipTo, the script will only show 
// the pulses for that event. If you don't specify an event number for skipTo, 
// it will cycle through all the events (like a movie), pausing for the number of seconds 
// specified by pauseTime.  Choose the max number of events to display with maxEvents.
//
// Note that you must supply the raw data filenumber 
// that NoiseGen was run on.  For example if you run over the first 500 events 
// in a series then fileNum is 1. This script will display traces for a detector 
// with 4 phonon and 2 charge channels, such as a cdmsII zip.
//
// - LLH (imported to cvs 4/13/2011)
//
void viewStoredTraces_cdmsII(string testfile="queens_Filter_06110311_1647.root", int fileNum=1, int detNum=1, int skipTo= -999)
{
  // user must choose these settings
  int maxEvents = 25; //max events to display
  int pauseTime = 2;  //number of seconds to pause when displaying event

  // Open the file
  TFile f(testfile.c_str());

  // Setup canvas
  TCanvas* can = new TCanvas("mycan","mycan",1000, 800);
  can->SetHighLightColor(10);		      
  can->SetFillColor(10);		      
  can->Divide(2,3);

  //Loop over entries
  for(int evtCtr=0; evtCtr < maxEvents; evtCtr++)
  {
    int eventNum = fileNum*10000 + evtCtr;

    if(eventNum == skipTo || skipTo== -999)
    {
      //load traces
      TH1D* h_pa = f.Get(Form("pulsedumps_zip%d/PARawPulse%d", detNum, eventNum));
      TH1D* h_pb = f.Get(Form("pulsedumps_zip%d/PBRawPulse%d", detNum, eventNum));
      TH1D* h_pc = f.Get(Form("pulsedumps_zip%d/PCRawPulse%d", detNum, eventNum));
      TH1D* h_pd = f.Get(Form("pulsedumps_zip%d/PDRawPulse%d", detNum, eventNum));
      TH1D* h_qi = f.Get(Form("pulsedumps_zip%d/QIRawPulse%d", detNum, eventNum));
      TH1D* h_qo = f.Get(Form("pulsedumps_zip%d/QORawPulse%d", detNum, eventNum));
      
      //format
      h_pa->SetLineColor(kBlue);
      h_pb->SetLineColor(kMagenta);
      h_pc->SetLineColor(kCyan);
      h_pd->SetLineColor(kGreen);
      
      h_qi->SetLineColor(kRed);
      
      FormatHisto(h_pa, Form("PA : event %d", eventNum));
      FormatHisto(h_pb, Form("PB : event %d", eventNum));
      FormatHisto(h_pc, Form("PC : event %d", eventNum));
      FormatHisto(h_pd, Form("PD : event %d", eventNum));
      
      FormatHisto(h_qi, Form("QI : event %d", eventNum));
      FormatHisto(h_qo, Form("QO : event %d", eventNum));
      
      //draw traces
      can->cd(1);
      h_pa->DrawCopy();
      
      can->cd(2);
      h_pb->DrawCopy();
      
      can->cd(3);
      h_pc->DrawCopy();
      
      can->cd(4);
      h_pd->DrawCopy();
      
      can->cd(5);
      h_qi->DrawCopy();
      
      can->cd(6);
      h_qo->DrawCopy();
      
      //refresh
      can->Update();
      gSystem->Sleep(pauseTime*1000);

    } // end if skip option choosen

  } // end loop over events

   return;
}

void FormatHisto(TH1D* h, TString title)
{
  h->SetStats(kFALSE);
  h->GetYaxis()->SetTitleOffset(1.3);
  h->SetXTitle("adc bins");
  h->SetYTitle("adc bins");
  h->SetTitle(title);
  
}

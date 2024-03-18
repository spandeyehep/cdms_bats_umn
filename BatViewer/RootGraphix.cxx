///////////////////////////////////////////////////////////////////////////////
//Class Name: RootGraphix
//Authors: B. Loer
//Description: Utility class used to draw interactive ROOT canvases within 
//a regular binary executable.
//
//File Import by: B. Loer
//Creation Date: Dec . 18, 2011
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////

#include "RootGraphix.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TCanvasImp.h"
#include "TRootCanvas.h"
#include "TGClient.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TH1.h"
#include "TF1.h"
#include "TColor.h"
#include "TMath.h"

#include <cmath>
#include <iostream>


RootGraphix* RootGraphix::GetInstance()
{
  static RootGraphix graphix;
  return &graphix;
}


void* RootGraphix::RunRootGraphix(void* mutexptr)
{
  TMutexObj* mutex = (TMutexObj*)(mutexptr);
  while(mutex->TestBit(fKeepRunning)){
    gSystem->Sleep(100);
    TLockGuard lock(mutex);
    if(gSystem->ProcessEvents())
      break;
  }
  return 0;
}

RootGraphix::RootGraphix() : fApp("app",0,0), fMutex(), fThread(RunRootGraphix)
{
  SetDefaultStyleSheet();
  fMutex.SetBit(fKeepRunning,true);
  fThread.Run(&fMutex);
}

RootGraphix::~RootGraphix()
{
  if(fThread.GetState() == TThread::kRunningState){
    fMutex.SetBit(fKeepRunning,false);
    fThread.Join();
  }
}

RootGraphix::Lock RootGraphix::AcquireLock()
{
  TLockGuard* lg = new TLockGuard(GetInstance()->GetMutex());
  return std::auto_ptr<TLockGuard>(lg);
}

void RootGraphix::ProcessFile(const char* file)
{
  GetInstance()->GetApplication()->ProcessFile(file);
}

TCanvas* RootGraphix::GetCanvas(double scale, bool preventclose)
{
  GetInstance(); //make sure gClient exists
  if(scale<0.1) scale=0.1;
  //if(scale>1) scale=1;
  int n=0;
  if(gROOT)
    n = gROOT->GetListOfCanvases()->GetSize();
  
  char name[100];
  sprintf(name, "canvas%d",n);
  UInt_t wx = gClient->GetDisplayWidth();
  UInt_t wy = gClient->GetDisplayHeight();
  if(wx<10){
    wx = 1024;
    wy = 768;
  }
  int topx = wx/2 * (n % 2) + 10*(n/4);
  int topy = wy/2 * (int)(n%4 > 1) + 10*(n/4);
  Lock glock = AcquireLock();
  TCanvas* canvas = new TCanvas(name, name,
				topx, topy, (int)(wx*scale), (int)(wy*scale));
  if(preventclose){
    TRootCanvas* imp = (TRootCanvas*)(canvas->GetCanvasImp());
    if(imp) imp->DontCallClose();
  }
  return canvas;
}

void RootGraphix::DividePad(TPad* pad, int npads, bool getlock)
{
  if(!pad || npads==1) return;
  if(npads<1){
    //std::cerr<<"RootGraphix::DividePad only goes up to 20 sub-pads\n";
    return;
  }
  double margin = 0.001;
  Lock glock;
  if(getlock) glock = AcquireLock();
  if(npads == 2)
    pad->Divide(2,1,margin,margin);
  else if(npads < 5)
    pad->Divide(2,2,margin,margin);
  else if(npads < 7)
    pad->Divide(3,2,margin,margin);
  else if(npads < 10)
    pad->Divide(3,3,margin,margin);
  else if(npads < 13)
    pad->Divide(4,3,margin,margin);
  else if(npads < 17)
    pad->Divide(4,4,margin,margin);
  else if(npads < 21)
    pad->Divide(5,4,margin,margin);
  else{
    int nrows = TMath::CeilNint(sqrt(npads));
    pad->Divide(nrows,nrows,margin,margin);
  }
    
  
}

void RootGraphix::SetDefaultStyleSheet()
{
  TStyle *mystyle=new TStyle("mystyle","mystyle");
  *mystyle = *(gROOT->GetStyle("Plain"));
  mystyle->SetName("mystyle");
  gROOT->SetStyle("mystyle");
    
  mystyle->SetCanvasColor(kWhite);
  mystyle->SetTitleFillColor(kWhite);
  mystyle->SetFuncWidth(2);
  mystyle->SetHistLineWidth(2);
  mystyle->SetLegendBorderSize(0);
  //mystyle->SetOptFit(1111);
  mystyle->SetStatBorderSize(0);
  mystyle->SetTitleBorderSize(0);
  mystyle->SetDrawBorder(0);
  mystyle->SetLabelSize(.04,"xyz");
  mystyle->SetTitleSize(.04,"xyz");
  mystyle->SetLabelFont(102,"xyz");
  mystyle->SetOptStat("");
  mystyle->SetStatFont(102);
  mystyle->SetTitleFont(102,"xyz");
  mystyle->SetTitleFont(102,"pad");
  mystyle->SetStatStyle(0);
  mystyle->SetStatX(1);
  mystyle->SetStatY(1);
  mystyle->SetStatW(.2);
  mystyle->SetStatH(.15);
  mystyle->SetTitleStyle(0);
  mystyle->SetTitleX(0.01);
  mystyle->SetTitleW(0.98);
  mystyle->SetTitleY(1.);
  mystyle->SetTitleH(0.1);
  mystyle->SetStatColor(0);
  mystyle->SetStatBorderSize(0);
  mystyle->SetFillColor(10);
  mystyle->SetFillStyle(0);
  mystyle->SetTextFont(102);
  mystyle->SetCanvasBorderMode(0);
  mystyle->SetPadBorderMode(1);
  mystyle->SetFrameBorderMode(0);
  mystyle->SetDrawBorder(0);

  mystyle->SetPalette(1,0);
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
 
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  mystyle->SetNumberContours(NCont);
  
}

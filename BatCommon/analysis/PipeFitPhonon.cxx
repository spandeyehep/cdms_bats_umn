/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: PipeFitPhonon
//Authors: M. Kos
//Description:  This class is the equivalent of the 5 parameter Time Domain fit from the  
//PipeFitter code.  The fit is only applied to phonon pulses.
//Original authors of the PipeFitter code are L. Duong, J. Yoo, and E. Ramberg 
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////



#include <cmath>
#include <iostream>
#include <iomanip>
#include "PipeFitPhonon.h"
#include "TH1F.h"
#include "TFile.h"

//====================== Beginning some external definitions ======================================

//Definitions here are used by PipeFitPhonon but are declared external to 
//it in order to be compatable with TMinuit.  

// === myfitfunc ===
//"static" ensures that fallfunc/risefunc has only file scope and prevents multiple declaration errors. 
//fallfunc/risefunc is handed off to Minuit.  It calculates chisq for your fit function.  
static void fallfunc(int &npar, double *gin, double &chi2, double *par, int iflag);

static void risefunc(int &npar, double *gin, double &chi2, double *par, int iflag);

//not used by Minuit but need by the rtsafe functions
static void drise(double *, double, double *,double *);

static void rise2(double *, double, double *,double *);

//define variables that need to be shared between fallfunc/risefunc and PipeFitPhonon
//as statics here (note static names should start with g)
static int gMidpt,gStart,gNDF,gSize;
static double gChi2,gRMS;
static vector<double> gPulse;

// === body of risefunc ===
// The parameters accepted by myfitfunc are passed in by Minuit.  Don't modify the argument 
// list in the signature b/c this is what Minuit expects it to look like.   Shown here is an example
// from the PipeFit time domain class.  Replace the body of myfitfunc with your own function.
// Change the name of myfitfunc to whatever please you.  

//Calculate Chi2 for risefunc with given parameters
//This function is defined outside the class to be compatible with TMinuit
void risefunc(int &npar, double *gin, double &chi2, double *par, int iflag){
 double A    = par[0];  // amplitude 
  double Toff = par[1];  // offset from first time bin 
  double Trf  = par[2];  // rise time 
  double Tf1  = par[3];  // fall time 1 
  double Frac = par[4];  // fraction of fall time 2 to fall time 1 
  
  // calculate chi2 sum
  chi2 = 0.;
  double tempchi2 = 0.;
  double fval;
  //cout<<"A "<< A <<" Toff "<<Toff<<" Trf "<<Trf<<" Tf1 "<<Tf1<<" Frac "<<Frac<<endl;
  //cout<<"gStart: "<< gStart <<" gMidpt: "<<gMidpt <<" gRms: "<<gRMS<<endl;  

//  cout <<"double size = " << sizeof(double) <<", float size = " << sizeof(float) << endl;

  // printf("i         fval         gPulse[i]        chi2         tempchi2      diff\n"); 
  for (int i= (int) gStart; i<= (int) gMidpt; i++){
     double pulseVal = gPulse[i];
     double t = (double)i;
     fval = A*(1.0-TMath::Exp(-(t-Toff)/Trf))*(TMath::Exp(-(t-Toff)/Tf1) - Frac*TMath::Exp(-(t-Toff)/Trf));
     tempchi2 = (fval-pulseVal)*(fval-pulseVal)/gRMS/gRMS;
     chi2 += (fval-pulseVal)*(fval-pulseVal)/gRMS/gRMS;
//      cout<<i<<" fval="<<setprecision (18)<<fval<<" pulse="<<gPulse[i]<<" chi2="<<chi2<<" tmp="<<tempchi2<<" res="<<fval-pulseVal
// 	 <<" gRMS=" << gRMS <<" r="<<(fval-pulseVal)/gRMS
// 	 <<endl;
  }

//   cout<<"chi2: "<< setprecision (18) <<chi2
//       <<" A=" << A <<"Toff=" << Toff <<" Trf=" << Trf <<" Tf1=" << Tf1 <<" Frac="<< Frac 
//       <<endl;

  if (iflag==3)
    {
      int ndf = (int) (gMidpt - gStart + 1 - npar);
      gNDF = ndf;
      gChi2 = chi2;
    }
  
  return;
}


void fallfunc(int &npar, double *gin, double &chi2, double *par, int iflag)
{
  int gFallFuncFitStep=4;  //choose every fourth bin in the chi2 calculation
  double A        = par[0];   // amplitude
  double Tf1      = par[1];   // fall time 1 
  double Tf2_frac = par[2];   // fraction of fall time 2 to fall time 1 
  double Tf2      = par[3];   // fall time 2 

  // calculate chi2 sum
  chi2 = 0.;
  double fval;
  double tempchi2 = 0.;
  
  //temp cout<<"gMidpt-16: "<<gMidpt-16<<" gSize: "<<gSize<<" FallFuncFitStep "<<gFallFuncFitStep<<" gRms: "<<setprecision(18)<<gRMS<<endl;
  //temp cout<<"A "<< setprecision(18)<<A<<" Tf1 "<<Tf1<<" Tf2_frac "<<Tf2_frac<<" Tf2 "<<Tf2<<endl;
 
  //temp printf("i         fval         gPulse[i]        chi2         tempchi2      diff\n");
  for (int i=(int)gMidpt-16; i<=(int)gSize; i+=gFallFuncFitStep)
    {
      fval = A*(TMath::Exp(-(i+1)/Tf1) + Tf2_frac*TMath::Exp(-(i+1)/Tf2));
      tempchi2 = (fval-gPulse[i])*(fval-gPulse[i])/gRMS/gRMS;
      chi2 += (fval-gPulse[i])*(fval-gPulse[i])/gRMS/gRMS;
      //temp cout<<i<<" "<<setprecision (18)<<fval<<" "<<gPulse[i]<<" "<<chi2<<" "<<tempchi2<<" "<<fval-gPulse[i]<<endl;
    }
  
  
  if (iflag==3){
      gNDF = (int) ((gSize-gMidpt+1+16)/gFallFuncFitStep-npar);
      gChi2 = chi2;
  }
  
  return;

}

//done defining risefunc/fallfunc

// Needs to be external to class
// calculate 2nd order derivatives of rise functions
void drise(double *pr, double t, double *df, double *ddf){
  
  double A = pr[0];
  double B = pr[1];
  double C = pr[2];
  double D = pr[3];
  double G = pr[4];

  double u,du,ddu,v,dv,ddv;

  u = 1-TMath::Exp(-(t-B)/C);
  du = (1/C)*(TMath::Exp(-(t-B)/C));
  ddu = (-1/C/C)*(TMath::Exp(-(t-B)/C));
  v = TMath::Exp(-(t-B)/D)-G*(TMath::Exp(-(t-B)/C));
  dv = (-1/D)*(TMath::Exp(-(t-B)/D))-(-G/C)*(TMath::Exp(-(t-B)/C));
  ddv = (1/D/D)*(TMath::Exp(-(t-B)/D))-(G/C/C)*(TMath::Exp(-(t-B)/C));

  //  *f = A*(u*v);
  *df = A*(du*v + u*dv);
  *ddf = A*(ddu*v + 2*du*dv + u*ddv);

  return ;
}

void rise2(double *pr, double t, double *f, double *df){
  
  double A = pr[0];
  double B = pr[1];
  double C = pr[2];
  double D = pr[3];
  double G = pr[4];
  double off = pr[5];

  
  double u,du,ddu,v,dv,ddv;
  //  double *ddf;

  u = 1-TMath::Exp(-(t-B)/C);
  du = (1/C)*(TMath::Exp(-(t-B)/C));
  ddu = (-1/C/C)*(TMath::Exp(-(t-B)/C));
  v = TMath::Exp(-(t-B)/D)-G*(TMath::Exp(-(t-B)/C));
  dv = (-1/D)*(TMath::Exp(-(t-B)/D))-(-G/C)*(TMath::Exp(-(t-B)/C));
  ddv = (1/D/D)*(TMath::Exp(-(t-B)/D))-(G/C/C)*(TMath::Exp(-(t-B)/C));

  *f = A*(u*v) - off;
  *df = A*(du*v + u*dv);
  //  *ddf = A*(ddu*v + 2*du*dv + u*ddv);

  return ;
}

//=========================== End of External Definitions ==============================================

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
PipeFitPhonon::PipeFitPhonon():
   kAccuracy(0.01),
   fIsInitialized(false)

{
   //   cout <<"Hello from PipeFitPhonon()" << endl;

   //Set the name here
   fClassName = "PipeFitPhonon";
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here
   fsize =  (int)kFailValue;
   fchi2rf = (double)kFailValue;
   fchi2ff = (double)kFailValue;
   fndfff = (int)kFailValue;
   fndfrf = (int)kFailValue;
   frise5 = (double)kFailValue;
   frms =   (double)kFailValue;
   fpflag = 0;
   fStartWindowMin = (int)kFailValue;
   fStartWindowMax = (int)kFailValue;
   fStartRMSMult = (double)kFailValue;
   fStartWalkMult = (double)kFailValue;
   fStartThreshCheck = (double)kFailValue;
   fMaxThreshCheck = (double)kFailValue;
   fStartLargeRMSMult = (double)kFailValue;
   fTestSmallRMS = (double)kFailValue;
   fStartTimeDefaultSmall = (int)kFailValue;
   fMaxBinStartDiff = (double)kFailValue;
   fMaxBinStartMult = (double)kFailValue;
   fMaxBinStartAdd = (double)kFailValue;
   fMaxBinAdd = (double)kFailValue;
   fMidpointDefault = (double)kFailValue;
   fFallFuncEnd = (double)kFailValue;
   fRiseFuncA0Default = (double)kFailValue;
   fRiseFuncT0Default = (double)kFailValue;
   fRiseFuncTauDefault = (double)kFailValue;
   fRiseFuncKappaDefault = (double)kFailValue;
   fRiseFuncA1Default = (double)kFailValue;
   fRiseFuncPulseHeightMult = (double)kFailValue;
   fRiseFuncStartBinDiff= (double)kFailValue;
   fRiseFuncTauMult = (double)kFailValue;
   fRiseFuncKappaMult = (double)kFailValue;
   fFallFuncAfAdd = (double)kFailValue;
   fFallFuncTf1Start = (double)kFailValue;
   fFallFuncTf2Start = (double)kFailValue;
   fFallFuncTfrStart = (double)kFailValue;
   fFallFuncStepSize1 = (double)kFailValue;
   fFallFuncStepSize2 = (double)kFailValue;
   fMaxTraceStartSat = (double)kFailValue;
   fMaxTraceDiffSat = (double)kFailValue;
   fPulseheightMaxSat = (double)kFailValue;
   fNumberSatBins = (double)kFailValue;
 
 
   //initialize rise function fit values
   for(int i = 0;i<5;i++){
     fRiseFuncVal[i] = (double)kFailValue;
     fRiseFuncSig[i] = (double)kFailValue;
   }

   for(int i = 0;i<4;i++){
     fFallFuncVal[i] = (double)kFailValue;
     fFallFuncSig[i] = (double)kFailValue;
   }

}

PipeFitPhonon::~PipeFitPhonon()
{
//   cout <<"Goodbye from PipeFitPhonon()" << endl;
}

//if your RQ isn't here, it won't be stored in BatROOT output!!
//is there a way to choose what you do and don't want in the output?
void PipeFitPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   
   //Rise function fit values
   fRQList.insert(pair<string,double>("PFa0", initVal));
   fRQList.insert(pair<string,double>("PFt0fit", initVal));
   fRQList.insert(pair<string,double>("PFtau", initVal));
   fRQList.insert(pair<string,double>("PFkappa", initVal));
   fRQList.insert(pair<string,double>("PFa1", initVal));

   //Uncerainties on the rise function fit results
   fRQList.insert(pair<string,double>("PFea0", initVal));
   fRQList.insert(pair<string,double>("PFet0fit", initVal));
   fRQList.insert(pair<string,double>("PFetau", initVal));
   fRQList.insert(pair<string,double>("PFekappa", initVal));
   fRQList.insert(pair<string,double>("PFea1", initVal));
   
   //Fall function fit values
   fRQList.insert(pair<string,double>("PFaf", initVal));
   fRQList.insert(pair<string,double>("PFtf1", initVal));
   fRQList.insert(pair<string,double>("PFtfr", initVal));
   fRQList.insert(pair<string,double>("PFtf2", initVal));

   //Uncertainties on the fall function fit values
   fRQList.insert(pair<string,double>("PFeaf", initVal));
   fRQList.insert(pair<string,double>("PFetf1", initVal));
   fRQList.insert(pair<string,double>("PFetf2", initVal));
   fRQList.insert(pair<string,double>("PFetfr", initVal));

   //Quality of fit checks
   fRQList.insert(pair<string,double>("PFrfchisq", initVal));
   fRQList.insert(pair<string,double>("PFrfchisq030", initVal));
   fRQList.insert(pair<string,double>("PFrfchisq3060", initVal));
   fRQList.insert(pair<string,double>("PFrfchisq60100", initVal));
   fRQList.insert(pair<string,double>("PFffchisq", initVal));
   fRQList.insert(pair<string,double>("PFrfeflag", initVal));
   fRQList.insert(pair<string,double>("PFffeflag", initVal));
   
   //Values calculated from the fit results, as in piperoot
   fRQList.insert(pair<string,double>("PFrfint", initVal));
   fRQList.insert(pair<string,double>("PFffint", initVal));
   fRQList.insert(pair<string,double>("PFr0", initVal));
   fRQList.insert(pair<string,double>("PFr10", initVal));
   fRQList.insert(pair<string,double>("PFr30", initVal));
   fRQList.insert(pair<string,double>("PFr20", initVal));
   fRQList.insert(pair<string,double>("PFr40", initVal));
   fRQList.insert(pair<string,double>("PFr60", initVal));
   fRQList.insert(pair<string,double>("PFr100", initVal));

   //Ranges of rise and fall function fits
   fRQList.insert(pair<string,double>("PFrfstart", initVal));
   fRQList.insert(pair<string,double>("PFrfend", initVal));
   fRQList.insert(pair<string,double>("PFffstart", initVal));
   fRQList.insert(pair<string,double>("PFffend", initVal));

   //Pulse classification flag (1=small,2=medium,3=large,4=saturated)
   fRQList.insert(pair<string,double>("PFpflag", initVal));
   

   return;
}

//do it here and not in the constructor
void PipeFitPhonon::InitializeParameters(const vector<double> &pulsevector, const vector<double> &det_thresh, int channum, double RMS)
{ 

  fsize = (int) pulsevector.size();
  fpulse = pulsevector;
  //Get RMS
  frms = RMS;
  //Get Height of Pulse
  fpulseheight = PulseTools::MaxADC(pulsevector,499,700);//FIXME
  fmaxbin = (double) PulseTools::MaxADCPoint(pulsevector,499,700);//FIXME
   
  //Get 5% risetime from RFTFWalk not what pipefitter uses *******************

  //This is what pipefitter uses
  double rise5 = PulseTools::PipeFitterWalk(pulsevector,fStartWalkMult*fpulseheight,fStartWindowMin,fStartWindowMax); 

  frise5 = rise5;

  //Get small pulse start time
   
  //Get risetime from RFTFWalk not what pipefitter uses *******************
  //fstart = PulseTools::RTFTWalkTimeP(pulsevector,2.0*frms/fpulseheight,500,700);
  //***********************************************************************
   
  //This is what pipefitter uses
  // 
  fstart = PulseTools::PipeFitterWalk(pulsevector,fStartRMSMult*frms,fStartWindowMin,fStartWindowMax); 
  
  //Get medium
   if (fpulseheight > fStartThreshCheck) fstart = frise5; 
   
   double thresh_temp = det_thresh[channum];
  
   if (thresh_temp < fMaxThreshCheck) thresh_temp = fMaxThreshCheck;


   //Get large
   if (fpulseheight > thresh_temp){
     //Get risetime from RFTFWalk not what pipefitter uses ******************* 
     //fstart = PulseTools::RTFTWalkTimeP(pulsevector,4.0*frms/fpulseheight,500,700);//FIXME 
   //*****************************************************************************
     //This is what pipefitter uses 
     fstart = PulseTools::PipeFitterWalk(pulsevector,fStartLargeRMSMult*frms,fStartWindowMin,fStartWindowMax); 
   }
   
   if (fpulseheight < fTestSmallRMS*frms){
    fIsSmall = true;
    fIsMedium = false;
    fIsLarge = false;
    fpflag = 1;
   }
   else if (fpulseheight < thresh_temp){
    fIsSmall = false;
    fIsMedium = true;
    fIsLarge = false;
    fpflag = 2;
   }
   else {
    fIsSmall = false;
    fIsMedium = false;
    fIsLarge = true;
    fpflag = 3;
   }

   fIsInitialized = true;

   // if (channum == 2){
   // TH1F *hpulse = new TH1F("hpulse","",2048,0,2048);
   //for(int j = 0;j<2048;j++){
   //  hpulse->SetBinContent(j,pulsevector[j]);
   //}
   //TFile *fr = new TFile("problem_pulse_1866_41.root","RECREATE");
   //hpulse->Write();
   //}
   return;
}

//a simplified example loosely based on PipeFit is here.  Replace with your own.
void PipeFitPhonon::DoCalc()
{
   if(!fIsInitialized) 
   { 
      cout <<"PipeFitPhonon::WARNING: TD not initialized, nothing to fit, returning without fitting!" << endl; 
      return;
   }
   
   frf_errflg = -1;
   ff_errflg = -1;
   gPulse = fpulse;
   gRMS = frms;  
  
   //if pulse is small
   if (fIsSmall){
     fstart = (double)fStartTimeDefaultSmall;
     fmidpt = (double) fsize-1;
     gMidpt = (int) fmidpt;
     gStart = (int) fstart; 
     //cout <<"gStart1 "<<gStart<<" gMidpt1 "<<gMidpt<<endl;
     //cout <<"fstart "<<fstart<<" fmidpt "<<fmidpt<<endl;
     
     fMinuitRiseFunc = new TMinuit(kNumParsRF);
     fMinuitRiseFunc->SetFCN(risefunc);
   
     //Calls minuit with risefunc
     FitRiseFunc();
   
     fchi2rf = gChi2;
     fndfrf = (int) gNDF;
   }//end if(fIsSmall)

   //for medium sized pulse
   if (fIsMedium){
     if(((double)fmaxbin-fstart)>=fMaxBinStartDiff){
       fmidpt = fMaxBinStartMult*(fmaxbin - fstart)+ fstart + fMaxBinStartAdd; 
     }else{
       fmidpt = (double)fmaxbin+fMaxBinAdd;
     }
     if (fmidpt > fMidpointDefault) fmidpt = fMidpointDefault;
     fsize = (int) fFallFuncEnd;  
     gSize = fsize;
     gMidpt = (int)fmidpt;
     gStart = (int)fstart;
     fMinuitRiseFunc = new TMinuit(kNumParsRF);
     fMinuitRiseFunc->SetFCN(risefunc);
    
     //Calls minuit with risefunc
     FitRiseFunc();
     fchi2rf = gChi2;
     fndfrf = gNDF;
     
     //Fall func number of parameters 
     fMinuitFallFunc = new TMinuit(kNumParsFF);
     fMinuitFallFunc->SetFCN(fallfunc);
     //Calls minuit with fallfunc
     FitFallFunc();
     fchi2ff = gChi2;
     fndfff = gNDF;
 }
   
   //for large sized pulse
   //only fit using fall function
   if (fIsLarge){
     if(((double)fmaxbin-fstart)>=fMaxBinStartDiff){
       fmidpt =  fMaxBinStartMult*(fmaxbin - fstart)+ fstart + fMaxBinStartAdd; 
     }else{
       fmidpt = fmaxbin+fMaxBinAdd;
       if (fmidpt > fMidpointDefault) fmidpt = fMidpointDefault; 
     }
        
     if (TEStest()) { 
       fmidpt = fMidpointDefault;
       fpflag = 4;
     }
     gMidpt = (int)fmidpt;
     fsize = (int) fFallFuncEnd;  
     gSize = fsize;
     fMinuitFallFunc = new TMinuit(kNumParsFF);
     fMinuitFallFunc->SetFCN(fallfunc);
     FitFallFunc();
     fchi2ff = gChi2;
     fndfff = gNDF;
   }// end if large pulses
      
   if (fIsLarge || fIsMedium) delete fMinuitFallFunc;
   if (fIsSmall || fIsMedium) delete fMinuitRiseFunc;
   
   if (fIsLarge || fIsMedium) fMinuitFallFunc = NULL;
   if (fIsSmall || fIsMedium) fMinuitRiseFunc = NULL;
   
   fpulse.clear();//set vector lenght to 0
   fIsInitialized = false;
     
   //Replace with your RQ values here
   if(fStoreRQs) {
     //Rise function fit results
     fRQList["PFa0"] = GetA0();
     fRQList["PFt0fit"] = GetT0Fit();
     fRQList["PFtau"] = GetTau();
     fRQList["PFkappa"] = GetKappa();
     fRQList["PFa1"] = GetA1();

     //Fall function fit results
     fRQList["PFaf"] = GetAf();
     fRQList["PFtf1"] = GetTf1();
     fRQList["PFtfr"] = GetTfr();
     fRQList["PFtf2"] = GetTf2();

     //Goodness of fit values
     
     if (GetDOFRF() != kFailValue && GetChi2RF() != kFailValue && GetDOFRF() > 0.0)
       fRQList["PFrfchisq"] = GetChi2RF()/GetDOFRF();
     else
       fRQList["PFrfchisq"] = kFailValue;

     if (GetDOFFF() != kFailValue && GetChi2FF() != kFailValue && GetDOFFF() > 0.0)
       fRQList["PFffchisq"] = GetChi2FF()/GetDOFFF();
     else
       fRQList["PFffchisq"] = kFailValue;

     fRQList["PFrfchisq030"] = GetT030Chisq();
     fRQList["PFrfchisq3060"] = GetT3060Chisq();
     fRQList["PFrfchisq60100"] = GetT60100Chisq();
     

     //Uncertainties on the rise function fit
     fRQList["PFea0"] = GetEA0();
     fRQList["PFea1"] = GetEA1();
     fRQList["PFet0fit"] = GetET0Fit();
     fRQList["PFetau"] = GetETau();
     fRQList["PFekappa"] = GetEKappa();

     //Uncertainties on the fall function fit
     fRQList["PFeaf"] = GetEAf();
     fRQList["PFetf1"] = GetETf1();
     fRQList["PFetfr"] = GetETfr();
     fRQList["PFetf2"] = GetETf2();

     //Derived quantities from the fit
     fRQList["PFr0"] = GetT0();
     fRQList["PFr10"] = GetT10();
     fRQList["PFr20"] = GetT20();
     fRQList["PFr30"] = GetT30();
     fRQList["PFr40"] = GetT40();
     fRQList["PFr60"] = GetT60();
     fRQList["PFr100"] = GetTPeak();
     fRQList["PFrfint"] = GetIntRiseFunc();
     fRQList["PFffint"] = GetIntFallFunc();

     //Fit ranges
     fRQList["PFrfstart"] = GetRiseFuncStart();
     fRQList["PFrfend"] = GetRiseFuncEnd();
     fRQList["PFffstart"] = GetFallFuncStart();
     fRQList["PFffend"] = GetFallFuncEnd();

     //Pulse flag
     fRQList["PFpflag"] = (double) fpflag;

     //Fit uncertainty error flag
     fRQList["PFrfeflag"] = (double) frf_errflg;
     fRQList["PFffeflag"] = (double) ff_errflg;
   }
   return;
}

bool PipeFitPhonon::TEStest(){
  int    flat          = 0    ;
  bool   TES_SATURATED = false;
  double maxtrace      = fMaxTraceStartSat;
  
  for (int iadc=0;iadc<fsize;iadc++){
    
    if(maxtrace<fpulse[iadc]) 
       maxtrace=fpulse[iadc];

    // if variation of trace is smaller than 20 
    // at large pulse (>2000)                  
    if(abs((int)(maxtrace-fpulse[iadc]))<fMaxTraceDiffSat && fpulse[iadc]>fPulseheightMaxSat) 
      {            
        flat = flat+1;      
        if(flat>fNumberSatBins)TES_SATURATED = true; // 120 time bins 
      }
  }
  return TES_SATURATED; 

}

 
void PipeFitPhonon::FitRiseFunc(){
  int i;
  double arglist[10], b1, b2;
  int ierflg = 0;
  double start[5];
  double rf_start[5];
  double rf_step[5];

  TString rf_names[5] = {"a0", "t0", "tau", "kappa","a1"};
  
  rf_start[0] = fRiseFuncA0Default; //starting a0 value (normalization)
  rf_start[1] = fRiseFuncT0Default; //starting t0 bin value
  rf_start[2] = fRiseFuncTauDefault; 
  rf_start[3] = fRiseFuncKappaDefault;
  rf_start[4] = fRiseFuncA1Default;
  
  fMinuitRiseFunc->SetPrintLevel(-1);
  
  start[0] = fRiseFuncPulseHeightMult*fpulseheight;
  start[1] = fstart - fRiseFuncStartBinDiff;
  start[2] = fRiseFuncTauMult * (fmaxbin - fstart);
  start[3] = fRiseFuncKappaMult * (fmaxbin - fstart);
  start[4] = rf_start[4];

  
// FIRST FIT WITH RF FUNCTION
  for (i=0; i<kNumParsRF; i++) {
    rf_step[i] = fabs(rf_start[i])/kNormFitStep;
    //    if(i==4)RF_STEP[i] = 0.;
    fMinuitRiseFunc->mnparm(i, rf_names[i], start[i], rf_step[i], 0, 0, ierflg);
    //  RF->mnparm(i, RF_NAMES[i], start[i], RF_STEP[i], 0, 0, ierflg);
  }

  arglist[0] = 2; // strategy setting for more precise error calculation
  fMinuitRiseFunc->mnexcm("SET STRAT", arglist, 1, ierflg);  

  arglist[0] = kMaxMIGRADCalls; // MAX # MIGRAD CALLS 
  fMinuitRiseFunc->mnexcm("MIGRAD", arglist, 1, frf_errflg);  

  for (i=0; i<kNumParsRF; i++){
    fMinuitRiseFunc->mnpout(i, rf_names[i], fRiseFuncVal[i],
               fRiseFuncSig[i], b1, b2, ierflg);
  }
  
  // IF FIT FAILS TRY AGAIN WITH LARGER STEP SIZE
  // MIGRAD not converged
  if (frf_errflg == 4) {
    for (i=0; i<kNumParsRF; i++) {
      fMinuitRiseFunc->mnpout(i, rf_names[i], fRiseFuncVal[i],
		 fRiseFuncSig[i], b1, b2, ierflg);
      start[i] = fRiseFuncVal[i];
      rf_step[i] = fabs(start[i]) / kNormFitStep;
      //      if(i==4)RF_STEP[i] = 0.;
      fMinuitRiseFunc->mnparm(i, rf_names[i], start[i], rf_step[i], 0, 0, ierflg);
      //      RF->mnparm(i, RF_NAMES[i], start[i], RF_STEP[i], 0, 0, ierflg);
    }
    fMinuitRiseFunc->mnexcm("MIGRAD", arglist, 1, frf_errflg);
  }
 
  //check if returned errors are reasonable, otherwise make 0
  for (i=0; i<kNumParsRF; i++){
    fMinuitRiseFunc->mnpout(i, rf_names[i], fRiseFuncVal[i], 
	       fRiseFuncSig[i], b1, b2, ierflg);
    if(std::isnan(double(fRiseFuncSig[i])) || 
       std::isinf(double(fRiseFuncSig[i]))){
      fRiseFuncSig[i]=0.;
    }
  }
  
  //leftover from Fortran, not sure if necessary but too scared to take it out [MK] 
  fMinuitRiseFunc->mnexcm("STOP" , arglist, 0, ierflg);

  return;
 
}

//Calculate Chi2 for fallfunc with given parameters
//This function is defined outside the class to be compatible with TMinuit
void PipeFitPhonon::FitFallFunc(){
  double arglist[10], b1, b2;
  int ierflg = 0;
  double start[4];
  double ff_step[4]; 
  TString ff_names[4] = {"a1", "tf1", "tfr", "tf2"};
  int i;

  start[0] = fpulseheight * fFallFuncAfAdd; //a1
  start[1] = fFallFuncTf1Start; 
  start[2] = fFallFuncTfrStart; 
  start[3] = fFallFuncTf2Start; 

  fMinuitFallFunc->SetPrintLevel(-1);  
  
  // TRY TO FIT
  for (i=0; i<kNumParsFF; i++) {
    ff_step[i] = fabs(start[i]) / kNormFitStep;
    if (i == 2)
      // -1 to 1 is the boundary for this parameter
      fMinuitFallFunc->mnparm(i, ff_names[i], start[i], start[i], -1.0, 1.0, ierflg);
    else
      // 0 to 0 implies no boundary for these parameters
      fMinuitFallFunc->mnparm(i, ff_names[i], start[i], start[i], 0, 0, ierflg);
  }

  arglist[0] = 2; // strategy setting for more precise error calculation
  fMinuitFallFunc->mnexcm("SET STRAT", arglist, 1, ierflg);  
    
  arglist[0] = kMaxMIGRADCalls; // max # MIGRAD calls 
  fMinuitFallFunc->mnexcm("MIGRAD", arglist, 1, ff_errflg);
  
  
  // IF FIT DOES NOT CONVERGE (ERROR FLAG == 4)
  // FIRST TRY TO CHANGE FIRST PARAMETER
  if (ff_errflg == 4) {
    for (i=0; i<kNumParsFF; i++) {
      ff_step[i] = fabs(start[i]) / kNormFitStep * fFallFuncStepSize1;
      if (i == 2)
      fMinuitFallFunc->mnparm(i, ff_names[i], start[i], ff_step[i], -1.0, 1.0, ierflg);
      else
      fMinuitFallFunc->mnparm(i, ff_names[i], start[i], ff_step[i], 0, 0, ierflg);
    }
    fMinuitFallFunc->mnexcm("MIGRAD", arglist, 1, ff_errflg);
  }
  
  
  // IF THE FIT DOES NOT IMPROVE
  // NEXT TRY TO FIT WITH EXPONENTIAL 
  // I.E. A*exp(-t/tf1)
  if (ff_errflg == 4) {
    start[2] = 0;
    for (i=0; i<kNumParsFF; i++) {
      ff_step[i] = fabs(start[i]) / kNormFitStep * fFallFuncStepSize2;
      fMinuitFallFunc->mnparm(i, ff_names[i], start[i], ff_step[i], 0, 0, ierflg);
    }
    fMinuitFallFunc->mnexcm("MIGRAD", arglist, 1, ff_errflg);
  }
  
  
  // GET FIT RESULTS AND
  // CHECK WHETHER FIT RESULTS ARE REASONABLE 
    for (i=0; i<kNumParsFF; i++){
    fMinuitFallFunc->mnpout(i, ff_names[i], fFallFuncVal[i],
	       fFallFuncSig[i], b1, b2, ierflg);
    if(std::isnan(double(fFallFuncSig[i])) || 
       std::isinf(double(fFallFuncSig[i]))){
      //do_again = do_again +1 ;
      fFallFuncSig[i]=0.;
      }
  }

 
  //probably obsolete but not sure
  fMinuitFallFunc->mnexcm("STOP" , arglist, 0, ierflg);
  
 
  return;

}

//========================================================================================
//PipeRoot functions

// calculate 1st order derivatives of rise functions
void PipeFitPhonon::RiseFirstDev(double *pr, double t, double *f, double *df){
  
  double A = pr[0];
  double B = pr[1];
  double C = pr[2];
  double D = pr[3];
  double G = pr[4];

  
  double u,du,ddu,v,dv,ddv;
  //  double *ddf;

  u = 1-TMath::Exp(-(t-B)/C);
  du = (1/C)*(TMath::Exp(-(t-B)/C));
  ddu = (-1/C/C)*(TMath::Exp(-(t-B)/C));
  v = TMath::Exp(-(t-B)/D)-G*(TMath::Exp(-(t-B)/C));
  dv = (-1/D)*(TMath::Exp(-(t-B)/D))-(-G/C)*(TMath::Exp(-(t-B)/C));
  ddv = (1/D/D)*(TMath::Exp(-(t-B)/D))-(G/C/C)*(TMath::Exp(-(t-B)/C));

  *f = A*(u*v);
  *df = A*(du*v + u*dv);
  //  *ddf = A*(ddu*v + 2*du*dv + u*ddv);

  return ;
}


double PipeFitPhonon::RiseFuncInt(double *pr) 
{
  double A,B,C,D,G;
  double y1,y2,alpha,beta;
  double t1,t2,intg_t1,intg_t2;
  //  double dT;

  A = pr[0];
  B = pr[1];
  C = pr[2];
  D = pr[3];
  G = pr[4];
  
  double st0_try=TMath::Log(pr[4])*(pr[2]*pr[3]/(pr[3]-pr[2]))+pr[1];
  if(st0_try > B){
    t1=st0_try;
  }else{
    t1=B;
  }

  t2 = pr[5]; // end of integral
  if (pr[6] == 1) t2 = 100000.;
  
  /* rf(t) = A*[1-exp(-(t-B)/C)]*[exp(-(t-B)/D)-G*exp(-(t-B)/C)] */
  
  y1 = (t1-B);
  y2 = (t2-B);  
  alpha = 1 / (1/C + 1/D);
  beta  = 1 / (1/C + 1/C);  
  intg_t1 = (-D*exp(-y1/D) + G*C*exp(-y1/C)) +
    (alpha*exp(-y1/alpha) - G*beta*exp(-y1/beta));
  intg_t2 = (-D*exp(-y2/D) + G*C*exp(-y2/C)) +
    (alpha*exp(-y2/alpha) - G*beta*exp(-y2/beta));
  
  return A*(intg_t2 - intg_t1);
}


double PipeFitPhonon::FallFuncInt(double *pf) {
  double A,D,G,H,dH;
  double t;
  
  A = pf[0]; //af
  D = pf[1]; //tf1
  G = pf[2]; //tfr
  H = pf[3]; //tf2
  t = pf[4] + 1.0;  // pf[4] = fmidpt
  dH = pf[5]; // = fFallFuncSig[3]
  
  // ff(t) = A*[exp(-t/D)+G*exp(-t/H)]
  if (((dH/H) > 1.0) || (H > 5000))                                         
    return A*D*exp(-t/D);                                                   
  else                                       
    return A*(D*exp(-t/D)+G*H*exp(-t/H));
}

double PipeFitPhonon::GetT0(){
 double prT0;
 prT0 = (double)kFailValue-1.0; //so that default output is -999999
 if (IsSmall() || IsMedium()){
   prT0 = fRiseFuncVal[1]; // this corresponds to t0fit
   double pr[5]={0.};
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   double prT0_try=TMath::Log(pr[4])*(pr[2]*pr[3]/(pr[3]-pr[2]))+pr[1];
   if(prT0_try > prT0 &&  prT0_try < fmidpt)prT0=prT0_try;    
 }

 return prT0 + 1.0; //since bins start from 1 and not 0
}


double PipeFitPhonon::GetTPeak(){

  double prPEAKT,prT0;
 double pr[7]={0.};
 
 prPEAKT = (double)kFailValue-1.0; //so that default output is -999999
 if (IsSmall() || IsMedium()){
   if (!std::isnan(fRiseFuncVal[0]) && !std::isinf(fRiseFuncVal[0]) 
       && !std::isnan(fRiseFuncVal[1]) && !std::isinf(fRiseFuncVal[1])
       && !std::isnan(fRiseFuncVal[2]) && !std::isinf(fRiseFuncVal[2])
       && !std::isnan(fRiseFuncVal[3]) && !std::isinf(fRiseFuncVal[3])
       && !std::isnan(fRiseFuncVal[4]) && !std::isinf(fRiseFuncVal[4])){
     //double r_chi2_rf = fchi2rf/fndfrf; //not needed because not using this now, but leaving it in as option
     pr[0] = fRiseFuncVal[0];
     pr[1] = fRiseFuncVal[1];
     pr[2] = fRiseFuncVal[2];
     pr[3] = fRiseFuncVal[3];
     pr[4] = fRiseFuncVal[4];
     pr[5] = fmidpt;
     pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
     prT0=pr[1];
     double prT0_try=TMath::Log(pr[4])*(pr[2]*pr[3]/(pr[3]-pr[2]))+pr[1];
     if(prT0_try > prT0 &&  prT0_try < fmidpt)prT0=prT0_try;
     // CALCULATE PAEK TIME USING drise FUNCTION 
     double pk0=prT0+1.;
     double pk1=prT0+300.;
     
     prPEAKT=rtsafe(drise,pr,pk0,pk1,kAccuracy);
          
    
   }
 }

 return prPEAKT+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetIntRiseFunc(){
  double prINTG1;
  prINTG1 = (double)kFailValue;
  double pr[7]={0.};
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   if (pr[1]>0. && pr[1]<800. && // start time should be 0--800
       pr[2]>0. && pr[2]<500. && // rise time should be 0--500
       pr[3]>0. && pr[3]<500.){  // fall time should be 0--500 
        prINTG1 = RiseFuncInt(pr);
        if (std::isinf(prINTG1) || std::isnan(prINTG1)) prINTG1 = kFailValue;
   }
  }

  return prINTG1;
}

double PipeFitPhonon::GetIntFallFunc(){
  double prINTG2;
  prINTG2 = (double)kFailValue;
  double pr[7]={0.};
  if (IsMedium() || IsLarge()){
    if (ff_errflg == 0){
      double r_chi2_ff = fchi2ff/fndfff;
      if (r_chi2_ff < 3.){
	pr[0] = fFallFuncVal[0];
	pr[1] = fFallFuncVal[1];
	pr[2] = fFallFuncVal[2];
	pr[3] = fFallFuncVal[3];
	pr[4] = fmidpt;
	pr[5] = fFallFuncSig[3];
	if (pr[1]>0. && pr[1]<1000. && // the first fall time
	    pr[3]>0. && pr[3]<3000.){  // the second fall time
	  prINTG2 = FallFuncInt(pr);
	}
      }
    }
  }
  
  return prINTG2;
}


double PipeFitPhonon::GetT10(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t10time = (double)kFailValue-1.0; //adjusted so that final output is -999999 if GetTPeak() = kFailValue
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.1;
   //cout << "GetT10 " << endl;
   //cout << "Tpeak " << GetTPeak() << " pkmax " << pkmax << " t0fit " << GetT0Fit() << endl; 
   t10time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t10time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT20(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t20time = (double)kFailValue-1.0;//adjusted so that final output is -999999 if GetTPeak() = kFailValue
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.2;
   t20time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t20time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT30(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t30time = (double)kFailValue-1.0;//adjusted so that final output is -999999 if GetTPeak() = kFailValue
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.3;
   t30time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t30time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT40(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t40time = (double)kFailValue-1.0;//adjusted so that final output is -999999 if GetTPeak() = kFailValue 
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.4;
   t40time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t40time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT50(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t50time = (double)kFailValue-1.0; //adjusted so that final output is -999999 if GetTPeak() = kFailValue
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.5;
   t50time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t50time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT60(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t60time = (double)kFailValue-1.0; //adjusted so that final output is -999999 if GetTPeak() = kFailValue 
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.6;
   t60time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t60time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT70(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t70time = (double)kFailValue-1.0; //adjusted so that final output is -999999 if GetTPeak() = kFailValue
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.7;
   t70time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t70time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT80(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t80time = (double)kFailValue-1.0;//adjusted so that final output is -999999 if GetTPeak() = kFailValue 
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; ;
   pr2[5] = pkmax*0.8;
   t80time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t80time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT90(){
  double pr[7]={0.};
  double pr2[7]={0.};
  double pkmax;
  double t90time = (double)kFailValue-1.0;//adjusted so that final output is -999999 if GetTPeak() = kFailValue 
  if (GetTPeak() != (double)kFailValue){
   pr[0] = fRiseFuncVal[0];
   pr[1] = fRiseFuncVal[1];
   pr[2] = fRiseFuncVal[2];
   pr[3] = fRiseFuncVal[3];
   pr[4] = fRiseFuncVal[4];
   pr[5] = fmidpt;
   pr[6] = (double)fpflag; //0 noise, 1-small pulse, 3-large pulse, 4-saturated pulse
   for(int i=0;i<7;i++) pr2[i] = pr[i];
   double peak_val[1],dpeak_val[1];
   RiseFirstDev(pr,GetTPeak()-1.0,peak_val,dpeak_val);
   pkmax = peak_val[0]; 
   pr2[5] = pkmax*0.9;
   t90time = rtsafe(rise2,pr2,GetT0()-1.0,GetTPeak()-1.0,kAccuracy);
  }
  return t90time+1;//since first time bin starts from 1
}

double PipeFitPhonon::GetT030Chisq(){
  double A    = fRiseFuncVal[0];  // amplitude 
  double Toff = fRiseFuncVal[1];  // offset from first time bin (t0fit) 
  double Trf  = fRiseFuncVal[2];  // rise time 
  double Tf1  = fRiseFuncVal[3];  // fall time 1 
  double Frac = fRiseFuncVal[4];  // fraction of fall time 2 to fall time 1
  double startbin = GetT0()-1.0; //BR internal binning convention
  double endbin = GetT30()-1.0; //BR internal binning convention
  double chi2 = 0.0;
  double fval;
  
  if (startbin != kFailValue && endbin != kFailValue 
      && startbin >= 0.0 && endbin < gPulse.size() && endbin >= 0.0){
    for (int i= (int) startbin; i<= (int) endbin ; i++){
      double pulseVal = gPulse[i];
      double t = (double)i;
      fval = A*(1.0-TMath::Exp(-(t-Toff)/Trf))*(TMath::Exp(-(t-Toff)/Tf1) - Frac*TMath::Exp(-(t-Toff)/Trf));
      chi2 += (fval-pulseVal)*(fval-pulseVal)/gRMS/gRMS;
    }
    return chi2;
    }
  else return (double) kFailValue;
}

double PipeFitPhonon::GetT3060Chisq(){
  double A    = fRiseFuncVal[0];  // amplitude 
  double Toff = fRiseFuncVal[1];  // offset from first time bin (t0fit) 
  double Trf  = fRiseFuncVal[2];  // rise time 
  double Tf1  = fRiseFuncVal[3];  // fall time 1 
  double Frac = fRiseFuncVal[4];  // fraction of fall time 2 to fall time 1
  double startbin = GetT30()-1.0; //BR internal binning convention
  double endbin = GetT60()-1.0;
  double chi2 = 0.0;
  double fval;
  if (startbin != kFailValue && endbin != kFailValue 
      && startbin >= 0.0 && endbin < gPulse.size() && endbin >= 0.0 ){
    for (int i= (int) startbin; i<= (int) endbin ; i++){
      double pulseVal = gPulse[i];
      double t = (double)i;
      fval = A*(1.0-TMath::Exp(-(t-Toff)/Trf))*(TMath::Exp(-(t-Toff)/Tf1) - Frac*TMath::Exp(-(t-Toff)/Trf));
      chi2 += (fval-pulseVal)*(fval-pulseVal)/gRMS/gRMS;
    }
    return chi2;
  }
  else return (double) kFailValue;
}

double PipeFitPhonon::GetT60100Chisq(){
  double A    = fRiseFuncVal[0];  // amplitude 
  double Toff = fRiseFuncVal[1];  // offset from first time bin (t0fit) 
  double Trf  = fRiseFuncVal[2];  // rise time 
  double Tf1  = fRiseFuncVal[3];  // fall time 1 
  double Frac = fRiseFuncVal[4];  // fraction of fall time 2 to fall time 1
  double startbin = GetT60()-1.0; //BR internal binning convention
  double endbin = GetTPeak()-1.0;
  double chi2 = 0.0;
  double fval;
  if (startbin != kFailValue && endbin != kFailValue 
      && startbin >= 0.0 && endbin < gPulse.size() && endbin >= 0.0 ){
    for (int i= (int) startbin; i<= (int) endbin ; i++){
      double pulseVal = gPulse[i];
      double t = (double)i;
      fval = A*(1.0-TMath::Exp(-(t-Toff)/Trf))*(TMath::Exp(-(t-Toff)/Tf1) - Frac*TMath::Exp(-(t-Toff)/Trf));
      chi2 += (fval-pulseVal)*(fval-pulseVal)/gRMS/gRMS;
    }
    return chi2;
  }
  else return (double) kFailValue;
}

double PipeFitPhonon::rtsafe(void (*funcd)(double *, double, double *, double *),double *pr, double x1, double x2, double xacc)
{
  //	void nrerror(char error_text[]);
	int j;
	double df,dx,dxold,f,fh,fl;
	double temp,xh,xl,rts;
		
	(*funcd)(pr,x1,&fl,&df);
	(*funcd)(pr,x2,&fh,&df);
	//	if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0))
	//		nrerror("Root must be bracketed in rtsafe");
	if (fl == 0.0) return x1;
	if (fh == 0.0) return x2;
	if (fl < 0.0) {
		xl=x1;
		xh=x2;
	} else {
		xh=x1;
		xl=x2;
	}
	rts=0.5*(x1+x2);
	dxold=fabs(x2-x1);
	dx=dxold;
	(*funcd)(pr,rts,&f,&df);
        //cout << " x1 " << x1 << " x2 " << x2 << " xh " << xh << " xl " << xl << " rts " << rts << endl;
	for (j=1;j<=kMaxIT;j++) {
		if ((((rts-xh)*df-f)*((rts-xl)*df-f) >= 0.0)
			|| (fabs(2.0*f) > fabs(dxold*df))) {
			dxold=dx;
			dx=0.5*(xh-xl);
			rts=xl+dx;
			if (xl == rts && !std::isnan(rts) && !std::isinf(rts)) return rts;
		} else {
			dxold=dx;
			dx=f/df;
			temp=rts;
			rts -= dx;
			if (temp == rts  && !std::isnan(rts) && !std::isinf(rts)) return rts;
		}
		if (fabs(dx) < xacc && !std::isnan(rts) && !std::isinf(rts)) return rts;
		(*funcd)(pr,rts,&f,&df);
		if (f < 0.0)
			xl=rts;
		else
			xh=rts;
		//cout << " rts iter " << rts << endl;
	}
	//	nrerror("Maximum number of iterations exceeded in rtsafe");
        rts = kFailValue - 1; //because when used we add 1 to be in PipeFitter bins 
	return rts;
}


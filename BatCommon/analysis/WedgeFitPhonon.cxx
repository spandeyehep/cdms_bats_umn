///////////////////////////////////////////////////////////////////////////////// 
//Class Name: WedgeFitPhonon
//Authors: Oleg Kamaev  (ported by Bruno Serfass into BatRoot code)
//Description: This class performs a 3-parameter fit on phonon pulses (below 20% rising edge amplitude)
// using a polynomial functional form 
//
//
//File Import By: B. Serfass
//Creation Date: Jan. 21, 2009
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <math.h>

#include "WedgeFitPhonon.h"
#include "PulseTools.h"
#include "PulseFilter.h"

//====================== Beginning some external definitions ======================================

//Definitions here are used by WedgeFitPhonon but are declared external to 
//it in order to be compatable with TMinuit.  

// === wedgefunc ===

// "static" ensures that wedgefunc has only file scope and prevents multiple declaration errors. 
// wedgefunc is handed off to Minuit.  It calculates chisq for your fit function.  

static void wedgefunc(int &npar, double *gin, double &chi2, double *par, int iflag);


//define variables that need to be shared between wedgefunc and WedgeFitPhonon
//as statics here (note static names should start with g)

static double gChi2, gRMS;
static int gStartPt, gEndPt, gNdof;
 
static vector<double> gPulse;

// === body of wedgefunc ===

// The parameters accepted by wedgefunc are passed in by Minuit.  Don't modify the argument 
// list in the signature b/c this is what Minuit expects it to look like.   Shown here is an example
// from the PipeFit time domain class.  Replace the body of myfitfunc with your own function.
// Change the name of myfitfunc to whatever please you.  

void wedgefunc(int &npar, double *gin, double &chi2, double *par, int iflag) {

   // Wedge  fitting parameters
   double par1 = par[0]; 
   double par2 = par[1]; 
   double par3 = par[2]; 
   
   // calculate chi2 sum
   chi2 = 0.;
   double fval;

   //looping over adc bins
   for (int i= (int) gStartPt; i<= (int) gEndPt; i++){
    
     double pulseADC = gPulse[i-1];

     if (i<par3) {
       fval =0.;
     } else {
       fval = par1*(i*i-par3*par3)+ par2*(i-par3);
     }
   
     chi2 += (fval-pulseADC)*(fval-pulseADC)/gRMS/gRMS;
   }
  

  //if done fitting (iflag == 3) then store the chisq so that WedgeFitPhonon can access the value
  if (iflag==3)
    {
      int ndf = (int) (gEndPt - gStartPt + 1 - npar);
      gNdof = ndf;
      gChi2 = chi2;
    }
  
  return;

} //done defining wedgefunc

//=========================== End of External Definitions ==============================================

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
WedgeFitPhonon::WedgeFitPhonon()
{
   //   cout <<"Hello from WedgeFitPhonon()" << endl;

    //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "WedgeFitPhonon";
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

WedgeFitPhonon::~WedgeFitPhonon()
{
//   cout <<"Goodbye from WedgeFitPhonon()" << endl;
}

//if your RQ isn't here, it won't be stored in BatROOT output!!
//is there a way to choose what you do and don't want in the output?
void WedgeFitPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("WFstart", initVal));
   fRQList.insert(pair<string,double>("WFend", initVal));
   fRQList.insert(pair<string,double>("WFpar1", initVal));
   fRQList.insert(pair<string,double>("WFpar2", initVal));
   fRQList.insert(pair<string,double>("WFpar3", initVal));
   fRQList.insert(pair<string,double>("WFepar1", initVal));
   fRQList.insert(pair<string,double>("WFepar2", initVal));
   fRQList.insert(pair<string,double>("WFepar3", initVal));
   fRQList.insert(pair<string,double>("WFchisq", initVal));
   fRQList.insert(pair<string,double>("WFeflag", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

  
// ========= Set parameters  =============

void WedgeFitPhonon::SetFitParameters(const vector<double>& aPulse,double sampleRate, int butterOrder,double butterCutoff, double rmsADC,double maxADC,int time10,int time20,int time60,int binSStart,int binAEnd,int binSt0,double par1Start)
{
   
   // === Set Default values === //

   fStartPt = 0;
   fEndPt = 0;
   
   fNdof = -999999;
   fChi2 = -999999.;
   fFitErrFlag = -999999;
   
   fParStartVal[0] = -999999.;
   fParFitVal[0] = -999999.;
   fParFitSig[0] = -999999.;

   fParStartVal[1] = -999999.;
   fParFitVal[1] = -999999.;
   fParFitSig[1] = -999999.;

   fParStartVal[2] = -999999.;
   fParFitVal[2] = -999999.;
   fParFitSig[2] = -999999.;

   // === filter pulse ===
   
   vector<double> aFilteredPulse; //copy the pulse in case we want to filter it
   aFilteredPulse = PulseFilter::ButterLowPass(aPulse, sampleRate, butterCutoff, butterOrder);
   
      
   // === determine fit range ===
   

   //start of calculation

   fStartPt = time10 - binSStart;
   fEndPt = fStartPt;
   
   //check validity of fStartPt and fEndPt

   if(fStartPt < 1 || (uint)fStartPt >= aPulse.size())
   {
      return;
   }
      
   // fine tune end of fit 

   if (ceil(0.2*maxADC)>=ceil(4*rmsADC)) 
   { 
      fEndPt = time20 + binAEnd;
   } 
   else if (ceil(0.6*maxADC)<=ceil(4*rmsADC))
   {
      fEndPt =  time60 + binAEnd;
   }
   else 
   {

      int jj = fStartPt+binSStart-binSt0;
      
      while(fEndPt==fStartPt && jj<(time10-binSt0+500))
      {
	 if (aFilteredPulse[jj-1]>ceil(4*rmsADC))
	    fEndPt=jj;
	 jj++;       
      }
   }
   

   // fine tune start of fit + initial value for t0
   
   fParStartVal[2] = 0;
   if (ceil(0.1*maxADC)<ceil(3*rmsADC))
   {
      int jj = fEndPt;
      while(fParStartVal[2] == 0 && jj>(fEndPt-500))
      {
	 if (aFilteredPulse[jj-1]<ceil(3*rmsADC))
	 {
	    fStartPt = jj-binSStart+1;
	    fParStartVal[2] = jj-binSt0+1;
	 }
	 jj=jj-1;
      }
   } else {
      fParStartVal[2] = time10-binSt0;
   }

   // set the other parameters
   fParStartVal[0]=par1Start;
   fParStartVal[1]=-2*fParStartVal[0]*fParStartVal[2]+1;
   fRMS = rmsADC;

   return;

}

         

// ========= Fitting funtion =============

void WedgeFitPhonon::DoCalc(const vector<double>& aPulse)
{ 

   // ==== check for null pulses ====
   if(aPulse.size() == 0)
   {
     cerr <<"WedgeFitPhonon::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	  << endl;
     exit(1);
   }


   // ==== initializations ====
   TString parName[3] = {"par1","par2","par3"}; 
 
   double stepSize =1;
   int ierflg = 0;
   fFitErrFlag = 0;
   double arglist[10]; //can be this many but we're only using the first one
   double b1, b2; //not used, but needed by Minuit
  
   // set global variable
   gStartPt = fStartPt;
   gEndPt = fEndPt;
   gRMS= fRMS;
   gPulse = aPulse;

   //check if starting parameters are valid before doing the fit
   if(gStartPt>0 && (uint)gEndPt <= aPulse.size() && (gEndPt-gStartPt) > (kNumPars-1) )
   {
      // ==== initialize TMinuit with wedgefunc ====
      fMyMinuitInstance = new TMinuit(kNumPars);
      fMyMinuitInstance->SetFCN(wedgefunc);
      
      
      // ==== setting the comment verbosity ====
      fMyMinuitInstance->SetPrintLevel(-1);
      
      
      // ==== pass each fitting parameter to minuit for initialization ====
      for(int parItr = 0; parItr < 3; parItr++)
      {
	 stepSize = fabs(fParStartVal[parItr]/kNormFitStep);
	 fMyMinuitInstance->mnparm(parItr, parName[parItr], fParStartVal[parItr], 
				   stepSize, 0, 0, ierflg);
      }
      
      
      // ==== set strategy ====
      arglist[0]=kStrategy;
      fMyMinuitInstance->mnexcm("SET STRAT", arglist,1,ierflg);
      
      
      // ==== do the fit  ====
      arglist[0] = kMaxMIGRADCalls; // MAX # MIGRAD CALLS
      fMyMinuitInstance->mnexcm("MIGRAD", arglist, 1, fFitErrFlag);
      
      
      // ==== if fit failed, try again ====
      if (fFitErrFlag==4) { 
	 for (int parItr=0; parItr<kNumPars; parItr++) 
	 {
	    
	    // get parameter value
	    fMyMinuitInstance->mnpout(parItr, parName[parItr], fParFitVal[parItr], 
				      fParFitSig[parItr], b1, b2, ierflg);
	    
	    // restart fit using parameter value as starting point
	    stepSize =  fParFitVal[parItr]/kNormFitStep;
	    fMyMinuitInstance->mnparm(parItr, parName[parItr], fParFitVal[parItr], 
				      stepSize, 0, 0, ierflg);
	 }
	 
	 arglist[0] = kMaxMIGRADCalls; // MAX # MIGRAD CALLS
	 fMyMinuitInstance->mnexcm("MIGRAD", arglist, 1, fFitErrFlag);
      }



      // ==== get  fit results ====     
      
      // parameters
      for (int parItr=0; parItr<kNumPars; parItr++)
      {
	 fMyMinuitInstance->mnpout(parItr, parName[parItr], fParFitVal[parItr], fParFitSig[parItr], b1, b2, ierflg);
      }
      
      // ==== cleanup ====
      
      fMyMinuitInstance->mnexcm("STOP" , arglist, 0, ierflg);
      
      // get the chi2 after stopping the fit
      fChi2 = gChi2; 
      fNdof = gNdof;

   } //end if fit start parameters are valid   
   else
   {
      fParFitVal[0] = -999999.;
      fParFitVal[1] = -999999.;
      fParFitVal[2] = -999999.;

      fParFitSig[0] = -999999.;
      fParFitSig[1] = -999999.;
      fParFitSig[2] = -999999.;

      fNdof = 0;
      fFitErrFlag = -999999; //to flag case where inputs are invalid
   }


   // ==== Now fill RQs ====
   
   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      fRQList["WFstart"] = fStartPt;
      fRQList["WFend"] = fEndPt;
      fRQList["WFpar1"] = fParFitVal[0];
      fRQList["WFpar2"] = fParFitVal[1];
      fRQList["WFpar3"] = fParFitVal[2];
      fRQList["WFepar1"] = fParFitSig[0];
      fRQList["WFepar2"] = fParFitSig[1];
      fRQList["WFepar3"] = fParFitSig[2];
      fRQList["WFchisq"] = (fNdof != 0 ? fChi2/(double)fNdof : 999999.); 
      fRQList["WFeflag"] = (double)  fFitErrFlag;    
   }


   return;
}

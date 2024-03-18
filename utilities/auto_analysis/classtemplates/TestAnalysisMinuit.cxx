#include <iostream>

#include "TestAnalysisMinuit.h"
#include "PulseTools.h"

//====================== Beginning some external definitions ======================================

//Definitions here are used by TestAnalysisMinuit but are declared external to 
//it in order to be compatable with TMinuit.  

// === myfitfunc ===
//"static" ensures that myfitfunc has only file scope and prevents multiple declaration errors. 
//myfitfunc is handed off to Minuit.  It calculates chisq for your fit function.  

static void myfitfunc(int &npar, double *gin, double &chi2, double *par, int iflag);

//define variables that need to be shared between myfitfunc and TestAnalysisMinuit
//as statics here (note static names should start with g)
static double gChi2;
static double gStart, gMidpt, gRMS;
static vector<double> gPulse;

// === body of myfitfunc ===
// The parameters accepted by myfitfunc are passed in by Minuit.  Don't modify the argument 
// list in the signature b/c this is what Minuit expects it to look like.   Shown here is an example
// from the PipeFit time domain class.  Replace the body of myfitfunc with your own function.
// Change the name of myfitfunc to whatever please you.  

void myfitfunc(int &npar, double *gin, double &chi2, double *par, int iflag) {

   //PipeFit fitting parameters
   double A    = par[0];  // amplitude 
   double Toff = par[1];  // offset from first time bin 
   double Trf  = par[2];  // rise time 
   double Tf1  = par[3];  // fall time 1 
   double Frac = par[4];  // fraction of fall time 2 to fall time 1 
  
   // calculate chi2 sum
   chi2 = 0.;
   double fval;

   //looping over adc bins
   for (int i= (int) gStart; i<= (int) gMidpt; i++){
    
    fval = A*(1-TMath::Exp(-(i-Toff)/Trf))*(TMath::Exp(-(i-Toff)/Tf1) - Frac*TMath::Exp(-(i-Toff)/Trf));
    chi2 += (fval-gPulse[i])*(fval-gPulse[i])/gRMS/gRMS;
  }
  
  //if done fitting (iflag == 3) then store the chisq so that TestAnalysisMinuit can access the value
  if (iflag==3)
    {
      int ndf = (int) (gMidpt - gStart + 1 - npar);
      gChi2 = chi2;
    }
  
  return;

} //done defining myfitfunc

//=========================== End of External Definitions ==============================================

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
TestAnalysisMinuit::TestAnalysisMinuit()
{
   //   cout <<"Hello from TestAnalysisMinuit()" << endl;

    //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "TestAnalysisMinuit";
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

TestAnalysisMinuit::~TestAnalysisMinuit()
{
//   cout <<"Goodbye from TestAnalysisMinuit()" << endl;
}

//if your RQ isn't here, it won't be stored in BatROOT output!!
//is there a way to choose what you do and don't want in the output?
void TestAnalysisMinuit::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(<pair<string,double>("alpha", initVal));
   fRQList.insert(<pair<string,double>("beta", initVal));
   fRQList.insert(<pair<string,double>("gamma", initVal));   

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//optional function, do it here and not in the constructor
// void TestAnalysisMinuit::InitializeParameters()
// {
//
//    return;
// }

//a simplified example very loosely based on PipeFit is here. Replace the fitting portion with your own code
void TestAnalysisMinuit::DoCalc(const vector<double>& aPulse)
{ 
   //check for null pulses
   if(aPulse.size() == 0)
   {
     cerr <<"TestAnalysis::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	  << endl;
     exit(1);
   }

   // ========= doing the fit! ==============

   //initializations
   TString parName[5] = {"a","b","c","d","e"};  
   double parStartVal[5];
   double parFitVal[5];
   double parSigVal[5];
   double stepsize = 0.1;
   int ierflg = 0;
   int oerflg = 0;
   double arglist[10]; //can be this many but we're only using the first one
   double b1, b2; //not used, but needed by Minuit
   parStartVal[0] = 1.0;
   parStartVal[1] = 1.0;
   parStartVal[2] = 1.0;
   parStartVal[3] = 1.0;
   parStartVal[4] = 1.0;

   // set global variables (replace with your values)
   gPulse = aPulse;
   gStart = 500;
   gMidpt = 600;
   gRMS = PulseTools::Std(aPulse);

   //initialize TMinuit with myfitfunc
   fMyMinuitInstance = new TMinuit(kNumPars);
   fMyMinuitInstance->SetFCN(myfitfunc);

   //setting the comment verbosity - the higher the number the more the comments
   arglist[0] = kMaxMIGRADCalls; // MAX # MIGRAD CALLS
   fMyMinuitInstance->SetPrintLevel(-1);
   
   //loop over parameter list and pass each fitting parameter to minuit for initialization
   for(int parItr = 0; parItr < 5; parItr++)
   {
      fMyMinuitInstance->mnparm(parItr, parName[parItr], parStartVal[parItr], stepsize, 0, 0, ierflg);
   }

   //do the fit 
   fMyMinuitInstance->mnexcm("MIGRAD", arglist, 1, oerflg);

   //get the fit results and err flags
   for (int parItr=0; parItr<kNumPars; parItr++)
   {
     fMyMinuitInstance->mnpout(parItr, parName[parItr], parFitVal[parItr], parSigVal[parItr], b1, b2, ierflg);
   }

  //cleanup - get chisq and dof after stopping
  fMyMinuitInstance->mnexcm("STOP" , arglist, 0, ierflg);

   // ========= done with fit! =================

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      fRQList["alpha"] = 1;
      fRQList["beta"] = 2;
      fRQList["gamma"] = 3;
   }


   return;
}

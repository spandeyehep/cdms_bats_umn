/////////////////////////////////////////////////////////////////////////////////////// 
//Class Name: SingleExponentialFit
//Authors: B. Serfass 
//
//
//Creation Date: 
//
//Modifications:
//
/////////////////////////////////////////////////////////////////////////////////////// 


// linux
#include <iostream>
#include <math.h>

// BatRoot
#include "SingleExponentialFit.h"



// /////////////////////   Beginning some external definitions ///////////////////////

//Definitions here are used by SingleExponentialFit but are declared external to 
//it in order to be compatable with TMinuit.  



// =======  Declare  expFunc ======

// "static" ensures that expFunc has only file scope and prevents multiple declaration errors. 
// expFunc is handed off to Minuit.  It calculates chisq for your fit function.  

static void expFunc(int &npar, double *gin, double &chi2, double *par, int iflag);


//define variables that need to be shared between expFunc and SingleExponentialFit
//as statics here (note static names should start with g)

static double gChi2, gRMS,gdT;
static int gStartPt, gEndPt, gNdof;
 
static vector<double> gPulse;

// =======  body of expFunc =======

// The parameters accepted by expFunc are passed in by Minuit.  Don't modify the argument 
// list in the signature b/c this is what Minuit expects it to look like.  

void expFunc(int &npar, double *gin, double &chi2, double *par, int iflag) {

   
   // ---- Fit  parameters ---
   double par1 = par[0]; // amplitude
   double par2 = par[1]; // exponential rate (1/ Tau_Fall)
   double par3 = par[2]; // baseline
   
 
   //--- looping over adc bins ----

   chi2 = 0.;
   double fval;

   for (int i= (int) gStartPt; i<= (int) gEndPt; i++){
    
      double pulseADC = gPulse[i-1];
      
      fval = par1*exp((i-gStartPt)*gdT*par2) + par3;
      chi2 += (fval-pulseADC)*(fval-pulseADC)/gRMS/gRMS;
    
    }
  


  // ---- store chi2 ----- 

  //  if done fitting (iflag == 3) then store the chisq so that SingleExponentialFit 
  //  can access the value

  if (iflag==3)
    {
      int ndf = (int) (gEndPt - gStartPt + 1 - npar);
      gNdof = ndf;
      gChi2 = chi2;
    }
  
  return;

} //done defining expFunc

// ///////////////////// End of External Definitions /////////////////////////////////




// constructor
SingleExponentialFit::SingleExponentialFit(const string& className)
{
   //   cout <<"Hello from SingleExponentialFit()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = className; 
   fStoreRQs = true;

   if (fClassName.find("TailFitPhonon")!=string::npos)
	fAnalysisInitials = "TFP";
   else 
        fAnalysisInitials = "TF";

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

   // Default: no constrain
   for (int parItr=0; parItr<kNumPars; parItr++)
     { 
       fParConstraintFlag[parItr] = 0; 
       fParMin[parItr]  = 0.; 
       fParMax[parItr]  = 0.; 
     }

}



SingleExponentialFit::~SingleExponentialFit()
{
//   cout <<"Goodbye from SingleExponentialFit()" << endl;
}




//if your RQ isn't here, it won't be stored in BatROOT output!!
void SingleExponentialFit::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>(fAnalysisInitials + "amp", initVal));
   fRQList.insert(pair<string,double>(fAnalysisInitials + "tau", initVal));
   fRQList.insert(pair<string,double>(fAnalysisInitials + "offset", initVal));
   fRQList.insert(pair<string,double>(fAnalysisInitials + "chisq", initVal));
   fRQList.insert(pair<string,double>(fAnalysisInitials + "eflag", initVal));
   fRQList.insert(pair<string,double>(fAnalysisInitials + "int", initVal));


   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

  

// initialize fit parameters
void SingleExponentialFit::InitializeFitParameters(double par1Init, double par2Init, double par3Init, double RMS)
{
   
   // ====== chi2 =======
   fNdof = -999999;
   fChi2 = -999999.;
   fFitErrFlag = -999999;
   
   fRMS = RMS;



   // ====== initialize fit parameters ======

   for (int ii=0;ii<3;ii++) {
     fParFitVal[ii] = -999999.;
     fParFitSig[ii] = -999999.;
     }
   
   fParStartVal[0]= par1Init;
   fParStartVal[1]= par2Init; 
   fParStartVal[2]= par3Init;

     
  
   return;

}


         

// ========= Fit  =============

void SingleExponentialFit::DoCalc(const vector<double>& aPulse)
{ 

   // ==== check for null pulses ====
   if(aPulse.size() == 0)
   {
     cerr <<"SingleExponentialFit::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
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
   gdT = fdT;

   //check if starting parameters are valid before doing the fit
   if(gStartPt>0 && gEndPt <= (int)aPulse.size() && (gEndPt-gStartPt) > (kNumPars-1) )
   {
      // ==== initialize TMinuit with expFunc ====
      fMyMinuitInstance = new TMinuit(kNumPars);
      fMyMinuitInstance->SetFCN(expFunc);
      
   
      // ==== setting the comment verbosity ====
      fMyMinuitInstance->SetPrintLevel(-1);
      
      
      // ==== pass each fitting parameter to minuit for initialization ====
      for(int parItr = 0; parItr < 3; parItr++)
      {

         // initialize
	 stepSize = fabs(fParStartVal[parItr]/kNormFitStep);
	 fMyMinuitInstance->mnparm(parItr, parName[parItr], fParStartVal[parItr], 
				   stepSize, fParMin[parItr], fParMax[parItr], ierflg);
     
        // fix parameter
        if(fParConstraintFlag[parItr]==2) {
          arglist[0]=parItr+1;
          fMyMinuitInstance->mnexcm("FIX", arglist,1,ierflg);
        }
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
				      stepSize, fParMin[parItr], fParMax[parItr], ierflg);

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
      fRQList[fAnalysisInitials + "amp"] = fParFitVal[0];
      fRQList[fAnalysisInitials + "tau"] = -1/fParFitVal[1];
      fRQList[fAnalysisInitials + "offset"] = fParFitVal[2];
      fRQList[fAnalysisInitials + "chisq"] = (fNdof != 0 ? fChi2/(double)fNdof : 999999.); 
      fRQList[fAnalysisInitials + "eflag"] = (double)  fFitErrFlag;    
      fRQList[fAnalysisInitials + "int"] = -fParFitVal[0]/fParFitVal[1];
   }


   return;
}

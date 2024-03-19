///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterCharge
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on charge pulses.  It does *not* perform the cross talk calculation.  This code was 
//  written for endcap charge channels.
// 
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: L. Hsu
//Creation Date: Jun. 6, 2009
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 



#include <iostream>
#include <limits>

#include "OptimalFilterCharge.h"
#include "PulseTools.h"

//only needed for testing
#include "TFile.h"
#include "TTree.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterCharge::OptimalFilterCharge() :
  fVolts(-999999.),
  fVolts0(-999999.),
  fDelay(-999999.),
  fChisq(-999999.),
  fdT(0.8e-6),
  fwindow1(-999999),
  fwindow2(-999999),
  fBias(-999999.),
  fTemplatesLoaded(false),
  fNormalizationsLoaded(false),
  fBiasVoltagesSet(false),
  fNBinsTemplates(0)

{

   //   cout <<"Hello from OptimalFilterCharge()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "OptimalFilterCharge"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

OptimalFilterCharge::~OptimalFilterCharge()
{
//   cout <<"Goodbye from OptimalFilterCharge()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
void OptimalFilterCharge::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here (-999999. indicates normal channel prefixes)
   fRQList.insert(pair<string,double>("OFnoXvolts", initVal));
   fRQList.insert(pair<string,double>("OFnoXvolts0", initVal));
   fRQList.insert(pair<string,double>("OFnoXdelay", initVal));  
   fRQList.insert(pair<string,double>("OFnoXchisq", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.
   
   return;
}


//This is the main call for this analysis
void OptimalFilterCharge::DoCalc(const vector<double>& aPulse)
{
//   cout <<"Hello from OptimalFilterCharge::DoCalc!" << endl;

   //do your calculation here!
   int nBins = aPulse.size();
   double sqrtdT = sqrt(fdT); //to save a little processing time


   //============== Some Preliminary Checks =================

   if(!fTemplatesLoaded || !fNormalizationsLoaded ) 
   {
     cerr <<"OptimalFilterCharge::ERROR!  attempting to run OptimalFilter without loading templates + normalizations + matrix!" << endl;
     exit(1);
   }

   //checking that the lengths of templates agree with the pulse lengths
   if(nBins != fNBinsTemplates) 
   { 
      cerr<<"OptimalFilterCharge::ERROR! Number of bins in pulse does not match number of bins in templates!" << endl; 
      exit(1);
   }

   //checking that the fit window is valid
   if(fwindow1 < 0 || fwindow2 < 0) 
   {
     cerr <<"OptimalFilterCharge::ERROR! Fit windows appear to be uninitialized" << endl; 
     exit(1);
   }


   //================== Calculations =====================
   TComplex comp_zero(0.,0.);

   //vectors for intermediate calculations
   vector<TComplex> pulseFFT;   
   vector<TComplex> pProd;
   vector<double> p_prod_ifftRe;

   //1. construct fitpulse fft's
   PulseTools::RealToComplexFFT(aPulse, pulseFFT);
   double amp0 = 0.;


   //2. construct p_prod = sqrt(dt) * pulse_fft * s*/J
   for(int binItr=0; binItr < nBins; binItr++)
   {
      pulseFFT[binItr] *= sqrtdT; 
      pProd.push_back(pulseFFT[binItr]*fOptimalFilter[binItr]); 
      
      //eventually we may just want to take the amplitude from the IFFT, they should be equiv.
      if(binItr != 0) amp0 += pProd[binItr].Re(); 
   } 

   //normalizing amp0's
   amp0 /= fSigToNoiseSq; 

   //ignoring DC component;
   pProd[0] = comp_zero; 


   //3. get delay from max amplitude
   vector<double> qi_p_ahat; //vector of all amplitudes, eventually restrict the search window
   double maxAmp = -1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
   double finalAmp = 0.0;
   int idelay = 0;
   
   //C2R - no imaginary component!    
   PulseTools::ComplexToRealIFFT(pProd, p_prod_ifftRe);  
   
   //pick out maximum amplitude and corresponding delay
   //there are two allowed windows due to the starting position of the pulse template
   //one window is at the very beginning of the digitizer window, and the other at 
   //the very end.  We skip the bins in between
   for(int binItr=0; binItr < nBins; binItr++)
   {
      //Note that there is no division by norm_fftsq b/c we're only getting delay!
      double amp =  p_prod_ifftRe[binItr];

      //special 0 bias voltage case - I don't fully understand how this works
      if(fBiasVoltagesSet && fBias == 0.0)
	 amp =  fabs(p_prod_ifftRe[binItr]);

      //storing the maximum amplitude
      if(amp > maxAmp) 
      { 
	 maxAmp = amp;
	 finalAmp = p_prod_ifftRe[binItr]/fNormFFT;
	 idelay = binItr; 
      }
	 
      //search up to fwindow1, and after fwindow2
      if(binItr == fwindow1 - 1)  
	 binItr = fwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
      
   }
   
//     cout <<"Max amp = " << maxAmp <<", at bin = " << idelay 
// 	 <<"\n amp = " << finalAmp
// 	 << endl;

//    cout <<"\namp0's as they're computed for phonons:"
// 	<<"\n amp0 = " << p_prod_ifftRe[fTrigTime]/fNormFFT
// 	<<"\namp0's as they're computed here:"
// 	<<"\n amp0 = " << amp0/fSigToNoiseSq
// 	<< endl;

   //compute phase factor with this delay
   int delay = (idelay < nBins/2 ? idelay : (idelay - nBins)); 

   
   //4. compute chisq
   double chisq = 0.0;

   for(int binItr=1; binItr < nBins; binItr++)
   {
      double theta = 2.0*TMath::Pi()*((double)binItr/(double)nBins)*(double)delay;

      TComplex phase_factor(cos(theta), sin(theta));
      TComplex fit_fft( finalAmp*fPulseTemplateFFT[binItr]/phase_factor );

      chisq += pow(TComplex::Abs(pulseFFT[binItr] - fit_fft), 2)/fNoiseFFTSq[binItr];

   }
   

   
   // ========= Next, store the results of this calculation as the RQ's.  ===========

   fVolts = finalAmp;
   fVolts0 = amp0;
   
   fDelay = delay*fdT; //delay time is relative to global trigger (offset is applied in the template), -1 is for c++ indexing convention
   fChisq = chisq;

   fRQList["OFnoXvolts"] = fVolts;
   fRQList["OFnoXvolts0"] = fVolts0;
   fRQList["OFnoXdelay"] = fDelay;
   fRQList["OFnoXchisq"] = fChisq;

   
   // ========== cleanup! ===========
   fTemplatesLoaded = false;
   fNormalizationsLoaded = false;


   return;
}

void OptimalFilterCharge::LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter)
{
   //these templates should all have the same lengths
   fNBinsTemplates = pulseTemplateFFT.size(); 

   if((int)optimalFilter.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterCharge::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
      exit(1);
   }


   fPulseTemplateFFT = pulseTemplateFFT; 
   fOptimalFilter = optimalFilter;


   fTemplatesLoaded = true;

   return;
}


void OptimalFilterCharge::LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTSq, 
					     const double& templateMax)
{
   //check that vector lengths match
   if((int)noiseFFTSq.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterCharge::ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
      exit(1);
   }


   fNormFFT = normFFT;
   fSigToNoiseSq = sigToNoiseSq;
   fNoiseFFTSq = noiseFFTSq;
   fTemplateMax = templateMax;


   fNormalizationsLoaded = true;

   return;
}


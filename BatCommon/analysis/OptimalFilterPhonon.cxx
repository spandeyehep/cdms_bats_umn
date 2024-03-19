///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterPhonon
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on single pulse  (based on the DarkPipe code)
// 
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 




#include <iostream>
#include <limits>

#include "TFile.h"
#include "TTree.h"

#include "OptimalFilterPhonon.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterPhonon::OptimalFilterPhonon(const string& className) :
  fAmp(-999999.),
  fAmp0(-999999.),
  fChisq(-999999.),
  fChisqLF(-999999.),
  fDelay(-999999.),
  fdT(0.8e-6),
  fwindow1(-999999),
  fwindow2(-999999),
  fCutoffFreq(-999999.),
  fTemplatesLoaded(false),
  fNormalizationsLoaded(false),	
  fNBinsTemplates(0)
{

   //   cout <<"Hello from OptimalFilterPhonon()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = className; 
   fStoreRQs = true;

   if (fClassName.find("OptimalFilterPhononGlitch1")!=string::npos)
	fAnalysisInitials = "glitch1OF";
   else if (fClassName.find("OptimalFilterPhononPileup")!=string::npos)
        fAnalysisInitials = "pileupOF";
   else if (fClassName.find("OptimalFilterPhononLFnoise1")!=string::npos)
        fAnalysisInitials = "lfnoise1OF";
   else if (fClassName.find("OptimalFilterPhononDMC")!=string::npos)
        fAnalysisInitials = "dmcOF";
   else
        fAnalysisInitials = "OF";


   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

OptimalFilterPhonon::~OptimalFilterPhonon()
{
//   cout <<"Goodbye from OptimalFilterPhonon()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void OptimalFilterPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>( fAnalysisInitials + "amps",  initVal));
   fRQList.insert(pair<string,double>( fAnalysisInitials + "amps0", initVal));
   fRQList.insert(pair<string,double>( fAnalysisInitials + "chisq", initVal));
   fRQList.insert(pair<string,double>( fAnalysisInitials + "chisqLF", initVal));
   fRQList.insert(pair<string,double>( fAnalysisInitials + "delay", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//This is the main call 
void OptimalFilterPhonon::DoCalc(const vector<double>& aPulse)
{
   //do your calculation here!

   //for debugging only
   //ConstructFakePulse(0.5e9, 525);

   int nBins = aPulse.size();
   double sqrtdT = sqrt(fdT); //for efficiency

   //============== Some Preliminary Checks =================

   if(!fTemplatesLoaded) 
   {
     cerr <<"OptimalFilterPhonon::ERROR!  attempting to run OptimalFilter without loading templates!" << endl;
     exit(1);
   }

   //checking that the lengths of templates agree with the pulse lengths
   if(nBins != fNBinsTemplates) 
   { 
      cerr<<"OptimalFilterPhonon::ERROR! Number of bins in pulse does not match number of bins in templates!" << endl; 
      exit(1);
   }

   //checking that the fit window is valid
   if(fwindow1 < 0 || fwindow2 < 0) 
   {
     cerr <<"OptimalFilterPhonon::ERROR! Fit window appears to be uninitialized!" << endl; 
     exit(1);
   }

   //================== Calculations =====================
   TComplex comp_zero(0.,0.);

   //vectors to hold intermediate calculations
   vector<TComplex> pProd;
   vector<TComplex> pulseFFT;
   vector<double> p_prod_ifftRe;

   //===== 1. construct fitpulse fft =====

   //PulseTools::RealToComplexFFT(fFakePulse, re_PulseFFT, im_PulseFFT); //for testing only
   PulseTools::RealToComplexFFT(aPulse, pulseFFT);
   double amp0 = 0.;

   //===== 2. construct p_prod = sqrt(fdT) * pulse_fft * s*/J =====

   for(int binItr=0; binItr < nBins; binItr++)
   {
     pulseFFT[binItr] *= sqrtdT;
     pProd.push_back(pulseFFT[binItr]*fOptimalFilter[binItr]);

     if(binItr != 0) amp0 += pProd[binItr].Re(); 

   } 

   //normalizing amp0's
   amp0 /= fSigToNoiseSq;
 
   //ignoring DC component;
   pProd[0] = comp_zero; 

   //===== 3. get max amplitude and delay =====

   vector<double> p_ahat; //vector of all amplitudes
   double maxAmp = -1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
   double finalAmp = 0.0; 
   int idelay = 0;

   PulseTools::ComplexToRealIFFT(pProd, p_prod_ifftRe);  //C2R - no imaginary component!    


   //pick out maximum amplitude and corresponding delay     
   for(int binItr=0; binItr < nBins; binItr++) {
    
    double amp =  p_prod_ifftRe[binItr];

    if(amp > maxAmp) { 
        maxAmp = amp; 
        finalAmp = p_prod_ifftRe[binItr]/fNormFFT;  
        idelay = binItr; }

     //search up to fwindow1, and after fwindow2
      if(binItr == fwindow1 - 1)  
	 binItr = fwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
   }      

    //compute phase factor with this delay
   int delay = (idelay < nBins/2 ? idelay : (idelay - nBins)); 
  

   //===== 4. compute chisq =====

   double chisq = 0; 
   double chisqLF = 0;

   // find bins for computation of  chi2 with low frequency cutoff
   double dnu = 1/(fdT*nBins); // frequency spacing
   int binCutPos = (int) floor(fCutoffFreq/dnu);   // positive frequencies
   int binCutNeg = (int) (nBins - ceil(fCutoffFreq/dnu) + 1); // negative frequencies
       

   // check cutoff range
   bool doCalcChisqLF = true;
   
   if (binCutPos > nBins/2   ||  binCutPos<0 || 
       binCutNeg > (nBins-1) ||  binCutNeg<(nBins/2+1)) {
        chisqLF = -999999.;
        doCalcChisqLF = false;
    }
        

   //ignoring DC component
   for(int binItr=1; binItr < nBins; binItr++)
     {
       double theta = 2.0*TMath::Pi()*((double)binItr/(double)nBins)*(double)delay;

       TComplex phase_factor(cos(theta), sin(theta));
       TComplex fit_fft( finalAmp*fPulseTemplateFFT[binItr]/phase_factor );
  
       double chisqBin =  pow(TComplex::Abs(pulseFFT[binItr] - fit_fft), 2)/fNoiseFFTSq[binItr];
       chisq += chisqBin;

       if (binItr<=binCutPos && doCalcChisqLF) 
         chisqLF += chisqBin;
      
       if (binItr>=binCutNeg && doCalcChisqLF)
         chisqLF += chisqBin;
      
     }
  

   //================== Store Results ====================

   //this is a little redundant, do I want to keep this?
   fAmp = finalAmp;
   fAmp0 = amp0;
   fChisq = chisq;
   fChisqLF = chisqLF;
   fDelay = delay*fdT;

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      fRQList[fAnalysisInitials + "amps"] = fAmp;
      fRQList[fAnalysisInitials + "amps0"] = fAmp0;
      fRQList[fAnalysisInitials + "chisq"] = fChisq;
      fRQList[fAnalysisInitials + "chisqLF"] = fChisqLF;
      fRQList[fAnalysisInitials + "delay"] = fDelay;
   }

   //============= Delete Templates to minimize copying  ================

   fPulseTemplate.clear();
   fNoiseFFTSq.clear();
   fPulseTemplateFFT.clear();
   fOptimalFilter.clear();

   // ========== cleanup! ===========
   fTemplatesLoaded = false;
   fNormalizationsLoaded = false;

   return;
}



void OptimalFilterPhonon::LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter)
{
   //these templates should all have the same lengths
   fNBinsTemplates = pulseTemplateFFT.size(); 

   if((int)optimalFilter.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterPhonon::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
      exit(1);
   }


   fPulseTemplateFFT = pulseTemplateFFT; 
   fOptimalFilter = optimalFilter;


   fTemplatesLoaded = true;

   return;
}


void OptimalFilterPhonon::LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTSq)
{
   //check that vector lengths match
   if((int)noiseFFTSq.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterPhonon::ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
      exit(1);
   }


   fNormFFT = normFFT;
   fSigToNoiseSq = sigToNoiseSq;
   fNoiseFFTSq = noiseFFTSq;
  

   fNormalizationsLoaded = true;

   return;
}



void OptimalFilterPhonon::LoadCutoffFreq(const double& cutoffFreq)
{
   //check cutoffFreq positive value
   if(cutoffFreq<0)
   {
      cerr <<"OptimalFilterPhonon::ERROR!  Cutoff frequency should be positive value" << endl;
      exit(1);
   }

   fCutoffFreq = cutoffFreq;  

   return;
}







//================= Temporary code for testing an development =========================== 

void OptimalFilterPhonon::ConstructFakePulse(double norm, int delay)
{
  if(!fTemplatesLoaded) { cerr <<"ERROR::Forgot to load templates!" << endl; }

     //Temp! using the template pulse as the data
   TFile f("testing_only/170319_1616_F0002_noisetrace.root");
   if(!f.IsOpen()) { cerr <<"ERROR opening test noise file, check path!" << endl; exit(1); }
   TTree* pulseTree = (TTree*)f.Get("pulseTree");

   const int nBins = 2048; //for soudan data only!

   //Set Branch Addresses
   float noise[nBins];
   pulseTree->SetBranchAddress("bsnPulse", noise);
   pulseTree->GetEntry(0);
   f.Close();
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
     if(binItr < delay)
       fFakePulse.push_back((double)noise[binItr]);
     else
       fFakePulse.push_back((double)noise[binItr] + norm*fPulseTemplate[binItr-delay]);
   }

   return;
}
 

///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterCharge
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on charge pulses, taking into account cross-talks  (based on the DarkPipe code)
// 
// Original author of the darkpipe code:  R. Schnee, with modifications from 
//                                        J. Cooley and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//Feb. 2009 - Initial import.  This first import behaves identially to old DP implementation
//Feb. 2010 - Added full chisq minimization calculation and the ability to choose
//            between the chisq minimization (slow) or the old amplitude max (fast)
//            routines, based on a threshold that is implemented through the config
//            interface (LLH).
//Feb. 2010 - Added delay and chisq interpolation based on simple parabolic fit to 
//            delay that yields minimum chisq and the two bins on either side of it.
//            This routine is only performed for the case of full chisq minimization.
//            There is also a separate flag to toggle the interpolation on and off.
//            Delays that are at the edge of the allowed window are not interpolated (LLH).
//
////////////////////////////////////////////////////////////////////////////////// 



#include <iostream>
#include <limits>

#include "OptimalFilterChargeX.h"
#include "PulseTools.h"

//only needed for testing
#include "TFile.h"
#include "TTree.h"
#include "TMatrixD.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterChargeX::OptimalFilterChargeX() :
  fQOvolts(-999999.),
  fQOvolts0(-999999.),
  fQIvolts(-999999.),
  fQIvolts0(-999999.),
  fDelay(-999999.),
  fChisq(-999999.),
  fInterDelay(-999999.), 
  fInterChisq(-999999.),
  fDiscreteDelay(-999999.), 
  fDiscreteChisq(-999999.),
  fdT(0.8e-6),
  fQxwindow1(-999999),
  fQxwindow2(-999999),
  fBiasQI(-999999.),
  fBiasQO(-999999.), 
  fMinQI(-999999.),
  fMinQO(-999999.),
  fDoFullChisqMin(0),
  fIsRandom(-999999),
  fDoDelayInterpolation(-999999),
  fTemplatesLoaded(false),
  fNormalizationsLoaded(false),
  fMatrixLoaded(false),
  fBiasVoltagesSet(false),
  fNBinsTemplates(0)

{

   //   cout <<"Hello from OptimalFilterChargeX()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "OptimalFilterChargeX"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

OptimalFilterChargeX::~OptimalFilterChargeX()
{
//   cout <<"Goodbye from OptimalFilterChargeX()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//Note: the RQ readout for this class is slightly specialized because some RQ's are 
//shared by both the QI and QO pulses
void OptimalFilterChargeX::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here (-999999. indicates normal channel prefixes)
   fRQList.insert(pair<string,double>("OFvolts", initVal));
   fRQList.insert(pair<string,double>("OFvolts0", initVal));

   //the initial value flags this so BatOutputManager will append Prefix "QS" *instead* of QI/QO
   //with this option activated you cannot have RQ's named both QSOFdelay and QIOFdelay/QOOFdelay!
   fRQList.insert(pair<string,double>("OFdelay", -123456.));  
   fRQList.insert(pair<string,double>("OFchisq", -123456.));
   fRQList.insert(pair<string,double>("OFisMinChisq", -123456.));
   fRQList.insert(pair<string,double>("OFdiscreteDelay", -123456.));   
   fRQList.insert(pair<string,double>("OFdiscreteChisq", -123456.));   
   

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.
   
   return;
}

//needed for this class because one instance belongs to more than one channel
void OptimalFilterChargeX::StoreAs(const string& chanType)
{
  if(chanType != "QI" && chanType != "QO")
    cout <<"OptimalFilterCharge::WARNING! incorrect chanType passed to StoreAs function, storing QI values" << endl;

  if(chanType == "QO")
  {
    fRQList["OFvolts"] = fQOvolts;
    fRQList["OFvolts0"] = fQOvolts0;
  }
  else
  {
    fRQList["OFvolts"] = fQIvolts;
    fRQList["OFvolts0"] = fQIvolts0;
  }

  //these will be identical between the QI and QO versions that are stored in PulseData
  //only one copy will be saved under channel prefix "QS"

  fRQList["OFdelay"] = fDelay;
  fRQList["OFchisq"] = fChisq;
  fRQList["OFisMinChisq"] = (double)fDoFullChisqMin;

  fRQList["OFdiscreteDelay"] = fDiscreteDelay;
  fRQList["OFdiscreteChisq"] = fDiscreteChisq;


  return;
}

//This is the main call for this analysis
void OptimalFilterChargeX::DoCalc(const vector<double>& aPulseQI, const vector<double>& aPulseQO)
{
   //do your calculation here!
   int nBins = aPulseQI.size();
   double sqrtdT = sqrt(fdT); //to save a little processing time

   //============== Some Preliminary Checks =================

   if(aPulseQI.size() != aPulseQO.size())
   {
     cerr <<"OptimalFilterChargeX::ERROR!  QI and QO pulses are not the same length, check input vectors!" << endl;
     exit(1);
   }

   if(!fTemplatesLoaded || !fNormalizationsLoaded || !fMatrixLoaded) 
   {
     cerr <<"OptimalFilterChargeX::ERROR!  attempting to run OptimalFilter without loading templates + normalizations + matrix!" << endl;
     exit(1);
   }

   //checking that the lengths of templates agree with the pulse lengths
   if(nBins != fNBinsTemplates) 
   { 
      cerr<<"OptimalFilterChargeX::ERROR! Number of bins in pulse does not match number of bins in templates!" << endl; 
      exit(1);
   }

   //checking that the fit window is valid
   if(fQxwindow1 < 0 || fQxwindow2 < 0) 
   {
     cerr <<"OptimalFilterChargeX::ERROR! Fit windows appear to be uninitialized" << endl; 
     exit(1);
   }

   if(fMinQI == -999999 || fMinQO == -999999)
   {
      cerr <<"OptimalFilterChargeX::ERROR!  The cutoff thresholds for performing chisq calculation are not set" << endl;
      exit(1);
   }

   if(fIsRandom == -999999)
   {
      cerr <<"OptimalFilterChargeX::ERROR!  IsRandom flag not set" << endl;
      exit(1);
   }

   if(fDoDelayInterpolation == -999999)
   {
      cerr <<"OptimalFilterChargeX::ERROR!  DoDelayInterpolation flag not set" << endl;
      exit(1);
   }

   //================== Calculations =====================
   TComplex comp_zero(0.,0.);

   //vectors for intermediate calculations
   vector<TComplex> pulseFFTQI;
   vector<TComplex> pulseFFTQO;
   
   vector<TComplex> pProdQI;
   vector<TComplex> pProdQO;

   vector<double> qi_p_prod_ifftRe;
   vector<double> qo_p_prod_ifftRe;

   //1. construct fitpulse fft's
   PulseTools::RealToComplexFFT(aPulseQI, pulseFFTQI);
   PulseTools::RealToComplexFFT(aPulseQO, pulseFFTQO);

   double amp0QI = 0.;
   double amp0QO = 0.;

   //2. construct p_prod = sqrt(dt) * pulse_fft * s*/J
   for(int binItr=0; binItr < nBins; binItr++)
   {
      pulseFFTQI[binItr] *= sqrtdT;
      pulseFFTQO[binItr] *= sqrtdT;

      pProdQI.push_back(pulseFFTQI[binItr]*fOptimalFilterQI[binItr]);
      pProdQO.push_back(pulseFFTQO[binItr]*fOptimalFilterQO[binItr]);      

      //should this include the dc component? - I don't understand this calculation
      //eventually we may just want to take the amplitude from the IFFT, they should be equiv.
      if(binItr != 0) amp0QI += pProdQI[binItr].Re();
      if(binItr != 0) amp0QO += pProdQO[binItr].Re();
   } 

   //normalizing amp0's
   amp0QI /= fSigToNoiseSqQI; 
   amp0QO /= fSigToNoiseSqQO; 

   //ignoring DC component;
   pProdQI[0] = comp_zero;
   pProdQO[0] = comp_zero;

   //3. get delay from max amplitude - at this point we are fitting qi and qo separately!!
   double maxAmp = -1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
   double ampQI = 0.0;
   double ampQO = 0.0;
   int idelay = 0;
   
   //C2R - no imaginary component!    
   PulseTools::ComplexToRealIFFT(pProdQI, qi_p_prod_ifftRe); 
   PulseTools::ComplexToRealIFFT(pProdQO, qo_p_prod_ifftRe);  
   
   //Looping over a subset of delays in the pulse.
   //Pick out maximum amplitude and corresponding delay from the inverse FFT.
   //In "shortcut mode", the delay corresponding to the max amplitude is the chosen delay.
   //In "full chisq minimization mode", we compute the chisq for each delay and choose the
   //delay corresponding to the minimum chisq in the range.   
   //There are two allowed windows due to the starting position of the pulse template
   //one window is at the very beginning of the digitizer window, and the other at 
   //the very end.  We skip the bins in between

   for(int binItr=0; binItr < nBins; binItr++)
   {
      //Note that there is no division by norm_fftsq b/c we're only getting delay!
      double amp =  qi_p_prod_ifftRe[binItr] + qo_p_prod_ifftRe[binItr];
      
      //special 0 bias voltage case - I don't fully understand how this works
      if(fBiasVoltagesSet && (fBiasQI == 0.0 || fBiasQI != fBiasQO))
 	 amp =  fabs(qi_p_prod_ifftRe[binItr]) + fabs(qo_p_prod_ifftRe[binItr]);
      
      //storing the maximum amplitude and corresponding delay
      if(amp > maxAmp) 
      { 
 	 maxAmp = amp;
 	 ampQI = qi_p_prod_ifftRe[binItr]/fNormFFTQI;
 	 ampQO = qo_p_prod_ifftRe[binItr]/fNormFFTQO;
 	 idelay = binItr; 
      }
      
      //search up to fQxwindow1, and after fQxwindow2
      if(binItr == fQxwindow1 - 1)  
	 binItr = fQxwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
      
   } //end loop for "max amp" delay calculation

   //compute phase factor with this delay
   int delay = (idelay < nBins/2 ? idelay : (idelay - nBins)); 
   
   //4. compute solution to system of two equations to get correct cross-talk contribution
   double rhs1 = 0.0;
   double rhs2 = 0.0;

   vector<TComplex> maxAmpPulseFFTQI = pulseFFTQI; //temporary container
   vector<TComplex> maxAmpPulseFFTQO = pulseFFTQO; //temporary container
   
   for(int binItr=0; binItr < nBins; binItr++)
   {
      double theta = 2.0*TMath::Pi()*((double)binItr/(double)nBins)*(double)delay;
      TComplex phase_factor(cos(theta), sin(theta));
      
      maxAmpPulseFFTQI[binItr] *= phase_factor;
      maxAmpPulseFFTQO[binItr] *= phase_factor;
      
      TComplex temp1 = maxAmpPulseFFTQI[binItr]*fOptimalFilterQI[binItr] + maxAmpPulseFFTQO[binItr]*fOptimalFilterQOX[binItr];
      TComplex temp2 = maxAmpPulseFFTQO[binItr]*fOptimalFilterQO[binItr] + maxAmpPulseFFTQI[binItr]*fOptimalFilterQIX[binItr];
      
      rhs1 += temp1.Re();
      rhs2 += temp2.Re();
   }
   
   //multiplying solutions with inverse of "chisq" matrix - this is a symmetric matrix
   //doing this explicitely because ROOT doesn't support matrix operations with complex numbers
   double sol1 = (fQInverse[0]*rhs1 + fQInverse[1]*rhs2);
   double sol2 = (fQInverse[2]*rhs1 + fQInverse[3]*rhs2);
   
   
   //5. compute chisq 
   double chisq = 0;        //ignore DC component?
   
   //Do we want the DC component?
   for(int binItr=1; binItr < nBins; binItr++)
   {
      TComplex tempArgQI = maxAmpPulseFFTQI[binItr] - (sol1*fPulseTemplateFFTQI[binItr] + sol2*fPulseTemplateFFTQIX[binItr]);
      TComplex tempArgQO = maxAmpPulseFFTQO[binItr] - (sol2*fPulseTemplateFFTQO[binItr] + sol1*fPulseTemplateFFTQOX[binItr]);

      chisq += tempArgQI.Rho2()/fNoiseFFTSqQI[binItr];
      chisq += tempArgQO.Rho2()/fNoiseFFTSqQO[binItr];
   }
   

   // ==== 6. Decide whether to stop with approximate delay or go on to full chisq min ====
   

   fDoFullChisqMin = 0;

   //is amplitude in QI above threshold?
   if(sol1*fTemplateMaxQI > fMinQI && fMinQI != 999999)
   {
      fDoFullChisqMin = 1;
   }
   

   //is amplitude in QO above threshold?
   if(sol2*fTemplateMaxQO > fMinQO && fMinQO != 999999)
   {
      fDoFullChisqMin = 1;
   }


   //is it a random?
   if(fIsRandom && (fMinQI != 999999 && fMinQO != 999999))
   {
      fDoFullChisqMin = 1;
   }

//       if(fDoFullChisqMin == 1)
//       {
// 	 cout <<"\nfMinQI = " << fMinQI <<", fMinQO = " << fMinQO
// 	      << endl;
//       }
//       else
//       {
// 	 cout <<"\nskip!" << endl;
//       }

   // ==== 7. Now go on and do the full chisq minimization calc ====
   //       i.e. repeat steps 4 & 5 for all delays in window 
   //       after finding minimum delay, do the delay interpolation
   
   if(fDoFullChisqMin)
   {
      double minChisq = numeric_limits<double>::infinity(); 
      
      //reset these values 
      chisq = 0.0; 
      sol1 = 0.0;
      sol2 = 0.0;
      delay = 0;

      for(int binItr=0; binItr < nBins; binItr++)
      {
	 int tempdelay = (binItr < nBins/2 ? binItr : (binItr - nBins));  
      
	 //temp4. compute solution to system of two equations to get correct cross-talk contribution
	 double temprhs1 = 0.0;
	 double temprhs2 = 0.0;
	 
	 vector<TComplex> minChisqPulseFFTQI = pulseFFTQI; //temporary container
	 vector<TComplex> minChisqPulseFFTQO = pulseFFTQO; //temporary container
	 
	 for(int tempbinItr=0; tempbinItr < nBins; tempbinItr++)
	 {
	    double theta = 2.0*TMath::Pi()*((double)tempbinItr/(double)nBins)*(double)tempdelay;
	    TComplex phase_factor(cos(theta), sin(theta));
	    
	    minChisqPulseFFTQI[tempbinItr] *= phase_factor;
	    minChisqPulseFFTQO[tempbinItr] *= phase_factor;
      
	    TComplex temp1 = minChisqPulseFFTQI[tempbinItr]*fOptimalFilterQI[tempbinItr] + minChisqPulseFFTQO[tempbinItr]*fOptimalFilterQOX[tempbinItr];
	    TComplex temp2 = minChisqPulseFFTQO[tempbinItr]*fOptimalFilterQO[tempbinItr] + minChisqPulseFFTQI[tempbinItr]*fOptimalFilterQIX[tempbinItr];
      
	    temprhs1 += temp1.Re();
	    temprhs2 += temp2.Re();
	 }
      
	 //multiplying solutions with inverse of "chisq" matrix - this is a symmetric matrix
	 //doing this explicitely because ROOT doesn't support matrix operations with complex numbers
	 double tempsol1 = (fQInverse[0]*temprhs1 + fQInverse[1]*temprhs2);
	 double tempsol2 = (fQInverse[2]*temprhs1 + fQInverse[3]*temprhs2);
	 
	 //temp5. compute chisq 
	 double tempchisq = 0;        //ignore DC component?
      
	 //Do we want the DC component?
	 for(int tempbinItr=1; tempbinItr < nBins; tempbinItr++)
	 {
	    TComplex tempArgQI = minChisqPulseFFTQI[tempbinItr] - (tempsol1*fPulseTemplateFFTQI[tempbinItr] + tempsol2*fPulseTemplateFFTQIX[tempbinItr]);
	    TComplex tempArgQO = minChisqPulseFFTQO[tempbinItr] - (tempsol2*fPulseTemplateFFTQO[tempbinItr] + tempsol1*fPulseTemplateFFTQOX[tempbinItr]);

	    tempchisq += tempArgQI.Rho2()/fNoiseFFTSqQI[tempbinItr];
	    tempchisq += tempArgQO.Rho2()/fNoiseFFTSqQO[tempbinItr];
	 }
	 
	 //store this delay/chisq pair
	 fDelayChisqMap.insert(pair<int, double>(tempdelay, tempchisq));

	 //store the minimum chisq
	 if(tempchisq < minChisq) 
	 { 
	    minChisq = tempchisq;

	    sol1 = tempsol1;
	    sol2 = tempsol2;
	    delay = tempdelay;
	    chisq = minChisq;
	 }

	 //search up to fQxwindow1, and after fQxwindow2
	 if(binItr == fQxwindow1 - 1)  
	    binItr = fQxwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1

      } // end loop over delay for min chisq search


      //Now do the delay interpolation 
      if(fDoDelayInterpolation == 1) 
	 CalcDelayInterpolation(delay, chisq);


   } // ==== endif do full chisq minimization calculation ====


   // ========= 8. Next, store the results of this calculation as the RQ's.  ===========

   fQIvolts = sol1*fTemplateMaxQI;
   fQOvolts = sol2*fTemplateMaxQO;
   
   fQIvolts0 = amp0QI;
   fQOvolts0 = amp0QO;
   
   //store the interpolated values, if they exist
   if(fInterDelay != -999999)
   {
      fDelay = fInterDelay*fdT;
      fChisq = fInterChisq;
   }
   else
   {
      fDelay = delay*fdT; //delay time is relative to global trigger (offset is applied in the template)
      fChisq = chisq;
   }   

   //store the discrete values - until we feel we understand the interpolation well enough
   fDiscreteDelay = delay*fdT;
   fDiscreteChisq = chisq;

   //NOTE: RQ storage is actually being implemented in the "StoreAs" function, which must be called by the user

   
   // ========== cleanup! ===========
   fTemplatesLoaded = false;
   fNormalizationsLoaded = false;
   fMatrixLoaded = false;

   return;
}

void OptimalFilterChargeX::LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& channelName)
{
   //these templates should all have the same lengths
   fNBinsTemplates = pulseTemplateFFT.size(); 

   if((int)optimalFilter.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterChargeX::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
      exit(1);
   }

   //checking channel name
   if(channelName != "QI" && channelName != "QO" && channelName != "QIX" && channelName != "QOX")
   {
      cerr <<"OptimalFilterChargeX::ERROR! ChannelName passed to LoadTemplates is invalid!"
	   << endl;
      exit(1);
   }

   if(channelName == "QI")
   {
     fPulseTemplateFFTQI = pulseTemplateFFT; 
     fOptimalFilterQI = optimalFilter;
   }
   if(channelName == "QO")
   {
     fPulseTemplateFFTQO = pulseTemplateFFT;
     fOptimalFilterQO = optimalFilter;
   }
   if(channelName == "QIX")
   {
     fPulseTemplateFFTQIX = pulseTemplateFFT;
     fOptimalFilterQIX = optimalFilter;
   }
   if(channelName == "QOX")
   {
     fPulseTemplateFFTQOX = pulseTemplateFFT;
     fOptimalFilterQOX = optimalFilter;
   }

   fTemplatesLoaded = true;

   return;
}

//only needs to load for QI and QO
void OptimalFilterChargeX::LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTSq, const double& templateMax,
					      const string& channelName)
{
   //check that vector lengths match
   if((int)noiseFFTSq.size() != fNBinsTemplates)
   {
      cerr <<"OptimalFilterChargeX::ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
      exit(1);
   }

   //checking channel name
   if(channelName != "QI" && channelName != "QO")
   {
      cerr <<"OptimalFilterChargeX::ERROR! ChannelName passed to LoadNormalizations is invalid!"
	   << endl;
      exit(1);
   }

   //copy the passed in templates to the members of this class
   if(channelName == "QI")
   {
      fNormFFTQI = normFFT;
      fSigToNoiseSqQI = sigToNoiseSq;
      fNoiseFFTSqQI = noiseFFTSq;
      fTemplateMaxQI = templateMax;
   }
   if(channelName == "QO")
   {
      fNormFFTQO = normFFT;
      fSigToNoiseSqQO = sigToNoiseSq;
      fNoiseFFTSqQO = noiseFFTSq;
      fTemplateMaxQO = templateMax;
   }

   fNormalizationsLoaded = true;

   return;
}

void OptimalFilterChargeX::LoadQInverse(const vector<double>& qInverse)
{ 
  if(qInverse.size() != 4)
  {
    cerr <<"OptimalFilterChargeX::ERROR! Attempting to load QInverse of any unexpected size, please check!" 
	 << endl;
    exit(1);
  }

  fQInverse = qInverse;

  fMatrixLoaded = true;
 
  return; 
}

//This is only done for the full chisq minimization
//The delay resolution is poor at low ionization energies so one
//should not expect any loss from not performing this for these events
//If the discrete delay choosen by the chisq minimization is at the edge of 
//the window, it does not do an interpolation.  This is done because this 
//calculation will not work properly if the true minimum is outside the window
void OptimalFilterChargeX::CalcDelayInterpolation(const int delay, const double chisq)
{

   //Get iterators that correspond to the minimum and the points on either side of it

   map<int, double>::const_iterator minIter = fDelayChisqMap.find((const int)delay); 
   map<int, double>::const_iterator lowIter = minIter;  
   map<int, double>::const_iterator highIter = minIter; 
   map<int, double>::const_iterator secondToLastIter = (--fDelayChisqMap.end()); 


   //solve for the parabola that fits the min plus one neighbor on each side

   if(minIter != fDelayChisqMap.begin() && minIter != secondToLastIter )
   {
      --lowIter;
      ++highIter;

      double xlow = lowIter->first;
      double xmin = minIter->first;
      double xhigh = highIter->first;

      double ylow = lowIter->second;
      double ymin = minIter->second;
      double yhigh = highIter->second;

      //first index is row, second is column
      TMatrixD xValMatrix(3,3);
      xValMatrix[0][0] = xlow*xlow;
      xValMatrix[0][1] = xlow;
      xValMatrix[0][2] = 1.0;
      
      xValMatrix[1][0] = xmin*xmin;
      xValMatrix[1][1] = xmin;
      xValMatrix[1][2] = 1.0;
      
      xValMatrix[2][0] = xhigh*xhigh;
      xValMatrix[2][1] = xhigh;
      xValMatrix[2][2] = 1.0;

      TMatrixD yValMatrix(3,1);
      yValMatrix[0][0] = ylow;
      yValMatrix[1][0] = ymin;
      yValMatrix[2][0] = yhigh;

//       cout <<"Matrix is\n" 
// 	   << xValMatrix[0][0] <<" " << xValMatrix[0][1] <<" " << xValMatrix [0][2] <<"\n"
// 	   << xValMatrix[1][0] <<" " << xValMatrix[1][1] <<" " << xValMatrix [1][2] <<"\n"
// 	   << xValMatrix[2][0] <<" " << xValMatrix[2][1] <<" " << xValMatrix [2][2] 
// 	   << endl;

      TMatrixD xInverse = xValMatrix;
      
      //Invert the matrix, if you can 
      if(xValMatrix.Determinant() != 0)
      {
	 xInverse.Invert();
      }
      else
      {
	 cout <<"ERROR! OptimalFilterChargeX::CalcDelayInterpolation"
	      <<"\nMatrix is not invertable!  This should not happen in this algorithm, check code!"
	      << endl;
	 
	 exit(1);
      }

      //Solve for parabolic fit with inverse of the x value matrix
      double a = xInverse[0][0]*ylow + xInverse[0][1]*ymin + xInverse[0][2]*yhigh;
      double b = xInverse[1][0]*ylow + xInverse[1][1]*ymin + xInverse[1][2]*yhigh;
      double c = xInverse[2][0]*ylow + xInverse[2][1]*ymin + xInverse[2][2]*yhigh;
   
      //Store the interpolated values
      fInterDelay = -b/(2*a);
      fInterChisq = c - b*b/(4*a);


   } //end if delay is within range that can be interpolated

   else
   {
      //Do nothing if the delay is at either edge of the window
      return;
   }


   return;
}

//================= code for testing and development =========================== 

//A toy simulation to estimate delay resolution for fixed set of paramters based on a collection of noise pulses
//this function is called from EventBuilder only for random trigger events.  Use the noise pulses to build a 
//"fake pulse" from the templates and noise pulses.  "DoCalc" stores the fit delay, amplitude and chisq as rq's.
//To run it in the BatRoot framework, call this routine instead of DoCalc (this routine then calls DoCalc after
//constructing the artificial pulse).  

void OptimalFilterChargeX::DelayResCalc(const vector<double>& aNoisePulseQI, const vector<double>& aNoisePulseQO, 
					int detNum)
{
   //Parameters 
   double trueQIamp = 0.5*0.0002099/fTemplateMaxQI; //choose amplitude
   double trueQOamp = 0.5*-3.41e-6/fTemplateMaxQO; //choose amplitude
   double trueDelay = 500.5; //choose delay - NOTE pretrigger time, 511.5, is subtracted from this, so use 1/2 integer delay

   //Construct Fake Pulse, store the fake pulses as class members
   ConstructFakePulse(trueQIamp, trueQOamp, trueDelay,
		      detNum, aNoisePulseQI, aNoisePulseQO);

      
   //Call DoCalc
   DoCalc(fFakePulseQI, fFakePulseQO);

   return;
}

void OptimalFilterChargeX::ConstructFakePulse(double normQI, double normQO, double delay, int detNum,
					      const vector<double>& aNoisePulseQI, const vector<double>& aNoisePulseQO)
{
   cout <<"Constructing the pulse!" << endl;

   if(!fTemplatesLoaded || !fNormalizationsLoaded || !fMatrixLoaded) 
   { 
      cerr <<"ERROR::Forgot to load templates!" << endl; 
      exit(1);
   }
   
   //offset delay in terms of the pre-trigger time
   delay += -511.5;

   //Getting the pulse templates 
   
   //new way...
   TFile f(Form("./templates/files/Z%d_Templates_161130_1533.root", detNum)); //the same for cand1 and cand2

   TH1F* h_templateQI = (TH1F*)f.Get(Form("zip%d/QI", detNum));
   TH1F* h_templateQO = (TH1F*)f.Get(Form("zip%d/QO", detNum));
   TH1F* h_templateQIX = (TH1F*)f.Get(Form("zip%d/QIX", detNum));
   TH1F* h_templateQOX = (TH1F*)f.Get(Form("zip%d/QOX", detNum));

   // ===== Construct the pulse! ========
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < fNBinsTemplates; binItr++)
   {
      //for a positive delay
      if(delay >= 0)
      {
	 if(binItr < delay)
	 {
	    fFakePulseQI.push_back(aNoisePulseQI[binItr]);
	    fFakePulseQO.push_back(aNoisePulseQO[binItr]);
	 }
	 if(binItr >= delay)
	 {
	    int shiftBin = binItr - (int)delay;
	    fFakePulseQI.push_back(normQI*h_templateQI->GetBinContent(shiftBin+1) + normQO*h_templateQIX->GetBinContent(shiftBin+1) 
				   + aNoisePulseQI[binItr] );
	    fFakePulseQO.push_back(normQO*h_templateQO->GetBinContent(shiftBin+1) + normQI*h_templateQOX->GetBinContent(shiftBin+1) 
				   + aNoisePulseQO[binItr]);
	 }
	 
      }//endif positive delay

      //for a negative delay
      else
      {
	 int shiftBin = binItr - (int)delay;
	 
	 if(shiftBin < 2048)
	 {
	    fFakePulseQI.push_back(normQI*h_templateQI->GetBinContent(shiftBin+1) + normQO*h_templateQIX->GetBinContent(shiftBin+1) 
				   + aNoisePulseQI[binItr] );
	    fFakePulseQO.push_back(normQO*h_templateQO->GetBinContent(shiftBin+1) + normQI*h_templateQOX->GetBinContent(shiftBin+1) 
				   + aNoisePulseQO[binItr]);
	 }
	 else
	 {
	    fFakePulseQI.push_back(aNoisePulseQI[binItr]);
	    fFakePulseQO.push_back(aNoisePulseQO[binItr]);
	 }
	 
      } //end if negative delay
      
   } //end loop over bins
   
   
   return;
}
 

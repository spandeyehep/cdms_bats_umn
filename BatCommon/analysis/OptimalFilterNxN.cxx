///////////////////////////////////////////////////////////////////////////////// 
//  Class Name:     OptimalFilterNxN
//  Author:         Bruno Serfass 
//  Description:    This class perfoms an optimal filtering using noise fft and 
//                  signal fft on phonon/charge pulses, taking into account cross-talks  
//                  (based on various MATLAB code from M.Pylse)
// 
//  Original author of the MATLAB code:  M. Pyle
//
//  File Import By: Bruno serfass
//  Creation Date:  Feb. 27, 2013
//
//  Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <limits>
#include <math.h>

#include "OptimalFilterNxN.h"
#include "PulseTools.h"

//only needed for testing
#include "TFile.h"
#include "TTree.h"
#include "TMatrixD.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterNxN::OptimalFilterNxN(const string& className) :
fDoDelayInterpolation(-999999),
fDoZdelayConstraint(-999999),
fdT(0.8e-6),
fZwindow1(-999999),
fZwindow2(-999999),
fNBinsTemplates(0),
fSinglePulse(false)
{
    //these members along with fRQlist are inherited from TCDMSAnalysis
    fClassName = className; 
    fStoreRQs = true; 
 

    // Special case of  "SingleChargePulse": keeping same name as OptimalFilterCharge2X2 
    // so that we don't need to turn on an extra algorithm in UserSettings

    if (className.compare("SingleChargePulse") == 0) {

        fClassName = "OptimalFilterCharge2X2"; 
        fStoreRQs = true;
        fSinglePulse = true;

    } 


    if (className.compare("OptimalFilterPhonon") == 0 ||
        className.compare("OptimalFilterCharge") == 0) 
                       fSinglePulse = true;


    //Construct the RQ list
    ConstructRQList();
}

OptimalFilterNxN::~OptimalFilterNxN()
{
    
}

////////////////////////////////////////////////////////

void OptimalFilterNxN::ConstructRQList()
{
    double initVal = -999999.;
  
    //construct the RQ list here (-999999. indicates normal channel prefixes)
 
    // Phonon pulses (single pulse)
    if (fClassName.compare("OptimalFilterPhonon") == 0) {

        fRQList.insert(pair<string,double>("OFflag", initVal));
        fRQList.insert(pair<string,double>("OFamps", initVal));
        fRQList.insert(pair<string,double>("OFamps0", initVal));
        fRQList.insert(pair<string,double>("OFdelay", initVal));  
        fRQList.insert(pair<string,double>("OFchisq", initVal));
        fRQList.insert(pair<string,double>("OFchisqBase", initVal));
        fRQList.insert(pair<string,double>("OFdiscreteAmps", initVal));
        fRQList.insert(pair<string,double>("OFdiscreteDelay", initVal));   
        fRQList.insert(pair<string,double>("OFdiscreteChisq", initVal)); 
 
    }


    // Charge pulses 
    if (fClassName.compare("OptimalFilterCharge") == 0) {
           
        fRQList.insert(pair<string,double>("OFnoXflag", initVal));
        fRQList.insert(pair<string,double>("OFnoXvolts", initVal));
        fRQList.insert(pair<string,double>("OFnoXvolts0", initVal));
        fRQList.insert(pair<string,double>("OFnoXdelay", initVal));  
        fRQList.insert(pair<string,double>("OFnoXchisq", initVal));
	fRQList.insert(pair<string,double>("OFnoXchisqBase", initVal));
        fRQList.insert(pair<string,double>("OFnoXdscrVolts", initVal));
        fRQList.insert(pair<string,double>("OFnoXdscrDelay", initVal));   
        fRQList.insert(pair<string,double>("OFnoXdscrChisq", initVal));   
    }
 

    if (fClassName.compare("OptimalFilterCharge2X2") == 0) {
    
        fRQList.insert(pair<string,double>("OFflag", initVal));
        fRQList.insert(pair<string,double>("OFvolts", initVal));
        fRQList.insert(pair<string,double>("OFvolts0", initVal));
        fRQList.insert(pair<string,double>("OFdiscreteVolts", initVal));
       

        //the initial value flags so that  BatOutputManager will append Prefix "QS"
        // (keeping also channel name)
        fRQList.insert(pair<string,double>("OFdelay", -987654.));  
        fRQList.insert(pair<string,double>("OFchisq", -987654.));
        fRQList.insert(pair<string,double>("OFchisqBase", -987654.));
        fRQList.insert(pair<string,double>("OFdiscreteDelay", -987654.));   
        fRQList.insert(pair<string,double>("OFdiscreteChisq", -987654.));   
       
       
    }
    
    //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.
    
    return;
}

////////////////////////////////////////////////////////

void OptimalFilterNxN::StoreAs(const string& chanName)
{


    // Phonon pulses (single pulse)
    if (fClassName.compare("OptimalFilterPhonon") == 0) {

      fRQList["OFflag"] = fOFflag;
      fRQList["OFamps"] = fAmp[chanName];
      fRQList["OFamps0"] = fAmp0[chanName];
      fRQList["OFdelay"]    = fDelay[chanName];
      fRQList["OFchisq"]    = fChisq[chanName];
      fRQList["OFchisqBase"]    = fChisqBase[chanName];
      fRQList["OFdiscreteAmps"] = fDiscreteAmp[chanName];
      fRQList["OFdiscreteDelay"]   = fDiscreteDelay[chanName];
      fRQList["OFdiscreteChisq"]   = fDiscreteChisq[chanName];

    }

    // Charge pulses
    if (fClassName.compare("OptimalFilterCharge") == 0) {
  
      fRQList["OFnoXflag"] = fOFflag;
      fRQList["OFnoXvolts"] = fAmp[chanName];
      fRQList["OFnoXvolts0"] = fAmp0[chanName];
      fRQList["OFnoXdelay"]    = fDelay[chanName];
      fRQList["OFnoXchisq"]    = fChisq[chanName];
      fRQList["OFnoXchisqBase"]    = fChisqBase[chanName];
      fRQList["OFnoXdscrVolts"] =  fDiscreteAmp[chanName];
      fRQList["OFnoXdscrDelay"]   = fDiscreteDelay[chanName];
      fRQList["OFnoXdscrChisq"]   = fDiscreteChisq[chanName];
 
    }


     if (fClassName.compare("OptimalFilterCharge2X2") == 0) {
       
       fRQList["OFflag"] = fOFflag;
       fRQList["OFvolts"] = fAmp[chanName];
       fRQList["OFvolts0"] = fAmp0[chanName];
       fRQList["OFdiscreteVolts"] = fDiscreteAmp[chanName];

       if (fSinglePulse) {

           fRQList["OFdelay"]    = fDelay[chanName];
           fRQList["OFchisq"]    = fChisq[chanName];
           fRQList["OFchisqBase"]    = fChisqBase[chanName];
           fRQList["OFdiscreteDelay"]   = fDiscreteDelay[chanName];
           fRQList["OFdiscreteChisq"]   = fDiscreteChisq[chanName];

       } else {

           // Get side
           string side = "S";
           if (chanName.find("S1") != string::npos)  side = "S1";
           if (chanName.find("S2") != string::npos)  side = "S2";
   
           // Fill RQ list  
           fRQList["OFdelay"]    = fDelay[side];
           fRQList["OFchisq"]    = fChisq[side];
           fRQList["OFchisqBase"]    = fChisqBase[side];
           fRQList["OFdiscreteDelay"]   = fDiscreteDelay[side];
           fRQList["OFdiscreteChisq"]   = fDiscreteChisq[side];


       }
    }

   return;
}

////////////////////////////////////////////////////////

void OptimalFilterNxN::LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& chanName)
{
    // template size   
    fNBinsTemplates = pulseTemplateFFT.size();

    //FFT normalizations to match MATLAB code
    double  dnu         = 1/(fdT*(double)fNBinsTemplates);
    double  sqrtdnu     = sqrt(dnu);
    double  isqrtdnu    = 1/sqrtdnu;
    
    if((int) optimalFilter.size() != fNBinsTemplates)
    {
        cerr <<"OptimalFilterNxN::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
        exit(1);
    }
    
    
    // fill maps with quantities
    fPulseTemplateFFT[chanName] = PulseTools::pulseScale(pulseTemplateFFT,TComplex(sqrtdnu,0));
    fOptimalFilter[chanName] = PulseTools::pulseScale(optimalFilter,TComplex(isqrtdnu,0));
   
    return;
}

////////////////////////////////////////////////////////

void OptimalFilterNxN::LoadWinverse(const vector<double>& Winverse,const string& side)
{ 
          
    if (Winverse.size() ==4) {
      
      // FIXME: assume this is for charge 2X2 algorithm
      // Channel name hardcoded. We might want to change to a map 
      
      string chanNameQI = "QI";
      string chanNameQO = "QO";

      if (side.compare("S") !=0) {
        chanNameQI = "QI" + side;
        chanNameQO = "QO" + side;
      }
 
      string chanNameQIX = chanNameQI + "X";
      string chanNameQOX = chanNameQO + "X";
   
      // Fill map: 
      // Positions in vector: QI, QOX, QIX, QO 
      // (FIXME: not sure about QIX and QOX but it is symmetric)

      map<string, double> matrixTemp;   

      fWinverse[chanNameQI]  = Winverse[0];
      fWinverse[chanNameQOX] = Winverse[1];  
      fWinverse[chanNameQIX] = Winverse[2];
      fWinverse[chanNameQO]  = Winverse[3];  
    
    } else if (Winverse.size() ==1) {
       
      // case single pulse 
      //  side = channel name
      fWinverse[side]  = Winverse[0];
  
    } else {

      cerr <<"OptimalFilterNxN::ERROR! Attempting to load Qinverse of an unexpected size, please check!" 
           << endl;
      exit(1);
    }
     
      
    return; 
}






////////////////////////////////////////////////////////

void OptimalFilterNxN::LoadNormalizations(const vector<double>& noiseFFTSq, const double& templateMax, const string& chanName)
{

    //FFT normalizations to match MATLAB code
    double  dnu     = 1/(fdT*(double)fNBinsTemplates);
       
    //check that vector lengths match
    if((int)noiseFFTSq.size() != fNBinsTemplates)
    {
        cerr <<"OptimalFilterNxN::ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
        exit(1);
    }
    
   
    // fill maps
    fTemplateMax[chanName] =  templateMax;
    fNoiseFFTSq[chanName] =  PulseTools::pulseScale(noiseFFTSq,dnu);

    return;
}




void OptimalFilterNxN::LoadNormalizations(const double& sigToNoiseSq, const vector<double>& noiseFFTSq, const double& templateMax, const string& chanName)
{
    // load Winverse
    vector<double> Winverse;
    Winverse.push_back(1/sigToNoiseSq);
    LoadWinverse(Winverse,chanName);

    // load normalization
    LoadNormalizations(noiseFFTSq, templateMax, chanName);

    return;
}



// construct Xwindow Vector
void  OptimalFilterNxN::SetXwindows(int xwindowMin, int xwindowMax, int traceLength) 
{  
       
    // (1) Conversion to cyclical window 
    //     (let's already use C++ vector notation [0:traceLength-1])
    

    // Window > tracelength -1
    if (xwindowMin>traceLength-1)
      xwindowMin = xwindowMin-traceLength;

    if (xwindowMax>traceLength-1)
      xwindowMax = xwindowMax-traceLength;
 

    // Negative windows
    if (xwindowMin<0)
      xwindowMin = xwindowMin + traceLength;

    if (xwindowMax<0)
      xwindowMax = xwindowMax + traceLength;


    // (2) check validity

    if (!(xwindowMin>=0 && xwindowMin<traceLength) ||
	!(xwindowMax>=0 && xwindowMax<traceLength)) {
           
      cerr <<"OptimalFilterNxN::SetXwindows ERROR! Window out of range" << endl;
      exit(1);
    }



    // (3) Let's fill vector of shifts

    if (xwindowMin <= xwindowMax) {

      // case either only position or "negative" shifts, or fixed shift
      for (int ii=xwindowMin;ii<=xwindowMax;ii++)              
	fXwindowVect.push_back(ii);
      
    } else {
     
      // case negative and positive shift   
      // window up to xwindowMax           
      for (int ii=0;ii<=xwindowMax;ii++) 
	fXwindowVect.push_back(ii);
      
      // window from xwindowMin to end
      for (int ii=xwindowMin;ii<traceLength;ii++)
	fXwindowVect.push_back(ii);
      
    }

    return; 
}






///////////////////////////////////////////////////////////////////////////
void OptimalFilterNxN::DoCalc(const vector<double>& aPulse, const string& chanName)
{ 

    // ==================================================
    //      Optimal Filter Single Pulse
    // ==================================================
 
    // Fill pulse map
    map<string, vector<double> >  aPulseMap;
    aPulseMap[chanName] = aPulse;
  
    
    // Call appriopriate function
    DoCalc(aPulseMap);

    return;
}
   

///////////////////////////////////////////////////////////////////////////
void OptimalFilterNxN::DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, const string& chanName1, const string& chanName2)
{ 

    // ==================================================
    //      Optimal Filter 2X2
    // ==================================================
 
    if(aPulse1.size() != aPulse2.size())
      {
        cerr <<"OptimalFilterNxN::ERROR! Found pulses with different length, check input vectors!" << endl;
        exit(1);
      }
    

    // Fill pulse map
    map<string, vector<double> >  aPulseMap;
    aPulseMap[chanName1] = aPulse1;
    aPulseMap[chanName2] = aPulse2;
    
    // Call appriopriate function
    DoCalc(aPulseMap);

    return;
}
    
   



///////////////////////////////////////////////////////////////////////////
void OptimalFilterNxN::DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, 
				    const vector<double>& aPulse3, const vector<double>& aPulse4,  
				    const string& chanName1, const string& chanName2, 
				    const string& chanName3, const string& chanName4)
{ 

    // ==================================================
    //      Optimal Filter 2x  2X2
    // ==============================================-===
  
    if(aPulse1.size() != aPulse2.size() || aPulse1.size() != aPulse3.size() || aPulse1.size() != aPulse4.size())
      {
        cerr <<"OptimalFilterNxN::ERROR! Found pulses with different length, check input vectors!" << endl;
        exit(1);
      }
    
    // Fill pulse map
    map<string, vector<double> >  aPulseMap;
    aPulseMap[chanName1] = aPulse1;
    aPulseMap[chanName2] = aPulse2;
    aPulseMap[chanName3] = aPulse3;
    aPulseMap[chanName4] = aPulse4;

    // Call appriopriate function
    DoCalc(aPulseMap);
  
    return;
}
    
 

/////////////////////////////////////////////////////////////////////////////////////
void OptimalFilterNxN::DoCalc(const map<string,vector<double> >& aPulseMap)
{

    ////////////////////////////////////////////////////// 
    //      Optimal Filter 2x2 (or 2 times 2x2 if 2 sides
    //      available, with possibility time constraint
    //      between S1 and S2)
    //////////////////////////////////////////////////////


    // =================================== 
    //  preliminiary checks
    // ===================================
      
    
    // check size of map, at least aPulseMapS1 should have pulses, S2 can be empty (single sided)
    if (aPulseMap.empty()) 
      {
	cerr <<"OptimalFilterNxN::ERROR! Empty input map, check your code !" << endl;
	exit(1);
      }
    
    // Loop pulse: check number of bins, templates/normalization available 
    int nBins = (aPulseMap.begin()->second).size();
    map<string,vector<double> >::const_iterator pulseIter;
    for (pulseIter=aPulseMap.begin(); pulseIter!=aPulseMap.end(); ++pulseIter) 
      {
	string chanName = pulseIter->first;
	int nBinsTemp =  (pulseIter->second).size(); 

	// number bins
	if (nBins != nBinsTemp || nBins != fNBinsTemplates) {
	  cerr <<"OptimalFilterNxN::DoCalc: ERROR! All pulses and templates should have same length!" << endl;
	  exit(1);  
	}

	// check templates/noise
	if (fOptimalFilter.find(chanName)==fOptimalFilter.end()
	    || fPulseTemplateFFT.find(chanName)==fPulseTemplateFFT.end() 
	    || fNoiseFFTSq.find(chanName) == fNoiseFFTSq.end()
	    || fWinverse.find(chanName)==fWinverse.end()) {
	  cerr <<"OptimalFilterNxN::DoCalc: ERROR! Missing Noise or Template informations!" << endl;
	  exit(1);  
	}
 	
      }
    
      
    // ===================================
    // Separate pulses between sides (if 
    // available)
    // ===================================
    map<string, vector<double> > aPulseMap1;
    map<string, vector<double> > aPulseMap2;
    
    for (pulseIter=aPulseMap.begin(); pulseIter!=aPulseMap.end(); ++pulseIter) 
      {
	string chanName = pulseIter->first;
	
	if (chanName.find("S1")!=string::npos)
	  aPulseMap1[chanName] = aPulseMap.find(chanName)->second;
	
	if (chanName.find("S2")!=string::npos)
	  aPulseMap2[chanName] = aPulseMap.find(chanName)->second;	
      }
    
    
    // case single sided detector (CDMSII)
    // -> use aPulseMap1 only
    if (aPulseMap1.empty() && aPulseMap2.empty())
      aPulseMap1 = aPulseMap; 
    
    // case either S1 OR S2  
    // -> use aPulseMap1  only
    if (aPulseMap1.empty() && !aPulseMap2.empty()) 
      aPulseMap1 = aPulseMap2;
    
    
    
    // =================================== 
    // Find best fit amplitude and chi2  
    // for all possible time shifts      
    // ===================================
    
    // NOTE: channel names are taken from pulseMap. The other maps (noiseFFTsq, OF, Qinverse)
    // may contain informations from extra channels, which won't be used
    
    // PulseMap1
    map<string, map<int, double> >   ampChisqAllDelayMap1; 
    ampChisqAllDelayMap1 = CalcOFallTimeShifts(aPulseMap1,fNoiseFFTSq,fOptimalFilter,fWinverse);
    
    // PulseMap2 (if available)
    map<string, map<int, double> >   ampChisqAllDelayMap2; 
    if (!aPulseMap2.empty())
      ampChisqAllDelayMap2 = CalcOFallTimeShifts(aPulseMap2,fNoiseFFTSq,fOptimalFilter,fWinverse);
    
    
    
    // =================================== 
    // Find delay minimim chisq (within
    // OF window X and/or Z) and get
    // amplitude 
    // ===================================
    
    // initialize parameters
    double          minChisq1 = numeric_limits<double>::infinity();
    double          minChisq2 = numeric_limits<double>::infinity();
    double          minChisqTot = numeric_limits<double>::infinity();
    int             minChisqDelay1 = 0;
    int             minChisqDelay2 = 0;
    
    // Get chisq-delay map
    map<int, double> delayChisqMap1 = ampChisqAllDelayMap1.find("Chisq")->second;
    map<int, double> delayChisqMap2;
    if (!aPulseMap2.empty()) delayChisqMap2 = ampChisqAllDelayMap2.find("Chisq")->second;
    
    
    // Loop through X window
    for (uint binItrX = 0; binItrX < fXwindowVect.size(); binItrX++) 
      {
	// delay stored in ChiSq map = binItrX
	int delayX = fXwindowVect[binItrX];
	
      	// Get ChiSq 1
        double chisq1Temp  = delayChisqMap1[delayX];   
	
	// Case time constraint between sides 
       	// Loop through Z window (if time constraint between sides)
	if (fDoZdelayConstraint==1 && !aPulseMap2.empty())  {
	  
	  // Loop through Z window 
	  for (int binItrZ = fZwindow1; binItrZ <= fZwindow2; binItrZ++)
	    {
	      // calculate X shifted delay 
	      int delayXshifted = delayX + binItrZ;
	      
	      if (delayXshifted<0)
		delayXshifted = delayXshifted+nBins;
	      
	      if (delayXshifted>nBins-1)
		delayXshifted = delayXshifted-nBins;
	      
	      // Total ChiSq = ChiSq1 + ChiSq2;
	      double chisqTemp = chisq1Temp + delayChisqMap2[delayXshifted];   
	      
	      
	      if (chisqTemp < minChisqTot) 
		{
		  minChisqDelay1 = delayX;
		  minChisqDelay2 = delayXshifted;
		  minChisqTot    =  chisqTemp;
		  
		}
	    }
	  
	} else {
	  
	  // Case sides independent
	  
	  double chisq1Temp  = delayChisqMap1[delayX];
	  
	  if (chisq1Temp < minChisq1) {
	    minChisqDelay1 = delayX;
	    minChisq1    =  chisq1Temp;
	  }
	  
	  if (!aPulseMap2.empty()) {
	    double chisq2Temp  = delayChisqMap2[delayX]; 
	    if (chisq2Temp < minChisq2) {
	      minChisqDelay2 = delayX;
	      minChisq2    =  chisq2Temp;
	    }
	  }
	}  
      }
    
    
    
    // =================================== 
    // Interpolate values
    // ===================================
    
    // Done independently 1/2
    map<string, double> interpValMap1;
    map<string, double> interpValMap2;
    
    if(fDoDelayInterpolation == 1) {
      interpValMap1 = CalcDelayInterpolation(minChisqDelay1, ampChisqAllDelayMap1);
      if (!aPulseMap2.empty())
	interpValMap2 = CalcDelayInterpolation(minChisqDelay2, ampChisqAllDelayMap2);
    }
    
    
    // =================================== 
    // Store RQ in maps
    // ===================================
    
    // NOTE: RQ storage is actually being implemented in the "StoreAs" 
    //       function, which must be called by the user
    
    
    // === PulseMap1 ===

    // loop pulse map to get channel name
    int counter = 0; // to store only once
    for (pulseIter=aPulseMap1.begin(); pulseIter!=aPulseMap1.end(); ++pulseIter) 
      {
	string chanName = pulseIter->first;
	
	// store amplitude map
	map<int,double>  ampDelayMap1 = ampChisqAllDelayMap1.find(chanName+"amp")->second;
	
	// store 0 delay amplitude
	fAmp0[chanName]    =   ampDelayMap1[0];
	
	// store discrete amplitude at min chisq
	fDiscreteAmp[chanName] = ampDelayMap1[minChisqDelay1]*fTemplateMax[chanName];
	
	// store interpolated amplitude (if available)
	if (!interpValMap1.empty())
	  fAmp[chanName] = interpValMap1[chanName + "amp"]*fTemplateMax[chanName];
	else
	  fAmp[chanName] = fDiscreteAmp[chanName];
	


	// store chisq/delay
	if (fSinglePulse || counter==0){

          string side = chanName;

	  if (!fSinglePulse) {
	    side = "S";
	    if (chanName.find("S1")!=string::npos) side = "S1";
	    if (chanName.find("S2")!=string::npos) side = "S2";
	  }
          


	  // discrete delay/amp
	  fDiscreteDelay[side]  = (minChisqDelay1 < nBins/2 ? minChisqDelay1 : (minChisqDelay1 - nBins))*fdT;//Added 01-08-12
	  //fDiscreteDelay = delay*fdT;//Original
	  fDiscreteChisq[side] = delayChisqMap1[minChisqDelay1];

          // chisqBase
          fChisqBase[side] = (ampChisqAllDelayMap1.find("ChisqBase")->second)[0];	

	  // interpolated 
	  if (!interpValMap1.empty()) {
	    fDelay[side] = interpValMap1["Delay"]*fdT;
	    fChisq[side] = interpValMap1["Chisq"];  
	  } else {
	    fDelay[side] = fDiscreteDelay[side];
	    fChisq[side] = fDiscreteChisq[side];
          }
	  
          counter++;
	}
      }
    
    
    
    // === PulseMap2 (if available) ===
    
    if (!aPulseMap2.empty()) {
      // loop pulse map to get channel name
      counter = 0; // to store only once
      for (pulseIter=aPulseMap2.begin(); pulseIter!=aPulseMap2.end(); ++pulseIter) 
	{
	  string chanName = pulseIter->first;
	  
	  map<int,double>  ampDelayMap2 = ampChisqAllDelayMap2[chanName+"amp"];
	  
	  // store 0 delay amplitude
	  fAmp0[chanName]    =   ampDelayMap2[0];
	  
	  // store discrete amplitude at min chisq
	  fDiscreteAmp[chanName] = ampDelayMap2[minChisqDelay2]*fTemplateMax[chanName];
	  
	  // store interpolated amplitude (if available)
	  if (!interpValMap2.empty())
	    fAmp[chanName] = interpValMap2[chanName + "amp"]*fTemplateMax[chanName];
          else
	    fAmp[chanName] = fDiscreteAmp[chanName];	 
	  
	  // store chisq/delay (only once)
	  if (fSinglePulse || counter==0){

             string side = chanName;

             if (!fSinglePulse) 
	           side = "S2"; // should only be for S2....
	    
	    // discrete delay/amp
	    fDiscreteDelay[side]  = (minChisqDelay2 < nBins/2 ? minChisqDelay2 : (minChisqDelay2 - nBins))*fdT;//Added 01-08-12
	    //fDiscreteDelay = delay*fdT;//Original
	    fDiscreteChisq[side] = delayChisqMap2[minChisqDelay2];
	      
            // chisqBase
            fChisqBase[side] = (ampChisqAllDelayMap2.find("ChisqBase")->second)[0];	

	    // interpolated 
	    if (!interpValMap2.empty()) {
	      fDelay[side] = interpValMap2["Delay"]*fdT;
	      fChisq[side] = interpValMap2["Chisq"];  
	    } else {
	      fDelay[side] = fDiscreteDelay[side];
	      fChisq[side] = fDiscreteChisq[side];
            }
	    
            counter++;
	  }
	}
    }


    // OF flag
    
    string flag1 = "2";
    string flag2 = "0";
    string flag3 = "0";

    if (aPulseMap.size()==1) flag1="1";
    if (fDoDelayInterpolation==1) flag2="1";
    if (fDoZdelayConstraint==1) flag3="1";

    string flag = flag1+flag2 + flag3;
    fOFflag = atof(flag.c_str());
    

    return;
}





map<string, map<int, double> >  OptimalFilterNxN::CalcOFallTimeShifts(const map<string, vector<double> >& pulseMap, 
									    const map<string, vector<double> >& noiseFFTSqMap,
									    const map<string, vector<TComplex> >& optimalFilterMap,
									    const map<string, double>& QinverseMap)
{
 
    ////////////////////////////////////////////////////////// 
    // Calculate Amplitude and Chi2 for all time shifts     //
    //////////////////////////////////////////////////////////
  
    // define output map (not using private data in case function
    // moved to another location
    // both chisq and amp stored in same map
    map<string, map<int, double> > ampChisqAllDelayMap;
  
    // =================================== 
    // Channels name (signal and Xtalk)
    // ===================================
    
    // number channels
    int nChan = pulseMap.size();  
   
    // number of bins
    int nBins = -999999; 
    
    //  Define vector of channel names
    vector<string> chanNames;  
    
    // Define vector of channel index names 
    // For example: "I" and "O" for "QI" or "QO" resp.
    //               "A" for "PA" or "PAS1"
    //               "1","2","3" for  "Q1"/"Q2"....
    // vector<string> chanIndexNames;

    
    map<string,vector<double> >::const_iterator pulseIter;
    for (pulseIter=pulseMap.begin(); pulseIter!=pulseMap.end(); ++pulseIter) 
      { 
	string chanName = pulseIter->first;
	chanNames.push_back(chanName);

        nBins   =  (pulseIter->second).size(); // should be same for all pulses
      }
    
    
    // Inverse of number of bins
    double  nBinsInv  = (double) 1/nBins;
   
    

    // =================================== 
    // Calculate pulse FFT
    // ===================================
    
    map<string, vector<TComplex> >  pulseFFTMap;     
    for (pulseIter=pulseMap.begin(); pulseIter!=pulseMap.end(); ++pulseIter) { 
      
      string chanName = pulseIter->first;
      vector<double> aPulse =  pulseIter->second;
      
      // get FFT
      vector<TComplex> aPulseFFT;
      PulseTools::RealToComplexFFT(aPulse, aPulseFFT);
      
      // Normalize for proper unit (FIXME: will need to clean units...) 
      for(int binItr = 0; binItr < nBins; binItr++)
	aPulseFFT[binItr] *= sqrt(nBinsInv);
      
      // store in map
      pulseFFTMap[chanName] =  aPulseFFT;
      
      //cleanup
      aPulseFFT.clear();
    }
    

    // =================================== 
    // Apply Optimal Filter
    // ===================================
 
    // Optimal Filter matrix was created  from the template  and
    // the noise information in BatNoise

    // Define channel map with the inverse FFT of pulseFFT*OF 
    map<string, vector<double> >   pulseProdiFFTMap;
        
    // Loop channels
    for (int chanIter = 0; chanIter < nChan; chanIter++) 
      {
	// Channel name
	const string chanName = chanNames[chanIter];
	
	// get pulseFFT and OF 
	vector<TComplex> aPulseFFT =  pulseFFTMap.find(chanName)->second;
	vector<TComplex> anOptimalFilter = optimalFilterMap.find(chanName)->second;
	
	// multiply 
	vector<TComplex> aPulseProd;
	for (int binItr = 0; binItr < nBins; binItr++) 
	  aPulseProd.push_back(aPulseFFT[binItr]*anOptimalFilter[binItr]);
	
	// Add Xtalk:
	// 1. get Xtalk channel indices
        // 2. multiply pulseFFT * OF and add chan+Xtalk(s) product
	
       	vector<int> xtalkIndices; 
	for (int chanIterX = 0; chanIterX < nChan; chanIterX++) 
	  if (chanIterX!=chanIter) xtalkIndices.push_back(chanIterX);
	
	int nChanX = xtalkIndices.size();
	for (int chanIterX = 0; chanIterX < nChanX; chanIterX++) 
	  {
	    // get names
	    int chanIndX =  xtalkIndices[chanIterX];
	    string chanNameX = chanNames[chanIndX];
	    string chanNameOFX = chanNameX + "X";
	    
	    // get pulseFFT and OF
	    vector<TComplex> aPulseFFTX =  pulseFFTMap.find(chanNameX)->second;
	    vector<TComplex> anOptimalFilterX = optimalFilterMap.find(chanNameOFX)->second;
	    
	    // multiply and add 
	    for (int binItr = 0; binItr < nBins; binItr++) 
	      aPulseProd[binItr] = aPulseProd[binItr]+aPulseFFTX[binItr]*anOptimalFilterX[binItr];
     
	  }


	//Invert FFT
	vector<double> aPulseProdifftRe;
	PulseTools::ComplexToRealIFFT(aPulseProd, aPulseProdifftRe); 
	for (int binItr = 0; binItr < nBins; binItr++)
	  aPulseProdifftRe[binItr] *= sqrt((double)nBins);
	
	//save in map
	pulseProdiFFTMap[chanName] = aPulseProdifftRe;

	// clean up
	aPulseProd.clear();
	aPulseProdifftRe.clear();
      }
	    
    
    
    // =============  Multiply by weighting matrix to get amplitudes at all times =============
      
    vector<double> chi2toVect;
    
    // loop channels
    for (int chanIter = 0; chanIter < nChan; chanIter++) 
      {

	// Channel name
	string chanName = chanNames[chanIter];
	
	// get iFFT(pulseFFT*OF) calculated above 
	vector<double> aPulseProdiFFT =  pulseProdiFFTMap.find(chanName)->second;

	// get Xtalk indices
	vector<int> xtalkIndices; 
	for (int chanIterX = 0; chanIterX < nChan; chanIterX++) 
	  if (chanIterX!=chanIter) xtalkIndices.push_back(chanIterX);
	

	// Loop delay shifts, calculate product, then save 
	// for each delays

	map<int,double> delayAmp;
       
        // first calculate amp without Xtalk
	for (int binItr = 0; binItr < nBins; binItr++) 
	  {
	    //int     tempdelay = (binItr < nBins/2 ? binItr : (binItr - nBins));//Original
	    int     tempdelay = binItr;
	    
            // calculate amplitude for signal
            delayAmp[tempdelay] = (QinverseMap.find(chanName)->second)*aPulseProdiFFT[binItr];
          }


        // add Xtalk   	   
	int nChanX = xtalkIndices.size();
	for (int chanIterX = 0; chanIterX < nChanX; chanIterX++) 
	   {
              // get names
              int chanIndX =  xtalkIndices[chanIterX];
              string chanNameX = chanNames[chanIndX];
	      string chanNameOFX = chanNameX + "X";

              // get iFFT(pulseFFT*OF) calculated above 
	      vector<double> aPulseProdiFFTX =  pulseProdiFFTMap.find(chanNameX)->second;
	

              for (int binItr = 0; binItr < nBins; binItr++) 
	         {
	           //int     tempdelay = (binItr < nBins/2 ? binItr : (binItr - nBins));//Original
	           int     tempdelay = binItr;
	    
                  // calculate amplitude for Xtalk and add
                  delayAmp[tempdelay] = delayAmp[tempdelay] + QinverseMap.find(chanNameOFX)->second*aPulseProdiFFTX[binItr];
                 }
             }
   
	//store in overall map
	string key = chanName + "amp";
	ampChisqAllDelayMap[chanName + "amp"]=delayAmp;
	
      }
	    
    
    // ============= Now is time to build the Chi2 =============
    
    // 1: Part of Chi2  independent of t0
     
    double                      chi2Base     = 0;
    vector<double>              theNoiseFFTsqtemp;
     
    // Calculate for each channel independently and then add
    for (int chanIter = 0; chanIter < nChan; chanIter++) 
      {
	string chanName =  chanNames[chanIter];
	
	vector<TComplex>  chi2BaseVecTemp     = pulseFFTMap.find(chanName)->second;
        vector<double>   noiseFFTsqTemp   = noiseFFTSqMap.find(chanName)->second;
        double chi2BaseTemp   = 0;
        
	// Not including DC component
        for (int binItr = 1; binItr < nBins; binItr++) 
          chi2BaseTemp = chi2BaseTemp + chi2BaseVecTemp[binItr].Rho2()/(noiseFFTsqTemp[binItr]); 
        
        //Store the total value
        chi2Base = chi2Base+chi2BaseTemp; 
    }
    
    // store chi2 base for future use
     map<int, double> chi2BaseMap;
     chi2BaseMap.insert(pair<int, double>(0, chi2Base));
     ampChisqAllDelayMap["ChisqBase"]= chi2BaseMap;


    // 2: Part of Chi2 that depends on t0

    vector<double>              sumChitoVect(nBins,0);
    
    for (int chanIter = 0; chanIter < nChan; chanIter++) 
      {
	string chanName =  chanNames[chanIter];
	
	// get amplitudes for all delay shift and iFFT(pulseFFT*OF)
	map<int,double> delayAmp = ampChisqAllDelayMap.find(chanName + "amp")->second;
	vector<double> aPulseProdiFFT =  pulseProdiFFTMap.find(chanName)->second;


	// calculate product and sum  all channel
	for (int binItr = 0; binItr < nBins; binItr++) 
	  sumChitoVect[binItr] = sumChitoVect[binItr]+delayAmp[binItr]*aPulseProdiFFT[binItr];
      }
    
   

    //3: Now generate full  Chi2 vector

    map<int,double> delayChisq;
    for(int binItr = 0; binItr < nBins; binItr++)
    {
      //int     tempdelay = (binItr < nBins/2 ? binItr : (binItr - nBins));//Original
      int     tempdelay = binItr;
      double  tempchisq = 0;
        
      tempchisq = chi2Base-sumChitoVect[binItr];
      delayChisq.insert(pair<int, double>(tempdelay, tempchisq));
    }
    
    // store ChiSq
    ampChisqAllDelayMap["Chisq"]=delayChisq;
   
    return ampChisqAllDelayMap;
}


map<string, double> OptimalFilterNxN::CalcDelayInterpolation(const int delay, const map<string, map<int, double> >& ampChisqAllDelayMap)
{
    //////////////////////////////////////////////////////////////////
    // Interpolate delay/chisq/Amp using a parabola    
    //
    // INPUT: 
    //    - delay: delay corresponding to min chisq (discrete value)
    //    - ampChisqAllDelayMap: map<string, map<int, double> with
    //        string:  - chanName + "amp"
    //                 - "Chisq"  (single chisq for all channels)                   
    //        map<int, double>: chisq/amp values for all shifts
    // 
    //////////////////////////////////////////////////////////////////

    // Define output 
    map<string, double>  interpValuesMap;
   
   
    // Get chisq (should be stored as "Chisq")
    map<int, double> delayChisqMap = ampChisqAllDelayMap.find("Chisq")->second;
    
    // number bins (of delay shift)
    int nBins = delayChisqMap.size();

    // Get the values corresponding to input "delay" and +/- 1 bins
    map<int, double>::const_iterator minIter = delayChisqMap.find((const int)delay); 
    map<int, double>::const_iterator lowIter = minIter;  
    map<int, double>::const_iterator highIter = minIter; 
    
    if(minIter == delayChisqMap.begin())  {
      lowIter = (--delayChisqMap.end()); 
      ++highIter; 
    } else if (minIter == (--delayChisqMap.end())) {
      --lowIter;
      highIter = delayChisqMap.begin();
    } else {
      --lowIter;
      ++highIter; 
    }
    
    
    // If the windowing constrained the system then there is a chance that
    //  the middle value is not the smallest value of the three!
    //    -> no interpolation
    if(!((*lowIter).second > (*minIter).second && 
         (*highIter).second > (*minIter).second))  return interpValuesMap;
    
    
    
    // To solve parabola equation, we define matrix using basis [-1 0 1] 
    // for delay
    TMatrixD xInverse(3,3);
    
    xInverse[0][0] = 0.5;
    xInverse[0][1] = -1.0;
    xInverse[0][2] = 0.5;
    
    xInverse[1][0] = -0.5;
    xInverse[1][1] = 0.0;
    xInverse[1][2] = 0.5;
    
    xInverse[2][0] = 0.0;
    xInverse[2][1] = 1.0;
    xInverse[2][2] = 0.0;
    
    
    
    //Solve for parabolic fit with inverse of the x value matrix
         	
    double xmin = minIter->first;
    double ylow = lowIter->second;
    double ymin = minIter->second;
    double yhigh = highIter->second;
    
    double a = xInverse[0][0]*ylow + xInverse[0][1]*ymin + xInverse[0][2]*yhigh;
    double b = xInverse[1][0]*ylow + xInverse[1][1]*ymin + xInverse[1][2]*yhigh;
    double c = xInverse[2][0]*ylow + xInverse[2][1]*ymin + xInverse[2][2]*yhigh;
    
    //Store the interpolated values
    double InterpDelayInd = -b/(2*a);
    double InterpDelay = xmin + InterpDelayInd; //translate to find the actual delay
    interpValuesMap["Chisq"]= c - b*b/(4*a);
    interpValuesMap["Delay"] = (InterpDelay < nBins/2 ? InterpDelay : (InterpDelay - nBins));
    


    //  Amplitudes interpolation
    map<string, map<int,double> >::const_iterator delayMapIter;
    for (delayMapIter=ampChisqAllDelayMap.begin(); delayMapIter!=ampChisqAllDelayMap.end(); ++delayMapIter) 
      {
	string namePar = delayMapIter->first;
	if (namePar.compare("Chisq")==0) continue;


	map<int, double> ampDelayMap = delayMapIter->second;
	
	
	// Get amp for -1,+0,+1 delay
	map<int, double>::const_iterator minIterAmp = ampDelayMap.find((const int)delay); 
	map<int, double>::const_iterator lowIterAmp = minIterAmp;  
	map<int, double>::const_iterator highIterAmp = minIterAmp; 
	
	if(minIterAmp == ampDelayMap.begin()) {
	  lowIterAmp = (--ampDelayMap.end());
	  ++highIterAmp; 
	} else if (minIterAmp == (--ampDelayMap.end())) {
	  --lowIterAmp;
	  highIterAmp = ampDelayMap.begin();
	} else {
	  --lowIterAmp;
	  ++highIterAmp; 
	}
	
	double ylowAmp = lowIterAmp->second;
        double yminAmp = minIterAmp->second;
        double yhighAmp = highIterAmp->second;


	
	//Solve for parabolic fit with inverse of the x value matrix
        double aAmp = xInverse[0][0]*ylowAmp + xInverse[0][1]*yminAmp + xInverse[0][2]*yhighAmp;
        double bAmp = xInverse[1][0]*ylowAmp + xInverse[1][1]*yminAmp + xInverse[1][2]*yhighAmp;
        double cAmp = xInverse[2][0]*ylowAmp + xInverse[2][1]*yminAmp + xInverse[2][2]*yhighAmp;

        
        //Store interpolated amplitudes
        interpValuesMap[namePar] = InterpDelayInd*InterpDelayInd*aAmp+InterpDelayInd*bAmp+cAmp;

      }

    return interpValuesMap;
}







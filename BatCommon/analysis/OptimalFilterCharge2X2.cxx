///////////////////////////////////////////////////////////////////////////////// 
//  Class Name:     OptimalFilterCharge2X2
//  Author:         Carlos Eduardo Martinez Amaya  / Bruno Serfass
//  Description:    This class perfoms an optimal filtering using noise fft and 
//                  signal fft on charge pulses, taking into account cross-talks  
//                  (based on the OF_Qxtalk.m MATLAB code)
// 
//  Original author of the MATLAB code:  M. Pyle
//
//  File Import By: Carlos Eduardo Martinez Amaya
//  Creation Date:  Feb. 17, 2012
//
//  Modifications:
//      B. Serfass (Jan. 8, 2013): modify structure to calculate charge 
//                                 OF for both sides
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <limits>
#include <math.h>

#include "OptimalFilterCharge2X2.h"
#include "PulseTools.h"

//only needed for testing
#include "TFile.h"
#include "TTree.h"
#include "TMatrixD.h"

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterCharge2X2::OptimalFilterCharge2X2() :
fDoDelayInterpolation(-999999),
fDoZdelayConstraint(-999999),
fdT(0.8e-6),
fQxwindow1(-999999),
fQxwindow2(-999999),
fQzwindow1(-999999),
fQzwindow2(-999999),
fNBinsTemplates(0)

{
    //these members along with fRQlist are inherited from TCDMSAnalysis
    fClassName = "OptimalFilterCharge2X2"; 
    fStoreRQs = true;
  
    //Construct the RQ list
    ConstructRQList();
}

OptimalFilterCharge2X2::~OptimalFilterCharge2X2()
{
    
}

////////////////////////////////////////////////////////

void OptimalFilterCharge2X2::ConstructRQList()
{
    double initVal = -999999.;
    
    //construct the RQ list here (-999999. indicates normal channel prefixes)
    fRQList.insert(pair<string,double>("OFvolts", initVal));
    fRQList.insert(pair<string,double>("OFdiscreteVolts", initVal));
    fRQList.insert(pair<string,double>("OFvolts0", initVal));
    
    //the initial value flags this so BatOutputManager will append Prefix "QS" *instead* of QI/QO
    //with this option activated you cannot have RQ's named both QSOFdelay and QIOFdelay/QOOFdelay!
    fRQList.insert(pair<string,double>("OFdelay", -123456.));  
    fRQList.insert(pair<string,double>("OFchisq", -123456.));
    fRQList.insert(pair<string,double>("OFdiscreteDelay", -123456.));   
    fRQList.insert(pair<string,double>("OFdiscreteChisq", -123456.));   
    
    
    //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.
    
    return;
}

////////////////////////////////////////////////////////

void OptimalFilterCharge2X2::StoreAs(const string& chanName)
{
   // check input
   if(chanName != "QI" && chanName != "QO" && 
      chanName != "QIS1" && chanName != "QOS1" &&
      chanName != "QIS2" && chanName != "QOS2") 
     {
       cerr <<"OptimalFilterCharge2X2::WARNING! incorrect channel name passed to StoreAs function" << endl;
       exit(1); 
     }
   
   // Get side
   string side = "S";
   if (chanName.find("S1") != string::npos)  side = "S1";
   if (chanName.find("S2") != string::npos)  side = "S2";
   
   // Fill RQ list  
   fRQList["OFvolts"] = fVolts[chanName];
   fRQList["OFdiscreteVolts"] = fDiscreteVolts[chanName];
   fRQList["OFvolts0"] = fVolts0[chanName];
   
   fRQList["OFdelay"]    = fDelay[side];
   fRQList["OFchisq"]    = fChisq[side];
   fRQList["OFdiscreteDelay"]   = fDiscreteDelay[side];
   fRQList["OFdiscreteChisq"]   = fDiscreteChisq[side];
   
   return;
}

////////////////////////////////////////////////////////

void OptimalFilterCharge2X2::LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& chanName)
{
    // template size   
    fNBinsTemplates = pulseTemplateFFT.size();

    //FFT normalizations to match MATLAB code
    double  dnu         = 1/(fdT*(double)fNBinsTemplates);
    double  sqrtdnu     = sqrt(dnu);
    double  isqrtdnu    = 1/sqrtdnu;
    
    if((int) optimalFilter.size() != fNBinsTemplates)
    {
        cerr <<"OptimalFilterCharge2X2::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
        exit(1);
    }
    
    //checking channel name
    if(chanName != "QI" && chanName != "QO" && chanName != "QIX" && chanName != "QOX" &&
       chanName != "QIS1" && chanName != "QOS1" && chanName != "QIS1X" && chanName != "QOS1X" &&
       chanName != "QIS2" && chanName != "QOS2" && chanName != "QIS2X" && chanName != "QOS2X")
    {
        cerr <<"OptimalFilterCharge2X2::ERROR! ChannelName passed to LoadTemplates is invalid!"
        << endl;
        exit(1);
    }
    

    // fill maps with quantities
    fPulseTemplateFFT[chanName] = PulseTools::pulseScale(pulseTemplateFFT,TComplex(sqrtdnu,0));
    fOptimalFilter[chanName] = PulseTools::pulseScale(optimalFilter,TComplex(isqrtdnu,0));
   
    return;
}

////////////////////////////////////////////////////////

void OptimalFilterCharge2X2::LoadQinverse(const vector<double>& qInverse,const string& side)
{ 
    if(qInverse.size() != 4)
    {
        cerr <<"OptimalFilterCharge2X2::ERROR! Attempting to load Qinverse of an unexpected size, please check!" 
        << endl;
        exit(1);
    }
 
    // Channel Names (FIXME: channel hard coded...)
      
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

    fQinverse[chanNameQI]  = qInverse[0];
    fQinverse[chanNameQOX] = qInverse[1];  
    fQinverse[chanNameQIX] = qInverse[2];
    fQinverse[chanNameQO]  = qInverse[3];  
    
      
    return; 
}

////////////////////////////////////////////////////////

void OptimalFilterCharge2X2::LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTSq, const double& templateMax, const string& chanName)
{

    //FFT normalizations to match MATLAB code
    double  dnu     = 1/(fdT*(double)fNBinsTemplates);
       
    //check that vector lengths match
    if((int)noiseFFTSq.size() != fNBinsTemplates)
    {
        cerr <<"OptimalFilterCharge2X2::ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
        exit(1);
    }
    
    //checking chan name
    if(chanName != "QI" && chanName != "QO" && 
       chanName != "QIS1" && chanName != "QOS1" &&
       chanName != "QIS2" && chanName != "QOS2")
    {
        cerr <<"OptimalFilterCharge2X2::ERROR! ChanName passed to LoadNormalizations is invalid!"
        << endl;
        exit(1);
    }
    
    // fill maps
    fNormFFT[chanName] = normFFT;
    fSigToNoiseSq[chanName] = sigToNoiseSq;
    fTemplateMax[chanName] =  templateMax;
    fNoiseFFTSq[chanName] =  PulseTools::pulseScale(noiseFFTSq,dnu);

    return;
}


///////////////////////////////////////////////////////////////////////////
void OptimalFilterCharge2X2::DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, const string& chanName1, const string& chanName2)
{ 

    // ==================================================
    //      Optimal Filter 2X2
    // ==================================================
 
    if(aPulse1.size() != aPulse2.size())
      {
        cerr <<"OptimalFilterCharge2X2::ERROR! Found pulses with different length, check input vectors!" << endl;
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
void OptimalFilterCharge2X2::DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, 
				    const vector<double>& aPulse3, const vector<double>& aPulse4,  
				    const string& chanName1, const string& chanName2, 
				    const string& chanName3, const string& chanName4)
{ 

    // ==================================================
    //      Optimal Filter 2x  2X2
    // ==============================================-===
  
    if(aPulse1.size() != aPulse2.size() || aPulse1.size() != aPulse3.size() || aPulse1.size() != aPulse4.size())
      {
        cerr <<"OptimalFilterCharge2X2::ERROR! Found pulses with different length, check input vectors!" << endl;
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
void OptimalFilterCharge2X2::DoCalc(const map<string,vector<double> >& aPulseMap)
{

    ////////////////////////////////////////////////////// 
    //      Optimal Filter 2x2 (or 2 times 2x2 if 2 sides
    //      available, with possibility time constraint
    //      between S1 and S2)
    //////////////////////////////////////////////////////


    // =================================== 
    //  preliminiary checks
    // ===================================
      
    //checking that the fit window is valid
    if(fQxwindow1 < 0 || fQxwindow2 < 0) 
      {
	cerr <<"OptimalFilterCharge2X2::ERROR! Fit windows appear to be uninitialized" << endl; 
	exit(1);
      }
    
    // check size of map, at least aPulseMapS1 should have pulses, S2 can be empty (single sided)
    if (aPulseMap.empty()) 
      {
	cerr <<"OptimalFilterCharge2X2::ERROR! Empty input map, check your code !" << endl;
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
	  cerr <<"OptimalFilterCharge2X2::DoCalc: ERROR! All pulses and templates should have same length!" << endl;
	  exit(1);  
	}

	// check templates/noise
	if (fOptimalFilter.find(chanName)==fOptimalFilter.end()
	    || fPulseTemplateFFT.find(chanName)==fPulseTemplateFFT.end() 
	    || fNoiseFFTSq.find(chanName) == fNoiseFFTSq.end()
	    || fQinverse.find(chanName)==fQinverse.end()) {
	  cerr <<"OptimalFilterCharge2X2::DoCalc: ERROR! Missing Noise or Template informations!" << endl;
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
    ampChisqAllDelayMap1 = CalcOFallTimeShifts(aPulseMap1,fNoiseFFTSq,fOptimalFilter,fQinverse);
    
    // PulseMap2 (if available)
    map<string, map<int, double> >   ampChisqAllDelayMap2; 
    if (!aPulseMap2.empty())
      ampChisqAllDelayMap2 = CalcOFallTimeShifts(aPulseMap2,fNoiseFFTSq,fOptimalFilter,fQinverse);
    
    
    
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
    for (int binItrX = 0; binItrX < nBins; binItrX++) 
      {
	// delay stored in ChiSq map = binItrX
	int delayX = binItrX;
	
      	// Get ChiSq 1
        double chisq1Temp  = delayChisqMap1[delayX];   
	
	// Case time constraint between sides 
       	// Loop through Z window (if time constraint between sides)
	if (fDoZdelayConstraint==1 && !aPulseMap2.empty())  {
	  
	  // Loop through Z window 
	  for (int binItrZ = fQzwindow1; binItrZ <= fQzwindow2; binItrZ++)
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
	
	//Search up to fQxwindow1, and after fQxwindow2
	if(binItrX == fQxwindow1 - 1)
	  binItrX = fQxwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
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
    int counter = 0; // to store 
    for (pulseIter=aPulseMap1.begin(); pulseIter!=aPulseMap1.end(); ++pulseIter) 
      {
	string chanName = pulseIter->first;
	
	// store amplitude map
	map<int,double>  ampDelayMap1 = ampChisqAllDelayMap1.find(chanName+"amp")->second;
	
	// store 0 delay amplitude
	fVolts0[chanName]    =   ampDelayMap1[0];
	
	// store discrete amplitude at min chisq
	fDiscreteVolts[chanName] = ampDelayMap1[minChisqDelay1]*fTemplateMax[chanName];
	
	// store interpolated amplitude (if available)
	if (!interpValMap1.empty())
	  fVolts[chanName] = interpValMap1[chanName + "amp"]*fTemplateMax[chanName];
	else
	  fVolts[chanName] = fDiscreteVolts[chanName];
	
	// store chisq/delay
	if (counter==0){
	  
	  string side = "S";
	  if (chanName.find("S1")!=string::npos) side = "S1";
	  if (chanName.find("S2")!=string::npos) side = "S2";
	  
	  // discrete delay/amp
	  fDiscreteDelay[side]  = (minChisqDelay1 < nBins/2 ? minChisqDelay1 : (minChisqDelay1 - nBins))*fdT;//Added 01-08-12
	  //fDiscreteDelay = delay*fdT;//Original
	  fDiscreteChisq[side] = delayChisqMap1[minChisqDelay1];
	  
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
	  fVolts0[chanName]    =   ampDelayMap2[0];
	  
	  // store discrete amplitude at min chisq
	  fDiscreteVolts[chanName] = ampDelayMap2[minChisqDelay2]*fTemplateMax[chanName];
	  
	  // store interpolated amplitude (if available)
	  if (!interpValMap2.empty())
	    fVolts[chanName] = interpValMap2[chanName + "amp"]*fTemplateMax[chanName];
          else
	    fVolts[chanName] = fDiscreteVolts[chanName];	 
	  
	  // store chisq/delay (only once)
	  if (counter==0){
	    string side = "S2"; // should only be for S2....
	    
	    // discrete delay/amp
	    fDiscreteDelay[side]  = (minChisqDelay2 < nBins/2 ? minChisqDelay2 : (minChisqDelay2 - nBins))*fdT;//Added 01-08-12
	    //fDiscreteDelay = delay*fdT;//Original
	    fDiscreteChisq[side] = delayChisqMap2[minChisqDelay2];
	    
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
    
    //NOTE: RQ storage is actually being implemented in the "StoreAs" function, which must be called by the user
    
    return;
}





map<string, map<int, double> >  OptimalFilterCharge2X2::CalcOFallTimeShifts(const map<string, vector<double> >& pulseMap, 
									    const map<string, vector<double> >& noiseFFTSqMap,
									    const map<string, vector<TComplex> >& optimalFilterMap,
									    const map<string, double>& QinverseMap)
{
 
    ////////////////////////////////////////////////////////// 
    // Calculate Amplitude and Chi2 for all time shifts     //
    //////////////////////////////////////////////////////////
  
    // define output map 
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


map<string, double> OptimalFilterCharge2X2::CalcDelayInterpolation(const int delay, const map<string, map<int, double> >& ampChisqAllDelayMap)
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







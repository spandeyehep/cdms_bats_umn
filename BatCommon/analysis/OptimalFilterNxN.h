///////////////////////////////////////////////////////////////////////////////// 
//  Class Name:     OptimalFilterNxN
//  Author:         Bruno Serfass
//  Description:    This class perfoms an optimal filtering using noise fft and 
//                  signal fft on charge pulses, taking into account cross-talks  
//                  (based Matt Pylse MATLAB code)
// 
//  Original author of the MATLAB code:  M. Pyle
//
//  File Import By: Bruno Serfass
//  Creation Date:  Feb. 27, 2013
//
//  Modifications:
////////////////////////////////////////////////////////////////////////////////// 


#ifndef _OptimalFilterNxN_H 
#define _OptimalFilterNxN_H

#include <iostream>
#include <map>
#include <vector>

#include "TComplex.h"
#include "TMatrixD.h"

#include "TCDMSAnalysis.h"

using namespace std;


class OptimalFilterNxN : public TCDMSAnalysis
{
public:
    
    OptimalFilterNxN(const string& className = "OptimalFilterCharge2X2");
    ~OptimalFilterNxN();
    
    // Calc functions
    void DoCalc(const map<string,vector<double> >& aPulseMap);
    
    // the above function can be also called using pulses direcly
    // rather than pulseMap. The default channel names are specified
    // below.
    
    void DoCalc(const vector<double>& aPulse, const string& chanName = "Q");

    void DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, 
		const string& chanName1 = "QI",const string& chanName2 = "QO" );
    
    void DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2,
		const vector<double>& aPulse3, const vector<double>& aPulse4,
		const string& chanName1 = "QIS1",const string& chanName2 = "QOS1",
		const string& chanName3 = "QIS2",const string& chanName4 = "QOS2");
    
    
    void StoreAs(const string& chanName);
    
    //Get functions

    //chanName = "QI","QO","QIS1","QOS1","QIS2","QOS2" 
    double GetQvolts(const string& chanName)  const { return fAmp.find(chanName)->second;  }
    double GetQvolts0(const string& chanName) const { return fAmp0.find(chanName)->second; }   

    // side = "S" (singe sided), "S1", "S2" 
    double GetDelay(const string& side="S") const { return fDelay.find(side)->second; }
    double GetChisq(const string& side="S") const { return fChisq.find(side)->second; }
    
    //Set parameters
    void SetSampleTime(double dt) 
    { 
        fdT = dt; 
        return; 
    }
    


    void SetXwindows(int xwindowMin, int xwindowMax,int traceLength);
    void SetZwindows(int zwindow1, int zwindow2) 
    {  
        fZwindow1 = zwindow1; 
        fZwindow2 = zwindow2; 
        
        return; 
    }



    //Get the templates from the Noise Files
    void LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& channelName);
    void LoadWinverse(const vector<double>& Winverse, const string& side = "S");
    void LoadNormalizations(const vector<double>& noiseFFTsq, const double& templateMax, const string& channelName);
    void LoadNormalizations(const double& sigToNoiseSq, const vector<double>& noiseFFTsq, const double& templateMax, const string& channelName);
    
   

    void SetDelayInterpolateFlag(const int& interpFlag)
    {
        fDoDelayInterpolation = interpFlag;
        return;
    }
    

    void SetZdelayConstraintFlag(const int& zdelayConstraintFlag)
    {
        fDoZdelayConstraint = zdelayConstraintFlag;
        return;
    }



    // Obsolete functions (for back compatibility)
    
     void LoadQinverse(const vector<double>& qInverse, const string& side = "S") 
       { LoadWinverse(qInverse,side); return;}

     void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTsq, 
                             const double& templateMax, const string& channelName)
       {  LoadNormalizations(noiseFFTsq,templateMax,channelName); return;}
      


    
private:
    
    void ConstructRQList();
    
    // The functions below do not use private data and can be moved
    // to PulseTools
    map<string, double>    CalcDelayInterpolation(const int delay, const map<string, map<int, double> >& ampChisqAllDelayMap);
    map<string, map<int, double> >  CalcOFallTimeShifts(const map<string, vector<double> >& pulseMap, 
							const map<string, vector<double> >& noiseFFTSqMap,
							const map<string, vector<TComplex> >& optimalFilterMap,
						        const map<string, double>& QinverseMap);	
      
    int fDoDelayInterpolation;
    int fDoZdelayConstraint;
    //Fit values
    map<string,double>  fAmp;  // string->chanName 
    map<string,double>  fDiscreteAmp;
    map<string,double>  fAmp0;
    map<string,double>  fDelay;  // string-> side
    map<string,double>  fInterpDelay;
    map<string,double>  fDiscreteDelay; 
    map<string,double>  fChisq;
    map<string,double>  fChisqBase;
    map<string,double>  fInterpChisq; 
    map<string,double > fDiscreteChisq;
    
       
    //Parameters
    double  fdT;

    // OF windows
    vector<int> fXwindowVect; // vector with delay shift
    int    fZwindow1; // Negative shift from 0 [fZwindow1:0]
    int    fZwindow2; // Positive shift from 0 [0:fZwindow2] 
    
    //Templates    
    int  fNBinsTemplates;

    // single pulse flag 
    bool fSinglePulse;

    //Normalization   
    map<string,double> fTemplateMax;
      
    //noise fft sq 
    map<string, vector<double> > fNoiseFFTSq;
    
    //Crosstalk Matrix
    map<string, double>  fWinverse; 
    
    //Pulse Templates FFT
    map<string, vector<TComplex> >  fPulseTemplateFFT;
     
    //OptimalFilter templates
    map<string, vector<TComplex> > fOptimalFilter; 
 
    // fit flag = 3 digit number:
    //
    //   First digit:   1 = single pulse fit, 2 = 2X2 fit
    //   Second digit:  0 = no interpolation, 1= interpolation
    //   third digit:   0 = no Z constraint, 1 = Z constraint
    //  
    //   Ex: 201 = 2X2 fit, no interpolation, Z constraint
  
    double fOFflag;

    
};
#endif

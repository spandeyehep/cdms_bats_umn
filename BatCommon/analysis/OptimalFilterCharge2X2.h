///////////////////////////////////////////////////////////////////////////////// 
//  Class Name:     OptimalFilterCharge2X2
//  Author:         Carlos Eduardo Martinez Amaya
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
////////////////////////////////////////////////////////////////////////////////// 


#ifndef _OptimalFilterCharge2X2_H 
#define _OptimalFilterCharge2X2_H

#include <iostream>
#include <map>
#include <vector>

#include "TComplex.h"
#include "TMatrixD.h"

#include "TCDMSAnalysis.h"

using namespace std;


class OptimalFilterCharge2X2 : public TCDMSAnalysis
{
public:
    
    OptimalFilterCharge2X2();
    ~OptimalFilterCharge2X2();
    
    // Calc functions
    void DoCalc(const map<string,vector<double> >& aPulseMap);
    
    // the above function can be also called using pulses direcly
    // rather than pulseMap. The default channel names are specified
    // below.
    
    void DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2, 
		const string& chanName1 = "QI",const string& chanName2 = "QO" );
    
    void DoCalc(const vector<double>& aPulse1, const vector<double>& aPulse2,
		const vector<double>& aPulse3, const vector<double>& aPulse4,
		const string& chanName1 = "QIS1",const string& chanName2 = "QOS1",
		const string& chanName3 = "QIS2",const string& chanName4 = "QOS2");
    
    
    void StoreAs(const string& chanName);
    
    //Get functions

    //chanName = "QI","QO","QIS1","QOS1","QIS2","QOS2" 
    double GetQvolts(const string& chanName)  const { return fVolts.find(chanName)->second;  }
    double GetQvolts0(const string& chanName) const { return fVolts0.find(chanName)->second; }   

    // side = "S" (singe sided), "S1", "S2" 
    double GetDelay(const string& side="S") const { return fDelay.find(side)->second; }
    double GetChisq(const string& side="S") const { return fChisq.find(side)->second; }
    
    //Set parameters
    void SetSampleTime(double dt) 
    { 
        fdT = dt; 
        return; 
    }
    
    void SetQxWindows(int qxwindow1, int qxwindow2) 
    {  
        fQxwindow1 = qxwindow1; 
        fQxwindow2 = qxwindow2; 
        
        return; 
    }
    
    void SetQzWindows(int qzwindow1, int qzwindow2) 
    {  
        fQzwindow1 = qzwindow1; 
        fQzwindow2 = qzwindow2; 
        
        return; 
    }


    //Get the templates from the Noise Files
    void LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& channelName);
    void LoadQinverse(const vector<double>& qInverse, const string& side = "S");
    void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTsq, const double& templateMax, const string& channelName);
    
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
    map<string,double>  fVolts;  // string->chanName 
    map<string,double>  fInterpVolts;
    map<string,double>  fDiscreteVolts;
    map<string,double>  fVolts0;
    map<string,double>  fDelay;  // string-> side
    map<string,double>  fInterpDelay;
    map<string,double>  fDiscreteDelay; 
    map<string,double>  fChisq;
    map<string,double>  fInterpChisq; 
    map<string,double > fDiscreteChisq;
    
       
    //Parameters
    double  fdT;

    // OF windows
    int    fQxwindow1; //search for best delay from 0th bin to this point ...
    int    fQxwindow2; //...or from this point to the last bin 
    int    fQzwindow1; // Negative shift from 0 [fQzwindow1:0]
    int    fQzwindow2; // Positive shift from 0 [0:fQzwindow2] 
    
    //Templates    
    int  fNBinsTemplates;
    
    //Normalizations    
    map<string,double> fNormFFT; 
    map<string,double> fSigToNoiseSq;
    map<string,double> fTemplateMax;
      
    //noise fft sq 
    map<string, vector<double> > fNoiseFFTSq;
    
    //Crosstalk Matrix
    map<string, double>  fQinverse; 
    
    //Pulse Templates FFT
    map<string, vector<TComplex> >  fPulseTemplateFFT;
     
    //OptimalFilter templates
    map<string, vector<TComplex> > fOptimalFilter; 
 
    
    
};
#endif

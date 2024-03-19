/////////////////////////////////////////////////////////////////////////////////
//  Class Name:     OptimalFilterPhonon1X2
//  Author:         Carlos Eduardo Martinez Amaya
//  Description:    This class perfoms an optimal filtering using two phonon 
//                  templates (template1=Ptemplate and template2=Signal-Ptemplate)
//                  on phonon pulses.
//                  (based on the OF_Qxtalk.m MATLAB code)
//
//
//  File Import By: Carlos Eduardo Martinez Amaya
//  Creation Date:  Aug. 10, 2012
//
//  Modifications:
//////////////////////////////////////////////////////////////////////////////////

#ifndef OptimalFilterPhonon1X2_H
#define OptimalFilterPhonon1X2_H

#include <iostream>
#include <vector>
#include <map>

#include "TComplex.h"
#include "TMatrixD.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the OptimalFilterPhonon1X2 Class.  It serves as a template for all other analysis classes
class OptimalFilterPhonon1X2 : public TCDMSAnalysis
{
   public:

    //do not modify the signature of this constructor
    //if you need a constructor with a different signature, than define a separate constructor
    OptimalFilterPhonon1X2();
    ~OptimalFilterPhonon1X2(); //destructor

    //Define functions
    void DoCalc(const vector<double>& aPulse);
    
    //Get functions
    double GetDelay() const { return fDelay; }
    double GetChisq() const { return fChisq; }
    
    //Set parameters
    void SetSampleTime(double dt)
    {
        fdT = dt;
        return;
    }
    
    void SetPxWindows(int pwindow1, int pwindow2)
    {
        fPxwindow1 = pwindow1;
        fPxwindow2 = pwindow2;
        
        return;
    }
    
    //Get the templates from the Noise Files
    void LoadTemplates(const vector<double>& templateSlow, const vector<double>& templateFast, const vector<TComplex>& templateSlowFFT,const vector<TComplex>& templateFastFFT);
    void LoadNormalizations(const vector<double>& noiseFFTSq);
    
    void SetDelayInterpolateFlag(const int& doDelayInt)
    {
        fDoDelayInterpolation = doDelayInt;
        return;
    }

   private:

    void ConstructRQList();
    void CalcDelayInterpolation(const int delay);
    
    int                             fDoDelayInterpolation;

    //Fit Values
    double                          fPamps;
    double                          fPamps0;
    double                          fInterPamps;//Interpolated
    double                          fDiscretePamps;
    double                          fRamps;
    double                          fRamps0;
    double                          fInterRamps;//Interpolated
    double                          fDiscreteRamps;
    double                          fChisq;
    double                          fDiscreteChisq;
    double                          fInterChisq;//Interpolated
    double                          fDchisqLF;
    double                          fDelay;
    double                          fDiscreteDelay;
    double                          fInterDelay;//Interpolated
    map<int, double>                fDelayChisqMap; //key is delay, val is chisq
    map<int, double>                fDelayPAmpMap; //key is delay, val is pulse amp
    map<int, double>                fDelayRAmpMap; //key is delay, val is residual amp
    
    //parameters
    double                          fdT;
    int                             fPxwindow1; //in adc bins
    int                             fPxwindow2; //in adc bins
    bool                            fTemplatesLoaded;
    bool                            fNormalizationsLoaded;
    
    //templates
    vector<double>                  fNoiseFFTSq;
    vector<double>                  fPulseTemplate;
    vector<double>                  fResidualTemplate;
    vector<TComplex>                fPulseTemplateFFT;
    vector<TComplex>                fResidualTemplateFFT;
    
    //precaution against reading in the wrong templates
    int fNBinsTemplates;
      

};

#endif /* OptimalFilterPhonon1X2_H */

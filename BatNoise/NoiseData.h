///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: NoiseData
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  This is a data storage class.  It contains all OptimalFilter noise quantities, 
//the pulse template and histograms for selecting noise traces on a given channel.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOISEANDTEMPLATEDATA_H
#define NOISEANDTEMPLATEDATA_H

#include <vector>
#include <map>

#include "TComplex.h"
#include "TH1D.h"
#include "TMatrixD.h"

using namespace std;

//!This is a data storage class for all quantities related to noise generation
class NoiseData 
{
      friend class NoiseBuilder; 

   public:

      NoiseData(const int detCode, const string& channelName);  //constructor
  
      ~NoiseData(); //destructor

      // access channel identifying information
      //string  GetChannelType()  const; //returns "phonon", "charge" 
      string  GetChannelName()     const  { return fChannelName; } //returns PA, PB, PC, PD, QI, QO, QIX, QOX
      int     GetDetectorCode()    const  { return fDetCode; }
      int     GetDetectorType()    const  { return fDetType; }
      int     GetDetectorNum()     const  { return fDetNum; }

      // construct noise selection histograms
      void InitializeNoiseSelectionHistogram(const string& histo, 
					     int nBins, double minBin, double maxBin); //creates histo if it doesn't yet exist
      void HistogramValue(const string& histoName, double val);
      TH1D RetrieveNoiseSelectionHistogram(const string& histo);  //read only access

      // access noise quantities as histograms - READ only access

      TH1D GetHistTemplateTime(); 
      TH1D GetHistTemplateFFTRe(); 
      TH1D GetHistTemplateFFTIm(); 

      TH1D GetHistOptimalFilterRe(); 
      TH1D GetHistOptimalFilterIm(); 
      TH1D GetHistNormFFT();
      TH1D GetHistSigToNoiseSq();
      TH1D GetHistNoisePSD(); 
      TH1D GetHistNoiseFFT(); 
      TH1D GetHistNoiseFFTsq(); 
      TH1D GetHistQInverse();
      TH1D GetHistSampleRate();

      TH1D GetHistEventList();
      
      // load template and compute the FFT, sample rate is for scaling the FFT and later storage
      void LoadSampleRate(const double& fsample); //this must come before LoadTemplate
      void LoadTemplate(const vector<double>& pulseTemplate);

      // for calculation 
      void AddToAveragePSD(const vector<double>& pulsePSD); 
      int GetTraceLengthType(); // 0 = unknow, 1=odd, 2=even (based on template size)

   private:

      NoiseData();  //constructor
      
      //Identifying info
      uint32_t fDetCode;    //format is specified in ChannelMapHelper
      int      fDetNum;     
      int      fDetType;     
      string   fChannelName;
      
      //Template
      vector<double>   fTemplateTime;
      vector<TComplex> fTemplateFFT;

      //Average PSD
      vector<double>   fAveragePSD;
      double           fPSDCount;

      //Noise
      vector<double>   fNoiseFFT;    //we throw away phase information so this is real
      vector<double>   fNoiseFFTsq;  //will this work for cross talk parameters?

      //Signal To Noise 
      double           fNormFFT;
      double           fSigToNoiseSq;
      vector<TComplex> fOptimalFilter;

      //cross talk matrix (duplicate copy stored in all charge NoiseData)
      TMatrixD fQInverse;

      //Other
      double         fSampleRate;     //loaded along with template  
      double         fFrequencyScale; //this should be = samplingFrequency/2 (nyquist)
      vector<double> fEventList;      //list of event numbers for events going into the PSD

      //for noise selection
      vector<double>   fMinMaxValues;
      vector<double>   fPileupValues;
      vector<double>   fPOFchisqValues;
      vector<double>   fPTBSslopeValues;


      double           fMinMaxCut; //only stored for PT and QT NoiseData objects
      double           fPileupCut; //same cut stored for all NoiseData objects for one detector
      double 	       fPOFchisqCut;             

      //Histograms
      vector<TH1D*>    fNoiseSelectionHistograms;
      
};

#endif /* NOISEANDTEMPLATEDATA_H */

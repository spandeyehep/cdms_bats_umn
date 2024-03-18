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

#include <iostream>

#include "BatRootTypes.h"
#include "NoiseData.h"
#include "PulseTools.h"
#include "ChannelMapHelper.h"

using namespace std;

////////////////////////////////////////////////////////

//default constructor
NoiseData::NoiseData(const int detCode, const string& channelName) :
  fDetCode(detCode),
  fDetNum(-999999),
  fDetType(-999999),
  fChannelName(channelName),
  fPSDCount(0.0),
  fSampleRate(-999999.),
  fFrequencyScale(-999999.),
  fMinMaxCut(-999999.),
  fPileupCut(-999999.)
{

   fDetNum = ChannelMapHelper::GetDetNumFromCode(fDetCode);
   fDetType = ChannelMapHelper::GetDetTypeFromCode(fDetCode);

//     cout <<"Creating a NoiseData with detector code " << fDetCode <<", with name " << fChannelName 
// 	 <<", detector number = " << fDetNum
// 	 << endl;

   //initialize QInverse if needed
   fQInverse.ResizeTo(2,2);

}

NoiseData::~NoiseData()
{
  //   cout <<"NoiseData::~NoiseData, before deleting, fNoiseSelectionHistograms size = " << fNoiseSelectionHistograms.size() << endl;

   //Delete new'd histogram objects - perhaps unecessary - was causing a seg fault, not sure why
 //   for(uint histItr=0; histItr < fNoiseSelectionHistograms.size(); histItr++)
//    {
//       delete fNoiseSelectionHistograms[histItr]; 
//    } 

//   cout <<"NoiseData::~NoiseData, after deleting, fNoiseSelectionHistograms size = " << fNoiseSelectionHistograms.size() << endl;

   fNoiseSelectionHistograms.clear();

}

//====================== Loading necessary information ===========================

void NoiseData::LoadSampleRate(const double& sampleRate)
{ 
   fSampleRate = sampleRate;
   fFrequencyScale = sampleRate/2.0;

   return;
}

//load the template and compute the FFT
void NoiseData::LoadTemplate(const vector<double>& pulseTemplate)
{ 
   fTemplateTime = pulseTemplate; 
   
   if(fSampleRate == -999999.)
   {
      cout <<"NoiseData::LoadTemplate ERROR!   Sample rate is not set, please to this before proceeding!"
	   << endl;
      exit(1);
   }

   //Compute and store the FFT
   PulseTools::RealToComplexFFT(pulseTemplate, fTemplateFFT);
   
   //normalize by sqrt(fsample)
   for(uint binCtr=0; binCtr < fTemplateFFT.size(); binCtr++)
   {
      TComplex scalefactor(1.0/sqrt(fSampleRate),0.0);
      fTemplateFFT[binCtr] *= scalefactor;
   }

   return; 
}

//====================== Histogram Manipulations ===========================

//creates this histogram if one with this name doesn't yet exist.  otherwise does nothing.
void NoiseData::InitializeNoiseSelectionHistogram(const string& histo, int nBins, double minBin, double maxBin)
{
   //Form a name with detNum and channel name
   string aName = Form("%s.zip%d.%s", histo.c_str(), fDetNum, fChannelName.c_str());

   //loop through list of histograms and see if we find a matching name
   for(uint histItr=0; histItr < fNoiseSelectionHistograms.size(); histItr++)
   {
      string checkName = fNoiseSelectionHistograms[histItr]->GetName();
      
      //if found, than just return
      if(aName == checkName) 
	 return;
   }   


   //if we made it this far, then no matching name was found, so create a new histogram
   fNoiseSelectionHistograms.push_back(new TH1D(aName.c_str(), aName.c_str(), nBins, minBin, maxBin));
   
//   cout <<"Created histogram! " << aName << endl;

   return;

}

void NoiseData::HistogramValue(const string& histo, double val)
{
   //Form a name with detNum and channel name
   string aName = Form("%s.zip%d.%s", histo.c_str(), fDetNum, fChannelName.c_str());

   //loop through list of histograms and see if we find a matching name
   for(uint histItr=0; histItr < fNoiseSelectionHistograms.size(); histItr++)
   {
      string checkName = fNoiseSelectionHistograms[histItr]->GetName();
      
      //if found, than store value and return
      if(aName == checkName) 
      {
	 fNoiseSelectionHistograms[histItr]->Fill(val);
	 return;
      }
   }   

   //if we made it this far, then no matching name was found, issue an error
   cout <<"NoiseData::HistogramValue ERROR! No histogram of name " << aName <<" was found.  Did you remember to initialize the histogram?" 
	<< endl;
   exit(1);

   return;
}

//read only access
TH1D NoiseData::RetrieveNoiseSelectionHistogram(const string& histo)  
{
   //Form a name with detNum and channel name
   string aName = Form("%s.zip%d.%s", histo.c_str(), fDetNum, fChannelName.c_str());

   //loop through list of histograms and see if we find a matching name
   for(uint histItr=0; histItr < fNoiseSelectionHistograms.size(); histItr++)
   {
      string checkName = fNoiseSelectionHistograms[histItr]->GetName();
      
      //if found, than store value and return
      if(aName == checkName) 
      {
	 return *fNoiseSelectionHistograms[histItr];
      }
   }   

   //if we made it this far, then no matching name was found, issue an error
   cout <<"NoiseData::RetrieveNoiseSelectionHistogram ERROR! No histogram of name " << aName <<" was found.  Did you remember to initialize the histogram?" 
	<< endl;
   exit(1);

}


//==========================================================================

//computes a running average
void NoiseData::AddToAveragePSD(const vector<double>& pulsePSD)
{ 
   if(pulsePSD.size() == 0) 
   {
      cout <<"NoiseData::AddToAveragePSD ERROR!  PSD passed to this function has length = 0." << endl;
      exit(1);
   }
   
   if(pulsePSD.size() != fAveragePSD.size() && fPSDCount != 0) 
   {
      cout <<"NoiseData::AddToAveragePSD ERROR!  PSD passed to this function has a different length than the running average." << endl;
      exit(1);
   }

   fPSDCount++; 

   //initialize fAveragePSD with the first PSD that is found
   if(fPSDCount == 1.0) fAveragePSD = pulsePSD; 

   if(fPSDCount != 1.0)
   {
      //Adding the PSD to the average, bin-by-bin
      //fAveragePSD = sqrt(sum_over_i(noisePSD_i^2)/fPSDCount)
      for(uint binCtr=0; binCtr < pulsePSD.size(); binCtr++)
      {
	 double avePSDsq = fAveragePSD[binCtr]*fAveragePSD[binCtr]; //so that we can add in quad
	 
	 avePSDsq *= (fPSDCount-1.0);   // remove old normalization 
	 avePSDsq +=  pulsePSD[binCtr]*pulsePSD[binCtr]; // add new pulse in quadrature
 	 avePSDsq *= 1.0/(fPSDCount);   // implement new normalization
	 
	 //store new value
 	 fAveragePSD[binCtr] = sqrt(avePSDsq);
      }
   }

   //in DP the DC component is set to infinity.  Setting to zero to avoid ROOT problems
   fAveragePSD[0] = 0.0;

//    cout <<"In NoiseData::AddToAveragePSD Adding to average! Number of counts = " << fPSDCount 
// 	<< endl; 


   return;
}

//============== Get Functions ======================

TH1D NoiseData::GetHistTemplateTime()
{
   return PulseTools::Vector2TH1D(fTemplateTime, fChannelName+"TemplateTime");
}

TH1D NoiseData::GetHistTemplateFFTRe()
{
   vector<double> templateFFTRe;

   for(uint binCtr=0; binCtr < fTemplateFFT.size(); binCtr++)
   {
      templateFFTRe.push_back(fTemplateFFT[binCtr].Re());
   }

   return PulseTools::Vector2TH1D(templateFFTRe, fChannelName+"TemplateFFTRe");
}

TH1D NoiseData::GetHistTemplateFFTIm()
{
   vector<double> templateFFTIm;

   for(uint binCtr=0; binCtr < fTemplateFFT.size(); binCtr++)
   {
      templateFFTIm.push_back(fTemplateFFT[binCtr].Im());
   }

   return PulseTools::Vector2TH1D(templateFFTIm, fChannelName+"TemplateFFTIm");

}

TH1D NoiseData::GetHistOptimalFilterRe()
{
   vector<double> optimalFilterRe;

   for(uint binCtr=0; binCtr < fOptimalFilter.size(); binCtr++)
   {
      optimalFilterRe.push_back(fOptimalFilter[binCtr].Re());
   }

   return PulseTools::Vector2TH1D(optimalFilterRe, fChannelName+"OptimalFilterRe");
}

TH1D NoiseData::GetHistOptimalFilterIm()
{
   vector<double> optimalFilterIm;

   for(uint binCtr=0; binCtr < fOptimalFilter.size(); binCtr++)
   {
      optimalFilterIm.push_back(fOptimalFilter[binCtr].Im());
   }

   return PulseTools::Vector2TH1D(optimalFilterIm, fChannelName+"OptimalFilterIm");

}

TH1D NoiseData::GetHistNormFFT()
{
   vector<double> tempVect;
   tempVect.push_back(fNormFFT);  //storing this as a 1 bin histogram

   return PulseTools::Vector2TH1D(tempVect, fChannelName+"NormFFT");
}

TH1D NoiseData::GetHistSampleRate()
{
   vector<double> tempVect;
   tempVect.push_back(fSampleRate);  //storing this as a 1 bin histogram

   return PulseTools::Vector2TH1D(tempVect, fChannelName+"SampleRate");
}

TH1D NoiseData::GetHistSigToNoiseSq()
{
   vector<double> tempVect;
   tempVect.push_back(fSigToNoiseSq);  //storing this as a 1 bin histogram

   return PulseTools::Vector2TH1D(tempVect, fChannelName+"SigToNoiseSq");
}

TH1D NoiseData::GetHistNoisePSD()
{
   //Vector2TH1D sets scale as xscale*nBins, so divide to take out nBins
   double xscale = ((fAveragePSD.size() != 0 || fFrequencyScale == -999999.) ? fFrequencyScale/fAveragePSD.size() : 1.0); 

   return PulseTools::Vector2TH1D(fAveragePSD, fChannelName+"NoisePSD", xscale);
}

TH1D NoiseData::GetHistNoiseFFT()
{
   return PulseTools::Vector2TH1D(fNoiseFFT, fChannelName+"NoiseFFT");
}

TH1D NoiseData::GetHistNoiseFFTsq()
{
   return PulseTools::Vector2TH1D(fNoiseFFTsq, fChannelName+"NoiseFFTsq");
}

TH1D NoiseData::GetHistQInverse()
{
   //storing the matrix in histogram form

   vector<double> qInverseVect;

   qInverseVect.push_back(fQInverse[0][0]);
   qInverseVect.push_back(fQInverse[0][1]);
   qInverseVect.push_back(fQInverse[1][0]);
   qInverseVect.push_back(fQInverse[1][1]);
   
  
   string QinverseName;

   if (fChannelName.find("S1")!=string::npos)
         QinverseName = "QS1inverse";
   else if (fChannelName.find("S2")!=string::npos) 
         QinverseName = "QS2inverse";
   else
        QinverseName = "Qinverse";


   return PulseTools::Vector2TH1D(qInverseVect, QinverseName);
}

TH1D NoiseData::GetHistEventList()
{
   return PulseTools::Vector2TH1D(fEventList, "EventList");
}



int NoiseData::GetTraceLengthType() 
{
  int traceLengthType = 0;

  int nBins = fTemplateTime.size();
  if (nBins>0) {
    if(nBins%2==0) traceLengthType = 2; // even
    else traceLengthType = 1; // odd
  }
  
  return traceLengthType;
}
  

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: CorrelationData
//Authors: L. Hsu
//Description:  This is a data storage class.  It holds noise covariance matrix elements between two channels.
//The channel order when building this class does not matter.
//
//File Import By: L. Hsu
//Creation Date: Apr. 12, 2010
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "BatRootTypes.h"
#include "CorrelationData.h"
#include "PulseTools.h"

using namespace std;

////////////////////////////////////////////////////////

//default constructor
CorrelationData::CorrelationData(const int detNum, 
				 const string& aChannelName, const string& anotherChannelName) :

  fDetNum(detNum),
  fAChannelName(aChannelName),
  fAnotherChannelName(anotherChannelName),
  fCovCount(0.0),
  fSampleRate(-999999.),
  fFrequencyScale(-999999.)
{

}

CorrelationData::~CorrelationData()
{
   
}

//==================== Identification Functions ============================

bool CorrelationData::IsChannelMatch(const string& aChannelName, const string& anotherChannelName)
{
   bool isMatch = false;

   if(aChannelName == fAChannelName && anotherChannelName == fAnotherChannelName)
   {
      isMatch = true;
   }

   //check the other permutation
   if(anotherChannelName == fAChannelName && aChannelName == fAnotherChannelName)
   {
      isMatch = true;
   }

   return isMatch;
}

//====================== Loading necessary information ===========================

void CorrelationData::LoadSampleRate(const double& sampleRate)
{ 
   fSampleRate = sampleRate;
   fFrequencyScale = sampleRate/2.0;

   return;
}


//==========================================================================

//computes a running average
void CorrelationData::AddToAverageCov(const vector<double>& aCov_Re, const vector<double>& aCov_Im)
{ 

   if(aCov_Re.size() == 0 || aCov_Im.size() == 0) 
   {
      cout <<"CorrelationData::AddToAverageCov ERROR!  Covariance vector passed to this function has length = 0." << endl;
      exit(1);
   }

   if(aCov_Re.size() != aCov_Im.size()) 
   {
      cout <<"CorrelationData::AddToAverageCov ERROR!  Real and Imaginary covariance vectors have different length!" << endl;
      exit(1);
   }
   
   if((aCov_Re.size() != fAverageCov_Re.size() || aCov_Im.size() != fAverageCov_Im.size()) && fCovCount != 0) 
   {
      cout <<"CorrelationData::AddToAverageCov ERROR!  Covariance vectors passed to this function have a different length than the running average." << endl;
      exit(1);
   }

   fCovCount++; 

   //initialize fAverageCov with the first Cov that is found
   if(fCovCount == 1.0) 
   {
      fAverageCov_Re = aCov_Re; 
      fAverageCov_Im = aCov_Im; 
   }

   if(fCovCount != 1.0)
   {
      //Adding the Cov to the average, bin-by-bin
      //fAverageCov = sum_over_i(cov_i/fCovCount)
      for(uint binCtr=1; binCtr < aCov_Re.size(); binCtr++)
      {
	 
	 fAverageCov_Re[binCtr] *= (fCovCount-1.0);   // remove old normalization 
	 fAverageCov_Im[binCtr] *= (fCovCount-1.0);   // remove old normalization 
	 
	 fAverageCov_Re[binCtr] +=  aCov_Re[binCtr]; // add new covariance (already in quad)
	 fAverageCov_Im[binCtr] +=  aCov_Im[binCtr]; // add new covariance (already in quad)
 	 
	 fAverageCov_Re[binCtr] *= 1.0/(fCovCount);   // implement new normalization
	 fAverageCov_Im[binCtr] *= 1.0/(fCovCount);   // implement new normalization

      }
   }

   //in DP the DC component is set to infinity.  Setting to zero to avoid ROOT problems
   fAverageCov_Re[0] = 0.0;
   fAverageCov_Im[0] = 0.0;

   return;
}


//============== Get Functions ======================

TH1D CorrelationData::GetHistNoiseCov()
{

   if(fAverageCov_Re.size() != fAverageCov_Im.size())
   {
      cout <<"CorrelationData::GetHistNoiseCov ERROR!  Real and Imaginary covariance vectors have different length!" << endl;
      exit(1);
   }

   //Vector2TH1D sets scale as xscale*nBins, so divide to take out nBins
   double xscale = ((fAverageCov_Re.size() != 0 || fFrequencyScale == -999999.) ? 
		    fFrequencyScale/fAverageCov_Re.size() : 1.0); 

   //Take the amplitude because the covariance is complex
   vector<double> ampAverageCov;

   for(uint binCtr = 0; binCtr < fAverageCov_Re.size(); binCtr++)
   {
      ampAverageCov.push_back(sqrt(fAverageCov_Re[binCtr]*fAverageCov_Re[binCtr] + fAverageCov_Im[binCtr]*fAverageCov_Im[binCtr]));
   }

   return PulseTools::Vector2TH1D(ampAverageCov, fAChannelName+fAnotherChannelName+"Cov", xscale);
}


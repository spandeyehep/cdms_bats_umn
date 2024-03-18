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

#ifndef CORRELATIONDATA_H
#define CORRELATIONDATA_H

#include <vector>
#include <map>

#include "TComplex.h"
#include "TH1D.h"
#include "TMatrixD.h"

using namespace std;

//!This is a data storage class for noise "covariance" between two channels
class CorrelationData 
{
      friend class NoiseBuilder; 

   public:

      CorrelationData(const int detNum, 
		      const string& aChannelName, const string& anotherChannelName);  //constructor
  
      ~CorrelationData(); //destructor


      // access channel identifying information
      string  GetChannelNames()     const  { return fAChannelName+fAnotherChannelName; } //returns the two channel names
      int     GetDetectorNum()      const  { return fDetNum; }
      bool    IsChannelMatch(const string& aChannelName, const string& anotherChannelName);


      // access noise quantities as histograms - READ only access
      TH1D GetHistNoiseCov(); 
      

      // sample rate is for scaling the Cov and later storage
      void LoadSampleRate(const double& fsample); //this must come before LoadTemplate


      // for calculation 
      void AddToAverageCov(const vector<double>& aCov_Re, const vector<double>& aCov_Im); 


   private:

      CorrelationData();  //constructor
      
      //Identifying info
      int fDetNum;          
      string fAChannelName;
      string fAnotherChannelName;
      

      //Average Cov - sum of covariance over many noise traces
      //has real and imaginary components
      vector<double>   fAverageCov_Re;
      vector<double>   fAverageCov_Im;
      double           fCovCount;


      //For normalizations
      double           fSampleRate;
      double           fFrequencyScale;
      
};

#endif /* CORRELATIONDATA_H */

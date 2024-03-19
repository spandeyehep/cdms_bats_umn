////////////////////////////////////////////////////////////////////////
// Class:  PSDIntegralPhonon Class
// Author: Jianjie Zhang
// Description: This class calculates the power of phonon signals in frequency bands
// 4-60, 10-20, 20-30, 30-50, 50-70, and 70-100kHz.
//
// File imported by: Jianjie Zhang
// Creation date: Oct. 19, 2010
// Modifications:
//
// Dec. 2012: changed to 0-10, 1-10 and total for LF noise studies (LLH)
//
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "PSDIntegralPhonon.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
PSDIntegralPhonon::PSDIntegralPhonon()
{
   //   cout <<"Hello from PSDIntegralPhonon()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "PSDIntegralPhonon"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

PSDIntegralPhonon::~PSDIntegralPhonon()
{
//   cout <<"Goodbye from PSDIntegralPhonon()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void PSDIntegralPhonon::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("PSDint0to1", initVal));
   fRQList.insert(pair<string,double>("PSDint1to10", initVal));
   fRQList.insert(pair<string,double>("PSDintall", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//optional function, do it here and not in the constructor
// void PSDIntegralPhonon::InitializeParameters()
// {
//
//    return;
// }

//This is the main call for your analysis
void PSDIntegralPhonon::DoCalc(const vector<double>& aPulse, double sampleRate)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"PSDIntegralPhonon::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

   //do your calculation here!

   vector<double> outPSD;
   PulseTools::Time2PSD( aPulse, sampleRate, outPSD ); // outPSD is the amplitude in unit of V/sqrt(Hz).

   // Square outPSD to get the PSD in units of power
   for( vector<double>::iterator outPSDit = outPSD.begin(); outPSDit != outPSD.end(); outPSDit++)
   {
     *outPSDit = (*outPSDit)*(*outPSDit);
   }
   
   int const nFFT = outPSD.size();
   double frequencyScale = sampleRate/2.; //Nyquist

//    cout <<"nFFT = " << outPSD.size() <<", and frequencyScale = " << frequencyScale 
// 	<< endl;

   // lowf and highf define the boundaries of each frequency band in units of kHz.
   double const lowf[3]  = {0.  , 1.e3 , 0.};
   double const highf[3] = {1e3, 10.e3, frequencyScale};
   double power[3];

   for( int band = 0; band < 3; band++ )
   {
     //The upper limit is exclusive in Area()
       power[band] = PulseTools::Area( outPSD, (int)( ceil( lowf[band] / (frequencyScale/nFFT) ) ),  (int)( ceil( highf[band] / (frequencyScale/nFFT) ) ) ) * (frequencyScale/nFFT);
   }

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {

     fRQList["PSDint0to1"] = power[0]; 
     fRQList["PSDint1to10"] = power[1];
     fRQList["PSDintall"] = power[2]; 
    
   }

   return;
}

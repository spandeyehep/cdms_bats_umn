///////////////////////////////////////////////////////////////////////////////// 
//Class Name: NoiseSelector
//Author:  M. Kos
//Description: This class select noise traces as done in PipeFitter code.
//  This is used by the PipeFitPhonon class to determine if a fit needs 
//  to be perfomred. 
// 
// Original authors of the PipeFitter code are L. Duong, J. Yoo, and E. Ramberg 
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 




#include <iostream>

#include "NoiseSelector.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
NoiseSelector::NoiseSelector() :
   kLowerBin(500), //FIXME, move to config
   kUpperBin(700), //FIXME, move to config
   kThresArea(2000.), //FIXME, move to config
   kSigmaRMS(3.0) //FIXME, move to config
{
  
   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "NoiseSelector"; 
   fStoreRQs = true;
   
   //Default is set to false (not noise)
   fIsNoise = false;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

NoiseSelector::~NoiseSelector()
{

}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void NoiseSelector::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("isnoise", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//do it here and not in the constructor
void NoiseSelector::InitializeParameters()
{
 
   return;
}

bool NoiseSelector::IsNoisePhonon(const vector<double>& pulsevector, double thvalue, double sigma, int zipnum){
  
   int nbins = pulsevector.size();
  if(nbins == 0)
  {
    cerr <<"PulseTools::IsNoisePhonon - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  bool noise = 0;

  //[1]check if RMS is not too flat
  //check rms limits
  //FIXME, constant in RMS
  if (fRMS < 1.0) return noise = 1;
    
  //if sum of net trace is less than thvalue (threshold value)
  //[2]check over which bins the area is calculated
  //and how rsum_trace is calculated
    double Area = PulseTools::Area(pulsevector);
    if (Area < thvalue) return noise = 1;

  //[3]if max adc is less than sigma*rms (sigma=3)
    if (fMaxADC < sigma*fRMS) return noise = 1;
    
  //[4] MAX_ADC_BIN cut
  //Unless the max_adc_bin is sensible no fit  
  
  int maxadcbinlimit = 499;//hard coded now, FIXME
  double maxadcpt = PulseTools::MaxADCPoint(pulsevector,499,700);//hard coded now, FIXME
  if(maxadcpt <= (double) maxadcbinlimit) return noise = 1;

  //[5] e-glitches cut
  // FIXME: constants in array, put in config file
  double ba_max_adc[30] = {4000,5000,6000,1800,5000,2000,
			   3000,4000,2600,3000,2000,2800,
			   3000,5000,3000,5000,5000,5000,
			   3000,5000,3000,5000,5000,5000,
			   5000,5000,3000,5000,5000,5000};

  double adcarea = PulseTools::Area(pulsevector,499,700);
  if (((adcarea/fMaxADC/200.0) < 0.3) && fMaxADC > 0.010*ba_max_adc[zipnum-1]){
        return noise = 1;
   
  }
 
  return noise;
  

}


bool NoiseSelector::IsNoiseCharge(const vector<double> &pulsevector,int lowbin,int hibin){

  int nbins = pulsevector.size();
  if(nbins == 0)
  {
    cerr <<"PulseTools::IsNoiseCharge - ERROR! empty pulse passed into this function" << endl;
    exit(1);
  }

  bool noise = 0;
  int hbin,lbin;
  
  //if bin limits not specified use whole range
  if (lowbin < 0) lbin = 0;
  if (hibin <= 0) hbin = nbins;
  if (lowbin >= 0) lbin = lowbin;
  if (hibin > 0) hbin = hibin; 
  
  
  //[1]check if RMS is not too flat
  //check rms limits for charge channel FIXME
  if (fRMS < 1.0) return noise = 1; //1.0?, FIXME

  return noise;
}

//This is the main call for your analysis
void NoiseSelector::DoCalc(const vector<double>& aPulse, const string &channelType,int zipnum, double RMS,double MaxADC)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"NoiseSelector::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

  //set RMS
  fRMS = RMS;

  //set MaxADC
  fMaxADC = MaxADC;
  
   //determine if charge pulse is noise
  if (channelType == "charge"){
    fIsNoise = IsNoiseCharge(aPulse,kLowerBin,kUpperBin); //FIXME  
  }

  //determine if phonon pulse is noise
  if (channelType == "phonon"){
    //Minimum area is 2000, sigma above RMS is 3.0 FIXME
    fIsNoise = IsNoisePhonon(aPulse,kThresArea,kSigmaRMS,zipnum);
  }

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
     fRQList["isnoise"] = (double)fIsNoise;
   }

   return;
}




void  NoiseSelector::DoCalc(map<string,double>& POFmap,map<string,double>& R20map, double ptThresh, double rdelThresh)
{
  // selection of noise event base on total phonon OF and RTFTwalk rdel

  //  ==== calculate PT ====

  double ptOF(0.);
  map<string,double>::iterator itP;
  for (itP=POFmap.begin(); itP!=POFmap.end();itP++)
   {
     ptOF += (*itP).second;
   }
       

  // ==== calculate Rdel ====

  double rdel = 0;

  if (R20map.size()>0)
   {   
    double MinR20(999999.);
    string primChan;
    map<string,double>::iterator itT;
    for (itT=R20map.begin(); itT!=R20map.end();itT++)
     {
      double R20 = (*itT).second;
      if(R20 < MinR20)
        {
          MinR20 = R20;
          primChan = (*itT).first;
        } 
     }
       

    double xdel=0.;
    double ydel=0.;
 
    if (primChan=="PA") 
      { xdel = (R20map["PA"]-R20map["PD"])*1e6;
       ydel = (-R20map["PA"]+R20map["PB"])*1e6; }
        
    if (primChan=="PB") 
      { xdel = (R20map["PB"]-R20map["PC"])*1e6;
        ydel = (R20map["PB"]-R20map["PA"])*1e6; }

    if (primChan=="PC") 
      {xdel = (-R20map["PC"]+R20map["PB"])*1e6;
        ydel = (R20map["PC"]-R20map["PD"])*1e6; }

    if (primChan=="PD") 
      {xdel = (-R20map["PD"]+R20map["PA"])*1e6;
       ydel = (-R20map["PD"]+R20map["PC"])*1e6; }


    rdel = sqrt(pow(xdel,2)+pow(ydel,2));
   }

 // ===== cuts ======
  
   if(ptOF*1e8>ptThresh && rdel<rdelThresh)
 	fIsNoise = false;
   else
        fIsNoise = true;

   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
     fRQList["isnoise"] = (double)fIsNoise;
   }

 return;
}

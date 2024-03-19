#include <iostream>

#include "TRandom3.h"

#include "SimulateFromRandoms.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
SimulateFromRandoms::SimulateFromRandoms()
{
   //   cout <<"Hello from SimulateFromRandoms()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "SimulateFromRandoms"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

SimulateFromRandoms::~SimulateFromRandoms()
{
//   cout <<"Goodbye from SimulateFromRandoms()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void SimulateFromRandoms::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("SIMamp", initVal));
   fRQList.insert(pair<string,double>("SIMdelay", initVal));
   fRQList.insert(pair<string,double>("SIMlibnum", initVal));
   fRQList.insert(pair<string,double>("SIMEventNumber", initVal));
   fRQList.insert(pair<string,double>("SIMSeriesNumber", initVal));
   fRQList.insert(pair<string,double>("SIMAvgX", initVal));
   fRQList.insert(pair<string,double>("SIMAvgY", initVal));
   fRQList.insert(pair<string,double>("SIMAvgZ", initVal));
   fRQList.insert(pair<string,double>("SIMRecoilEnergy", initVal));
   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}


// Construct the event RQ list that is handed off to BatOutputManager

//optional function, do it here and not in the constructor
// void SimulateFromRandoms::InitializeParameters()
// {
//
//    return;
// }


//Simple routine to simulate the PT pulse with a flat energy spectrum.
//User chooses the bounds and can choose whether or not to 
//implement an (adhoc) model of position dependence
void SimulateFromRandoms::SimPTwFlatSpec(vector<double>& aPulse, double minE, double maxE, 
					 double ptcal, const int seed)
{
  //check that PT template has been loaded
  if(fPTemplate.size() == 0)
  {
    cerr <<"SimulateFromRandoms::SimPTwFlatSpec ERROR!  PT template not loaded!"
	 << endl;
    exit(1);
  }

  //check for mismatch in template and pulse lengths
  if(aPulse.size() != fPTemplate.size())
  {
    cerr <<"SimulateFromRandoms::SimPTwFlatSpec ERROR!  trace and template lengths don't match!"
	 << endl;
    exit(1);
  }

  //Generate fake energy by sampling a flat spectrum from minE to maxE
  TRandom3 rand(seed);
  double roughAmpkev = rand.Uniform(minE, maxE); //in keV

  double trueAmp = roughAmpkev/ptcal; //in BatRoot normalized units
  double trueDelay = 0.; 
              
  //make sure there is a different pulse for each call
  vector<double> simulatedPulse  = PulseTools::ConstructFakePulse(trueAmp,
								  trueDelay,
								  aPulse, 
								  fPTemplate); 
  
  aPulse = simulatedPulse;

  //store the true amplitude and delay of the simulated pulse
  if(fStoreRQs) {
    fRQList["SIMPTamp"] = trueAmp;
    fRQList["SIMPTdelay"] = trueDelay;
  }

  return;
}



void SimulateFromRandoms::SimPwDataTemplate(vector<double>& noisePulse, vector<double>& templatePulse, double ptE, double ptcal, map<string, double> dataAmp, string chanName, int libNum, double evnum, double sernum, double DMCAvgX, double DMCAvgY, double DMCAvgZ, double DMCRecoilEnergy)
{
    //check that PT template has been loaded
    if(templatePulse.size() == 0)
    {
	cerr <<"SimulateFromRandoms::SimPwDataTemplate ERROR!  template not loaded!"
	     << endl;
	exit(1);
    }

    //check for mismatch in template and pulse lengths
    if(noisePulse.size() != templatePulse.size())
    {
      cerr <<"SimulateFromRandoms::SimPwDataTemplate ERROR!  trace and template lengths don't match!"
	   << endl;
      exit(1);
    }

    double trueAmp = ptE / ptcal; //in BatRoot normalized units
    double trueDelay = 0.; 
    
    //make sure there is a different pulse for each call
    vector<double> simulatedPulse  = PulseTools::ConstructFakePulse(trueAmp,
								    trueDelay,
								    noisePulse, 
								    templatePulse); 
    
    noisePulse = simulatedPulse;
    
    //store the true amplitude and delay of the simulated pulse
    if(fStoreRQs) 
    {
	double pulseMax = *std::max_element(templatePulse.begin(), templatePulse.end());
	double pulseE = ptE / ptcal  * dataAmp[chanName] / dataAmp["PT"];
	fRQList["SIMamp"] = pulseE;
	fRQList["SIMdelay"] = trueDelay;
	fRQList["SIMlibnum"] = libNum;
	fRQList["SIMEventNumber"] = evnum;
	fRQList["SIMSeriesNumber"] = sernum;
	fRQList["SIMAvgX"] = DMCAvgX;
	fRQList["SIMAvgY"] = DMCAvgY;
	fRQList["SIMAvgZ"] = DMCAvgZ;
	fRQList["SIMRecoilEnergy"] = DMCRecoilEnergy;
    }

    return;

}



// Construct a fake pulse with a fixed amplitude. Simulated pulse amplitude
// is assumed to be in keV and is scaled appropriately by the supplied
// calibration constant to match the noise in amps.
void SimulateFromRandoms::SimPMonoenergetic(vector<double>& aPulse, double pulseE, 
           double ptcal)
{
  //check that PT template has been loaded
  if(fPTemplate.size() == 0)
  {
    cerr <<"SimulateFromRandoms::SimPMonoenergetic ERROR!  PT template not loaded!"
   << endl;
    exit(1);
  }

  //check for mismatch in template and pulse lengths
  if(aPulse.size() != fPTemplate.size())
  {
    cerr <<"SimulateFromRandoms::SimPMonoenergetic ERROR!  trace and template lengths don't match!"
   << endl;
    exit(1);
  }

  double trueAmp = pulseE/ptcal; //in BatRoot normalized units
  double trueDelay = 0.; 
              
  //make sure there is a different pulse for each call
  vector<double> simulatedPulse  = PulseTools::ConstructFakePulse(trueAmp,
                  trueDelay,
                  aPulse, 
                  fPTemplate); 
  
  aPulse = simulatedPulse;

  //store the true amplitude and delay of the simulated pulse
  if(fStoreRQs) {
    fRQList["SIMamp"] = trueAmp;
    fRQList["SIMdelay"] = trueDelay;
  }

  return;
}


// Construct a fake pulse with a fixed amplitude. Simulated pulse amplitude
// is assumed to be in keV and is scaled appropriately by the supplied
// calibration constant to match the noise in amps.
void SimulateFromRandoms::SimQMonoenergetic(vector<double>& aPulse, map<string, double>& pulseE, 
                          double qcal, string chanName)
{
  //check that Q template has been loaded
  if(fQTemplate.size() == 0)
  {
    cerr <<"SimulateFromRandoms::SimQMonoenergetic ERROR! Q template not loaded!"
   << endl;
    exit(1);
  }

  //check for mismatch in template and pulse lengths
  if(aPulse.size() != fQTemplate.size())
  {
    cerr <<"SimulateFromRandoms::SimQMonoenergetic ERROR!  trace and template lengths don't match!"
   << endl;
    exit(1);
  }

  // determine the crosstalk channel name
  string chanNameX;
  if(chanName.compare("QIS1") == 0)
    chanNameX = "QOS1";
  else if(chanName.compare("QOS1") == 0)
    chanNameX = "QIS1";
  else if(chanName.compare("QOS2") == 0)
    chanNameX = "QIS2";
  else if(chanName.compare("QIS2") == 0)
    chanNameX = "QOS2";

  double trueAmp = pulseE[chanName]/qcal; //in BatRoot normalized units
  double trueDelay = 0.; 
  double trueAmpX = pulseE[chanNameX]/qcal; //in BatRoot normalized units
  double trueDelayX = 0.; 
              
  // Step 1: inject fake pulse into random trace
  vector<double> simulatedPulseNoX  = PulseTools::ConstructFakePulse(trueAmp,
                  trueDelay,
                  aPulse, 
                  fQTemplate); 

  // Step 2: inject crosstalk pulse into (fake pulse + random noise)
  vector<double> simulatedPulse  = PulseTools::ConstructFakePulse(trueAmpX,
                  trueDelayX,
                  simulatedPulseNoX, 
                  fQTemplateX); 
  
  // Step 3: overwrite the random pulse
  aPulse = simulatedPulse;

  // store the true amplitude and delay of the simulated pulse (don't save
  // crosstalk amplitudes, which should be the same as the pulse ones).
  if(fStoreRQs && chanName.find("X") == string::npos) {
    fRQList["SIMamp"] = trueAmp;
    fRQList["SIMdelay"] = trueDelay;
  }

  return;
}



void SimulateFromRandoms::LoadQTemplates(const vector<double> aQTemplate, const vector<double> aQTemplateX)
{
  fQTemplate = aQTemplate;
  fQTemplateX = aQTemplateX;
  return;
}

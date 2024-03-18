///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataiZIP
//Authors: L. Hsu
//Description: This class is intended for the 1-inch, 12-channel iZIP, with
//mercedes phonon channel layout. Generates primary rrq's for analysis (xy-delays, 
//partitions, yields, timing quantities).  
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <limits> //it was found that TMath.h does not always have #include<limits>, so it was explicitly
//included here [ANV]

#include "TMath.h"

#include "GenRRQDataiZIPSoudan.h"
#include "BatCalibTypes.h"


GenRRQDataiZIPSoudan::GenRRQDataiZIPSoudan(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fDetType(-999999),
   fIsFirstProduction(0),
   fCheckOFChargeXRQ(false),
   fCheckOFChargeRQ(false),
   fCheckF5ChargeXRQ(false),
   fPreviousEventSeriesNumber(-999999),
   fNWorkingPhonon(0),
   fNRLookupTable(NULL)
{
//   cout <<"Hello from GenRRQDataiZIPSoudan! " << endl;

  // for side 1
  kTheta1Vect[0] = 150.*(TMath::Pi()/180.);
  kTheta1Vect[1] = 270.*(TMath::Pi()/180.);
  kTheta1Vect[2] = 30.*(TMath::Pi()/180.);

  // for side 2
  kTheta2Vect[0] = 330.*(TMath::Pi()/180.);
  kTheta2Vect[1] = 90.*(TMath::Pi()/180.);
  kTheta2Vect[2] = 210.*(TMath::Pi()/180.);

}

GenRRQDataiZIPSoudan::~GenRRQDataiZIPSoudan()
{
//   cout <<"Goodbye from GenRRQDataiZIPSoudan()" << endl;
}


//initialize desired output variables here (this happens before looping over events in the zip trees)
void GenRRQDataiZIPSoudan::ConstructRRQList()
{
     double initVal = -999999.;

     // ======== necessery RRQs for CAP =======
     // Note anything added outside of here is not going to be in the output

     fRRQList.insert(pair<string,double>("Empty", initVal));
     fRRQList.insert(pair<string,double>("DetType", initVal));
 

     // ========== Charge OF  ==========

     if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
     {

       // energy
       fRRQList.insert(pair<string,double>("qi1OF", initVal));
       fRRQList.insert(pair<string,double>("qo1OF", initVal));
       fRRQList.insert(pair<string,double>("qi2OF", initVal));
       fRRQList.insert(pair<string,double>("qo2OF", initVal));

       fRRQList.insert(pair<string,double>("qimaxOF", initVal));
       
       fRRQList.insert(pair<string,double>("qi1OF0", initVal));
       fRRQList.insert(pair<string,double>("qo1OF0", initVal));
       fRRQList.insert(pair<string,double>("qi2OF0", initVal));
       fRRQList.insert(pair<string,double>("qo2OF0", initVal));

       fRRQList.insert(pair<string,double>("qsum1OF", initVal));
       fRRQList.insert(pair<string,double>("qsum2OF", initVal));
       fRRQList.insert(pair<string,double>("qsummaxOF", initVal));

       // luke phonon related RRQs
       fRRQList.insert(pair<string,double>("plukeqOF", initVal));
       fRRQList.insert(pair<string,double>("plukeqOFi", initVal));
       fRRQList.insert(pair<string,double>("pgqOF", initVal));
      
       // partition
       fRRQList.insert(pair<string,double>("qrpart1OF", initVal));
       fRRQList.insert(pair<string,double>("qrpart2OF", initVal));
       fRRQList.insert(pair<string,double>("qrpartsym1OF", initVal));
       fRRQList.insert(pair<string,double>("qrpartsym2OF", initVal));
       fRRQList.insert(pair<string,double>("qzpartOF", initVal));
       fRRQList.insert(pair<string,double>("qzpartOFi", initVal));
       fRRQList.insert(pair<string,double>("qzpartOFo", initVal));
     }



      
     // ========== charge F5  ==========
     
     if(fCheckF5ChargeXRQ)
     {
       // energy
       fRRQList.insert(pair<string,double>("qi1F5", initVal));
       fRRQList.insert(pair<string,double>("qo1F5", initVal));
       fRRQList.insert(pair<string,double>("qi2F5", initVal));
       fRRQList.insert(pair<string,double>("qo2F5", initVal));

       fRRQList.insert(pair<string,double>("qsum1F5", initVal));
       fRRQList.insert(pair<string,double>("qsum2F5", initVal));
       fRRQList.insert(pair<string,double>("qsummaxF5", initVal));

       // luke phonon related RRQs
       fRRQList.insert(pair<string,double>("plukeqF5", initVal));
       fRRQList.insert(pair<string,double>("pgqF5", initVal));
       
       //partition
       fRRQList.insert(pair<string,double>("qrpart1F5", initVal));
       fRRQList.insert(pair<string,double>("qrpart2F5", initVal));
       fRRQList.insert(pair<string,double>("qrpartsym1F5", initVal));
       fRRQList.insert(pair<string,double>("qrpartsym2F5", initVal));
       fRRQList.insert(pair<string,double>("qzpartF5", initVal));
     }



     // ========== phonon  OF (single channels)  ==========

     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {

       // energy
       fRRQList.insert(pair<string,double>("pa1OF", initVal));
       fRRQList.insert(pair<string,double>("pb1OF", initVal));
       fRRQList.insert(pair<string,double>("pc1OF", initVal));
       fRRQList.insert(pair<string,double>("pd1OF", initVal));
       
       fRRQList.insert(pair<string,double>("pa2OF", initVal));
       fRRQList.insert(pair<string,double>("pb2OF", initVal));
       fRRQList.insert(pair<string,double>("pc2OF", initVal));
       fRRQList.insert(pair<string,double>("pd2OF", initVal));
       
       fRRQList.insert(pair<string,double>("pa1OF0", initVal)); 
       fRRQList.insert(pair<string,double>("pb1OF0", initVal));
       fRRQList.insert(pair<string,double>("pc1OF0", initVal));
       fRRQList.insert(pair<string,double>("pd1OF0", initVal));
       
       fRRQList.insert(pair<string,double>("pa2OF0", initVal)); 
       fRRQList.insert(pair<string,double>("pb2OF0", initVal));
       fRRQList.insert(pair<string,double>("pc2OF0", initVal));
       fRRQList.insert(pair<string,double>("pd2OF0", initVal));
       
       fRRQList.insert(pair<string,double>("psumOF", initVal));
       fRRQList.insert(pair<string,double>("psumOF0", initVal));

       fRRQList.insert(pair<string,double>("psum1OF", initVal));
       fRRQList.insert(pair<string,double>("psum2OF", initVal));
       fRRQList.insert(pair<string,double>("psumi1OF", initVal));
       fRRQList.insert(pair<string,double>("psumi2OF", initVal));
       fRRQList.insert(pair<string,double>("psumo1OF", initVal));
       fRRQList.insert(pair<string,double>("psumo2OF", initVal));

       // primary sensor
       fRRQList.insert(pair<string,double>("pprimechan1OF", initVal));
       fRRQList.insert(pair<string,double>("pprimechan2OF", initVal));
       fRRQList.insert(pair<string,double>("pprimechani1OF", initVal));
       fRRQList.insert(pair<string,double>("pprimechani2OF", initVal));

       //partition
       fRRQList.insert(pair<string,double>("pxpart1OF", initVal));
       fRRQList.insert(pair<string,double>("pypart1OF", initVal));
       fRRQList.insert(pair<string,double>("prxypart1OF", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart1OF", initVal)); 
       fRRQList.insert(pair<string,double>("prpartsym1OF", initVal)); 
       fRRQList.insert(pair<string,double>("pthetapart1OF", initVal));

       fRRQList.insert(pair<string,double>("pxpart2OF", initVal));
       fRRQList.insert(pair<string,double>("pypart2OF", initVal));
       fRRQList.insert(pair<string,double>("prxypart2OF", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart2OF", initVal));
       fRRQList.insert(pair<string,double>("prpartsym2OF", initVal));
       fRRQList.insert(pair<string,double>("pthetapart2OF", initVal));

       fRRQList.insert(pair<string,double>("pxpartOF", initVal));
       fRRQList.insert(pair<string,double>("pypartOF", initVal));
       fRRQList.insert(pair<string,double>("prxypartOF", initVal));
       fRRQList.insert(pair<string,double>("prpartOF", initVal));
       fRRQList.insert(pair<string,double>("prpartsymOF", initVal));
       fRRQList.insert(pair<string,double>("pthetapartOF", initVal));
       fRRQList.insert(pair<string,double>("pzsumpartOF", initVal));
 

       // recoil E/yield (need charge OF)
 
       if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
       {
         fRRQList.insert(pair<string,double>("precoilsumOF", initVal));
         fRRQList.insert(pair<string,double>("ysumOF", initVal));
         fRRQList.insert(pair<string,double>("ygsumOF", initVal));
       }

       //don't need charge OF if use gamma or neutron assumption [ANV]
       fRRQList.insert(pair<string,double>("precoilsumOFg", initVal));
       fRRQList.insert(pair<string,double>("precoilsumOFnL", initVal));

     }

	 
	 if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhononDMC", fDetNum))
     {

       // energyDMCOF
		 
       fRRQList.insert(pair<string,double>("pa1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pb1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pc1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pd1dmcOF", initVal));
       
       fRRQList.insert(pair<string,double>("pa2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pb2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pc2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pd2dmcOF", initVal));
              
       fRRQList.insert(pair<string,double>("psumdmcOF", initVal));
       
       fRRQList.insert(pair<string,double>("psum1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("psum2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("psumi1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("psumi2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("psumo1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("psumo2dmcOF", initVal));

     
       //partition
       fRRQList.insert(pair<string,double>("pxpart1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pypart1dmcOF", initVal));
       fRRQList.insert(pair<string,double>("prxypart1dmcOF", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart1dmcOF", initVal)); 
       fRRQList.insert(pair<string,double>("prpartsym1dmcOF", initVal)); 
       fRRQList.insert(pair<string,double>("pthetapart1dmcOF", initVal));

       fRRQList.insert(pair<string,double>("pxpart2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pypart2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("prxypart2dmcOF", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("prpartsym2dmcOF", initVal));
       fRRQList.insert(pair<string,double>("pthetapart2dmcOF", initVal));

       fRRQList.insert(pair<string,double>("pxpartdmcOF", initVal));
       fRRQList.insert(pair<string,double>("pypartdmcOF", initVal));
       fRRQList.insert(pair<string,double>("prxypartdmcOF", initVal));
       fRRQList.insert(pair<string,double>("prpartdmcOF", initVal));
       fRRQList.insert(pair<string,double>("prpartsymdmcOF", initVal));
       fRRQList.insert(pair<string,double>("pthetapartdmcOF", initVal));
       fRRQList.insert(pair<string,double>("pzsumpartdmcOF", initVal));
 
	 }
     

     // ==========  Phonon PT OF ==========
    
     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
     {
       // energy
       fRRQList.insert(pair<string,double>("ptOF", initVal));
       fRRQList.insert(pair<string,double>("ptOF0", initVal));
      
       // yield/ recoil E  (need charge OF !)
       if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
       {
         fRRQList.insert(pair<string,double>("precoiltOF", initVal));
         fRRQList.insert(pair<string,double>("ytOF", initVal));
         fRRQList.insert(pair<string,double>("ygtOF", initVal));
       }

       //don't need charge OF if use gamma or neutron assumption [ANV]
       fRRQList.insert(pair<string,double>("precoiltOFg", initVal));

      }
	 
	 
	 if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononDMC", fDetNum))
     {
		 fRRQList.insert(pair<string,double>("ptdmcOF", initVal));
	 }


	 
     // ==========  Phonon PS1/PS2 OF ==========

     if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhonon", fDetNum))
     {
        // energy/partition 
        fRRQList.insert(pair<string,double>("ps1OF", initVal));
        fRRQList.insert(pair<string,double>("ps2OF", initVal));
        fRRQList.insert(pair<string,double>("pzpartOF", initVal));
     }


	 
	 if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhononDMC", fDetNum))
     {
		 // energy/partition 
        fRRQList.insert(pair<string,double>("ps1dmcOF", initVal));
        fRRQList.insert(pair<string,double>("ps2dmcOF", initVal));
        fRRQList.insert(pair<string,double>("pzpartdmcOF", initVal));
     }


     // ==========  Phonon PT NF ==========

   
     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum)) 
     {
       // energy
       fRRQList.insert(pair<string,double>("ptNF", initVal));
       fRRQList.insert(pair<string,double>("ptNF0", initVal));
    
       // yield: need charge OF
       if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
        {
          fRRQList.insert(pair<string,double>("precoiltNF", initVal));
          fRRQList.insert(pair<string,double>("precoiltNFi", initVal));
          fRRQList.insert(pair<string,double>("ytNF", initVal));
          fRRQList.insert(pair<string,double>("ytNFi", initVal));
          fRRQList.insert(pair<string,double>("ygtNF", initVal));
        }

       //don't need charge OF if use gamma or neutron assumption [ANV]
       fRRQList.insert(pair<string,double>("precoiltNFg", initVal));
       fRRQList.insert(pair<string,double>("precoiltNFnL", initVal));
     }



     // ========== OF resolution ==========

     if (fIOMan.IsOFresFilled()) {

       fRRQList.insert(pair<string,double>("pa1delayres", initVal));
       fRRQList.insert(pair<string,double>("pb1delayres", initVal));
       fRRQList.insert(pair<string,double>("pc1delayres", initVal));
       fRRQList.insert(pair<string,double>("pd1delayres", initVal));
       fRRQList.insert(pair<string,double>("pa2delayres", initVal));
       fRRQList.insert(pair<string,double>("pb2delayres", initVal));
       fRRQList.insert(pair<string,double>("pc2delayres", initVal));
       fRRQList.insert(pair<string,double>("pd2delayres", initVal));


       fRRQList.insert(pair<string,double>("pa1ampres", initVal));
       fRRQList.insert(pair<string,double>("pb1ampres", initVal));
       fRRQList.insert(pair<string,double>("pc1ampres", initVal));
       fRRQList.insert(pair<string,double>("pd1ampres", initVal));
       fRRQList.insert(pair<string,double>("pa2ampres", initVal));
       fRRQList.insert(pair<string,double>("pb2ampres", initVal));
       fRRQList.insert(pair<string,double>("pc2ampres", initVal));
       fRRQList.insert(pair<string,double>("pd2ampres", initVal));


       fRRQList.insert(pair<string,double>("qi1delayres", initVal));
       fRRQList.insert(pair<string,double>("qo1delayres", initVal));
       fRRQList.insert(pair<string,double>("qi2delayres", initVal));
       fRRQList.insert(pair<string,double>("qo2delayres", initVal));

       fRRQList.insert(pair<string,double>("qi1ampres", initVal));
       fRRQList.insert(pair<string,double>("qo1ampres", initVal));
       fRRQList.insert(pair<string,double>("qi2ampres", initVal));
       fRRQList.insert(pair<string,double>("qo2ampres", initVal));
     }
 
     // ==========  Phonon Integral  ==========
    
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {

       // energy
       fRRQList.insert(pair<string,double>("pa1INT", initVal));
       fRRQList.insert(pair<string,double>("pb1INT", initVal));
       fRRQList.insert(pair<string,double>("pc1INT", initVal));
       fRRQList.insert(pair<string,double>("pd1INT", initVal));

       fRRQList.insert(pair<string,double>("pa2INT", initVal));
       fRRQList.insert(pair<string,double>("pb2INT", initVal));
       fRRQList.insert(pair<string,double>("pc2INT", initVal));
       fRRQList.insert(pair<string,double>("pd2INT", initVal));
       
       fRRQList.insert(pair<string,double>("psum1INT", initVal));
       fRRQList.insert(pair<string,double>("psum2INT", initVal));
       fRRQList.insert(pair<string,double>("psumINT", initVal));

       fRRQList.insert(pair<string,double>("psumi1INT", initVal));
       fRRQList.insert(pair<string,double>("psumi2INT", initVal));
       fRRQList.insert(pair<string,double>("psumo1INT", initVal));
       fRRQList.insert(pair<string,double>("psumo2INT", initVal));
       
      

       // partitions 

       fRRQList.insert(pair<string,double>("pxpart1INT", initVal));
       fRRQList.insert(pair<string,double>("pypart1INT", initVal));
       fRRQList.insert(pair<string,double>("prxypart1INT", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart1INT", initVal));
       fRRQList.insert(pair<string,double>("prpartsym1INT", initVal));
       fRRQList.insert(pair<string,double>("pthetapart1INT", initVal));

       fRRQList.insert(pair<string,double>("pxpart2INT", initVal));
       fRRQList.insert(pair<string,double>("pypart2INT", initVal));
       fRRQList.insert(pair<string,double>("prxypart2INT", initVal)); //check if needed
       fRRQList.insert(pair<string,double>("prpart2INT", initVal));
       fRRQList.insert(pair<string,double>("prpartsym2INT", initVal));
       fRRQList.insert(pair<string,double>("pthetapart2INT", initVal));

      
       fRRQList.insert(pair<string,double>("pxpartINT", initVal));
       fRRQList.insert(pair<string,double>("pypartINT", initVal));
       fRRQList.insert(pair<string,double>("prxypartINT", initVal));
       fRRQList.insert(pair<string,double>("prpartINT", initVal));
       fRRQList.insert(pair<string,double>("prpartsymINT", initVal));
       fRRQList.insert(pair<string,double>("pthetapartINT", initVal));
       fRRQList.insert(pair<string,double>("pzsumpartINT", initVal));


       // yields (with charge OF)

       if(fCheckOFChargeXRQ | fCheckOFChargeRQ)
        {
           fRRQList.insert(pair<string,double>("precoilsumINT", initVal));
           fRRQList.insert(pair<string,double>("ygsumINT", initVal));
           fRRQList.insert(pair<string,double>("ysumINT", initVal));
        }


       // yields (with charge F5)
       if(fCheckF5ChargeXRQ)
    
       {
         fRRQList.insert(pair<string,double>("precoilsumF5INT", initVal));
         fRRQList.insert(pair<string,double>("ygsumF5INT", initVal));
         fRRQList.insert(pair<string,double>("ysumF5INT", initVal));
       }

       //don't need charge OF if use gamma or neutron assumption [ANV]
       fRRQList.insert(pair<string,double>("precoilsumINTg", initVal));

     } //end if pulse integral available

     // ==========  Phonon Tail Fit  ==========
     // note that energy can be obtained from tail fit
     // in several ways.  Only one suffix is used at the moment.
     
     if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
     {

       // energy
       fRRQList.insert(pair<string,double>("pa1TFP", initVal));
       fRRQList.insert(pair<string,double>("pb1TFP", initVal));
       fRRQList.insert(pair<string,double>("pc1TFP", initVal));
       fRRQList.insert(pair<string,double>("pd1TFP", initVal));

       fRRQList.insert(pair<string,double>("pa2TFP", initVal));
       fRRQList.insert(pair<string,double>("pb2TFP", initVal));
       fRRQList.insert(pair<string,double>("pc2TFP", initVal));
       fRRQList.insert(pair<string,double>("pd2TFP", initVal));
       
       fRRQList.insert(pair<string,double>("psum1TFP", initVal));
       fRRQList.insert(pair<string,double>("psum2TFP", initVal));
       fRRQList.insert(pair<string,double>("psumTFP", initVal));

       // partitions - skip for now


       // yields (with charge OF)

       if(fCheckOFChargeXRQ | fCheckOFChargeRQ)
        {
           fRRQList.insert(pair<string,double>("precoilsumTFP", initVal));
           fRRQList.insert(pair<string,double>("ygsumTFP", initVal));
           fRRQList.insert(pair<string,double>("ysumTFP", initVal));
        }


       // yields (with charge F5)
       if(fCheckF5ChargeXRQ)
    
       {
         fRRQList.insert(pair<string,double>("precoilsumF5TFP", initVal));
         fRRQList.insert(pair<string,double>("ygsumF5TFP", initVal));
         fRRQList.insert(pair<string,double>("ysumF5TFP", initVal));
       }

     } //end if tail fit available




     // ==========  Phonon RTFT Walk (single channels)  ==========
     
     if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
     {

       // partition
       fRRQList.insert(pair<string,double>("pxdel1WK", initVal));
       fRRQList.insert(pair<string,double>("pydel1WK", initVal));
       
       fRRQList.insert(pair<string,double>("pxdel2WK", initVal));
       fRRQList.insert(pair<string,double>("pydel2WK", initVal));
       
       fRRQList.insert(pair<string,double>("prdel1WK", initVal));
       fRRQList.insert(pair<string,double>("prdel2WK", initVal));

       // primary sensor
       fRRQList.insert(pair<string,double>("pprimechan1WK", initVal));
       fRRQList.insert(pair<string,double>("pprimechan2WK", initVal));


       // Timing parameter
       fRRQList.insert(pair<string,double>("pminrt1WK_1040", initVal));
       fRRQList.insert(pair<string,double>("pminrt2WK_1040", initVal));
     
       fRRQList.insert(pair<string,double>("pminrt1WK_1070", initVal));
       fRRQList.insert(pair<string,double>("pminrt2WK_1070", initVal));

       fRRQList.insert(pair<string,double>("pminrt1WK_10100", initVal));
       fRRQList.insert(pair<string,double>("pminrt2WK_10100", initVal));


     }
      
     // ==========  Phonon RTFT Walk and Phonon OF (single channels)  ==========
     
     if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) && fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon",fDetNum))
     {

       // primary sensor
       fRRQList.insert(pair<string,double>("pprimechan1OFWK", initVal));
       fRRQList.insert(pair<string,double>("pprimechan2OFWK", initVal));

       // Timing parameter
       fRRQList.insert(pair<string,double>("pminrt1OFWK_1040", initVal));
       fRRQList.insert(pair<string,double>("pminrt2OFWK_1040", initVal));
     
       fRRQList.insert(pair<string,double>("pminrt1OFWK_1070", initVal));
       fRRQList.insert(pair<string,double>("pminrt2OFWK_1070", initVal));

       fRRQList.insert(pair<string,double>("pminrt1OFWK_10100", initVal));
       fRRQList.insert(pair<string,double>("pminrt2OFWK_10100", initVal));



     }
    

     // ==========  PS1/PS2 Phonon RTFT Walk  ==========
     if(fIOMan.CheckBatRootPhononAlg("PSIDES_ConstFreqRTFTWalkPhonon", fDetNum)) 
       {
            // z del   
            fRRQList.insert(pair<string,double>("pzdelWK", initVal)); 

            // Timing parameters
            fRRQList.insert(pair<string,double>("ps1rtWK_1040", initVal));
            fRRQList.insert(pair<string,double>("ps2rtWK_1040", initVal));
     
            fRRQList.insert(pair<string,double>("ps1rtWK_1070", initVal));
            fRRQList.insert(pair<string,double>("ps2rtWK_1070", initVal));

            fRRQList.insert(pair<string,double>("ps1rtWK_4070", initVal));
            fRRQList.insert(pair<string,double>("ps2rtWK_4070", initVal));

            fRRQList.insert(pair<string,double>("ps1rtftWK_8080", initVal));
            fRRQList.insert(pair<string,double>("ps2rtftWK_8080", initVal));

       }


     // ==========  PS1/PS2 Phonon RTFT Walk  ==========
     if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum))
             fRRQList.insert(pair<string,double>("ptftWK_9520", initVal)); 
         

     // ========== Simulation ===========
     if(fIOMan.CheckBatRootUserSettingsFlags("DO_SIM_FROM_PULSE") ||
	fIOMan.CheckBatRootUserSettingsFlags("DO_SIM_FROM_TEMPLATE"))
     { 
	 if(fIOMan.CheckBatRootUserSettingsFlags("DO_PHONONSIM"))
	 {
	     if(fIOMan.CheckBatRootUserSettingsFlags("DO_PTSIM"))
		 fRRQList.insert(pair<string,double>("PTSIMenergy", initVal));
	     if(fIOMan.CheckBatRootUserSettingsFlags("DO_PSIDESSIM"))
	     {
		 fRRQList.insert(pair<string,double>("PS1SIMenergy", initVal));
		 fRRQList.insert(pair<string,double>("PS2SIMenergy", initVal));
	     }
	     if(fIOMan.CheckBatRootUserSettingsFlags("DO_PCHANSIM"))
	     {
		 for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
		     fRRQList.insert(pair<string,double>(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"SIMenergy", initVal));
	     }
	 }

	 if(fIOMan.CheckBatRootUserSettingsFlags("DO_CHARGESIM"))
	 {
	     for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNChargeChan; chanItr++)
		 fRRQList.insert(pair<string,double>(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"SIMenergy", initVal));
	 }
     }


    //Now tell the io manager to make an output tree based on this list    

    string treeName(Form("calibzip%d", fDetNum));
    fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

    return;
}



void GenRRQDataiZIPSoudan::ResetRRQValues()
{
   double initVal = -999999.;

   map<string, double>::iterator rrqListItr = fRRQList.begin();

   for( ; rrqListItr!=fRRQList.end(); rrqListItr++)
   {
      rrqListItr->second = initVal;
   }

   return;
}

// set branch addresses before looping over events
// this reads the first entry to guarantee that the DetType is filled, which we need to setup the output RRQ's
void GenRRQDataiZIPSoudan::ActivateRQs()
{

   //Any RQ that is needed for calculation should be specified here
   //otherwise it is not read from the file
  
   //to first order, everything needs these
   fIOMan.Activate("Empty");
   fIOMan.Activate("DetType");
   fIOMan.Activate("SeriesNumber"); //for keeping track within supermerged files

   // activate channel status
   bool readChanStatus = fIOMan.CheckBatRootUserSettingsFlags("READ_DET_STATUS_FILE");

   if(readChanStatus) {
     for(int chanItr=0; chanItr < BatCalibTypes::kiZIPSoudanNAllChan; chanItr++)
              fIOMan.Activate(BatCalibTypes::kiZIPSoudanChannelNames[chanItr]+"status");
   }
   
   //Read the entries until one gets to the first non-empty value - b/c detType and channel status are not stored for empty events
   int maxEntries = fIOMan.GetMaxEntries();

   for(int eventCtr = 0; eventCtr < maxEntries; eventCtr++)
   {
     fIOMan.ReadNextEntry(eventCtr);
     
     if(fIOMan.Get("Empty") == 0.0) 
     {
       //Store the Det_Type variable
       fDetType = (int)fIOMan.Get("DetType");
     
       // setup vectors that disable broken channels for some calculations
       CreateOnOffChannelSwitches(); 

       break;
     }

   }
	
   if(fDetType == -999999)
   {
     cout <<"WARNING! Ran through entire file and all entries are empty for this detector"
	  <<"\nThis is likely on a problem if you want to read this single dump in matlab"
	  <<"\nIf this is not selective read-out data, check file!" 
	  << endl;  
   }



   // ========== MySQL Databasee Informations ===========
   bool readDatabase = fIOMan.CheckBatRootUserSettingsFlags("READ_DATABASE");
   if(readDatabase) {

      fIOMan.Activate("BaseTemp");
  
      // Fill map with BaseTemp>0
      for(int eventCtr = 0; eventCtr < maxEntries; eventCtr++)
        {
         fIOMan.ReadNextEntry(eventCtr);
         double baseTemp = fIOMan.Get("BaseTemp");
  
         if (baseTemp>0) 
            fGoodBaseTempMap.insert(pair<int,double>(eventCtr,baseTemp));
        }
   }
	
   
   //  ========== CHARGE RQs Activation  ==========
   for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNChargeChan; chanItr++)
   {

     //For TF and MC we currently don't have external files
     //this leaves the option to pull the values (prespecified) from the config files
     //this is less than ideal and should be fixed in the longer term by integrating
     //these values in the raw data stream
     if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
     {
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"bias"); 
     }

     fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"gain");
     fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"norm");


     // --------  Charge OF --------
     if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
     {
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"OFvolts");
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"OFvolts0");
     }    

     // -------- Charge F5 --------
     if(fCheckF5ChargeXRQ)
            fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"F5volts");


     // -------- misc -------
     fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"sat");

   } //end loop over charge channels


  

   //  ========== PHONON  RQs (single channels) Activation  ==========
   for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
   {

     // -------- Gain/norm --------
     fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr] + "gain");
     fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr] + "norm");
     

     // --------  Phonon OF --------
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
       //optimal filter
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"OFamps");
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"OFamps0");
     }

	 // --------  Phonon OF DMC  --------
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhononDMC", fDetNum))
     {
       //optimal filter
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"dmcOFamps");
 
     }
	 

     // --------  Phonon  INT --------
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       //integral routine
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"INTall");       
     }

     // --------  Phonon  Tail Fit --------
     if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
     {
       //integral routine
       fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"TFPint");       
     }
     
   }
    

   //  ========== PHONON PT RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
   {
         fIOMan.Activate("PTOFamps");
         fIOMan.Activate("PTOFamps0");
   }


   //  ========== PHONON PS1/PS2 RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhonon", fDetNum)) {
         fIOMan.Activate("PS1OFamps");
         fIOMan.Activate("PS2OFamps");  
    } 


   //  ========== PHONON PT DMC RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononDMC", fDetNum))
   {
         fIOMan.Activate("PTdmcOFamps");
   }


   //  ========== PHONON PS1/PS2 DMC RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhononDMC", fDetNum)) {
         fIOMan.Activate("PS1dmcOFamps");
         fIOMan.Activate("PS2dmcOFamps");  
   } 


   //  ========== PHONON PT NF  RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
   {
         fIOMan.Activate("PTNFamps");
         fIOMan.Activate("PTNFamps0");
   }
      

     
   //  ========== PHONON RTFTWALK (single channels)   RQs Activation  ========== 

   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
      for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
      {
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr10");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr20");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr30");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr40");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr50");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr70");
	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr80");
         fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKr100");


	 fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"WKf80");

      }

    } //endif do ConstFreqRTFTWalk



    //  ========== PS1/PS2  PHONON RTFTWALK RQs Activation  ========== 

    if(fIOMan.CheckBatRootPhononAlg("PSIDES_ConstFreqRTFTWalkPhonon", fDetNum))
     {

	 fIOMan.Activate("PS1WKr10");
	 fIOMan.Activate("PS1WKr20");
	 fIOMan.Activate("PS1WKr30");
	 fIOMan.Activate("PS1WKr40");
	 fIOMan.Activate("PS1WKr50");
	 fIOMan.Activate("PS1WKr70");
	 fIOMan.Activate("PS1WKr80");
	 fIOMan.Activate("PS1WKf80");	 

	 fIOMan.Activate("PS2WKr10");
	 fIOMan.Activate("PS2WKr20");
	 fIOMan.Activate("PS2WKr30");
	 fIOMan.Activate("PS2WKr40");
	 fIOMan.Activate("PS2WKr50");
	 fIOMan.Activate("PS2WKr70");
	 fIOMan.Activate("PS2WKr80");
	 fIOMan.Activate("PS2WKf80");	 
      } //endif do PSIDES_ConstFreqRTFTWalk


    //  ========== PT  PHONON RTFTWALK RQs Activation  ========== 
    if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum))
     {
         fIOMan.Activate("PTWKf95");
         fIOMan.Activate("PTWKf20");
    }


    // =========== Activate Simulation Truth Quantities ===========
    if(fIOMan.CheckBatRootUserSettingsFlags("DO_PHONONSIM"))
    {
	if(fIOMan.CheckBatRootUserSettingsFlags("DO_PTSIM"))
	    fIOMan.Activate("PTSIMamp");
	if(fIOMan.CheckBatRootUserSettingsFlags("DO_PSIDESSIM"))
	{
	    fIOMan.Activate("PS1SIMamp");
	    fIOMan.Activate("PS2SIMamp");
	}
	if(fIOMan.CheckBatRootUserSettingsFlags("DO_PCHANSIM"))
	{
	    for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
		fIOMan.Activate(BatCalibTypes::kiZIPSoudanPhononChan[chanItr]+"SIMamp");
	}
    }

    if(fIOMan.CheckBatRootUserSettingsFlags("DO_CHARGESIM"))
    {
	for(int chanItr=0; chanItr < BatCalibTypes::kiZIPSoudanNChargeChan; chanItr++)
	    fIOMan.Activate(BatCalibTypes::kiZIPSoudanChargeChan[chanItr]+"SIMamp");
    }

   return;

}

//Only calculate this table for first (non-empty) event in a series
//This assumes the bias voltage does not change during a series.
//We could calculate the table on an event-by-event basis (assuming bias
//varies), but this could be costly in terms of time because the Lindhard
//function is somewhat computationally intensive to calculate. [LLH]
void GenRRQDataiZIPSoudan::CalcLindhardLookupTable()
{

  //only calculate the lookup table if its the first, non-empty event in a new series
  if(fIOMan.Get("SeriesNumber") == fPreviousEventSeriesNumber)
  {
    return;
  }

  // === clear out old table if it exists ===
  if(fNRLookupTable != 0)
  {
    delete fNRLookupTable;
  }

  // === setup initial values needed ===
  double A = fUserData.GetDoubleParameter(fDetNum, "AMASS");
  double Z = fUserData.GetDoubleParameter(fDetNum, "Z");
  double Vbias = fabs(fIOMan.Get("QIS1bias") - fIOMan.Get("QIS2bias")); 

  //hardcoded parameters for lookup table here, but these shouldn't change
  //lookup values will be built from 0 recoil energy up to fMaxInterpolatedRecoil
  double prmin = 1e-2; //keV
  fMaxInterpolatedRecoil = fUserData.GetDoubleParameter("MAXINTERP_RECOIL"); //keV

  //number of entries in lookup table
  const int ni = 500; 

  //arrays to hold initial values (which iterpolation is based on)
  double pri[ni], yi[ni], pti[ni];


  // === evalute lindhard in uniform step from prmin to fMaxInterpolatedRecoil ===

  double k=0.133*pow(Z, (2./3.))/sqrt(A);
  double stepsize = (log10(fMaxInterpolatedRecoil) - log10(prmin))/(double)(ni-1);

  //set values at 0 recoil energy and 0 ionization yield
  pri[0] = 0.;
  yi [0] = 0.;
  pti[0] = 0.;

  for(int ctr = 1; ctr<ni; ctr++)
  {
    //step uniformly on a logarithmic scale
    double step = log10(prmin) + stepsize*(double)ctr;
    pri[ctr] = pow(10, step); 

    double epsilon = 11.5*pri[ctr]/pow(Z,(7./3.)); //not to be confused with energy for eh pair
    double gepsilon = 3.*pow(epsilon, 0.15) + 0.7*pow(epsilon,0.6) + epsilon;

    yi[ctr] = k*gepsilon/(1. + k*gepsilon);
    pti[ctr] = pri[ctr]*(1. + yi[ctr]*(Vbias/fEpsilon));

  }
  
  //store max interpolated pt
  fMaxInterpolatedpt = pti[ni-1];

  //load values into TGraph and use Eval to retrieve interpolated values later
  fNRLookupTable = new TGraph(ni, pti, pri);

  return;
}


// ================== Calculations ======================
//
// DoCalibration controls looping over events!
//
// ======================================================

void GenRRQDataiZIPSoudan::DoCalibration(int maxEvents, int detNum, UserDataManager& myUserData)
{

   // --- 1. Store description of detector for local access ---

   fUserData = myUserData;
   fDetNum = detNum;


   // --- 2.  Activate the rqs ---

   //store some flags for local use
   fCheckOFChargeXRQ = (fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) 
			|| fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge2X2", fDetNum));
   fCheckOFChargeRQ = fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum);
   fCheckF5ChargeXRQ = fIOMan.CheckBatRootChargeAlg("F5ChargeX", fDetNum);
   
   //retrive the zipTree
   string zipTreeName(Form("zip%d", fDetNum));
   int isValidTree = fIOMan.LoadTree("rqDir", zipTreeName);

   //return to main loop if the tree doesn't exist (as is case for Hybrid running conditions)
   if(isValidTree == 0)
   {
      return;
   }
   
   // add  eventTree
   fIOMan.AddFriendTree("rqDir", "eventTree");


   ActivateRQs();  //fDetType and detector status maps are read from rq file and set in ActivateRQs()

   // add OF resolution (if available)
   fIOMan.FillOFResolution(fDetNum, fDetType, fDelaySig, fAmpSig); 

   // --- 3. Setup the output rrq's ---

   ConstructRRQList();


   // ----- Check if detType known  ----
   
   



   // --- 4a. Retrieve calibration constants (outside loop over events to be efficient) ---
   
   // only if detType known (not -999999)

   if(fDetType != -999999)
     {
       fEpsilon = fUserData.GetDoubleParameter(fDetNum, "EPSILON"); 
       
       fChargeCal = fUserData.GetVectDoubleParameter(fDetNum, "Q_CALIBRATION_OF");   
       fChargeF5Cal = fUserData.GetVectDoubleParameter(fDetNum, "Q_CALIBRATION_F5"); 
       
       //relative calibration
       fPhononRelCal = fUserData.GetVectDoubleParameter(fDetNum, "P_RELATIVE_CALIBRATION"); 
       
       //overall calibration    
       fPhononOFCalVect = fUserData.GetOverallCalibration(fDetNum,"PSUM_CALIBRATION_OF",fBrokenPhononChannels);
       fPhononOFCal = fPhononOFCalVect[0];
       
       fTotPhononOFCalVect = fUserData.GetOverallCalibration(fDetNum, "PT_CALIBRATION_OF",fBrokenPhononChannels); 
       fTotPhononOFCal = fTotPhononOFCalVect[0];
       
       
       fTotPhononNFCalVect = fUserData.GetOverallCalibration(fDetNum, "PT_CALIBRATION_NF",fBrokenPhononChannels); 
       fTotPhononNFCal = fTotPhononNFCalVect[0];
       
       
       fPhononIntCalVect = fUserData.GetOverallCalibration(fDetNum, "PSUM_CALIBRATION_INT",fBrokenPhononChannels); 
       fPhononIntCal = fPhononIntCalVect[0];
       
       fPhononTailCalVect = fUserData.GetOverallCalibration(fDetNum, "P_CALIBRATION_TFP",fBrokenPhononChannels); 
       fPhononTailCal = fPhononTailCalVect[0];

       
       //FIXME when this is available in RQ files
       fOFMaxTemplate = fUserData.GetVectDoubleParameter(fDetNum, "OF_MAX_TEMPLATE");  


       // Some checks before performing calculations
       
       if(fPhononRelCal.size() != (uint)BatCalibTypes::kiZIPSoudanNPhononChan)
	 {
	   cout <<"GenRRQDataiZIPSoudan::DoCalibration ERROR! "
		<<"Number of phonon relative calibration constants does not match number of phonon channels, please check"
		<< endl;
	   exit(1);
	 }
       
     }

   // Read database flag
   bool readDatabase = fIOMan.CheckBatRootUserSettingsFlags("READ_DATABASE");

     


   // ----- 5. Loop over all events and do the calculations! -----
  
   cout <<"In DoCalc, maxEvents = " << maxEvents << endl;
   cout <<"Size of this tree is = " << fIOMan.GetMaxEntries() << endl;

   int maxEntries = fIOMan.GetMaxEntries();

   for(int eventCtr = 0; (eventCtr < maxEvents && eventCtr < maxEntries) ; eventCtr++)
   {
      //cout <<"eventCtr = " << eventCtr << endl;

      //read the next entry from the file
      fIOMan.ReadNextEntry(eventCtr);
      
      //skip this entry if it was not read out (for selective readout)
      if(fIOMan.Get("Empty") != 0.0) 
      {
 	 fIOMan.FillOutputRRQTree(); //fills with empty "default" of -999999, no need to reset     
 	 continue;
      }
      else
      {
	 fRRQList["Empty"] = 0; //not empty
      }




      // --- calculate temperature dependent phonon calibration ----

      double baseTemp =  -999999;
	  if (fUserData.GetIntParameter("USE_DEFAULT_BASETEMP"))
		  baseTemp = fUserData.GetDoubleParameter(fDetNum,"CALIB_DEFAULT_BASETEMP");
	  
	  
      if (readDatabase) {
 
        // get temperature
        baseTemp =  fIOMan.Get("BaseTemp");

        // check temperature validity
        // if not >0, check nearest event with meaningful 
        // temperature information

        if (baseTemp<=0) {
        
           // use nearest temperature
           int eventNearest = -999999;
           int eventBefore  = -999999;
           int eventAfter   = -999999;

           for (map<int,double>::iterator it=fGoodBaseTempMap.begin(); it!=fGoodBaseTempMap.end(); ++it)
            { 
               eventAfter = it->first;
               if (eventAfter>eventCtr)
                   break;
               else 
                   eventBefore= it->first;
            }
                   
           
           if (eventBefore!=-999999 && (abs(eventCtr-eventBefore) <= abs(eventCtr - eventAfter)))
                eventNearest=eventBefore;
           else
                eventNearest= eventAfter;
     

           if (eventNearest!=-999999)
                baseTemp = fGoodBaseTempMap[eventNearest];
	   else
                baseTemp = fUserData.GetDoubleParameter(fDetNum,"CALIB_DEFAULT_BASETEMP"); 

        }
	  }

	  
        
	  if (baseTemp>0) { 
  
		  double minBaseTemp = fUserData.GetDoubleParameter(fDetNum,"CALIB_MIN_BASETEMP");
		  double maxBaseTemp = fUserData.GetDoubleParameter(fDetNum,"CALIB_MAX_BASETEMP");
		  
		  if (baseTemp<minBaseTemp) baseTemp = minBaseTemp;
          if (baseTemp>maxBaseTemp) baseTemp = maxBaseTemp;
		  

          // Calculate calibration: a*T^2+bT++b
    
          if (fPhononOFCalVect.size()==3) 
              fPhononOFCal = 1/(fPhononOFCalVect[0]*pow(baseTemp,2.0) + fPhononOFCalVect[1]*baseTemp + fPhononOFCalVect[2]);

          if (fTotPhononOFCalVect.size()==3)
              fTotPhononOFCal = 1/(fTotPhononOFCalVect[0]*pow(baseTemp,2.0) + fTotPhononOFCalVect[1]*baseTemp + fTotPhononOFCalVect[2]);
 
          if (fTotPhononNFCalVect.size()==3)
              fTotPhononNFCal = 1/(fTotPhononNFCalVect[0]*pow(baseTemp,2.0) +fTotPhononNFCalVect[1]*baseTemp + fTotPhononNFCalVect[2]);
 
          if (fPhononIntCalVect.size()==3) 
              fPhononIntCal = 1/(fPhononIntCalVect[0]*pow(baseTemp,2.0) + fPhononIntCalVect[1]*baseTemp+fPhononIntCalVect[2]);
    
          if (fPhononTailCalVect.size()==3) 
              fPhononTailCal = 1/(fPhononTailCalVect[0]*pow(baseTemp,2.0) + fPhononTailCalVect[1]*baseTemp+fPhononTailCalVect[2]);
      
	  }
 
	  // double check user calibrations are correct
	  if (baseTemp==-999999 && 
           (fPhononOFCalVect.size()!=1   
			|| fTotPhononOFCalVect.size() !=1    
			|| fTotPhononNFCalVect.size() !=1
			|| fPhononIntCalVect.size() !=1 
			|| fPhononTailCalVect.size() !=1)) {
		  
		  cout <<"ERROR! GenRRQDataiZIPSoudan::ApplyPhononCalibration: "
			   <<"No temperature available: All the calibration constants should be single numbers."
			   <<"check calibration file or temperature database reading!"
	      << endl;
		  exit(1);
	  }


      // ---- mandatory calculations ---- 
      
      //the order in which these are called matters 

      ApplyPhononCalibration();     //1. apply phonon relative calibration
      
      CalcPhononDelays();           //2. calculate xy delays (needs relative phonon cal)
      
      ApplyChargeCalibration();     //3. apply charge calibration and position correction (needs phonon delays)
      
      CalcLindhardLookupTable();    //4a. generate lookup to convert tot phonon to recoil w/ NR hypothesis, calc 1X per series
      CalcTotalEnergies();          //4b. calculate qsum and recoil energies
      CalcYields();                 //4c. calculate yields
      
      CalcPartitions();             //5. calculate phonon and charge partitions
      
      if(fIOMan.IsOFresFilled())
		  CalcOFResolutions();       //6. calculate optimal filter resolutions (only if available)
      
      FindPrimaryPhononChannel();  // find primary channel
            
      CalcConstFreqRTFTWalkRRQ();  // ConstFreqRTFTWalk timing
      

      // --- Store some misc items ---
      
      fRRQList["DetType"] = fDetType; //inefficient, but needed for pull teeth right now
      
      fPreviousEventSeriesNumber = fIOMan.Get("SeriesNumber");
      
      // ---- Store the data and reset the RRQ list values ----


      fIOMan.FillOutputRRQTree();      
      ResetRRQValues();
 
     
   } //Done looping over events
 

   // --- 8. Write the tree ---
   
   fIOMan.WriteOutputRRQTree();


   // --- Done! ---

   fIOMan.DeleteActiveTree();

   return;
}

// ============== the following routines are called from within a loop over events ================

//This function stores a 1 or 0 for each channel depending on the channel status flag
//This allows us to disable contributions of broken channels to certain summed quantities.  Channel 
//status flags did not exist in BatRoot until 3/2013.  For those data, assume channel status is 0.
void GenRRQDataiZIPSoudan::CreateOnOffChannelSwitches()
{
  fNWorkingPhonon = 0;
  bool readChanStatus = fIOMan.CheckBatRootUserSettingsFlags("READ_DET_STATUS_FILE");

  //First fill phonon switches
  for(int chanCtr=0; chanCtr < BatCalibTypes::kiZIPSoudanNPhononChan; chanCtr++)
  {
    string chanName = BatCalibTypes::kiZIPSoudanPhononChan[chanCtr];
    int pFlag;

    if(readChanStatus)
    {
      pFlag = (int)fIOMan.Get(chanName+"status");
    }
    else
    {
      pFlag = 0;
    }

    fPhononOnOffSwitches.insert(pair<string,int>(chanName, (pFlag < 2 ? 1 : 0)) );  //don't use if high resistance short
    fNWorkingPhonon += fPhononOnOffSwitches[chanName]; 
    if (pFlag>1) fBrokenPhononChannels.push_back(chanName);


    //    cout <<"Channel is " << chanName <<", status = " << fPhononOnOffSwitches[chanName] << endl;
  } 

  //First fill charge switches
  for(int chanCtr=0; chanCtr < BatCalibTypes::kiZIPSoudanNChargeChan; chanCtr++)
  {
    string chanName = BatCalibTypes::kiZIPSoudanChargeChan[chanCtr];
    int cFlag;
    
    if(readChanStatus)
    {
      cFlag = (int)fIOMan.Get(chanName+"status");
    }
    else
    {
      cFlag = 0;
    }

    //don't use if FET is disabled, feedback shorted, 
    //bias line grounded or unusual noise
    int cSwitch = (cFlag < 1 ? 1 : 0); 
    fChargeOnOffSwitches.insert( pair<string,int>(chanName, cSwitch) );

    //    fChargeOnOffSwitches["QIS1"] = 0; //TEMP!!!
			
    //cout <<"Channel is " << chanName <<", status = " << fChargeOnOffSwitches[chanName] << endl;
  } 



  return;
}

void GenRRQDataiZIPSoudan::ApplyPhononCalibration()
{
  

  // ========== Optimal Filter energy =========

  // --- single channels ---

  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
  {
    fRRQList["pa1OF"] = fPhononRelCal[0]*fPhononOFCal*fIOMan.Get("PAS1OFamps");
    fRRQList["pb1OF"] = fPhononRelCal[1]*fPhononOFCal*fIOMan.Get("PBS1OFamps");
    fRRQList["pc1OF"] = fPhononRelCal[2]*fPhononOFCal*fIOMan.Get("PCS1OFamps");
    fRRQList["pd1OF"] = fPhononRelCal[3]*fPhononOFCal*fIOMan.Get("PDS1OFamps");
    
    fRRQList["pa2OF"] = fPhononRelCal[4]*fPhononOFCal*fIOMan.Get("PAS2OFamps");
    fRRQList["pb2OF"] = fPhononRelCal[5]*fPhononOFCal*fIOMan.Get("PBS2OFamps");
    fRRQList["pc2OF"] = fPhononRelCal[6]*fPhononOFCal*fIOMan.Get("PCS2OFamps");
    fRRQList["pd2OF"] = fPhononRelCal[7]*fPhononOFCal*fIOMan.Get("PDS2OFamps");
    
    fRRQList["pa1OF0"] = fPhononRelCal[0]*fPhononOFCal*fIOMan.Get("PAS1OFamps0");
    fRRQList["pb1OF0"] = fPhononRelCal[1]*fPhononOFCal*fIOMan.Get("PBS1OFamps0");
    fRRQList["pc1OF0"] = fPhononRelCal[2]*fPhononOFCal*fIOMan.Get("PCS1OFamps0");
    fRRQList["pd1OF0"] = fPhononRelCal[3]*fPhononOFCal*fIOMan.Get("PDS1OFamps0");
    
    fRRQList["pa2OF0"] = fPhononRelCal[4]*fPhononOFCal*fIOMan.Get("PAS2OFamps0");
    fRRQList["pb2OF0"] = fPhononRelCal[5]*fPhononOFCal*fIOMan.Get("PBS2OFamps0");
    fRRQList["pc2OF0"] = fPhononRelCal[6]*fPhononOFCal*fIOMan.Get("PCS2OFamps0");
    fRRQList["pd2OF0"] = fPhononRelCal[7]*fPhononOFCal*fIOMan.Get("PDS2OFamps0");
    
  }

  // --- PSIDES  ---
  if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhonon", fDetNum)) {
    fRRQList["ps1OF"] = fTotPhononOFCal*fIOMan.Get("PS1OFamps");
    fRRQList["ps2OF"] = fTotPhononOFCal*fIOMan.Get("PS2OFamps");
  }

  // --- PT ---
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
  {
    fRRQList["ptOF"] = fTotPhononOFCal*fIOMan.Get("PTOFamps"); 
    fRRQList["ptOF0"] = fTotPhononOFCal*fIOMan.Get("PTOFamps0");
  }

  // ========== Optimal Filter energy =========
  
  // --- single channels ---

  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhononDMC", fDetNum))
  {
    fRRQList["pa1dmcOF"] = fPhononRelCal[0]*fPhononOFCal*fIOMan.Get("PAS1dmcOFamps");
    fRRQList["pb1dmcOF"] = fPhononRelCal[1]*fPhononOFCal*fIOMan.Get("PBS1dmcOFamps");
    fRRQList["pc1dmcOF"] = fPhononRelCal[2]*fPhononOFCal*fIOMan.Get("PCS1dmcOFamps");
    fRRQList["pd1dmcOF"] = fPhononRelCal[3]*fPhononOFCal*fIOMan.Get("PDS1dmcOFamps");
    
    fRRQList["pa2dmcOF"] = fPhononRelCal[4]*fPhononOFCal*fIOMan.Get("PAS2dmcOFamps");
    fRRQList["pb2dmcOF"] = fPhononRelCal[5]*fPhononOFCal*fIOMan.Get("PBS2dmcOFamps");
    fRRQList["pc2dmcOF"] = fPhononRelCal[6]*fPhononOFCal*fIOMan.Get("PCS2dmcOFamps");
    fRRQList["pd2dmcOF"] = fPhononRelCal[7]*fPhononOFCal*fIOMan.Get("PDS2dmcOFamps");    
  }

  // --- PSIDES  ---
  if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhononDMC", fDetNum)) {
    fRRQList["ps1dmcOF"] = fTotPhononOFCal*fIOMan.Get("PS1dmcOFamps");
    fRRQList["ps2dmcOF"] = fTotPhononOFCal*fIOMan.Get("PS2dmcOFamps");
  }

  // --- PT ---
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononDMC", fDetNum))
  {
    fRRQList["ptdmcOF"] = fTotPhononOFCal*fIOMan.Get("PTdmcOFamps"); 
  }

  // ========== NS Optimal Filter energy ==========
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
  {
    fRRQList["ptNF"] = fTotPhononNFCal*fIOMan.Get("PTNFamps"); 
    fRRQList["ptNF0"] = fTotPhononNFCal*fIOMan.Get("PTNFamps0");
  }



  // ========== Integral energy ==========
  if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
  {
    fRRQList["pa1INT"] = fPhononRelCal[0]*fPhononIntCal*fIOMan.Get("PAS1INTall");
    fRRQList["pb1INT"] = fPhononRelCal[1]*fPhononIntCal*fIOMan.Get("PBS1INTall");
    fRRQList["pc1INT"] = fPhononRelCal[2]*fPhononIntCal*fIOMan.Get("PCS1INTall");
    fRRQList["pd1INT"] = fPhononRelCal[3]*fPhononIntCal*fIOMan.Get("PDS1INTall");
    
    fRRQList["pa2INT"] = fPhononRelCal[4]*fPhononIntCal*fIOMan.Get("PAS2INTall");
    fRRQList["pb2INT"] = fPhononRelCal[5]*fPhononIntCal*fIOMan.Get("PBS2INTall");
    fRRQList["pc2INT"] = fPhononRelCal[6]*fPhononIntCal*fIOMan.Get("PCS2INTall");
    fRRQList["pd2INT"] = fPhononRelCal[7]*fPhononIntCal*fIOMan.Get("PDS2INTall");
  }


  // ========== Phonon Tail Fit ==========
  if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
  {
    fRRQList["pa1TFP"] = fPhononRelCal[0]*fPhononTailCal*fIOMan.Get("PAS1TFPint");
    fRRQList["pb1TFP"] = fPhononRelCal[1]*fPhononTailCal*fIOMan.Get("PBS1TFPint");
    fRRQList["pc1TFP"] = fPhononRelCal[2]*fPhononTailCal*fIOMan.Get("PCS1TFPint");
    fRRQList["pd1TFP"] = fPhononRelCal[3]*fPhononTailCal*fIOMan.Get("PDS1TFPint");
    
    fRRQList["pa2TFP"] = fPhononRelCal[4]*fPhononTailCal*fIOMan.Get("PAS2TFPint");
    fRRQList["pb2TFP"] = fPhononRelCal[5]*fPhononTailCal*fIOMan.Get("PBS2TFPint");
    fRRQList["pc2TFP"] = fPhononRelCal[6]*fPhononTailCal*fIOMan.Get("PCS2TFPint");
    fRRQList["pd2TFP"] = fPhononRelCal[7]*fPhononTailCal*fIOMan.Get("PDS2TFPint");
  }

  
  if(fIOMan.CheckBatRootUserSettingsFlags("DO_PHONONSIM"))
  {
      if(fIOMan.CheckBatRootUserSettingsFlags("DO_PTSIM"))
	  fRRQList["PTSIMenergy"] = fTotPhononNFCal*fIOMan.Get("PTSIMamp");
      if(fIOMan.CheckBatRootUserSettingsFlags("DO_PSIDESSIM"))
      {
	  fRRQList["PS1SIMenergy"] = fTotPhononOFCal*fIOMan.Get("PS1SIMamp");
	  fRRQList["PS2SIMenergy"] = fTotPhononOFCal*fIOMan.Get("PS2SIMamp");
      }
      if(fIOMan.CheckBatRootUserSettingsFlags("DO_PCHANSIM"))
      {
	  fRRQList["PAS1SIMenergy"] = fPhononRelCal[0]*fTotPhononOFCal*fIOMan.Get("PAS1SIMamp");
	  fRRQList["PBS1SIMenergy"] = fPhononRelCal[1]*fTotPhononOFCal*fIOMan.Get("PBS1SIMamp");
	  fRRQList["PCS1SIMenergy"] = fPhononRelCal[2]*fTotPhononOFCal*fIOMan.Get("PCS1SIMamp");
	  fRRQList["PDS1SIMenergy"] = fPhononRelCal[3]*fTotPhononOFCal*fIOMan.Get("PDS1SIMamp");

	  fRRQList["PAS2SIMenergy"] = fPhononRelCal[4]*fTotPhononOFCal*fIOMan.Get("PAS2SIMamp");
	  fRRQList["PBS2SIMenergy"] = fPhononRelCal[5]*fTotPhononOFCal*fIOMan.Get("PBS2SIMamp");
	  fRRQList["PCS2SIMenergy"] = fPhononRelCal[6]*fTotPhononOFCal*fIOMan.Get("PCS2SIMamp");
	  fRRQList["PDS2SIMenergy"] = fPhononRelCal[7]*fTotPhononOFCal*fIOMan.Get("PDS2SIMamp");
      }
  }

  return;
}

//Note that the user can make a choice in whether constant frequency or variable frequency 
//rtftwalk values are used for the delay calculations in this function.   
//This is done by setting the flag in the user settings file "DefaultToConstFreqRTFTWalk" 
void GenRRQDataiZIPSoudan::CalcPhononDelays()
{
  
  // constant frequency rtftwalk

  if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
  {
    fRRQList["pxdel1WK"] = -(fIOMan.Get("PBS1WKr20")*cos(kTheta1Vect[0]) + 
			     fIOMan.Get("PCS1WKr20")*cos(kTheta1Vect[1]) 
			     + fIOMan.Get("PDS1WKr20")*cos(kTheta1Vect[2]))*1e6;
    
    fRRQList["pydel1WK"] = -(fIOMan.Get("PBS1WKr20")*sin(kTheta1Vect[0]) + 
			     fIOMan.Get("PCS1WKr20")*sin(kTheta1Vect[1])
			     + fIOMan.Get("PDS1WKr20")*sin(kTheta1Vect[2]))*1e6;
    
    fRRQList["pxdel2WK"] = -(fIOMan.Get("PBS2WKr20")*cos(kTheta2Vect[0]) + 
			     fIOMan.Get("PCS2WKr20")*cos(kTheta2Vect[1]) 
			     + fIOMan.Get("PDS2WKr20")*cos(kTheta2Vect[2]))*1e6;
         
    fRRQList["pydel2WK"] = -(fIOMan.Get("PBS2WKr20")*sin(kTheta2Vect[0]) + 
			     fIOMan.Get("PCS2WKr20")*sin(kTheta2Vect[1])
			     + fIOMan.Get("PDS2WKr20")*sin(kTheta2Vect[2]))*1e6;

  }

  // Z delay (use PS1/PS2 quantities)
  if(fIOMan.CheckBatRootPhononAlg("PSIDES_ConstFreqRTFTWalkPhonon", fDetNum))
          fRRQList["pzdelWK"] = (fIOMan.Get("PS2WKr20") - fIOMan.Get("PS1WKr20"))*1e6;

  
  return;
} 

void GenRRQDataiZIPSoudan::ApplyChargeCalibration()
{

  // Loop over two sides and do the same calculation for each

  for(int sideItr = 0; sideItr < 2; sideItr++)
  {
   
    // ========== optimal filter w/ cross talk calc ==========
    if(fCheckOFChargeXRQ)
    {
      //to hold calibration constants
      double qiScale = fChargeCal[0+sideItr*4];
      double qoScale = fChargeCal[1+sideItr*4];
      double qix = fChargeCal[2+sideItr*4]; //implement x-talk [ANV] 
      double qox = fChargeCal[3+sideItr*4]; //implement x-talk [ANV] 
 

      //names for convenient retrieval of the values
      string qi = Form("qi%dOF", sideItr+1);
      string qo = Form("qo%dOF", sideItr+1);
      string qi0 = Form("qi%dOF0", sideItr+1);
      string qo0 = Form("qo%dOF0", sideItr+1);
      string QIOFvolts = Form("QIS%dOFvolts", sideItr+1); 
      string QOOFvolts = Form("QOS%dOFvolts", sideItr+1); 
      string QIOFvolts0 = Form("QIS%dOFvolts0", sideItr+1);
      string QOOFvolts0 = Form("QOS%dOFvolts0", sideItr+1);
      
      //implement simple cross-talk [ANV]
      fRRQList[qi] = qiScale*fIOMan.Get(QIOFvolts) + qix*fIOMan.Get(QOOFvolts);
      fRRQList[qo] = qoScale*fIOMan.Get(QOOFvolts) + qox*fIOMan.Get(QIOFvolts);
      fRRQList[qi0] = qiScale*fIOMan.Get(QIOFvolts0) + qix*fIOMan.Get(QOOFvolts0);
      fRRQList[qo0] = qoScale*fIOMan.Get(QOOFvolts0) + qox*fIOMan.Get(QIOFvolts0);
    }


    // ========== F5 fit ==========
    if(fCheckF5ChargeXRQ)
    {
      //to hold calibration constants
      double qiScale = fChargeF5Cal[0+sideItr*4];
      double qoScale = fChargeF5Cal[1+sideItr*4];
      double qix = fChargeF5Cal[2+sideItr*4]; //implement x-talk [ANV] 
      double qox = fChargeF5Cal[3+sideItr*4]; //implement x-talk [ANV] 
 

      //names for convenient retrieval of the values
      string qi = Form("qi%dF5", sideItr+1);
      string qo = Form("qo%dF5", sideItr+1);
      string QIF5volts = Form("QIS%dF5volts", sideItr+1); 
      string QOF5volts = Form("QOS%dF5volts", sideItr+1); 
          
      //implement simple cross-talk [ANV]
      fRRQList[qi] = qiScale*fIOMan.Get(QIF5volts) + qix*fIOMan.Get(QOF5volts);
      fRRQList[qo] = qoScale*fIOMan.Get(QOF5volts) + qox*fIOMan.Get(QIF5volts);

    }


    // =========== Charge simulation ==========
    if(fIOMan.CheckBatRootUserSettingsFlags("DO_CHARGESIM"))
    {
	double qiScale = fChargeCal[0+sideItr*4];
	double qoScale = fChargeCal[1+sideItr*4];

	string qiSIMenergy = Form("QIS%dSIMenergy", sideItr+1);
	string qoSIMenergy = Form("QOS%dSIMenergy", sideItr+1);
	string qiSIMamp = Form("QIS%dSIMamp", sideItr+1);
	string qoSIMamp = Form("QOS%dSIMamp", sideItr+1);
	fRRQList[qiSIMenergy] = qiScale*fIOMan.Get(qiSIMamp);
	fRRQList[qoSIMenergy] = qoScale*fIOMan.Get(qoSIMamp);
    }

  } // end loop over sides
   
  return;
}

//ApplyPhononCalibration() and ApplyChargeCalibration() must be called before this
void GenRRQDataiZIPSoudan::CalcTotalEnergies()
{
  //get qi/qo bias - they are used for all the detector below
  double qi1Bias = -999999;
  double qi2Bias = -999999;

  if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
  {
    qi1Bias = fIOMan.Get("QIS1bias"); 
    qi2Bias = fIOMan.Get("QIS2bias");
  }
  else
  {
    qi1Bias = fUserData.GetDoubleParameter(fDetNum, "QIS1BIAS");
    qi2Bias = fUserData.GetDoubleParameter(fDetNum, "QIS2BIAS");
  }

  //qifac should always be positive 
  double qifac1 = fabs(qi1Bias/fEpsilon);
  double qifac2 = fabs(qi2Bias/fEpsilon);
  double qifacDelta = fabs(qi1Bias - qi2Bias)/fEpsilon;

  // ========== Charge OF summmed energies ==========

  if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
  {
    
    double qsummin; //only needed for intermediate calc  
    double qimin; //only needed for intermediate calc  

    //Note that if a charge channel is broken, its not included in qsum1(2)
    double qsum1OF = fChargeOnOffSwitches["QIS1"]*fRRQList["qi1OF"] 
                   + fChargeOnOffSwitches["QOS1"]*fRRQList["qo1OF"]; 
    double qsum2OF = fChargeOnOffSwitches["QIS2"]*fRRQList["qi2OF"] 
                   + fChargeOnOffSwitches["QOS2"]*fRRQList["qo2OF"];
    
    fRRQList["qsum1OF"] = qsum1OF;
    fRRQList["qsum2OF"] = qsum2OF;
    
    //Now determing qsummax. If one side has a broken channel, make qsummax
    //the qsum of the working side by default
    if((fChargeOnOffSwitches["QIS1"] == 0 || fChargeOnOffSwitches["QOS1"]==0) &&
       (fChargeOnOffSwitches["QIS2"] == 1 && fChargeOnOffSwitches["QOS2"]==1) )
    {
      //cout <<"MODIFYING qsummaxOF!!" << endl;

      //Case I. qinner is broken on S1
      //assign qsum2 to qsummax, and 0 to qsummin
      //this must use alternate luke definition
      if(fChargeOnOffSwitches["QIS1"] == 0)
      {
	fRRQList["qsummaxOF"] = qsum2OF;
	fRRQList["qimaxOF"] = fRRQList["qi2OF"];
	qsummin = 0; 
	qimin = 0;
      }
      //Case II. qinner working but not qouter (on S1).
      //Using qinner on both sides to determine qsummin and qsummax.
      //This will use the regular luke definition, but will
      //only be correct for qinner events.  This is preferable
      //to using qsum2 alone (as in Case I) or checking the min/max
      //between qi1min and qsum2.  In the former case, this 
      //produces fewer outliers.  In the latter, a tighter zero
      //charge band [LLH]
      else 
      {
	fRRQList["qimaxOF"] = max(fRRQList["qi1OF"],fRRQList["qi2OF"]);
	fRRQList["qsummaxOF"] = fRRQList["qimaxOF"]; 
	qsummin =  min(fRRQList["qi1OF"],fRRQList["qi2OF"]);
	qimin =  qsummin;
      }
    }
    //Case III.  all charge channels are functional
    else
    {

      fRRQList["qimaxOF"] = max(fRRQList["qi1OF"],fRRQList["qi2OF"]);
      fRRQList["qsummaxOF"] = max(fRRQList["qsum1OF"],fRRQList["qsum2OF"]);
      qsummin = min(fRRQList["qsum1OF"],fRRQList["qsum2OF"]);
      qimin =  min(fRRQList["qi1OF"],fRRQList["qi2OF"]);
    }
  

    // ========== Luke from ionization ==========
     
    if(fChargeOnOffSwitches["QIS1"] == 1 && fChargeOnOffSwitches["QIS2"] ==1)
    {
      //cout <<"performing ***DEFAULT*** plukeq definition!" << endl;

      //Notes (LLH Jan. 2013):  This definition assumes that qi1Bias + qi2Bias = 0 
      //OR that one of the two biases is grounded.   
      //It should work ok for detectors that have functional
      //charge readout, but will not give the correct value for detectors with
      //disabled FETs.  Note it won't be correct for qouter events if one (or more
      //qouter) is broken.
      fRRQList["plukeqOF"] = fabs(qsum1OF*qifac1 - qsum2OF*qifac2) 
 	                     + qsummin * qifacDelta;

      fRRQList["plukeqOFi"] = fabs(fRRQList["qi1OF"]*qifac1 - fRRQList["qi2OF"]*qifac2) 
 	                     + qimin * qifacDelta;

      fRRQList["pgqOF"] = fRRQList["qsummaxOF"] +  fRRQList["plukeqOF"];
    }
    else if(fChargeOnOffSwitches["QIS1"] == 0 && fChargeOnOffSwitches["QIS2"] ==1)
    {    

      //cout <<"performing ***ALTERNATE*** plukeqOF for FET disabled dets!" <<"qifacDelta = " << qifacDelta << endl;

      //this definition will only be correct for bulk, qinner events.
      //It is meant to kludge these values for detectors that have 
      //disabled FETs on the S1 qinner OR grounded QIS1
      fRRQList["plukeqOF"] = qsum2OF * qifacDelta;

      fRRQList["plukeqOFi"] = fRRQList["qi2OF"] * qifacDelta;

      fRRQList["pgqOF"] = qsum2OF +  fRRQList["plukeqOF"];      

    }

  } // end calc of charge OF energies
 



  // ========== Charge F5 summmed energies ==========

  if(fCheckF5ChargeXRQ)
  {
    double qsummin; //only needed for intermediate calc  
    double qsum1F5 = fChargeOnOffSwitches["QIS1"]*fRRQList["qi1F5"] 
                   + fChargeOnOffSwitches["QOS1"]*fRRQList["qo1F5"]; 
    double qsum2F5 = fChargeOnOffSwitches["QIS2"]*fRRQList["qi2F5"] 
                   + fChargeOnOffSwitches["QOS2"]*fRRQList["qo2F5"];
    
    fRRQList["qsum1F5"] = qsum1F5;
    fRRQList["qsum2F5"] = qsum2F5;

    //Now determing qsummax. If one side has a broken channel, make qsummax
    //the qsum of the working side by default
    if((fChargeOnOffSwitches["QIS1"] == 0 || fChargeOnOffSwitches["QOS1"]==0) &&
       (fChargeOnOffSwitches["QIS2"] == 1 && fChargeOnOffSwitches["QOS2"]==1) )
    {
       //cout <<"MODIFYING qsummaxF5!!" << endl;

      //Case I. qinner is broken on S1
      //assign qsum2 to qsummax, and 0 to qsummin
      //this must use alternate luke definition
      if(fChargeOnOffSwitches["QIS1"] == 0)
      {
	fRRQList["qsummaxF5"] = qsum2F5;
	fRRQList["qimaxF5"] = fRRQList["qi2F5"];
	qsummin = 0; 
      }
      //Case II. qinner working but not qouter (on S1).
      //Using qinner on both sides to determine qsummin and qsummax.
      //This will use the regular luke definition, but will
      //only be correct for qinner events.  This is preferable
      //to using qsum2 alone (as in Case I) or checking the min/max
      //between qi1min and qsum2.  In the former case, this 
      //produces fewer outliers.  In the latter, a tighter zero
      //charge band [LLH]
      else 
      {
	fRRQList["qimaxF5"] = max(fRRQList["qi1F5"],fRRQList["qi2F5"]);
	fRRQList["qsummaxF5"] = fRRQList["qimaxF5"]; 
	qsummin =  min(fRRQList["qi1F5"],fRRQList["qi2F5"]);
      }

    }
    //Case III.  all charge channels are functional
    else
    {

      fRRQList["qimaxF5"] = max(fRRQList["qi1F5"],fRRQList["qi2F5"]);
      fRRQList["qsummaxF5"] = max(fRRQList["qsum1F5"],fRRQList["qsum2F5"]);
      qsummin = min(fRRQList["qsum1F5"],fRRQList["qsum2F5"]);
    }  
 
    // ========== Luke from ionization ==========

    if(fChargeOnOffSwitches["QIS1"] == 1 && fChargeOnOffSwitches["QIS2"] ==1)
    {
      //      cout <<"performing ***DEFAULT*** plukeq definition!" << endl;

      //Notes (LLH Jan. 2013):  This definition assumes that qi1Bias + qi2Bias = 0 
      //OR that one of the two biases is grounded.   
      //It should work ok for detectors that have functional
      //charge readout, but will not give the correct value for detectors with
      //disabled FETs.  Note it won't be correct for qouter events if one (or more
      //qouter) is broken.
      fRRQList["plukeqF5"] = fabs(qsum1F5*qifac1 - qsum2F5*qifac2) 
	                     + qsummin * qifacDelta;

      fRRQList["pgqF5"] = fRRQList["qsummaxF5"] +  fRRQList["plukeqF5"];
    }
    else if(fChargeOnOffSwitches["QIS1"] == 0 && fChargeOnOffSwitches["QIS2"] ==1)
    {    

      //      cout <<"performing ***ALTERNATE*** plukeqF5 for FET disabled dets!" << endl;

      //this definition will only be correct for bulk, qinner events.
      //It is meant to kludge these values for detectors that have 
      //disabled FETs on the S1 qinner
      fRRQList["plukeqF5"] = qsum2F5 * qifacDelta;

      fRRQList["pgqF5"] = qsum2F5 +  fRRQList["plukeqF5"];      

    }

  } // end calc of charge F5 energies
 

  

  // ========== Phonon OF summed energies ==========
  
  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pa1 = fPhononOnOffSwitches["PAS1"]*fRRQList["pa1OF"];
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1OF"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1OF"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1OF"];
    double pa2 = fPhononOnOffSwitches["PAS2"]*fRRQList["pa2OF"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2OF"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2OF"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2OF"];

    //these could be done in a loop with a side iterator but more cumbersome than charge
    //case because psumi#OF changes from side 1 to 2, for example [ANV]
  
    //independent of the psumi#OF and psumo#OF but could be psumi#OF + psumo#OF [ANV]
    fRRQList["psum1OF"] = pa1 + pb1 + pc1 + pd1;
    fRRQList["psum2OF"] = pa2 + pb2 + pc2 + pd2;

    //now get sum of sides [ANV]
    fRRQList["psumOF"] = (fRRQList["psum1OF"] + fRRQList["psum2OF"]); 

    fRRQList["psumOF0"] = fPhononOnOffSwitches["PAS1"]*fRRQList["pa1OF0"] 
                        + fPhononOnOffSwitches["PBS1"]*fRRQList["pb1OF0"] 
                        + fPhononOnOffSwitches["PCS1"]*fRRQList["pc1OF0"] 
                        + fPhononOnOffSwitches["PDS1"]*fRRQList["pd1OF0"] 
                        + fPhononOnOffSwitches["PAS2"]*fRRQList["pa2OF0"] 
                        + fPhononOnOffSwitches["PBS2"]*fRRQList["pb2OF0"] 
                        + fPhononOnOffSwitches["PCS2"]*fRRQList["pc2OF0"] 
                        + fPhononOnOffSwitches["PDS2"]*fRRQList["pd2OF0"];

    //NOTE: if you add an iZIP type you MUST add a condition here [ANV]
    if(fDetType == BatCalibTypes::kiZIPSoudanBiFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1OF"] = pc1 + pd1;
      fRRQList["psumi2OF"] = pb2 + pc2;
      fRRQList["psumo1OF"] = pa1 + pb1;
      fRRQList["psumo2OF"] = pa2 + pd2;
    }
    else if(fDetType == BatCalibTypes::kiZIPSoudanTriFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1OF"] = pb1 + pc1 + pd1;
      fRRQList["psumi2OF"] = pb2 + pc2 + pd2;
      fRRQList["psumo1OF"] = pa1;
      fRRQList["psumo2OF"] = pa2;
    }

    //can only calculate the luke correction if charge is reconstructed [ANV]
    if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
         fRRQList["precoilsumOF"] = fRRQList["psumOF"] - fRRQList["plukeqOF"];

    //if one uses two assumptions can calculate the recoil energy without charge [ANV]
    // 1) event is due to a gamma (electron recoil)
    // 2) event is a bulk event (non-surface)
    fRRQList["precoilsumOFg"] = fRRQList["psumOF"]/(1 + qifacDelta);

    //using Lindhard for ionization yield, if within boundaries of lookup table
    if(fRRQList["psumOF"] > 0)
    {  
      if(fRRQList["psumOF"] < fMaxInterpolatedpt) 
      {
	fRRQList["precoilsumOFnL"] = fNRLookupTable->Eval(fRRQList["psumOF"], 0, "S"); 
      }
    }
    else
    {
      //assume zero yield for event with negative pt, so pt=pr
      fRRQList["precoilsumOFnL"] = fRRQList["psumOF"];
    }
  }




  
  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhononDMC", fDetNum))
  {
	//temporary holders for phonon energies by channel, taking into account switches
	double pa1 = fPhononOnOffSwitches["PAS1"]*fRRQList["pa1dmcOF"];
	double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1dmcOF"];
	double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1dmcOF"];
	double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1dmcOF"];
	double pa2 = fPhononOnOffSwitches["PAS2"]*fRRQList["pa2dmcOF"];
	double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2dmcOF"];
	double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2dmcOF"];
	double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2dmcOF"];
	
	//these could be done in a loop with a side iterator but more cumbersome than charge
	//case because psumi#OF changes from side 1 to 2, for example [ANV]
	
    //independent of the psumi#OF and psumo#OF but could be psumi#OF + psumo#OF [ANV]
    fRRQList["psum1dmcOF"] = pa1 + pb1 + pc1 + pd1;
    fRRQList["psum2dmcOF"] = pa2 + pb2 + pc2 + pd2;

    //now get sum of sides [ANV]
    fRRQList["psumdmcOF"] = (fRRQList["psum1dmcOF"] + fRRQList["psum2dmcOF"]); 

   
    //NOTE: if you add an iZIP type you MUST add a condition here [ANV]
    if(fDetType == BatCalibTypes::kiZIPSoudanBiFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1dmcOF"] = pc1 + pd1;
      fRRQList["psumi2dmcOF"] = pb2 + pc2;
      fRRQList["psumo1dmcOF"] = pa1 + pb1;
      fRRQList["psumo2dmcOF"] = pa2 + pd2;
    }
    else if(fDetType == BatCalibTypes::kiZIPSoudanTriFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1dmcOF"] = pb1 + pc1 + pd1;
      fRRQList["psumi2dmcOF"] = pb2 + pc2 + pd2;
      fRRQList["psumo1dmcOF"] = pa1;
      fRRQList["psumo2dmcOF"] = pa2;
    }

    //can only calculate the luke correction if charge is reconstructed [ANV]
    //if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
    //     fRRQList["precoilsumOF"] = fRRQList["psumOF"] - fRRQList["plukeqOF"];

    //if one uses two assumptions can calculate the recoil energy without charge [ANV]
    // 1) event is due to a gamma (electron recoil)
    // 2) event is a bulk event (non-surface)
    //fRRQList["precoilsumOFg"] = fRRQList["psumOF"]/(1 + qifacDelta);

    //using Lindhard for ionization yield, if within boundaries of lookup table
	// if(fRRQList["psumOF"] > 0)
	// {  
    //  if(fRRQList["psumOF"] < fMaxInterpolatedpt) 
    //  {
	//fRRQList["precoilsumOFnL"] = fNRLookupTable->Eval(fRRQList["psumOF"], 0, "S"); 
    //  }
	// }
    //else
    //{
	//assume zero yield for event with negative pt, so pt=pr
	//fRRQList["precoilsumOFnL"] = fRRQList["psumOF"];
	// }
  }

  

  // ========== PT Phonon OF recoil energy ==========
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum)  && 
     (fCheckOFChargeXRQ || fCheckOFChargeRQ))
              fRRQList["precoiltOF"] = fRRQList["ptOF"] - fRRQList["plukeqOF"];

  // ========== PT Phonon NF recoil energy (gamma assumption) ==========
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum)){ 
    //if one uses two assumptions can calculate the recoil energy without charge [ANV]
    // 1) event is due to a gamma (electron recoil)
    // 2) event is a bulk event (non-surface)
    fRRQList["precoiltOFg"] = fRRQList["ptOF"]/(1 + qifacDelta);
   }


  // ========== PT Phonon NF recoil energy ==========
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum)  && 
       (fCheckOFChargeXRQ || fCheckOFChargeRQ))
  {
    fRRQList["precoiltNF"] = fRRQList["ptNF"] - fRRQList["plukeqOF"];
    fRRQList["precoiltNFi"] = fRRQList["ptNF"] - fRRQList["plukeqOFi"];
  }

  // ========== PT Phonon NF recoild energy (gamma and NR assumptions) ==========
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum)){ 
    //if one uses two assumptions can calculate the recoil energy without charge [ANV]
    // 1) event is due to a gamma (electron recoil)
    // 2) event is a bulk event (non-surface)

    fRRQList["precoiltNFg"] = fRRQList["ptNF"]/(1 + qifacDelta);

    //using Lindhard for ionization yield, if within boundaries of lookup table
    if(fRRQList["ptNF"] > 0)
    {  
      if(fRRQList["ptNF"] < fMaxInterpolatedpt) 
      {
	fRRQList["precoiltNFnL"] = fNRLookupTable->Eval(fRRQList["ptNF"], 0, "S"); 
      }
    }
    else
    {
      //assume zero yield for event with negative pt, so pt=pr
      fRRQList["precoiltNFnL"] = fRRQList["ptNF"];
    }
   }


  // ========== Phonon integral summed energies ==========
  if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pa1 = fPhononOnOffSwitches["PAS1"]*fRRQList["pa1INT"];
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1INT"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1INT"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1INT"];
    double pa2 = fPhononOnOffSwitches["PAS2"]*fRRQList["pa2INT"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2INT"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2INT"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2INT"];

    fRRQList["psum1INT"] = pa1 + pb1 + pc1 + pd1;
    fRRQList["psum2INT"] = pa2 + pb2 + pc2 + pd2;
    fRRQList["psumINT"]  = (fRRQList["psum1INT"] + fRRQList["psum2INT"]);

    //this WAS done only for the tri-fold pattern we probably won't use anything else
    //in Run 133 but should check for detector type so generalizable [ANV]
    //NOTE: if you add an iZIP type you MUST add a condition here [ANV]
    if(fDetType == BatCalibTypes::kiZIPSoudanBiFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1INT"] = pc1 + pd1;
      fRRQList["psumi2INT"] = pb2 + pc2;
      fRRQList["psumo1INT"] = pa1 + pb1;
      fRRQList["psumo2INT"] = pa2 + pd2;
    }
    else if(fDetType == BatCalibTypes::kiZIPSoudanTriFold)
    {
      //see http://cdms.berkeley.edu/wiki/doku.php?id=analysis:r132:r132home for labeling [ANV]
      fRRQList["psumi1INT"] = pb1 + pc1 + pd1;
      fRRQList["psumi2INT"] = pb2 + pc2 + pd2;
      fRRQList["psumo1INT"] = pa1;
      fRRQList["psumo2INT"] = pa2;
    }
    

    // phonon integral recoil energy

    if(fCheckOFChargeXRQ | fCheckOFChargeRQ)
        fRRQList["precoilsumINT"] = fRRQList["psumINT"] - fRRQList["plukeqOF"];


    if(fCheckF5ChargeXRQ)
      fRRQList["precoilsumF5INT"] = fRRQList["psumINT"] - fRRQList["plukeqF5"];


    //if one uses two assumptions can calculate the recoil energy without charge [ANV]
    // 1) event is due to a gamma (electron recoil)
    // 2) event is a bulk event (non-surface)
    fRRQList["precoilsumINTg"] = fRRQList["psumINT"]/(1 + qifacDelta);

  }

  // ========== Phonon tail fit summed energies ==========
  if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pa1 = fPhononOnOffSwitches["PAS1"]*fRRQList["pa1TFP"];
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1TFP"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1TFP"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1TFP"];
    double pa2 = fPhononOnOffSwitches["PAS2"]*fRRQList["pa2TFP"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2TFP"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2TFP"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2TFP"];

    fRRQList["psum1TFP"] = pa1 + pb1 + pc1 + pd1;
    fRRQList["psum2TFP"] = pa2 + pb2 + pc2 + pd2;
    fRRQList["psumTFP"]  = fRRQList["psum1TFP"] + fRRQList["psum2TFP"];

    // recoil energy

    if(fCheckOFChargeXRQ | fCheckOFChargeRQ)
        fRRQList["precoilsumTFP"] = fRRQList["psumTFP"] - fRRQList["plukeqOF"];


    if(fCheckF5ChargeXRQ)
      fRRQList["precoilsumF5TFP"] = fRRQList["psumTFP"] - fRRQList["plukeqF5"];
  }

  //Done!

  return;
}

//CalcTotalEnergies must be called before this
void GenRRQDataiZIPSoudan::CalcYields()
{

  // --- using charge OF  ---
  
  if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
  {
   
     // phonon OF 
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
        fRRQList["ysumOF"] = fRRQList["qsummaxOF"]/fRRQList["precoilsumOF"];
        fRRQList["ygsumOF"] = fRRQList["pgqOF"]/fRRQList["psumOF"];
     }



     // phonon PT optimal filter 
     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
     {
        fRRQList["ytOF"] = fRRQList["qsummaxOF"]/fRRQList["precoiltOF"];
        fRRQList["ygtOF"] = fRRQList["pgqOF"]/fRRQList["ptOF"];
     }


    
     // phonon PT non-stationary optimal filter
     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
     {
       fRRQList["ytNF"] = fRRQList["qsummaxOF"]/fRRQList["precoiltNF"];
       fRRQList["ytNFi"] = fRRQList["qimaxOF"]/fRRQList["precoiltNFi"];
       fRRQList["ygtNF"] = fRRQList["pgqOF"]/fRRQList["ptNF"];
     }


     // phonon integral 
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       fRRQList["ygsumINT"] = fRRQList["pgqOF"]/fRRQList["psumINT"];
       fRRQList["ysumINT"] = fRRQList["qsummaxOF"]/fRRQList["precoilsumINT"];
     }


     // phonon tail 
     if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
     {
       fRRQList["ygsumTFP"] = fRRQList["pgqOF"]/fRRQList["psumTFP"];
       fRRQList["ysumTFP"] = fRRQList["qsummaxOF"]/fRRQList["precoilsumTFP"];
     }
  }
   

  // --- using F5 ----
  if(fCheckF5ChargeXRQ)
  {
     // phonon  integral quantities
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     { 
       fRRQList["ygsumF5INT"] = fRRQList["pgqF5"]/fRRQList["psumINT"];
       fRRQList["ysumF5INT"] = fRRQList["qsummaxF5"]/fRRQList["precoilsumF5INT"];
     }

     // phonon  integral quantities
     if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
     { 
       fRRQList["ygsumF5TFP"] = fRRQList["pgqF5"]/fRRQList["psumTFP"];
       fRRQList["ysumF5TFP"] = fRRQList["qsummaxF5"]/fRRQList["precoilsumF5TFP"];
     }
  }    

  return;
}


void GenRRQDataiZIPSoudan::CalcPartitions()
{

  // === Charge OF ===

  if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
  {
    fRRQList["qrpart1OF"] =  fRRQList["qo1OF"]/fRRQList["qsum1OF"];
    fRRQList["qrpart2OF"] =  fRRQList["qo2OF"]/fRRQList["qsum2OF"];

    fRRQList["qrpartsym1OF"] =  (fRRQList["qi1OF"] - fRRQList["qo1OF"])/fRRQList["qsum1OF"];
    fRRQList["qrpartsym2OF"] =  (fRRQList["qi2OF"] - fRRQList["qo2OF"])/fRRQList["qsum2OF"];
    
    fRRQList["qzpartOF"] = (fRRQList["qsum1OF"] - fRRQList["qsum2OF"])
                          /(fRRQList["qsum1OF"] + fRRQList["qsum2OF"]);
    fRRQList["qzpartOFi"] = (fRRQList["qi1OF"] - fRRQList["qi2OF"])
                           /(fRRQList["qi1OF"] + fRRQList["qi2OF"]);
    fRRQList["qzpartOFo"] = (fRRQList["qo1OF"] - fRRQList["qo2OF"])
                           /(fRRQList["qo1OF"] + fRRQList["qo2OF"]);
  }


  // === Charge F5  ===

  if(fCheckF5ChargeXRQ)
  {
    fRRQList["qrpart1F5"] =  fRRQList["qo1F5"]/fRRQList["qsum1F5"];
    fRRQList["qrpart2F5"] =  fRRQList["qo2F5"]/fRRQList["qsum2F5"];

    fRRQList["qrpartsym1F5"] =  (fRRQList["qi1F5"] - fRRQList["qo1F5"])/fRRQList["qsum1F5"];
    fRRQList["qrpartsym2F5"] =  (fRRQList["qi2F5"] - fRRQList["qo2F5"])/fRRQList["qsum2F5"];
  }



  // === Phonon OF ===
  
  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1OF"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1OF"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1OF"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2OF"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2OF"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2OF"];

    //shouldn't this depend on the trifold pattern? [ANV]
    fRRQList["pxpart1OF"] = (pb1*cos(kTheta1Vect[0]) + pc1*cos(kTheta1Vect[1]) + pd1*cos(kTheta1Vect[2])) 
                            / fRRQList["psumi1OF"];
    
    fRRQList["pypart1OF"] = (pb1*sin(kTheta1Vect[0]) + pc1*sin(kTheta1Vect[1]) + pd1*sin(kTheta1Vect[2])) 
                            / fRRQList["psumi1OF"]; 
    
    fRRQList["pxpart2OF"] = (pb2*cos(kTheta2Vect[0]) + pc2*cos(kTheta2Vect[1]) + pd2*cos(kTheta2Vect[2])) 
                            / fRRQList["psumi2OF"];
  
    fRRQList["pypart2OF"] = (pb2*sin(kTheta2Vect[0]) + pc2*sin(kTheta2Vect[1]) + pd2*sin(kTheta2Vect[2])) 
                            / fRRQList["psumi2OF"];



    fRRQList["prpart1OF"] = fRRQList["psumo1OF"]/fRRQList["psum1OF"];
    fRRQList["prpart2OF"] = fRRQList["psumo2OF"]/fRRQList["psum2OF"];

    fRRQList["prpartsym1OF"] = (fRRQList["psumi1OF"] - fRRQList["psumo1OF"])/fRRQList["psum1OF"];
    fRRQList["prpartsym2OF"] = (fRRQList["psumi2OF"] - fRRQList["psumo2OF"])/fRRQList["psum2OF"];

    //avoid using pow() due to slowness [ANV]
    //btw I hate referring to mapped values many times, I hope it's smarter than
    //doing string compares...
    //http://stackoverflow.com/questions/3381739/hash-map-string-compares-and-stdmap
    //http://blog.onnerby.se/2010/08/benchmarking-associative-array.html
    double pxpart1OF2 = fRRQList["pxpart1OF"]*fRRQList["pxpart1OF"];
    double pypart1OF2 = fRRQList["pypart1OF"]*fRRQList["pypart1OF"];
    double pxpart2OF2 = fRRQList["pxpart2OF"]*fRRQList["pxpart2OF"];
    double pypart2OF2 = fRRQList["pypart2OF"]*fRRQList["pypart2OF"];

    fRRQList["prxypart1OF"] = sqrt(pxpart1OF2 + pypart1OF2);
    fRRQList["prxypart2OF"] = sqrt(pxpart2OF2 + pypart2OF2);


    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpart1OF"] > 0 || fRRQList["pypart1OF"]>0){
      fRRQList["pthetapart1OF"] = atan2(fRRQList["pxpart1OF"],fRRQList["pypart1OF"]);
    }
    if(fRRQList["pxpart2OF"] > 0 || fRRQList["pypart2OF"]>0){
      fRRQList["pthetapart2OF"] = atan2(fRRQList["pxpart2OF"],fRRQList["pypart2OF"]);
    }

    //total quantities [ANV]

    fRRQList["pxpartOF"] = (fRRQList["pxpart1OF"]*fRRQList["psumi1OF"] + fRRQList["pxpart2OF"]*fRRQList["psumi2OF"])
                            /(fRRQList["psumi1OF"] + fRRQList["psumi2OF"]);

    fRRQList["pypartOF"] = (fRRQList["pypart1OF"]*fRRQList["psumi1OF"] + fRRQList["pypart2OF"]*fRRQList["psumi2OF"])
                            /(fRRQList["psumi1OF"] + fRRQList["psumi2OF"]);

    //avoid uisng pow() due to slowness [ANV]
    double pxpartOF2 = fRRQList["pxpartOF"]*fRRQList["pxpartOF"];
    double pypartOF2 = fRRQList["pypartOF"]*fRRQList["pypartOF"];

    fRRQList["prxypartOF"] = sqrt(pxpartOF2 + pypartOF2);

    fRRQList["prpartOF"] = (fRRQList["psumo1OF"] + fRRQList["psumo2OF"])/fRRQList["psumOF"];
    double psumo = fRRQList["psumo1OF"] + fRRQList["psumo2OF"];
    double psumi = fRRQList["psumi1OF"] + fRRQList["psumi2OF"];
    fRRQList["prpartsymOF"] = (psumi - psumo)/fRRQList["psumOF"];

    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpartOF"] > 0 || fRRQList["pypartOF"]>0){
      fRRQList["pthetapartOF"] = atan2(fRRQList["pxpartOF"],fRRQList["pypartOF"]);
    }

    fRRQList["pzsumpartOF"] = ( fRRQList["psum1OF"] - fRRQList["psum2OF"] ) / fRRQList["psumOF"];
  } //end if OF quantities exist




  // === Phonon PS1/PS2 OF ===
  if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhonon", fDetNum))
        fRRQList["pzpartOF"] = ( fRRQList["ps1OF"] - fRRQList["ps2OF"] ) / (fRRQList["ps1OF"] + fRRQList["ps2OF"]);



  // OF DMC 
  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhononDMC", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1dmcOF"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1dmcOF"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1dmcOF"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2dmcOF"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2dmcOF"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2dmcOF"];

    //shouldn't this depend on the trifold pattern? [ANV]
    fRRQList["pxpart1dmcOF"] = (pb1*cos(kTheta1Vect[0]) + pc1*cos(kTheta1Vect[1]) + pd1*cos(kTheta1Vect[2])) 
                            / fRRQList["psumi1dmcOF"];
    
    fRRQList["pypart1dmcOF"] = (pb1*sin(kTheta1Vect[0]) + pc1*sin(kTheta1Vect[1]) + pd1*sin(kTheta1Vect[2])) 
                            / fRRQList["psumi1dmcOF"]; 
    
    fRRQList["pxpart2dmcOF"] = (pb2*cos(kTheta2Vect[0]) + pc2*cos(kTheta2Vect[1]) + pd2*cos(kTheta2Vect[2])) 
                            / fRRQList["psumi2dmcOF"];
  
    fRRQList["pypart2dmcOF"] = (pb2*sin(kTheta2Vect[0]) + pc2*sin(kTheta2Vect[1]) + pd2*sin(kTheta2Vect[2])) 
                            / fRRQList["psumi2dmcOF"];



    fRRQList["prpart1dmcOF"] = fRRQList["psumo1dmcOF"]/fRRQList["psum1dmcOF"];
    fRRQList["prpart2dmcOF"] = fRRQList["psumo2dmcOF"]/fRRQList["psum2dmcOF"];

    fRRQList["prpartsym1dmcOF"] = (fRRQList["psumi1dmcOF"] - fRRQList["psumo1dmcOF"])/fRRQList["psum1dmcOF"];
    fRRQList["prpartsym2dmcOF"] = (fRRQList["psumi2dmcOF"] - fRRQList["psumo2dmcOF"])/fRRQList["psum2dmcOF"];

    //avoid using pow() due to slowness [ANV]
    //btw I hate referring to mapped values many times, I hope it's smarter than
    //doing string compares...
    //http://stackoverflow.com/questions/3381739/hash-map-string-compares-and-stdmap
    //http://blog.onnerby.se/2010/08/benchmarking-associative-array.html
    double pxpart1dmcOF2 = fRRQList["pxpart1dmcOF"]*fRRQList["pxpart1dmcOF"];
    double pypart1dmcOF2 = fRRQList["pypart1dmcOF"]*fRRQList["pypart1dmcOF"];
    double pxpart2dmcOF2 = fRRQList["pxpart2dmcOF"]*fRRQList["pxpart2dmcOF"];
    double pypart2dmcOF2 = fRRQList["pypart2dmcOF"]*fRRQList["pypart2dmcOF"];

    fRRQList["prxypart1dmcOF"] = sqrt(pxpart1dmcOF2 + pypart1dmcOF2);
    fRRQList["prxypart2dmcOF"] = sqrt(pxpart2dmcOF2 + pypart2dmcOF2);


    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpart1dmcOF"] > 0 || fRRQList["pypart1dmcOF"]>0){
      fRRQList["pthetapart1dmcOF"] = atan2(fRRQList["pxpart1dmcOF"],fRRQList["pypart1dmcOF"]);
    }
    if(fRRQList["pxpart2dmcOF"] > 0 || fRRQList["pypart2dmcOF"]>0){
      fRRQList["pthetapart2dmcOF"] = atan2(fRRQList["pxpart2dmcOF"],fRRQList["pypart2dmcOF"]);
    }

    //total quantities [ANV]

    fRRQList["pxpartdmcOF"] = (fRRQList["pxpart1dmcOF"]*fRRQList["psumi1dmcOF"] + fRRQList["pxpart2dmcOF"]*fRRQList["psumi2dmcOF"])
                            /(fRRQList["psumi1dmcOF"] + fRRQList["psumi2dmcOF"]);

    fRRQList["pypartdmcOF"] = (fRRQList["pypart1dmcOF"]*fRRQList["psumi1dmcOF"] + fRRQList["pypart2dmcOF"]*fRRQList["psumi2dmcOF"])
                            /(fRRQList["psumi1dmcOF"] + fRRQList["psumi2dmcOF"]);

    //avoid uisng pow() due to slowness [ANV]
    double pxpartdmcOF2 = fRRQList["pxpartdmcOF"]*fRRQList["pxpartdmcOF"];
    double pypartdmcOF2 = fRRQList["pypartdmcOF"]*fRRQList["pypartdmcOF"];

    fRRQList["prxypartdmcOF"] = sqrt(pxpartdmcOF2 + pypartdmcOF2);

    fRRQList["prpartdmcOF"] = (fRRQList["psumo1dmcOF"] + fRRQList["psumo2dmcOF"])/fRRQList["psumdmcOF"];
    double psumo = fRRQList["psumo1dmcOF"] + fRRQList["psumo2dmcOF"];
    double psumi = fRRQList["psumi1dmcOF"] + fRRQList["psumi2dmcOF"];
    fRRQList["prpartsymdmcOF"] = (psumi - psumo)/fRRQList["psumdmcOF"];

    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpartdmcOF"] > 0 || fRRQList["pypartdmcOF"]>0){
      fRRQList["pthetapartdmcOF"] = atan2(fRRQList["pxpartdmcOF"],fRRQList["pypartdmcOF"]);
    }

    fRRQList["pzsumpartdmcOF"] = ( fRRQList["psum1dmcOF"] - fRRQList["psum2dmcOF"] ) / fRRQList["psumdmcOF"];
  } //end if OF quantities exist

  // DMC
  if(fIOMan.CheckBatRootPhononAlg("PSIDES_OptimalFilterPhononDMC", fDetNum))
	  fRRQList["pzpartdmcOF"] = ( fRRQList["ps1dmcOF"] - fRRQList["ps2dmcOF"] ) / (fRRQList["ps1dmcOF"] + fRRQList["ps2dmcOF"]);


  

  // === Phonon Integral ===
  if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
  {
    //temporary holders for phonon energies by channel, taking into account switches
    double pb1 = fPhononOnOffSwitches["PBS1"]*fRRQList["pb1INT"];
    double pc1 = fPhononOnOffSwitches["PCS1"]*fRRQList["pc1INT"];
    double pd1 = fPhononOnOffSwitches["PDS1"]*fRRQList["pd1INT"];
    double pb2 = fPhononOnOffSwitches["PBS2"]*fRRQList["pb2INT"];
    double pc2 = fPhononOnOffSwitches["PCS2"]*fRRQList["pc2INT"];
    double pd2 = fPhononOnOffSwitches["PDS2"]*fRRQList["pd2INT"];

    //shouldn't this depend on the trifold pattern? [ANV]
    fRRQList["pxpart1INT"] = (pb1*cos(kTheta1Vect[0]) + 
			      pc1*cos(kTheta1Vect[1]) + pd1*cos(kTheta1Vect[2])) 
                            / fRRQList["psumi1INT"];
    
    fRRQList["pypart1INT"] = (pb1*sin(kTheta1Vect[0]) + 
			    pc1*sin(kTheta1Vect[1]) + pd1*sin(kTheta1Vect[2])) 
                            / fRRQList["psumi1INT"]; 
    
    fRRQList["pxpart2INT"] = (pb2*cos(kTheta2Vect[0]) + 
			      pc2*cos(kTheta2Vect[1]) + pd2*cos(kTheta2Vect[2])) 
                            / fRRQList["psumi2INT"];
  
    fRRQList["pypart2INT"] = (pb2*sin(kTheta2Vect[0]) + 
	  		      pc2*sin(kTheta2Vect[1]) + pd2*sin(kTheta2Vect[2])) 
                            / fRRQList["psumi2INT"];



    fRRQList["prpart1INT"] = fRRQList["psumo1INT"]/fRRQList["psum1INT"];
    fRRQList["prpart2INT"] = fRRQList["psumo2INT"]/fRRQList["psum2INT"];

    fRRQList["prpartsym1INT"] = (fRRQList["psumi1INT"] - fRRQList["psumo1INT"])/fRRQList["psum1INT"];
    fRRQList["prpartsym2INT"] = (fRRQList["psumi2INT"] - fRRQList["psumo2INT"])/fRRQList["psum2INT"];

    //avoid using pow() due to slowness [ANV]
    //btw I hate referring to mapped values many times, I hope it's smarter than
    //doing string compares...
    //http://stackoverflow.com/questions/3381739/hash-map-string-compares-and-stdmap
    //http://blog.onnerby.se/2010/08/benchmarking-associative-array.html
    double pxpart1INT2 = fRRQList["pxpart1INT"]*fRRQList["pxpart1INT"];
    double pypart1INT2 = fRRQList["pypart1INT"]*fRRQList["pypart1INT"];
    double pxpart2INT2 = fRRQList["pxpart2INT"]*fRRQList["pxpart2INT"];
    double pypart2INT2 = fRRQList["pypart2INT"]*fRRQList["pypart2INT"];

    fRRQList["prxypart1INT"] = sqrt(pxpart1INT2 + pypart1INT2);
    fRRQList["prxypart2INT"] = sqrt(pxpart2INT2 + pypart2INT2);


    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpart1INT"] > 0 || fRRQList["pypart1INT"]>0){
      fRRQList["pthetapart1INT"] = atan2(fRRQList["pxpart1INT"],fRRQList["pypart1INT"]);
    }
    if(fRRQList["pxpart2INT"] > 0 || fRRQList["pypart2INT"]>0){
      fRRQList["pthetapart2INT"] = atan2(fRRQList["pxpart2INT"],fRRQList["pypart2INT"]);
    }

    //total quantities [ANV]
    
    fRRQList["pxpartINT"] = (fRRQList["pxpart1INT"]*fRRQList["psumi1INT"] + fRRQList["pxpart2INT"]*fRRQList["psumi2INT"])
                            /(fRRQList["psumi1INT"] + fRRQList["psumi2INT"]);

    fRRQList["pypartINT"] = (fRRQList["pypart1INT"]*fRRQList["psumi1INT"] + fRRQList["pypart2INT"]*fRRQList["psumi2INT"])
                            /(fRRQList["psumi1INT"] + fRRQList["psumi2INT"]);

    //avoid uisng pow() due to slowness [ANV]
    double pxpartINT2 = fRRQList["pxpartINT"]*fRRQList["pxpartINT"];
    double pypartINT2 = fRRQList["pypartINT"]*fRRQList["pypartINT"];

    fRRQList["prxypartINT"] = sqrt(pxpartINT2 + pypartINT2);

    fRRQList["prpartINT"] = (fRRQList["psumo1INT"] + fRRQList["psumo2INT"])/fRRQList["psumINT"];
    double psumo = fRRQList["psumo1INT"] + fRRQList["psumo2INT"];
    double psumi = fRRQList["psumi1INT"] + fRRQList["psumi2INT"];
    fRRQList["prpartsymINT"] = (psumi - psumo)/fRRQList["psumINT"];

    //define theta partition but both x and y can't be zero [ANV]
    if(fRRQList["pxpartINT"] > 0 || fRRQList["pypartINT"]>0){
      fRRQList["pthetapartINT"] = atan2(fRRQList["pxpartINT"],fRRQList["pypartINT"]);
    }

    fRRQList["pzsumpartINT"] = ( fRRQList["psum1INT"] - fRRQList["psum2INT"] ) / fRRQList["psumINT"];

  } //end if integral quantities exist


  return;
}
    

//calibrate the OF resolution quantities
void GenRRQDataiZIPSoudan::CalcOFResolutions()
{

   // === calibrate the OFdelay and OFamplitude resolutions ===
   //      note values stored in order QIS1, QOS1, PAS1, PBS1,PCS1, PDS1,QIS2,.... 


   // charge 
   double tempQI1Max = fOFMaxTemplate[0];  
   double tempQO1Max = fOFMaxTemplate[1];
   double tempQI2Max = fOFMaxTemplate[2];  
   double tempQO2Max = fOFMaxTemplate[3];


   fRRQList["qi1delayres"] = fChargeCal[0]*tempQI1Max*fDelaySig[0];
   fRRQList["qo1delayres"] = fChargeCal[1]*tempQO1Max*fDelaySig[1];      
   fRRQList["qi1ampres"] = fChargeCal[0]*tempQI1Max*fAmpSig[0];
   fRRQList["qo1ampres"] = fChargeCal[1]*tempQO1Max*fAmpSig[1];

   fRRQList["qi2delayres"] = fChargeCal[4]*tempQI2Max*fDelaySig[6];
   fRRQList["qo2delayres"] = fChargeCal[5]*tempQO2Max*fDelaySig[7];      
   fRRQList["qi2ampres"] = fChargeCal[4]*tempQI2Max*fAmpSig[6];
   fRRQList["qo2ampres"] = fChargeCal[5]*tempQO2Max*fAmpSig[7];



   // loop over phonon channels and calculae for each
   for(int chanItr=0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
     {
       string prefix = BatCalibTypes::kiZIPSoudanPhononChan[chanItr];
       string prefixCal = BatCalibTypes::kiZIPSoudanPhononCal[chanItr];
       
       int sigItr = chanItr;
       if (prefix.find("S1")!=string::npos) 
          sigItr = sigItr+2;
       else
          sigItr = sigItr+4;
     
       fRRQList[prefixCal+"delayres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononRelCal[chanItr]*fPhononOFCal*fDelaySig[sigItr];  
       fRRQList[prefixCal+"ampres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononRelCal[chanItr]*fPhononOFCal*fAmpSig[sigItr]; 

     }

   return;
}

// ==============================  Optional RRQ calculations ==============================================


void GenRRQDataiZIPSoudan::FindPrimaryPhononChannel()
{
    
   // max OF amplitdude

   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {
     
     //loop over channels to find max amplitude
     double maxAmpS1 = -1.0*numeric_limits<double>::infinity();
     double maxAmpS2 = -1.0*numeric_limits<double>::infinity();
     double maxAmpS1i = -1.0*numeric_limits<double>::infinity();
     double maxAmpS2i = -1.0*numeric_limits<double>::infinity();

     for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
      {
        string calibChanName = BatCalibTypes::kiZIPSoudanPhononCal[chanItr];
	string chanName      = BatCalibTypes::kiZIPSoudanPhononChan[chanItr];
        double chanValOF     = fPhononOnOffSwitches[chanName]*fRRQList[calibChanName + "OF"];
 
        if (chanValOF>maxAmpS1 && calibChanName.find("1")!=string::npos) {
          maxAmpS1 = chanValOF;
          fRRQList["pprimechan1OF"] = chanItr+1;
        }

       
        if (chanValOF>maxAmpS2 && calibChanName.find("2")!=string::npos) {
          maxAmpS2 = chanValOF;
          fRRQList["pprimechan2OF"] = chanItr-3;
        }


        if (chanValOF>maxAmpS1i && calibChanName.find("1")!=string::npos && calibChanName.find("a")==string::npos) {
          maxAmpS1i = chanValOF;
          fRRQList["pprimechani1OF"] = chanItr+1;
        }

       
        if (chanValOF>maxAmpS2i && calibChanName.find("2")!=string::npos && calibChanName.find("a")==string::npos) {
          maxAmpS2i = chanValOF;
          fRRQList["pprimechani2OF"] = chanItr-3;
        }
      }
    }
  
 
   // min delay

   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
   
     double minDelS1 = 1.0*numeric_limits<double>::infinity();
     double minDelS2 = 1.0*numeric_limits<double>::infinity();

     for(int chanItr = 0; chanItr < BatCalibTypes::kiZIPSoudanNPhononChan; chanItr++)
     {
       string chanName      = BatCalibTypes::kiZIPSoudanPhononChan[chanItr];
       double chanValWK     = fPhononOnOffSwitches[chanName]*fIOMan.Get(chanName + "WKr20");
 
       if (chanValWK<minDelS1 && chanValWK != 0 && chanName.find("1")!=string::npos) 
       {
	 minDelS1 = chanValWK;
	 fRRQList["pprimechan1WK"] = chanItr+1;
       }

       
       if (chanValWK<minDelS2 && chanValWK != 0 && chanName.find("2")!=string::npos) 
       {
	 minDelS2 = chanValWK;
	 fRRQList["pprimechan2WK"] = chanItr-3;
       }

     } //end loop over channels

   } //end if ConstFreqRTFTWalkPhonon is used

  
   // combined OF amplitude and delay [ANV]/[LLH]
   // this must come AFTER the above two calculations

   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) && fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon",fDetNum))
   {   
     if (fRRQList["pprimechan1OF"] == 1) 
     {
       fRRQList["pprimechan1OFWK"] = fRRQList["pprimechan1WK"]; //use min delay 
     }
     else{
       fRRQList["pprimechan1OFWK"] = fRRQList["pprimechan1OF"]; //use max OF
     }

     if (fRRQList["pprimechan2OF"] == 1) 
     {
       fRRQList["pprimechan2OFWK"] = fRRQList["pprimechan2WK"]; //use min delay
     }
     else{
       fRRQList["pprimechan2OFWK"] = fRRQList["pprimechan2OF"]; //use max OF
     }
       
   } //end if both OF and ConstFreqRTFTWalk used
 
   return;
}
     
  


void GenRRQDataiZIPSoudan::CalcConstFreqRTFTWalkRRQ()
{

   
   // ----- primary channel rrq's  ------

   // primary channel using WK delay
   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
     {
       // S1
       if (fRRQList["pprimechan1WK"]>0) { 
                
           int  primChanIndex1 = (int) (fRRQList["pprimechan1WK"]-1);
           string primChanName1 = BatCalibTypes::kiZIPSoudanPhononChan[primChanIndex1];
               
           fRRQList["pminrt1WK_1040"] = (fIOMan.Get(primChanName1+"WKr40") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
           fRRQList["pminrt1WK_1070"] = (fIOMan.Get(primChanName1+"WKr70") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
           fRRQList["pminrt1WK_10100"] = (fIOMan.Get(primChanName1+"WKr100") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
        }

       // S2 
       if (fRRQList["pprimechan2WK"]>0) { 
       
           int  primChanIndex2 = (int) (fRRQList["pprimechan2WK"]+3);
           string primChanName2 = BatCalibTypes::kiZIPSoudanPhononChan[primChanIndex2];
       
           fRRQList["pminrt2WK_1040"] = (fIOMan.Get(primChanName2+"WKr40") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
           fRRQList["pminrt2WK_1070"] = (fIOMan.Get(primChanName2+"WKr70") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
           fRRQList["pminrt2WK_10100"] = (fIOMan.Get(primChanName2+"WKr100") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
        }
    }

   // primary channel using both OK/WK 
   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) && fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon",fDetNum))
     {
        // S1
        if (fRRQList["pprimechan1OFWK"]>0) {

          int  primChanIndex1 = (int) (fRRQList["pprimechan1OFWK"]-1);
          string primChanName1 = BatCalibTypes::kiZIPSoudanPhononChan[primChanIndex1];
      
          fRRQList["pminrt1OFWK_1040"] = (fIOMan.Get(primChanName1+"WKr40") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
          fRRQList["pminrt1OFWK_1070"] = (fIOMan.Get(primChanName1+"WKr70") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
          fRRQList["pminrt1OFWK_10100"] = (fIOMan.Get(primChanName1+"WKr100") - fIOMan.Get(primChanName1+"WKr10"))*1e6; //in microseconds
        }

      if (fRRQList["pprimechan2OFWK"]>0) {
         int  primChanIndex2 = (int) (fRRQList["pprimechan2OFWK"]+3);
         string primChanName2 = BatCalibTypes::kiZIPSoudanPhononChan[primChanIndex2];
      
         fRRQList["pminrt2OFWK_1040"] = (fIOMan.Get(primChanName2+"WKr40") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
         fRRQList["pminrt2OFWK_1070"] = (fIOMan.Get(primChanName2+"WKr70") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
         fRRQList["pminrt2OFWK_10100"] = (fIOMan.Get(primChanName2+"WKr100") - fIOMan.Get(primChanName2+"WKr10"))*1e6; //in microseconds
      } 
    }
 

   // ----- S1/S2 rrq's  ------
   
   if(fIOMan.CheckBatRootPhononAlg("PSIDES_ConstFreqRTFTWalkPhonon", fDetNum)) 
       {

         fRRQList["ps1rtWK_1040"] =  (fIOMan.Get("PS1WKr40") - fIOMan.Get("PS1WKr10"))*1e6; //in microseconds
         fRRQList["ps2rtWK_1040"] =  (fIOMan.Get("PS2WKr40") - fIOMan.Get("PS2WKr10"))*1e6; //in microseconds

         fRRQList["ps1rtWK_1070"] =  (fIOMan.Get("PS1WKr70") - fIOMan.Get("PS1WKr10"))*1e6; //in microseconds
         fRRQList["ps2rtWK_1070"] =  (fIOMan.Get("PS2WKr70") - fIOMan.Get("PS2WKr10"))*1e6; //in microseconds

         fRRQList["ps1rtWK_4070"] =  (fIOMan.Get("PS1WKr70") - fIOMan.Get("PS1WKr40"))*1e6; //in microseconds
         fRRQList["ps2rtWK_4070"] =  (fIOMan.Get("PS2WKr70") - fIOMan.Get("PS2WKr40"))*1e6; //in microseconds
  
         fRRQList["ps1rtftWK_8080"] =  (fIOMan.Get("PS1WKf80") - fIOMan.Get("PS1WKr80"))*1e6; //in microseconds
         fRRQList["ps2rtftWK_8080"] =  (fIOMan.Get("PS2WKf80") - fIOMan.Get("PS2WKr80"))*1e6; //in microseconds

       }

   // ----- PT rrq's ------
    if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum)) 
      fRRQList["ptftWK_9520"] =  (fIOMan.Get("PTWKf95") - fIOMan.Get("PTWKf20"))*1e6; //in microseconds
     

   return;
}





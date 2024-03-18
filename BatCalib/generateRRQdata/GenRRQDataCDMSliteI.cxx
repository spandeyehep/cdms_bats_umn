/////////////////////////////////////////////////////////////////////////////////
//Class Name: GenRRQDataCDMSliteI
//Authors: Y. Ricci, B. Serfass
//Description: This class is intended for the CDMSlite Run2 (started in December 2013).
//Applies phonon calibration (relative amplitude scaling).  Generates
//primary rrq's for analysis (xy-delays, partitions, yields, timing quantities).
//Get rid of the charge rrqs.
//
//File Import By: Y. Ricci
//Creation Date: March 13, 2014
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <math.h>
#include <limits>

#include "TMath.h"

#include "GenRRQDataCDMSliteI.h"
#include "BatCalibTypes.h"


GenRRQDataCDMSliteI::GenRRQDataCDMSliteI(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fDetType(-999999),
   fCheckOFChargeXRQ(false),
   fCheckOFChargeRQ(false),
   fIsFirstProduction(0)
{
//   cout <<"Hello from GenRRQDataCDMSliteI! " << endl;

   kThetaVect[0] = 150.*(TMath::Pi()/180.);
   kThetaVect[1] = 270.*(TMath::Pi()/180.);
   kThetaVect[2] = 30.*(TMath::Pi()/180.);

}

GenRRQDataCDMSliteI::~GenRRQDataCDMSliteI()
{
//   cout <<"Goodbye from GenRRQDataCDMSliteI()" << endl;
}


//initialize desired output variables here (this happens before looping over events in the zip trees)
void GenRRQDataCDMSliteI::ConstructRRQList()
{
     double initVal = -999999.;

     //construct the RRQ list here.  Note anything added outside of here 
     //is not going to be in the output

     fRRQList.insert(pair<string,double>("Empty", initVal));
     fRRQList.insert(pair<string,double>("DetType", initVal));

  
     // Optimal Filter charge 
     if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
      {
	fRRQList.insert(pair<string,double>("qiOF", initVal));
	fRRQList.insert(pair<string,double>("qoOF", initVal));
	fRRQList.insert(pair<string,double>("qiOF0", initVal));
	fRRQList.insert(pair<string,double>("qoOF0", initVal));
	fRRQList.insert(pair<string,double>("qsumOF", initVal));
	fRRQList.insert(pair<string,double>("qrpartOF", initVal)); 
      }


 // ========== phonon  OF (single channels)  ==========
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
       
       // Energies

       fRRQList.insert(pair<string,double>("paOF", initVal));
       fRRQList.insert(pair<string,double>("pbOF", initVal));
       fRRQList.insert(pair<string,double>("pcOF", initVal));
       fRRQList.insert(pair<string,double>("pdOF", initVal));

       fRRQList.insert(pair<string,double>("paOF0", initVal));
       fRRQList.insert(pair<string,double>("pbOF0", initVal));
       fRRQList.insert(pair<string,double>("pcOF0", initVal));
       fRRQList.insert(pair<string,double>("pdOF0", initVal));       

       fRRQList.insert(pair<string,double>("psumOF", initVal));
       fRRQList.insert(pair<string,double>("psumiOF", initVal));
       fRRQList.insert(pair<string,double>("psumoOF", initVal));
       
       //corrected individual channels
       fRRQList.insert(pair<string,double>("paOFc", initVal));
       fRRQList.insert(pair<string,double>("pbOFc", initVal));
       fRRQList.insert(pair<string,double>("pcOFc", initVal));
       fRRQList.insert(pair<string,double>("pdOFc", initVal));

       fRRQList.insert(pair<string,double>("paOF0c", initVal));
       fRRQList.insert(pair<string,double>("pbOF0c", initVal));
       fRRQList.insert(pair<string,double>("pcOF0c", initVal));
       fRRQList.insert(pair<string,double>("pdOF0c", initVal));       


      // fRRQList.insert(pair<string,double>("precoilsumOF", initVal));
      // fRRQList.insert(pair<string,double>("precoilgOF", initVal));  
      // fRRQList.insert(pair<string,double>("precoiliOF", initVal)); 

      // partitions

       fRRQList.insert(pair<string,double>("pxpartOF", initVal));
       fRRQList.insert(pair<string,double>("pypartOF", initVal));
       fRRQList.insert(pair<string,double>("prxypartOF", initVal));
       fRRQList.insert(pair<string,double>("prpartOF", initVal));

       // yields

       // fRRQList.insert(pair<string,double>("ysumOF", initVal));
       // fRRQList.insert(pair<string,double>("ygsumOF", initVal)); 
       // fRRQList.insert(pair<string,double>("ysumiOF", initVal)); 
       // fRRQList.insert(pair<string,double>("ygsumiOF", initVal));

     }

     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
     {
       
       // Energies
       fRRQList.insert(pair<string,double>("paOF1X2P", initVal));
       fRRQList.insert(pair<string,double>("pbOF1X2P", initVal));
       fRRQList.insert(pair<string,double>("pcOF1X2P", initVal));
       fRRQList.insert(pair<string,double>("pdOF1X2P", initVal));

       fRRQList.insert(pair<string,double>("paOF1X2P0", initVal));
       fRRQList.insert(pair<string,double>("pbOF1X2P0", initVal));
       fRRQList.insert(pair<string,double>("pcOF1X2P0", initVal));
       fRRQList.insert(pair<string,double>("pdOF1X2P0", initVal));       


       //[wap]: Jorge, it is unclear why these were added
       fRRQList.insert(pair<string,double>("psumOF1X2", initVal));
       //[wap]: Jorge, it is unclear what this RRQ represents
       fRRQList.insert(pair<string,double>("psumiOF1X2", initVal));

       //residual amplitudes
       fRRQList.insert(pair<string,double>("paOF1X2R", initVal));
       fRRQList.insert(pair<string,double>("pbOF1X2R", initVal));
       fRRQList.insert(pair<string,double>("pcOF1X2R", initVal));
       fRRQList.insert(pair<string,double>("pdOF1X2R", initVal));

       fRRQList.insert(pair<string,double>("paOF1X2R0", initVal));
       fRRQList.insert(pair<string,double>("pbOF1X2R0", initVal));
       fRRQList.insert(pair<string,double>("pcOF1X2R0", initVal));
       fRRQList.insert(pair<string,double>("pdOF1X2R0", initVal));   


      // partitions

       fRRQList.insert(pair<string,double>("pxpartOF1X2", initVal));
       fRRQList.insert(pair<string,double>("pypartOF1X2", initVal));

       fRRQList.insert(pair<string,double>("pa2tdelr", initVal));
       fRRQList.insert(pair<string,double>("pb2tdelr", initVal));
       fRRQList.insert(pair<string,double>("pc2tdelr", initVal));
       fRRQList.insert(pair<string,double>("pd2tdelr", initVal));

       //[wap]: Jorge, it is unclear what this rrq is
       fRRQList.insert(pair<string,double>("prxypartOF1X2", initVal));
       fRRQList.insert(pair<string,double>("prpartOF1X2", initVal));
       fRRQList.insert(pair<string,double>("prpartOF1X2noPhiCorr", initVal));
     }

    // ========= Total phonons OF and NF =============

     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
     {
       // energy
       fRRQList.insert(pair<string,double>("ptOF", initVal));
       fRRQList.insert(pair<string,double>("ptOF0", initVal));
       // calibrated, but *uncorrected* energy [wap] (same as ptOF)
       fRRQList.insert(pair<string,double>("ptOFuc", initVal));
       fRRQList.insert(pair<string,double>("ptOF0uc", initVal));
       // calibrated, and *corrected* energy [wap]
       fRRQList.insert(pair<string,double>("ptOFc", initVal));
       fRRQList.insert(pair<string,double>("ptOF0c", initVal));

        // primary channel
       fRRQList.insert(pair<string,double>("pprimechanOF", initVal));

        // Inner primary channel 
       fRRQList.insert(pair<string,double>("pprimechaniOF", initVal));
     } 
      
     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum)) 
     {
       // energy
       fRRQList.insert(pair<string,double>("ptNF", initVal));
       fRRQList.insert(pair<string,double>("ptNF0", initVal));
       // calibrated, but *uncorrected* energy [wap] (same as ptNF)
       fRRQList.insert(pair<string,double>("ptNFuc", initVal));
       fRRQList.insert(pair<string,double>("ptNF0uc", initVal));
       // calibrated, and *corrected* energy [wap]
       fRRQList.insert(pair<string,double>("ptNFc", initVal));
       fRRQList.insert(pair<string,double>("ptNF0c", initVal));
     }

     if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum)) 
     {
       // energy
       fRRQList.insert(pair<string,double>("ptOF1X2P", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2P0", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2R", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2R0", initVal));
       // calibrated, but *uncorrected* energy [wap] (same as ptOF1X2P)
       fRRQList.insert(pair<string,double>("ptOF1X2Puc", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2P0uc", initVal));
       // calibrated, and *corrected* energy [wap]
       fRRQList.insert(pair<string,double>("ptOF1X2Pc", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2P0c", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2Rc", initVal));
       fRRQList.insert(pair<string,double>("ptOF1X2R0c", initVal));
     }
    
    // ========== OF resolution ==========
    
    if (fIOMan.IsOFresFilled()) {
        
        fRRQList.insert(pair<string,double>("padelayres", initVal));
        fRRQList.insert(pair<string,double>("pbdelayres", initVal));
        fRRQList.insert(pair<string,double>("pcdelayres", initVal));
        fRRQList.insert(pair<string,double>("pddelayres", initVal));
        
        fRRQList.insert(pair<string,double>("paampres", initVal));
        fRRQList.insert(pair<string,double>("pbampres", initVal));
        fRRQList.insert(pair<string,double>("pcampres", initVal));
        fRRQList.insert(pair<string,double>("pdampres", initVal));
	}
/*        
    // ==========  Phonon Integral  ==========
    
    if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
    {
        
        // energy
        fRRQList.insert(pair<string,double>("pa1INT", initVal));
        fRRQList.insert(pair<string,double>("pb1INT", initVal));
        fRRQList.insert(pair<string,double>("pc1INT", initVal));
        fRRQList.insert(pair<string,double>("pd1INT", initVal));
        
        fRRQList.insert(pair<string,double>("psumINT", initVal));
        
        fRRQList.insert(pair<string,double>("psumiINT", initVal));
        fRRQList.insert(pair<string,double>("psumoINT", initVal));
        fRRQList.insert(pair<string,double>("psumoINT", initVal));
    
        
    } //end if pulse integral available

    
    // ==========  Phonon Tail Fit  ==========
    // note that energy can be obtained from tail fit
    // in several ways.  Only one suffix is used at the moment.
    
    if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
    {
        
        // energy
        fRRQList.insert(pair<string,double>("paTFP", initVal));
        fRRQList.insert(pair<string,double>("pbTFP", initVal));
        fRRQList.insert(pair<string,double>("pcTFP", initVal));
        fRRQList.insert(pair<string,double>("pdTFP", initVal));
        
        fRRQList.insert(pair<string,double>("psumTFP", initVal));
        
        
    } //end if tail fit available
*/    
        
    // ==========  Phonon RTFT Walk (single channels)  ==========
    
    if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
    {
        
        // partition
        fRRQList.insert(pair<string,double>("pxdelWK", initVal));
        fRRQList.insert(pair<string,double>("pydelWK", initVal));
        
        fRRQList.insert(pair<string,double>("prdelWK", initVal));

        
        // primary sensor
        fRRQList.insert(pair<string,double>("pprimechanWK", initVal));
        
        // Timing parameter
        fRRQList.insert(pair<string,double>("pminrtWK_1040", initVal));

        fRRQList.insert(pair<string,double>("pminrtWK_1070", initVal));

        fRRQList.insert(pair<string,double>("pminrtWK_10100", initVal));

    }
    // ==========  Phonon RTFT Walk and Phonon OF (single channels)  ==========
    
    if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) && fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon",fDetNum))
    {
        
        // primary sensor
        fRRQList.insert(pair<string,double>("pprimechanOFWK", initVal));
        
        // Timing parameter
        fRRQList.insert(pair<string,double>("pminrtOFWK_1040", initVal));
        
        fRRQList.insert(pair<string,double>("pminrtOFWK_1070", initVal));
        
        fRRQList.insert(pair<string,double>("pminrtOFWK_10100", initVal));
        
    }
    // ==========  Phonon RTFT Walk  ==========
     if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum))
         fRRQList.insert(pair<string,double>("ptftWK_9520", initVal)); 
         



    //Now tell the io manager to make an output tree based on this list    

    string treeName(Form("calibzip%d", fDetNum));
    fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

    return;
}

void GenRRQDataCDMSliteI::ResetRRQValues()
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
void GenRRQDataCDMSliteI::ActivateRQs()
{

   //Any RQ that is needed for calculation should be specified here
   //otherwise it is not read from the file
  
   //to first order, everything needs this
   fIOMan.Activate("Empty");
   fIOMan.Activate("DetType");
   fIOMan.Activate("SeriesNumber"); //for keeping track within supermerged files

   // activate channel status
   bool readChanStatus = fIOMan.CheckBatRootUserSettingsFlags("READ_DET_STATUS_FILE");
    
   if(readChanStatus) {
   for(int chanItr=0; chanItr < BatCalibTypes::kZIPFLIPNAllChan; chanItr++)
       fIOMan.Activate(BatCalibTypes::kZIPFLIPChannelNames[chanItr]+"status");
}
    
   //Read the entries until one gets to the first non-empty value - b/c detType is not stored for empty events
   int maxEntries = fIOMan.GetMaxEntries();

   for(int eventCtr = 0; eventCtr < maxEntries; eventCtr++)
   {
     fIOMan.ReadNextEntry(eventCtr);
     
     if(fIOMan.Get("Empty") == 0.0) 
     {
       //Store the Det_Type variable
       fDetType = (int)fIOMan.Get("DetType");
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
      fIOMan.Activate("HVnamps");  

      // Fill map with BaseTemp>0
      for(int eventCtr = 0; eventCtr < maxEntries; eventCtr++)
        {
         fIOMan.ReadNextEntry(eventCtr);
         double baseTemp = fIOMan.Get("BaseTemp");
  
         if (baseTemp>0) 
            fGoodBaseTempMap.insert(pair<int,double>(eventCtr,baseTemp));
        }
   }
	

   // --- Qbias, gain, norms ---
   /*
   //For TF and MC we currently don't have external files
   //this leaves the option to pull the values (prespecified) from the config files
   //this is less than ideal and should be fixed in the longer term by integrating
   //these values in the raw data stream
   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      fIOMan.Activate("QIbias"); 
      fIOMan.Activate("QObias");
   }

   fIOMan.Activate("QIgain");
   fIOMan.Activate("QOgain");

   fIOMan.Activate("QInorm");
   fIOMan.Activate("QOnorm");

   */


   // --- Charge Optimal Filter ---

   if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
   {
     fIOMan.Activate("QIOFvolts");
     fIOMan.Activate("QOOFvolts");
     fIOMan.Activate("QIOFvolts0");
     fIOMan.Activate("QOOFvolts0");
     fIOMan.Activate("QSOFdelay");
     
     fIOMan.Activate("QIsat");
     fIOMan.Activate("QOsat");

   }

  
   // ----- Phonon RQ's -----

   // --- Phonon Optimal Filter and Integral ---
   
   for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
      // optimal filter
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps0");
     }

  
     // gain
     fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "gain");

     // norm
     fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "norm");

 /*    // --------  Phonon  INT --------
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       //integral routine
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"INTall");       
     }

     // --------  Phonon  Tail Fit --------
     if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
     {
       //integral routine
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"TFPint");       
     }
*/
   }

    

   //  ========== PHONON PT RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
   {
         fIOMan.Activate("PTOFamps");
         fIOMan.Activate("PTOFamps0");
   }

   //  ========== PHONON 1X2 RQs Activation ==========
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
   {
         fIOMan.Activate("PAOF1X2Ramps");
         fIOMan.Activate("PBOF1X2Ramps");
         fIOMan.Activate("PCOF1X2Ramps");
         fIOMan.Activate("PDOF1X2Ramps");
         fIOMan.Activate("PAOF1X2Ramps0");
         fIOMan.Activate("PBOF1X2Ramps0");
         fIOMan.Activate("PCOF1X2Ramps0");
         fIOMan.Activate("PDOF1X2Ramps0");
         fIOMan.Activate("PAOF1X2Pamps");
         fIOMan.Activate("PBOF1X2Pamps");
         fIOMan.Activate("PCOF1X2Pamps");
         fIOMan.Activate("PDOF1X2Pamps");
         fIOMan.Activate("PAOF1X2Pamps0");
         fIOMan.Activate("PBOF1X2Pamps0");
         fIOMan.Activate("PCOF1X2Pamps0");
         fIOMan.Activate("PDOF1X2Pamps0");
         fIOMan.Activate("PAOF1X2delay");
         fIOMan.Activate("PBOF1X2delay");
         fIOMan.Activate("PCOF1X2delay");
         fIOMan.Activate("PDOF1X2delay");
   }

   //  ========== PHONON PT 1X2 RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum))
   {
         fIOMan.Activate("PTOF1X2Ramps");
         fIOMan.Activate("PTOF1X2Ramps0");
         fIOMan.Activate("PTOF1X2Pamps");
	 fIOMan.Activate("PTOF1X2Pamps0");
         fIOMan.Activate("PTOF1X2delay");
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
        for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
        {
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr10");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr20");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr30");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr40");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr50");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr70");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr80");
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr100");
            
            
            fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKf80");
            
        }
        
    } //endif do ConstFreqRTFTWalk
   

   //  ========== PT  PHONON RTFTWALK RQs Activation  ========== 
   if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum))
    {
         fIOMan.Activate("PTWKf95");
         fIOMan.Activate("PTWKf20");
    }

 
   return;

}

// ================== Calculations ======================
//
// DoCalibration controls looping over events!
//
// ======================================================

void GenRRQDataCDMSliteI::DoCalibration(int maxEvents, int detNum, UserDataManager& myUserData)
{

   //FIXME - pass in the detector type from BatCalib main
   cout <<"Hello from GenRRQDataCDMSliteI::DoCalibration!" << endl;

   // --- 1. Store description of detector for local access ---

   fUserData = myUserData;
   fDetNum = detNum;
   fIsGe = fUserData.IsGe(fDetNum);
   fIsSi = fUserData.IsSi(fDetNum);

  
   // --- 2.  Activate the rqs ---
   
   fCheckOFChargeXRQ = (fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) 
			|| fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge2X2", fDetNum));
   fCheckOFChargeRQ = fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum);

   //retrive the zipTree
   string zipTreeName(Form("zip%d", fDetNum));
   int isValidTree = fIOMan.LoadTree("rqDir", zipTreeName);

   //return to main loop if the tree doesn't exist (as is case for Hybrid running conditions)
   if(isValidTree == 0) return;
   
   // add  eventTree
   fIOMan.AddFriendTree("rqDir", "eventTree");
   
   ActivateRQs();  //fDetType is read from rq file and set in ActivateRQs()

   // add OF resolution (if available)
   fIOMan.FillOFResolution(fDetNum, fDetType, fDelaySig, fAmpSig);
    
    
   // --- 3. Setup the output rrq's ---

   ConstructRRQList();


    // --- 4a. Retrieve calibration constants (outside loop over events to be efficient) ---
    
    fEpsilon = fUserData.GetDoubleParameter(fDetNum, "EPSILON");
    
    fChargeCal = fUserData.GetVectDoubleParameter(fDetNum, "Q_CALIBRATION_OF");
    //fChargeF5Cal = fUserData.GetVectDoubleParameter(fDetNum, "Q_CALIBRATION_F5");
    
    //relative calibration
    fPhononRelCal = fUserData.GetVectDoubleParameter(fDetNum, "P_RELATIVE_CALIBRATION"); 

    //overall calibration
    fPhononOFCal = fUserData.GetDoubleParameter(fDetNum, "PSUM_CALIBRATION_OF");

    fTotPhononOFCal = fUserData.GetDoubleParameter(fDetNum, "PT_CALIBRATION_OF"); ;
    
    fTotPhononNFCal = fUserData.GetDoubleParameter(fDetNum, "PT_CALIBRATION_NF");
    
    // calibration/correction coefficients
    // for HVcurrent, Base temp, 2T resid
    fPhononOFCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PT_CALIBRATION_OF_VECT"); ;
    fPhononNFCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PT_CALIBRATION_NF_VECT"); ;
    fPhonon2TCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PT_CALIBRATION_2T_VECT"); ;
    fPhonon2TaCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PA_CALIBRATION_2T_VECT"); ;
    fPhonon2TbCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PB_CALIBRATION_2T_VECT"); ;
    fPhonon2TcCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PC_CALIBRATION_2T_VECT"); ;
    fPhonon2TdCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PD_CALIBRATION_2T_VECT"); ;
    fPhonon2TarCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PA_CALIBRATION_2Tr_VECT"); ;
    fPhonon2TbrCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PB_CALIBRATION_2Tr_VECT"); ;
    fPhonon2TcrCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PC_CALIBRATION_2Tr_VECT"); ;
    fPhonon2TdrCalVect = fUserData.GetVectDoubleParameter(fDetNum, "PD_CALIBRATION_2Tr_VECT"); ;

    fVnom = fUserData.GetDoubleParameter(fDetNum, "VNOM_HV_CALIBRATION");
    fRb = fUserData.GetDoubleParameter(fDetNum, "RB_HV_CALIBRATION");
    
    fRadPhiCorrVect = fUserData.GetVectDoubleParameter(fDetNum, "RAD_PHI_CORR"); ;

    fPartitionCorrVect = fUserData.GetVectDoubleParameter(fDetNum, "PARTITION_COEFF"); ;
    //fPhononIntCal = fUserData.GetVectDoubleParameter(fDetNum, "PSUM_CALIBRATION_INT");    
    //fPhononTailCal = fUserData.GetDoubleParameter(fDetNum, "P_CALIBRATION_TFP");	
   

   // --- 5. Some checks before performing calculations ----

   if(fPhononRelCal.size() != (uint)BatCalibTypes::kZIPFLIPNPhononChan)
   {
      cout <<"GenRRQDataCDMSliteI::DoCalibration ERROR! "
	   <<"Number of phonon calibration constants does not match number of phonon channels, please check"
	   << endl;
      exit(1);
   }

   // Read database flag
   bool readDatabase = fIOMan.CheckBatRootUserSettingsFlags("READ_DATABASE");
   
   // ----- 6. Loop over all events and do the calculations! -----

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
            
      // ---- mandatory calculations ---- 
 
      // Start with the HV current correction
      double Vnom = fVnom; //([jm] this value is read from config file)
      double Rb = fRb;  //[jm]:  this value is read from the Calib config file
      double Epsilon = fEpsilon;

      double HVnamps = 0.0;
      if(readDatabase)
	HVnamps = fIOMan.Get("HVnamps");

      if( fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum) ||
	  fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum) )	  
      {
	fPhononOFCalCorr =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
      }

      if( fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum) || 
	  fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum) )
      {
	fPhonon2TCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
	// individual channel slow amplitudes
	fPhonon2TaCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TbCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TcCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TdCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
	// individual channel fast amplitudes	
	fPhonon2TarCal =  1/(1. - (HVnamps * 1.0e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TbrCal =  1/(1. - (HVnamps * 1.0e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TcrCal =  1/(1. - (HVnamps * 1.0e-9 * Rb)/(Vnom + Epsilon));
	fPhonon2TdrCal =  1/(1. - (HVnamps * 1.0e-9 * Rb)/(Vnom + Epsilon));

      }

      if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
      {
	fPhononNFCal =  (1)/(1 - (HVnamps * 1e-9 * Rb)/(Vnom + Epsilon));
      }

      // Get Temperature from DB or use default value 
      double baseTemp =  -999999;   //[jm] is -99999 a good init value? Or should we use 0 instead?
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
      }

      // double check temperature
      if (baseTemp==-999999  && 
	  ( fPhononOFCalVect.size()!=1  
	    || fPhononNFCalVect.size() !=1    
	    || fPhononOFCalVect.size() !=1
	    || fPhonon2TCalVect.size() !=1
	    || fPhonon2TaCalVect.size() !=1
	    || fPhonon2TbCalVect.size() !=1
	    || fPhonon2TcCalVect.size() !=1
	    || fPhonon2TdCalVect.size() !=1
	    || fPhonon2TarCalVect.size() !=1
	    || fPhonon2TbrCalVect.size() !=1
	    || fPhonon2TcrCalVect.size() !=1
	    || fPhonon2TdrCalVect.size() !=1 ))
      {
	cout <<"ERROR! GenRRQDataiZIPSoudan::ApplyPhononCalibration: "
	     <<"No temperature available: All the calibration constants should be single numbers."
	     <<"check calibration file or temperature database reading!"
	     << endl;
	exit(1);
      }


      // Calculate Base Temp correction
      // take ptNF/ptOF/pt2TSlow and multiply by correction (1 + C2*(Tbase - <Tbase>))
      // call this C2 coefficient fPhonon**CalVect[1]
      // call <Tbase> fPhonon**CalVect[2]

      if( fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum) ||
	  fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum) )
      {
	fPhononOFCalCorr = fPhononOFCalCorr*(1 + fPhononOFCalVect[1]*(baseTemp - fPhononOFCalVect[2]));
      }

      if( fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum) || 
	  fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum) )
      {
	fPhonon2TCal = fPhonon2TCal*(1 + fPhonon2TCalVect[1]*(baseTemp - fPhonon2TCalVect[2]));
	// individual channel slow amplitudes
	fPhonon2TaCal = fPhonon2TaCal*(1 + fPhonon2TaCalVect[1]*(baseTemp - fPhonon2TaCalVect[2]));
	fPhonon2TbCal = fPhonon2TbCal*(1 + fPhonon2TbCalVect[1]*(baseTemp - fPhonon2TbCalVect[2]));
	fPhonon2TcCal = fPhonon2TcCal*(1 + fPhonon2TcCalVect[1]*(baseTemp - fPhonon2TcCalVect[2]));
	fPhonon2TdCal = fPhonon2TdCal*(1 + fPhonon2TdCalVect[1]*(baseTemp - fPhonon2TdCalVect[2]));
	// individual channel fast (residual) amplitudes
	fPhonon2TarCal = fPhonon2TarCal*(1 + fPhonon2TarCalVect[1]*(baseTemp - fPhonon2TarCalVect[2]));
	fPhonon2TbrCal = fPhonon2TbrCal*(1 + fPhonon2TbrCalVect[1]*(baseTemp - fPhonon2TbrCalVect[2]));
	fPhonon2TcrCal = fPhonon2TcrCal*(1 + fPhonon2TcrCalVect[1]*(baseTemp - fPhonon2TcrCalVect[2]));
	fPhonon2TdrCal = fPhonon2TdrCal*(1 + fPhonon2TdrCalVect[1]*(baseTemp - fPhonon2TdrCalVect[2]));
      }

      if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
      {
	fPhononNFCal = fPhononNFCal*(1 + fPhononNFCalVect[1]*(baseTemp - fPhononNFCalVect[2]));
      }

      // +++++++++
      // Calculate the 2Tresid amplitude correction
      
      // - first calculate the corrected 2T residual amplitude.
      // note that you need to multiply the resdidual amp by the derived HV current
      // and Base temp correction from above
      
      // - second calculate the multiplication factor from the 2T correction
      // take ptNF/ptOF/pt2TSlow and multiply by correction (1 + C3*(2Tresid - <2T_resid>))
      // call this C3 coefficient fPhonon**CalVect[3]
      // call <2T_resid> fPhonon**CalVect[4]

      // - Last, multiply the overall calibration going from amps to keV

      // +++++++++

      if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum))
      {
	if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
	  {
	    // [wap]: previously had to divide by the FFT normalization
	    // CorrOF2Tr = fIOMan.Get("PTOF1X2Ramps") * (fPhonon2TarCalVect[0]) * (fPhononOFCal); 
	    // no longer needed
	    CorrOF2Tr = fIOMan.Get("PTOF1X2Ramps") * (fPhononOFCalCorr); 
	    fPhononOFCalCorr = fPhononOFCalCorr*(1 + fPhononOFCalVect[3]*(CorrOF2Tr - fPhononOFCalVect[4]));
	    fPhononOFCalCorr = fPhononOFCalCorr*(fPhononOFCalVect[0]);
	  }
	if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
	  { 
	    //	    CorrNF2Tr = fIOMan.Get("PTOF1X2Ramps") * (fPhonon2TarCalVect[0]) * (fPhononNFCal); 
	    CorrNF2Tr = fIOMan.Get("PTOF1X2Ramps") * (fPhononNFCal); 
	    fPhononNFCal = fPhononNFCal*(1 + fPhononNFCalVect[3]*(CorrNF2Tr - fPhononNFCalVect[4]));
	    fPhononNFCal = fPhononNFCal*(fPhononNFCalVect[0]);
	  }	
	// this is the total phonon 2T amplitude
	Corr2T2Tr = fIOMan.Get("PTOF1X2Ramps") * (fPhonon2TCal);
	fPhonon2TCal = fPhonon2TCal*(1 + fPhonon2TCalVect[3]*(Corr2T2Tr - fPhonon2TCalVect[4]));
	fPhonon2TCal = fPhonon2TCal*(fPhonon2TCalVect[0]);
      }

      if( fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
      {
	// these are the individual channel 2T slow amp
	// note that the correction of the individual channels
	// are done with the individual channels' residual amplitudes
	Corr2Ta2Tr = fIOMan.Get("PAOF1X2Ramps")  * (fPhonon2TaCal);
	Corr2Tb2Tr = fIOMan.Get("PBOF1X2Ramps")  * (fPhonon2TbCal);
	Corr2Tc2Tr = fIOMan.Get("PCOF1X2Ramps")  * (fPhonon2TcCal);
	Corr2Td2Tr = fIOMan.Get("PDOF1X2Ramps")  * (fPhonon2TdCal);

	// individual channel slow amplitudes
	fPhonon2TaCal = fPhonon2TaCal*(1 + fPhonon2TaCalVect[3]*(Corr2Ta2Tr - fPhonon2TaCalVect[4]));
	fPhonon2TbCal = fPhonon2TbCal*(1 + fPhonon2TbCalVect[3]*(Corr2Tb2Tr - fPhonon2TbCalVect[4]));
	fPhonon2TcCal = fPhonon2TcCal*(1 + fPhonon2TcCalVect[3]*(Corr2Tc2Tr - fPhonon2TcCalVect[4]));
	fPhonon2TdCal = fPhonon2TdCal*(1 + fPhonon2TdCalVect[3]*(Corr2Td2Tr - fPhonon2TdCalVect[4]));
	// [wap] the individual channels should be
	// scaled by the same factor as the total OF.
	// previously was multiplying by (e.g. first column)
	// fPhonon2TaCalVect[0]
	fPhonon2TaCal = fPhonon2TaCal*(fPhonon2TCalVect[0]);
	fPhonon2TbCal = fPhonon2TbCal*(fPhonon2TCalVect[0]);
	fPhonon2TcCal = fPhonon2TcCal*(fPhonon2TCalVect[0]);
	fPhonon2TdCal = fPhonon2TdCal*(fPhonon2TCalVect[0]);
		
      }

      //the order in which these are called matters !

      ApplyPhononCalibration();     //1. apply phonon relative calibration
      
      CalcPhononDelays();           //2. calculate xy delays (needs relative phonon cal)
      
      Calc2TRadialParameter();      //3. use 2T delays to calculate the radial parameter

      ApplyChargeCalibration();     //4. apply charge calibration and position correction (needs phonon delays)
      
      CalcTotalEnergiesAndYields(); //5. calculate qsum and recoil energies
      
      CalcPartitions();             //6. calculate phonon and charge partitions

      if(fIOMan.IsOFresFilled())
      CalcOFResolutions();          //7. calculate optimal filter resolutions


      // ---- optional calculations ----
      
      // find primary channel
      FindPrimaryPhononChannel();  //most optional calculations need this
      
      // ConstFreqRTFTWalk timing
      if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) ||
         fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum))
         	 CalcConstFreqRTFTWalkRRQ(); 
      

      // --- Store some misc items ---
      
      fRRQList["DetType"] = fDetType; //inefficient, but needed for pull teeth right now
      //fRRQList["pprimechanOFWK"] = fPrimaryPhononChan + 1; //1 to offset c++ and matlab conventions 
      //fRRQList["pprimechan"] = fRRQList["pprimechanOFWK"]; 
      
      // ---- Store the data and reset the RRQ list values ----


      fIOMan.FillOutputRRQTree();      
      ResetRRQValues();
 
   } //Done looping over events
 

   // --- 7. Write the tree ---
   
   fIOMan.WriteOutputRRQTree();


   // --- Done! ---

   fIOMan.DeleteActiveTree();

   return;
}

// ==================== the following routines are called from within a loop over events =====================


void GenRRQDataCDMSliteI::ApplyPhononCalibration()
{
  // these are the individual OF channels currently no corrections are made for them
  // instead the individual corrections are made for the 2T individual channels
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {
     //Optimal Filter energy
     fRRQList["paOF"] = fPhononRelCal[0]*fPhononOFCal*fIOMan.Get("PAOFamps");
     fRRQList["pbOF"] = fPhononRelCal[1]*fPhononOFCal*fIOMan.Get("PBOFamps");
     fRRQList["pcOF"] = fPhononRelCal[2]*fPhononOFCal*fIOMan.Get("PCOFamps");
     fRRQList["pdOF"] = fPhononRelCal[3]*fPhononOFCal*fIOMan.Get("PDOFamps");

     fRRQList["paOF0"] = fPhononRelCal[0]*fPhononOFCal*fIOMan.Get("PAOFamps0");
     fRRQList["pbOF0"] = fPhononRelCal[1]*fPhononOFCal*fIOMan.Get("PBOFamps0");
     fRRQList["pcOF0"] = fPhononRelCal[2]*fPhononOFCal*fIOMan.Get("PCOFamps0");
     fRRQList["pdOF0"] = fPhononRelCal[3]*fPhononOFCal*fIOMan.Get("PDOFamps0");


   }
   //
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
   {
     //Optimal Filter 2T energy
     fRRQList["paOF1X2P"] = fPhononRelCal[0]*fPhonon2TaCal*fIOMan.Get("PAOF1X2Pamps");
     fRRQList["pbOF1X2P"] = fPhononRelCal[1]*fPhonon2TbCal*fIOMan.Get("PBOF1X2Pamps");
     fRRQList["pcOF1X2P"] = fPhononRelCal[2]*fPhonon2TcCal*fIOMan.Get("PCOF1X2Pamps");
     fRRQList["pdOF1X2P"] = fPhononRelCal[3]*fPhonon2TdCal*fIOMan.Get("PDOF1X2Pamps");

     fRRQList["paOF1X2P0"] = fPhononRelCal[0]*fPhonon2TaCal*fIOMan.Get("PAOF1X2Pamps0");
     fRRQList["pbOF1X2P0"] = fPhononRelCal[1]*fPhonon2TbCal*fIOMan.Get("PBOF1X2Pamps0");
     fRRQList["pcOF1X2P0"] = fPhononRelCal[2]*fPhonon2TcCal*fIOMan.Get("PCOF1X2Pamps0");
     fRRQList["pdOF1X2P0"] = fPhononRelCal[3]*fPhonon2TdCal*fIOMan.Get("PDOF1X2Pamps0");
   }

   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
   {
     //Optimal Filter 2T energy

     fRRQList["paOF1X2R"] = fPhononRelCal[0]*fPhonon2TarCal*fIOMan.Get("PAOF1X2Ramps");
     fRRQList["pbOF1X2R"] = fPhononRelCal[1]*fPhonon2TbrCal*fIOMan.Get("PBOF1X2Ramps");
     fRRQList["pcOF1X2R"] = fPhononRelCal[2]*fPhonon2TcrCal*fIOMan.Get("PCOF1X2Ramps");
     fRRQList["pdOF1X2R"] = fPhononRelCal[3]*fPhonon2TdrCal*fIOMan.Get("PDOF1X2Ramps");

     fRRQList["paOF1X2R0"] = fPhononRelCal[0]*fPhonon2TarCal*fIOMan.Get("PAOF1X2Ramps0");
     fRRQList["pbOF1X2R0"] = fPhononRelCal[1]*fPhonon2TbrCal*fIOMan.Get("PBOF1X2Ramps0");
     fRRQList["pcOF1X2R0"] = fPhononRelCal[2]*fPhonon2TcrCal*fIOMan.Get("PCOF1X2Ramps0");
     fRRQList["pdOF1X2R0"] = fPhononRelCal[3]*fPhonon2TdrCal*fIOMan.Get("PDOF1X2Ramps0");

   }

  // --- PT OF ---
   if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon", fDetNum))
  {
    fRRQList["ptOF"] = fPhononOFCalVect[0]*fIOMan.Get("PTOFamps"); 	
    fRRQList["ptOF0"] = fPhononOFCalVect[0]*fIOMan.Get("PTOFamps0");   
    // [wap] these are the calibrabrated but *uncorrected* energy estimators
    fRRQList["ptOFuc"] = fPhononOFCalVect[0]*fIOMan.Get("PTOFamps"); 
    fRRQList["ptOF0uc"] = fPhononOFCalVect[0]*fIOMan.Get("PTOFamps0");
    // [wap] these are the calibrabrated and *corrected* energy estimators
    fRRQList["ptOFc"] = fPhononOFCalCorr*fIOMan.Get("PTOFamps"); 
    fRRQList["ptOF0c"] = fPhononOFCalCorr*fIOMan.Get("PTOFamps0");
  }
  // --- PT NF ---
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhononNS", fDetNum))
  {
    fRRQList["ptNF"] = fPhononNFCalVect[0]*fIOMan.Get("PTNFamps"); 
    fRRQList["ptNF0"] = fPhononNFCalVect[0]*fIOMan.Get("PTNFamps0");
    // [wap] these are the calibrabrated but *uncorrected* energy estimators
    fRRQList["ptNFuc"] = fPhononNFCalVect[0]*fIOMan.Get("PTNFamps"); 
    fRRQList["ptNF0uc"] = fPhononNFCalVect[0]*fIOMan.Get("PTNFamps0");
    // [wap] these are the calibrabrated and *corrected* energy estimators
    fRRQList["ptNFc"] = fPhononNFCal*fIOMan.Get("PTNFamps"); 
    fRRQList["ptNF0c"] = fPhononNFCal*fIOMan.Get("PTNFamps0");
}
  // --- PT 2T ---
  if(fIOMan.CheckBatRootPhononAlg("PT_OptimalFilterPhonon1X2", fDetNum))
  {
    fRRQList["ptOF1X2P"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Pamps"); 
    fRRQList["ptOF1X2P0"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Pamps0");
    fRRQList["ptOF1X2R"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Ramps"); 
    fRRQList["ptOF1X2R0"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Ramps0");
    // [wap] these are the calibrabrated but *uncorrected* energy estimators
    fRRQList["ptOF1X2Puc"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Pamps"); 
    fRRQList["ptOF1X2P0uc"] = fPhonon2TCalVect[0]*fIOMan.Get("PTOF1X2Pamps0");
    // [wap] these are the calibrabrated and *corrected* energy estimators
    fRRQList["ptOF1X2Pc"] = fPhonon2TCal*fIOMan.Get("PTOF1X2Pamps"); 
    fRRQList["ptOF1X2P0c"] = fPhonon2TCal*fIOMan.Get("PTOF1X2Pamps0");
    fRRQList["ptOF1X2Rc"] = fPhonon2TCal*fIOMan.Get("PTOF1X2Ramps"); 
    fRRQList["ptOF1X2R0c"] = fPhonon2TCal*fIOMan.Get("PTOF1X2Ramps0");
  }

   return;
}

// Calculates the x and y Phonon delays using WK quantities.
//The radial Phonon delay (prdelWK) needs the primary channel RRQ and is calculated later in the code
void GenRRQDataCDMSliteI::CalcPhononDelays()
{
   double pxdel = -999999;
   double pydel = -999999;

   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {   
     pxdel = -(fIOMan.Get("PBWKr20")*cos(kThetaVect[0]) + fIOMan.Get("PCWKr20")*cos(kThetaVect[1]) 
	       + fIOMan.Get("PDWKr20")*cos(kThetaVect[2]))*1e6;
   
     pydel = -(fIOMan.Get("PBWKr20")*sin(kThetaVect[0]) + fIOMan.Get("PCWKr20")*sin(kThetaVect[1])
	       + fIOMan.Get("PDWKr20")*sin(kThetaVect[2]))*1e6;
      
     //  ===== Store the values =====
     
     fRRQList["pxdelWK"] = pxdel;
     fRRQList["pydelWK"] = pydel;
    

   }

   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon1X2", fDetNum))
     {
       // fPartitionCorrVect[0] is the calibration to bring corrected ptNF to keVee
       // fPartitionCorrVect[1] is the coefficient that balances these calibrations for different detector voltages
       // given that this framework was first developed on and a custom application for T5Z2 at 70V.
       // fPhonon2TarCalVect[0] is the above overall scale factor to the individual channel
       // residual amplitudes
       // fPhononRelCal[0] is the PA relative calibration. dividing each channel by this factor
       // brings the relative channel coefficients back into the convention that the matlab code
       // uses
       pa2t_delr = fIOMan.Get("PAOF1X2delay") - fIOMan.Get("PTOF1X2delay") - (fRRQList["paOF1X2R"])/(fRRQList["ptNFc"]*fPartitionCorrVect[0]*fPartitionCorrVect[1])/fPhononRelCal[0]/fPhonon2TarCalVect[0];
       pb2t_delr = fIOMan.Get("PBOF1X2delay") - fIOMan.Get("PTOF1X2delay") - (fRRQList["pbOF1X2R"])/(fRRQList["ptNFc"]*fPartitionCorrVect[0]*fPartitionCorrVect[1])/fPhononRelCal[0]/fPhonon2TbrCalVect[0];
       pc2t_delr = fIOMan.Get("PCOF1X2delay") - fIOMan.Get("PTOF1X2delay") - (fRRQList["pcOF1X2R"])/(fRRQList["ptNFc"]*fPartitionCorrVect[0]*fPartitionCorrVect[1])/fPhononRelCal[0]/fPhonon2TcrCalVect[0];
       pd2t_delr = fIOMan.Get("PDOF1X2delay") - fIOMan.Get("PTOF1X2delay") - (fRRQList["pdOF1X2R"])/(fRRQList["ptNFc"]*fPartitionCorrVect[0]*fPartitionCorrVect[1])/fPhononRelCal[0]/fPhonon2TdrCalVect[0];

       px2t_delr = (pb2t_delr - pd2t_delr)*cos(30./180. * TMath::Pi());//[wap]: 30deg angle 
       py2t_delr = pc2t_delr - (pb2t_delr + pd2t_delr)/2;

       // [wap]: individual channel delays
       // not critical
       /*
       fRRQList["pa2tdelr"] = pa2t_delr;
       fRRQList["pb2tdelr"] = pb2t_delr;
       fRRQList["pc2tdelr"] = pc2t_delr;
       fRRQList["pd2tdelr"] = pd2t_delr;
       */
       fRRQList["pxpartOF1X2"] = px2t_delr;
       fRRQList["pypartOF1X2"] = py2t_delr;
     }


   return;
} 

void GenRRQDataCDMSliteI::Calc2TRadialParameter()
{

  int signPx = Sign(fRRQList["pxpartOF1X2"]);
  double phi = (signPx + 1.0)/2.0*TMath::Pi() + TMath::ATan(fRRQList["pypartOF1X2"]/fRRQList["pxpartOF1X2"]) - TMath::Pi()/2.;
  //calculate minimum of interior channel delays
  double minBC = std::min(pb2t_delr,pc2t_delr);
  double minBCD = std::min(minBC,pd2t_delr);

  double pr2tdelr = minBCD - pa2t_delr;

  fRRQList["prpartOF1X2noPhiCorr"] = pr2tdelr;

  double pr2tdelrc = pr2tdelr - fRadPhiCorrVect[0] + (fRadPhiCorrVect[1]*TMath::Exp((-1.)*TMath::Abs(phi)/0.3) + TMath::Abs(TMath::Abs(1-TMath::Abs(phi/TMath::Pi()) - 1./3) - 1./3)*fRadPhiCorrVect[2]);


  fRRQList["prpartOF1X2"] = pr2tdelrc;

}

int GenRRQDataCDMSliteI::Sign(double x)
{
  if (x > 0) return 1;
  if (x < 0) return -1;
  return 0;
}

void GenRRQDataCDMSliteI::ApplyChargeCalibration()
{

   // can't do this if no charge calibration
   if(!fCheckOFChargeXRQ & !fCheckOFChargeRQ)
   {
     return;
   }

   //to hold calibration constants
   double qia = fChargeCal[0];
   double qoa = fChargeCal[1];
   //double qix = fChargeCal[2];
   //double qox = fChargeCal[3];

   double qi = -999999;
   double qo = -999999;
   double qi0 = -999999;
   double qo0 = -999999;

 
   //2.  Check for saturation (either QI or QO)  
   int isSat = ( (fIOMan.Get("QIsat")> 0 || fIOMan.Get("QOsat")>0) ? 1 : 0);


   if(isSat != 1) 
   {
      // ==== No Cross-talk ====
      
      qo0 = qoa*fIOMan.Get("QOOFvolts0");
      qi0 = qia*fIOMan.Get("QIOFvolts0");
  
      qo = qoa*fIOMan.Get("QOOFvolts");
      qi = qia*fIOMan.Get("QIOFvolts");
    
  
   } //end if not saturated

  

   // ===== Store the values now =====

   fRRQList["qiOF"] = qi;
   fRRQList["qoOF"] = qo;

   fRRQList["qiOF0"] = qi0;
   fRRQList["qoOF0"] = qo0;

   return;
}

void GenRRQDataCDMSliteI::CalcTotalEnergiesAndYields()
{

   /*//get qi/qo bias - they are used for all the detector below
   double qiBias = -999999;
   double qoBias = -999999;

   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      qiBias = fIOMan.Get("QIbias"); 
      qoBias = fIOMan.Get("QObias");
   }
   else
   {
      qiBias = fUserData.GetDoubleParameter(fDetNum, "QIBIAS");
      qiBias = fUserData.GetDoubleParameter(fDetNum, "QOBIAS");
   }

   //double qifac = fabs(qiBias/fEpsilon); //should always be positive?
   */


   // === total ionization energy ===
   int isChargeOn = 0;
   if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
     {
       isChargeOn = 1;
       fRRQList["qsumOF"] = fRRQList["qiOF"] + fRRQList["qoOF"];
     }
   

   // === optimal filter quantities ===
   
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {

     // total phonon energy
   
     double psum = fRRQList["paOF"] + fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"];
     fRRQList["psumOF"] = psum;
     fRRQList["psumiOF"] = fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]; 
     fRRQList["psumoOF"] = fRRQList["paOF"]; 
      
     // recoil energies
   
     /*  ------------ NOT DONE  ------------

     fRRQList["precoilgOF"] = psum/(1.0+qifac); 


     if(isChargeOn)
     {
       fRRQList["precoilsumOF"] = psum - (fRRQList["qsumOF"]*qifac);
       fRRQList["precoiliOF"] = psum - (fRRQList["qiOF"]*qifac); 
     }      

     // yields
     
     if(isChargeOn)
     {
       fRRQList["ysumOF"] = fRRQList["qsumOF"]/fRRQList["precoilsumOF"];
       fRRQList["ygsumOF"] = fRRQList["qsumOF"]/fRRQList["precoilgOF"];
       fRRQList["ysumiOF"] = fRRQList["qiOF"]/fRRQList["precoiliOF"];
       fRRQList["ygsumiOF"] = fRRQList["qiOF"]/fRRQList["precoilgOF"];

     }
   
 
     */
 

   }

/*
   // === integral quantities ===

   if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
   {
     // total phonon energy
   
     double psumINT = fRRQList["paINT"] + fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"];
     fRRQList["psumINT"] = psumINT;
     fRRQList["psumiINT"] = fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"]; 
     fRRQList["psumoINT"] = fRRQList["paINT"]; 	 
     
     
     // recoil energies
     
     fRRQList["precoilgsumINT"] = psumINT/(1.0+qifac); 

     if(isChargeOn)
     {
       fRRQList["precoilsumINT"] = psumINT - (fRRQList["qsumOF"]*qifac);
       fRRQList["precoilsumiINT"] = psumINT - (fRRQList["qiOF"]*qifac); 
     }     

     // yields

     if(isChargeOn)
     {
       fRRQList["ysumINT"] = fRRQList["qsumOF"]/fRRQList["precoilsumINT"];
       fRRQList["ygsumINT"] = fRRQList["qsumOF"]/fRRQList["precoilgsumINT"];
       fRRQList["ysumiINT"] = fRRQList["qiOF"]/fRRQList["precoilsumiINT"];
       fRRQList["ygsumiINT"] = fRRQList["qiOF"]/fRRQList["precoilgsumINT"];
     }
     
   }
    
       // ========== Phonon Tail Fit ==========
    if(fIOMan.CheckBatRootPhononAlg("TailFitPhonon", fDetNum))
    {
        fRRQList["psumTFP"] = fRRQList["paTFP"] + fRRQList["pbTFP"] + fRRQList["pcTFP"] + fRRQList["pdTFP"];
        
    }
*/  

   return;
}


//mostly just for convenience
void GenRRQDataCDMSliteI::CalcPartitions()
{

  // === Charge partitions ===
  
  if(fCheckOFChargeXRQ || fCheckOFChargeRQ)
     fRRQList["qrpartOF"] =  fRRQList["qoOF"]/fRRQList["qsumOF"];
   
  

  // === optimal filter quantities ===

  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum) )
  {
    // partition for mercedes (triangle plots)
    fRRQList["pxpartOF"] = (fRRQList["pbOF"]*cos(kThetaVect[0]) + fRRQList["pcOF"]*cos(kThetaVect[1]) + 
			    fRRQList["pdOF"]*cos(kThetaVect[2])) 
                         / (fRRQList["psumiOF"]);
  
    fRRQList["pypartOF"] = (fRRQList["pbOF"]*sin(kThetaVect[0]) + fRRQList["pcOF"]*sin(kThetaVect[1]) + 
			    fRRQList["pdOF"]*sin(kThetaVect[2])) 
                         / (fRRQList["psumiOF"]);
  
    fRRQList["prxypartOF"] = sqrt(fRRQList["pxpartOF"]*fRRQList["pxpartOF"] + 
				  fRRQList["pypartOF"]*fRRQList["pypartOF"]);

    // inner/outer partition
    fRRQList["prpartOF"] = fRRQList["paOF"]/ fRRQList["psumOF"];
    
  }
                            
  return;
}
     
//calibrate the OF resolution quantities
void GenRRQDataCDMSliteI::CalcOFResolutions()
{
/*
   // === calibrate the OFdelay and OFamplitude resolutions ===
   //      note values stored in order QI, QO, PA, PB, ...

   double tempQIMax = fOFMaxTemplate[0];
   double tempQOMax = fOFMaxTemplate[1];

   fRRQList["qidelayres"] = fChargeCal[0]*tempQIMax*fDelaySig[0];
   fRRQList["qodelayres"] = fChargeCal[1]*tempQOMax*fDelaySig[1];
      
   fRRQList["qiampres"] = fChargeCal[0]*tempQIMax*fAmpSig[0];
   fRRQList["qoampres"] = fChargeCal[1]*tempQOMax*fAmpSig[1];
*/
   // loop over phonon channels and calculae for each
   for(int chanItr=0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
      string prefix = BatCalibTypes::kZIPFLIPPhononChan[chanItr];
      string prefixCal = BatCalibTypes::kZIPFLIPPhononCal[chanItr];

      fRRQList[prefixCal+"delayres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononRelCal[chanItr]*fPhononOFCal*fDelaySig[chanItr+2];  
      fRRQList[prefixCal+"ampres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononRelCal[chanItr]*fPhononOFCal*fAmpSig[chanItr+2]; 

   }

   return;
}

// ==============================  Optional RRQ calculations ==============================================

//Assume calculation is done w/ const freq rtftwalk, rrq name should reflect that
//FIXME this needs some revision to incorporate flexibility of OF/INT 
void GenRRQDataCDMSliteI::FindPrimaryPhononChannel()
    {
        
        // max OF amplitdude
        
        if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
        {
            
            //loop over channels to find max amplitude
            double maxAmp = -1.0*numeric_limits<double>::infinity();
            double maxAmpi = -1.0*numeric_limits<double>::infinity();
          
            
            for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
            {
                string calibChanName = BatCalibTypes::kZIPFLIPPhononCal[chanItr];
                string chanName      = BatCalibTypes::kZIPFLIPPhononChan[chanItr];
                double chanValOF     = fRRQList[calibChanName + "OF"];
                
                if (chanValOF>maxAmp) {
                    maxAmp = chanValOF;
                    fRRQList["pprimechanOF"] = chanItr+1;
                }
                
                if (chanValOF>maxAmpi && calibChanName.find("a")==string::npos) {
                    maxAmpi = chanValOF;
                    fRRQList["pprimechaniOF"] = chanItr+1;
                }
                
            }
        }
        
        
        // min delay
        
        if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
        {
            
            double minDel = 1.0*numeric_limits<double>::infinity();
            
            
            for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
            {
                string chanName      = BatCalibTypes::kZIPFLIPPhononChan[chanItr];
                double chanValWK     = fIOMan.Get(chanName + "WKr20");
                
                if (chanValWK<minDel && chanValWK != 0)
                {
                    minDel = chanValWK;
                    fRRQList["pprimechanWK"] = chanItr+1;
                }
                
            } //end loop over channels
            
        } //end if ConstFreqRTFTWalkPhonon is used
        
        
        // combined OF amplitude and delay [ANV]/[LLH]
        // this must come AFTER the above two calculations
        
        if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum) && fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon",fDetNum))
        {   
            if (fRRQList["pprimechanOF"] == 1)
            {
                fRRQList["pprimechanOFWK"] = fRRQList["pprimechanWK"]; //use min delay
            }
            else{
                fRRQList["pprimechanOFWK"] = fRRQList["pprimechanOF"]; //use max OF
            }
            
        } //end if both OF and ConstFreqRTFTWalk used
        
        return;
    }
     
void GenRRQDataCDMSliteI::CalcConstFreqRTFTWalkRRQ()
{


   // Total phonon RRQs...
   if(fIOMan.CheckBatRootPhononAlg("PT_ConstFreqRTFTWalkPhonon", fDetNum)){

     fRRQList["ptftWK_9520"] = (fIOMan.Get("PTWKf20") - fIOMan.Get("PTWKf95"))*1e6; //in microseconds
  
   }

    // Primary sensor RRQs...

    //If WK rq's don't exist, then skip the rest
    if(!fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
    {
        return;
    }

    //primary channel number with A = 1, B= 2 etc... -1 is took go back to cpp convention of array indices starting at 0 instead of 1 in matlab
    
    //fPrimaryPhononChanOF = fRRQList["pprimechanOF"] - 1;
    //string primChanNameOF = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChanOF];

    //inner primary channel number with A = 1, B= 2 etc...
    fPrimaryInnerPhononChanOF = fRRQList["pprimechaniOF"] - 1;
    fPrimaryPhononChanWK = fRRQList["pprimechanWK"] - 1;
    fPrimaryPhononChanOFWK = fRRQList["pprimechanOFWK"] - 1;

    //Check if the primary channels are not 1,2,3,4 as it should. Let the RRQs be the initial -999999 if this is the case and calculate them otherwise
    if(fPrimaryInnerPhononChanOF<1 || fPrimaryInnerPhononChanOF>4){}
    	else
    	{
    		string primiChanNameOF = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChanOF];
        	fRRQList["prdelWK"] = (fIOMan.Get(primiChanNameOF+"WKr20") - fIOMan.Get("PAWKr20"))*1e6;
    	}
    
    if(fPrimaryPhononChanWK < 1 || fPrimaryPhononChanWK > 4){}
    	else
    	{
    		string primChanNameWK = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChanWK];
		fRRQList["pminrtWK_1040"] = (fIOMan.Get(primChanNameWK+"WKr40") - fIOMan.Get(primChanNameWK+"WKr10"))*1e6; //in microseconds
   		fRRQList["pminrtWK_1070"] = (fIOMan.Get(primChanNameWK+"WKr70") - fIOMan.Get(primChanNameWK+"WKr10"))*1e6; //in microseconds
   		fRRQList["pminrtWK_10100"] = (fIOMan.Get(primChanNameWK+"WKr100") - fIOMan.Get(primChanNameWK+"WKr10"))*1e6; //in microseconds
    	}
    
    if(fPrimaryPhononChanOFWK<1 || fPrimaryPhononChanOFWK>4){}
    	else
    	{
        	string primChanNameOFWK = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChanOFWK];
		fRRQList["pminrtOFWK_1040"] = (fIOMan.Get(primChanNameOFWK+"WKr40") - fIOMan.Get(primChanNameOFWK+"WKr10"))*1e6; //in microseconds
		fRRQList["pminrtOFWK_1070"] = (fIOMan.Get(primChanNameOFWK+"WKr70") - fIOMan.Get(primChanNameOFWK+"WKr10"))*1e6; //in microseconds
		fRRQList["pminrtOFWK_10100"] = (fIOMan.Get(primChanNameOFWK+"WKr100") - fIOMan.Get(primChanNameOFWK+"WKr10"))*1e6; //in microseconds

    	}

   return;
}





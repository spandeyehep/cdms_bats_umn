///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: PulseData
//Author: L. Hsu, M. Kos, B. Serfass
//Description:  This is a container class which stores data from the Pulse record in the raw data file.
//In particular, this class stores the pulse from a single channel.  The pulse data are filled directly by this class, 
//which knows how to parse the raw record.  This class also stores modifications of the raw pulse and allows
//the user to retrieve them for later analysis.  Finally, this class stores instances of the analysis
//routines that are performed on this pulse. These are stored in the vector of TCDMSAnalyses for later
//retrieval of the RQ values.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
// Update Dec 7, 2012 by B. Loer (bloer@fnal.gov):
//  Read in timing information (dt and t0) from the raw data record
//  @todo Standardize between this location and DetectorConfigManager
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PULSEDATA_H
#define PULSEDATA_H

#include "zlib.h"
#include <vector>
#include <map>
#include "stdint.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This class reads the raw pulse record and stores all the info that is pertinent to a single raw pulse (see header file for more info).
class PulseData 
{

   //the only classes in BatRoot that can modify members of this one
   friend class EventBuilder; 
   friend class NoiseBuilder; //only used for noise file generation
   friend class PulseEvtBuilder; // class for making ROOT input files for pulse simulation (similar to EventBuilder) [AJA]

   public:
      PulseData();  //constructor
  
      ~PulseData(); //destructor

      void Reset(); //clears data members for next filling

      //Get pulse info (all const functions) - note GetBaselineSubPulse and GetBaselineSubNormPulse are being filled for veto pulses
      const vector<double>& GetRawPulse() const             { return fPulseVector; }            
      const vector<double>& GetBaselineSubPulse()     const { return fBSPulseVector; } //baseline subtracted            
      const vector<double>& GetBaselineSubNormPulse() const { return fBSNPulseVector; } //baseline subtracted AND normalized (only different from above when norm != 1)           
      const vector<double>& GetTestPulse() const            { return fTestPulseVector; }            

      bool IsPhononPulse()      const  { return fIsPhonon; }
      bool IsChargePulse()      const  { return fIsCharge; }
      bool IsVetoPulse()        const  { return fIsVeto;   }
      bool IsZipPulse()         const  { return fIsZip;    }
      bool IsNoiseMonitorPulse()    const  { return fIsNoiseMonitor;    }
      bool IsOther()            const  { return fIsOther;    }
      string  GetChannelType()  const; //returns "phonon", "charge" or "veto"

      int  GetDetectorCode()    const  { return fDetCode;    }
      int  GetDetectorChannel() const  { return fDetChannel; }
      int  GetDetectorNum()     const  { return fDetNum;     }
      int  GetDetectorType()    const  { return fDetType;    }
      int  GetNADCBins()        const  { return fNADCBins;   }
      uint32_t GetSampleDt()    const  { return fSampleDt;   }
      int32_t  GetTriggerT0()   const  { return fTriggerT0;  }
      
      string  GetChannelName()  const;


      //For raw data reading
      void ReadRawPulseRecord(gzFile& localRawDataPtr, uint32_t recordLength, uint32_t recordID, bool dispflag);
      void ReadRawPulseBuffer(uint32_t* buffer, uint32_t recordLength, uint32_t recordID, bool dispflag);
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }


      // Fill pulse data externally (only if empty, no modification allowed through this function)
      void SetRawPulseRecord(uint32_t detCode, const vector<double>& rawPulse, uint32_t sampleDt, int32_t triggerT0); 


      // Allow modification of PulseData - these should be used with great caution!
      // These were implemented for raw data kludges, such as for CDMSliteRun1 and
      // DCRC revC readout of 100 mm iZIP.
      // These fuctions should only be used inside RawDataReader:ModifyRawData()

      void SetChannelConfig(uint32_t detCode);  // set detector code and all related parameters (channel name, type)
      void FlipADCRawPulse();                   // flip adc bin values in pairs, a fix for the DCRC revC readout
    

      //public methods for querying RQLists - provides read only access!
      TCDMSAnalysis GetPulseAnalysis(const string& analysisName);  
      bool HasPulseAnalysis(const string& analysisName);  
      vector<TCDMSAnalysis> GetAnalysisCollection()    { return fAnalysisCollection; } //could be slow?
      

   private:

      bool fFlipBytes; //initialized to false

      // === From the Raw Data Files ===
      
      //Identifying info
      uint32_t fDetCode;    //x in xyyz (see raw data formats), 8/9 indicates sum of pulses on Q/P, otherwise follow raw data convention
      int fDetChannel;      
      int fDetNum;
      int fDetType;         //1=BLIP, 2=FLIP, 3=VETO, 4=ZIP, 5=mercedesZIP, 6=endcap 
      uint32_t fNADCBins;
      uint32_t fSampleDt;   ///< Time between samples, in ns
      int32_t fTriggerT0;   ///< Trigger time, in ns

      bool fIsPhonon;
      bool fIsCharge;
      bool fIsVeto;
      bool fIsZip;
      bool fIsNoiseMonitor;
      bool fIsOther;        //neither zip or veto or noise monitor pulse 

      string fChannelName;  //zip channel names assigned by raw data formats.  Veto traces are "V" but aren't used anywhere

      // === Calculated Quantities ===
      
      //the pulse
      vector<double> fPulseVector;         //original
      vector<double> fBSPulseVector;       //baseline subtracted (BS)
      vector<double> fBSNPulseVector;      //baseline subtracted AND normalized (BSN) - when norm = 1, this is the same as fBSPulseVector
      vector<double> fTestPulseVector;     //for debugging

      //the analysis objects
      vector<TCDMSAnalysis> fAnalysisCollection;

      //add associated function to get analysis of certain type - or just query for rq??  
      void StorePulseAnalysis(TCDMSAnalysis& analysisClass);

};

#endif /* PULSEDATA_H */

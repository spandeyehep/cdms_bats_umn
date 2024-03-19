///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: RawDataReader
//Author: L. Hsu
//Description:  This class manages the navigation through the raw data file.   It provides wrapper functions
//(of zlib routines) that allow the user to open, close, rewind and step through the various records of the 
//raw data file.  It manages the filling of the various data storage classes and hands them off to the rest 
//of the processing routine for analysis.   Note, some snippets were originally ported from DarkPipe and 
//PipeFitter raw data reading routines.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//Nov. 22, 2010: Adding DetectorConfigData
//May, 2013:  Adding ModifyRawData (B. Serfass)
//Dec, 2013:  Adding Midas data reading (B. Serfass)
//Jan. 2018:  Adding UMN5Q_R65 mappings
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <string>
#include "zlib.h"
#include <regex.h>

#include "RawDataReader.h"
#include "ChannelMapHelper.h"


using namespace std;

////////////////////////////////////////////////////////

//default constructor
RawDataReader::RawDataReader() :
   fdiagnosticPrints(false),
   fverbosity(0),
   fReadDetectorConfig(false),
   fReadAdmin(false),
   fReadHistory(false),
   fReadTrigger(false),
   fReadGPS(false),
   fReadZipPulses(false),
   fReadVetoPulses(false),
   fReadNoiseMonitorPulses(false),
   fIsZipPulsesModified(false),	
   fgzRawDataPtr(NULL),
   fByteCheckDone(false),
   fFlipBytes(false),
   fCurrentEventPosition(0),
   fNextEventPosition(0),
   fEventLength(0),
   fCurrentRecordPosition(0),
   fNextRecordHeaderPosition(0),
   fEventCategory(0xffff),
   fEventType(0xffff),
   fIsMidasData(false),
   fIsMidasOnline(false),
   fNbMidasEventTriggers(0),
   fCurrentMidasEventTrigger(0),
   fMidasEventNumber(0),
   fCurrentEventNumber(0),
   fMidasSeriesNumber(0),
   fgSystem(NULL)
{
  //cout <<"Constructing RawDataReader()" << endl;
}

RawDataReader::~RawDataReader()
{
   //cout <<"Goodbye from RawDataReader()" << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////


void RawDataReader::RegisterDetectorConfigData(DetectorConfigData* dataPtr)
{
   fReadDetectorConfig = true;
   fDetectorConfigPtr = dataPtr;

   return;
}

void RawDataReader::RegisterAdminData(AdminData* dataPtr)
{
   fReadAdmin = true;
   fAdminPtr = dataPtr;
   fAdminPtr->ConstructRQList();

   return;
}

void RawDataReader::RegisterHistoryData(HistoryData* dataPtr, const int nTowers)
{
   fReadHistory = true;
   fHistoryPtr = dataPtr;
   fHistoryPtr->ConstructRQList(nTowers);

   return;
}

void RawDataReader::RegisterTriggerData(TriggerData* dataPtr, const int nTowers)
{
   fReadTrigger = true;
   fTriggerPtr = dataPtr;
   fTriggerPtr->ConstructRQList(nTowers);

   return;
}

void RawDataReader::RegisterGPSData(GPSData* dataPtr)
{
   fReadGPS = true;
   fGPSPtr = dataPtr;

   return;
}

void RawDataReader::RegisterZipPulseMap(map< int, vector<PulseData> >* mapOfZipPulsesPtr)
{
   fReadZipPulses = true;
   fMapOfZipPulsesPtr = mapOfZipPulsesPtr;

   return;
}

void RawDataReader::RegisterVetoPulseVector(vector<PulseData>* vectorOfVetoPulsesPtr)
{
   fReadVetoPulses = true;
   fListOfVetoPulsesPtr = vectorOfVetoPulsesPtr;
   //PulseData class automatically constructs RQ list

   return;
}

void RawDataReader::RegisterNoiseMonitorPulseVector(vector<PulseData>* vectorOfNoiseMonitorPulsesPtr)
{
   fReadNoiseMonitorPulses = true;
   fListOfNoiseMonitorPulsesPtr = vectorOfNoiseMonitorPulsesPtr;
   //PulseData class automatically constructs RQ list

   return;
}


void RawDataReader::RegisterGUITSystem(TSystem* gSystem){
  fgSystem = gSystem;
  return;
}


#ifdef HAVE_MIDAS
void RawDataReader::RegisterMidasInstance(TMidasControl* midas) 
{
  fIsMidasData = true;
  fIsMidasOnline = true;
  fMidas = midas;
  fODB = midas;
  return;
}

void RawDataReader::RegisterMidasInstanceOffline(TMidasControl* midas) 
{
  fIsMidasData = true;
  fIsMidasOnline = false;
  fMidas = midas;
  fODB = midas;
  return;
}

#endif


//////////////////////////////////////////////////////////////////////////////////////////////

void RawDataReader::OpenRawDataFile(const string& inputRawDataDir, const string& inputRawDataFile)
{  
  
   // check if online data reading (Midas)
   if (fIsMidasOnline) return;
  
   // Raw data full path
   fRawDataPath = inputRawDataDir+inputRawDataFile;

   //parse out the series number and dump from filename (or attempt to)
   fGotSeriesInfoFromFilename = ParseSeriesAndDumpFromRawPath();

   //first attempt to open assuming file has a .gz extension (gzipped file)
   fgzRawDataPtr = gzopen((fRawDataPath+".gz").c_str(), "rb");   
   
   //if failing to open with .gz, then try to open without .gz extension
   if (fgzRawDataPtr == NULL) 
   {
      fgzRawDataPtr = gzopen(fRawDataPath.c_str(), "rb");   

      //report the file that was opened
      if(fgzRawDataPtr != NULL)  
      {
	 cout <<"Opened raw file: " <<fRawDataPath << endl;
	 return;
      }
   }
   
   //if it still fails then return an error
   if(fgzRawDataPtr == NULL) {
      cerr <<"RawDataReader::ERROR opening file " << fRawDataPath << endl;
      exit(1);
   }

   //if you make it to here, report the .gz file that was opened
   cout <<"\nOpened raw file: " <<fRawDataPath+".gz" << endl;

   return;
   
}

void RawDataReader::CloseRawDataFile()
{  

   gzclose(fgzRawDataPtr);   
   
   //Reset RawDataReader placeholders
   fCurrentEventPosition = 0;
   fNextEventPosition = 0;
   fEventLength = 0;
   fCurrentRecordPosition = 0;
   fNextRecordHeaderPosition = 0;
   
   // reset midas parameters
   fNbMidasEventTriggers = 0;
   fCurrentMidasEventTrigger = 0;
   fMidasEventNumber = 0;
   fCurrentEventNumber = 0;
   fMidasEvent.Clear();
   
   return;
}


void RawDataReader::ResetRawDataFile()
{  

   //Go back to the beginning of the file
   gzrewind(fgzRawDataPtr);   
  
   //Reset RawDataReader placeholders
   fCurrentEventPosition = 0;
   fNextEventPosition = 0;
   fEventLength = 0;
   fCurrentRecordPosition = 0;
   fNextRecordHeaderPosition = 0;
   
   // Reset Midas parameters
   fNbMidasEventTriggers = 0;
   fCurrentMidasEventTrigger = 0;
   fMidasEventNumber = 0;
   fCurrentEventNumber = 0;
   fMidasEvent.Clear();
   
   return;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//the endian check isn't stored in all file types
//use event header word to check endianness in addition to here
void RawDataReader::ReadFileHeader(bool dispflag)
{
 
   // check if online data reading (Midas)
   if (fIsMidasOnline) return;
  
   // Soudan data: first word is endianness descriptor
   //second is DAQ version number (not being used at the moment)
   //third is either the event header of the detector config header
   //fourth is either the record length of the config rec or the start of the event
   //Test for Angie const int nread = 344; //4+configsize/4
   const int nread = 2;
   uint32_t header[nread];

   if(fgzRawDataPtr == NULL)  
   {
      cerr <<"RawDataReader::ERROR file not open!  Did you remember to call RawDataReader::OpenRawDataFile()?" << endl; 
      exit(1);
   }

   //returns -1 for error, and 0 for end of file
   int readCheck = gzread(fgzRawDataPtr, (char*)header, nread*sizeof(uint32_t));

   if(readCheck < 0) {
      cerr <<"RawDataReader::ERROR reading File Header ! \n " << endl;
      exit(1);
   }


   //This is for older file formats such as SUF and UCB
   if(header[0] == 0x0)
   {
      uint32_t extraHeaderWord;

      int extraCheck = gzread(fgzRawDataPtr, (char*)&extraHeaderWord, sizeof(uint32_t));
      printf("Detected older file type!  file header line3     : %#x \n", extraHeaderWord);  
   
      if(extraCheck < 0) {
	 cerr <<"RawDataReader::ERROR reading File Header ! \n " << endl;
	 exit(1);
      }
      
   }
   //Otherwise check for a modern-day file header
   else if (header[0] != 0x04030201 && header[0] != 0x01020304)
   {

     // Assume MIDAS data 
     fIsMidasData = true;
     
     //Go back to the beginning of the file
     gzrewind(fgzRawDataPtr);  
     

     // Read Midas Event Header and 
     // check EventID = 0x8000 (begin file/run)
     
     int readCheck = ReadMidasEventHeader(false);
     
     
     if (readCheck) {
       int eventId = fMidasEvent.GetEventId();
   
       if ((eventId & 0xFFFF) == 0x8000)  { 

	 cout<< "RawDataReader::ReadFileHeader:  Reading a MIDAS file!"<<endl;
	
	 // SeriesNumber
	 if(!fGotSeriesInfoFromFilename)
	  fMidasSeriesNumber = fMidasEvent.GetSerialNumber();
	 else
          fCurrentEventNumber += fMidasDumpNumber*10000 -1; //standard scaling

	 // read file header data record
	 int status = fMidasEvent.ReadDataRecord(fgzRawDataPtr);     
	 if (status)
	   return;
       } 
     }
     
     printf("file header line1    : %#x \n", header[0]);
     printf("file header line2    : %#x \n", header[1]);  
     
     cerr <<"RawDataReader::ReadFileHeader ERROR unrecognized File Header ! \n " << endl;
     exit(1);
   }
   

   // ------ SOUDAN DATA  ------
   
   //Set the endianness flag for newer data 
   //do this here that it can be used when reading the detector config record 
   if(header[0] == 0x01020304 )
   {
      fFlipBytes = false;
      if(!fByteCheckDone) { SetEndianParam(); }

      if(dispflag) 
	cout <<"Reading file header, read and write endian are the same" << endl;

   }   
   else if(header[0] == 0x04030201)
   {
      fFlipBytes = true;
      if(!fByteCheckDone) { SetEndianParam(); }

      if(dispflag) 
	cout <<"Reading file header, read and write endian are opposite" << endl;
   }   


   //print statements for debugging the file header read
   if(dispflag)
   {
     printf(">> readFileHeader \n");

     if(fFlipBytes == false)
     {
       printf("file header line1    : %#x \n", header[0]);
       printf("file header line2    : %#x \n", header[1]);  
     }
     else
     {
       printf("file header line1    : %#x \n", EndianHelper::Swap4ByteWord(header[0]));
       printf("file header line2    : %#x \n", EndianHelper::Swap4ByteWord(header[1]));  
     }

   }


   // --- read next word and check record type to see if its a detector config record ---

   uint32_t nextHeader[nread];
   if( gzread(fgzRawDataPtr, (char*)nextHeader, nread*sizeof(uint32_t)) < 0 )
   {
     cerr <<"RawDataReader::ERROR reading File Header ! \n " << endl;
     exit(1);
   }

   //cout <<"Checking for config record, nextRecordID = " << nextHeader[0] << endl;

   //check whether next record is a detector config.  If not, then assume its the start of
   //an event header so rewind by one word to allow the reader to start with the event record.
   //Note: Prior to Nov 2010, there was no detector config record
   
   uint32_t nextRecordID = (fFlipBytes ? EndianHelper::Swap4ByteWord(nextHeader[0]) : 
			    nextHeader[0]);

   if(nextRecordID == BatRootTypes::kDetectorConfigID)
   {
     uint32_t configRecordLength = (fFlipBytes? EndianHelper::Swap4ByteWord(nextHeader[1]) : 
				    nextHeader[1]);

     if(fReadDetectorConfig) 
     {
       fDetectorConfigPtr->ReadDetectorConfigRecord(fgzRawDataPtr, configRecordLength, dispflag);
     }
     else
     {
       gzseek(fgzRawDataPtr, configRecordLength, SEEK_CUR);
     }

   }
   else 
   {

     if(dispflag == true)
     {
       cout <<"Reading file header.  No detector configuration record found!"
	    <<"\nRewinding by two words for event reading"
	    << endl;
     }

     //rewind by one word if this is an older-style file (i.e. no DetectorConfigData record)
     gzseek(fgzRawDataPtr, -nread*sizeof(uint32_t), SEEK_CUR);

   }

   return;

}

//////////////////////////////////////////////////////////////////////////////////////////////
//Reads Event Header and stores useful information for navigating raw data file
int RawDataReader::ReadEventHeader(bool dispflag)
{  
   dispflag = false; //TEMP llhsu

   const int nread = 2;
   uint32_t header[2]; //store read values

   //If reading has already started, position pointer at the next event
   if(fNextEventPosition != 0)   gzseek(fgzRawDataPtr, (fNextEventPosition-gztell(fgzRawDataPtr)), SEEK_CUR);
//   if(fNextEventPosition != 0)   gzseek(fgzRawDataPtr, fNextEventPosition, SEEK_SET);

   if(dispflag)
   cout <<"RawDataReader::Reading Event Header at position: " 
	<< gztell(fgzRawDataPtr)
	<< endl;
   
   //returns -1 for error, and 0 for end of file
   int readCheck = gzread(fgzRawDataPtr, (char*)header, nread*sizeof(uint32_t));
   if(readCheck < 0) {
      cerr <<"RawDataReader::ERROR reading Event Header ! \n" << endl;
      exit(1);
   }
   if(readCheck == 0) {
      cout <<"RawDataReader::Reached end of file ! \n" << endl;
      return readCheck;
   }

   uint16_t tempUpper = (header[0] & 0xffff0000) >> 16;
   uint16_t tempLower = (header[0] & 0x0000ffff);
   uint16_t eventID = 0;

   uint32_t eventType = 0;
   uint32_t eventClass = 0;
   uint32_t eventCategory = 0;
   
   //for debugging only
   // printf("   tempUpper: %#x \n", tempUpper);   
   //printf("   tempLower: %#x \n", tempLower);   

   //Check for the following cases
   //endian check done here as well for older data
   if(tempUpper == 0xa980 )
   {
      fFlipBytes = false;
      eventClass = (tempLower&0xf000) >> 12;
      eventCategory = (tempLower&0x0f00) >> 8;
      eventType = tempLower&0x00ff;
      eventID = tempUpper;

      if(dispflag) cout <<"read and write endian are the same" << endl;
   }   
   if(tempLower == 0x80a9)
   {
      fFlipBytes = true;
      eventClass = (tempUpper&0x00f0) >> 4;
      eventCategory = tempUpper&0x000f;
      eventType = (tempUpper&0xff00) >> 8;
      eventID = ((tempLower&0x00ff)<<8)+((tempLower&0xff00)>>8);

      if(dispflag) cout <<"read and write endian are opposite" << endl;
   }   
   
   //Set this only once per file
   if(!fByteCheckDone) { SetEndianParam(); }

   if(dispflag) cout <<"Flipping bytes? " << fFlipBytes << endl;

   //storing info useful for navigating through the file
   if(fFlipBytes) header[1] = EndianHelper::Swap4ByteWord(header[1]);
   fEventLength = header[1]; //record length
   fCurrentEventPosition = gztell(fgzRawDataPtr); //store the file ptr position for this event after reading event header
   fNextEventPosition = fCurrentEventPosition + fEventLength;

   fEventCategory = eventCategory; //for noise checks
   fEventType     = eventType;     //stored as an RQ

   if(dispflag){
      printf(">> readEventHeader \n");
      printf("    EventType       : %#x \n", eventType); 
      printf("    EventClass      : %#x \n", eventClass); 
      printf("    EventCategory   : %#x \n", eventCategory); 
      printf("    EventID         : %#x \n", eventID);
      printf("    EventLength     : %d \n",  fEventLength); //in bytes
      printf("    UnmodEventHeader: %#x \n", header[0]);
   }
      
   if(eventID != 0xa980) 
   { 
      cerr <<"RawDataReader::ERROR parsing event header!!!!!!!" << endl; 
      exit(1);
   }

   //If the read succeeded then get the current position
   fNextRecordHeaderPosition = gztell(fgzRawDataPtr); //after reading eventheader, file is already at position to read next record
     
   return readCheck;

}


//////////////////////////////////////////////////////////////////////////////////////////////
//Reads Event Header and stores useful information for navigating raw data file
int RawDataReader::ReadMidasEventHeader(bool dispflag)
{  
  
   // clear MidasEvent
   fMidasEvent.Clear();

   //set verbosity of MidasEvent object
   fMidasEvent.SetDiagnosticPrints(fdiagnosticPrints);
   fMidasEvent.SetVerbosity(fverbosity);


   // Read MidasEvent Header
   int readCheck = fMidasEvent.ReadEventHeader(fgzRawDataPtr);
  

   // Store usefull quantities
   fEventLength = fMidasEvent.GetDataSize(); //record length
   fCurrentEventPosition = gztell(fgzRawDataPtr); //store the file ptr position for this event after reading event header
   fNextEventPosition = fCurrentEventPosition + fEventLength;

   //FIXME I don't think this is where the trig mask should go, just event category [ANV] 
   //fEventCategory = fMidasEvent.GetTriggerMask();
   //fEventCategory = fMidasEvent.GetEventCategory();
    

   if (dispflag) {
     cout << "fEventLength: " << fEventLength <<endl;
     cout << "fCurrentEventPosition: " << fCurrentEventPosition <<endl;
     cout << "fNextEventPosition: " <<fNextEventPosition<<endl;
     cout << "fEventCategory: " << fEventCategory<<endl;
   }
   
   return readCheck;
}









//////////////////////////////////////////////////////////////////////////////////////////////

//The main routine for reading raw events!
//This assumes the file header has been read (or previous event has been read).
//Note that as of Nov. 2010, the next record may be an event record or a detector config record
//this method must now determine which it is before proceeding?
int RawDataReader::ReadRawDataRecord()
{   
  
   bool dispflag = false; 
   
   int eventStatus = 0;

   if (!fIsMidasData) {
       
     uint32_t recordID = 0;
     uint32_t recordLength = 0;
     int zipListEndDetCode = 0;
     int vetoListEndDetCode = 0;
     
     //Read Event Header and setup for next data block to be read
     eventStatus = ReadEventHeader(dispflag);  //fills fEventLength
     
     if(eventStatus == 0) {  return eventStatus; }  //stop, since we're at the end of the file
     
     //fNextRecordHeaderPosition was set in ReadEventHeader
     
     if(dispflag)
       {
	 cout <<"fCurrentEventPosition = " << fCurrentEventPosition
	      <<"\nfEventLength = " << fEventLength
	      <<"\nsum of above = " << fNextEventPosition
	      <<"\nfNextRecordHeaderPosition = " << fNextRecordHeaderPosition
	      << endl;
       }
     
     //Read each record ID and fill the data object accordingly
     //GetNextRecordID automatically positions file ptr at start of next record
     while(fNextRecordHeaderPosition < fNextEventPosition)
       {
	 //update record info
	 recordID = GetNextRecordID(dispflag);
	 recordLength = fNextRecordHeaderPosition - fCurrentRecordPosition;
	 
	 
	 //Read Admin record
	 if((recordID == BatRootTypes::kAdminRecordID || recordID == BatRootTypes::kAdminRecordID64 ) && fReadAdmin)   
	   { fAdminPtr->ReadRawDataRecord(fgzRawDataPtr, recordLength, recordID, dispflag); } 
	 
	 
	 //Read History record
	 if(recordID == BatRootTypes::kSoudanHistoryRecordID && fReadHistory) { fHistoryPtr->ReadSoudanHistoryRecord(fgzRawDataPtr, recordLength, dispflag); }
	 
	 
	 //Read Trigger record
	 if(recordID == BatRootTypes::kTriggerRecordID && fReadTrigger) { fTriggerPtr->ReadTrigger(fgzRawDataPtr, recordLength, dispflag); }
	 
	 
	 //Read External record
	 if(recordID == BatRootTypes::kExternalRecordID && fReadGPS) { fGPSPtr->ReadGPS(fgzRawDataPtr, recordLength, dispflag); }
	 
	 
	 //Read Pulse records
	 if((recordID == BatRootTypes::kPulseRecordID || recordID == BatRootTypes::kPulseRecordExpandedCodeID)  
	    && (fReadVetoPulses || fReadZipPulses || fReadNoiseMonitorPulses ))  
	   { 
	     PulseData tempPulseData;
	     tempPulseData.SetFlipBytes(fFlipBytes); 
	     
	     
	     tempPulseData.ReadRawPulseRecord(fgzRawDataPtr, recordLength, recordID, dispflag); //tempPulseData is filled now!
	     
	     if(tempPulseData.IsVetoPulse() && fReadVetoPulses) StorePulsesByDetCode(tempPulseData, zipListEndDetCode, vetoListEndDetCode);
	     if(tempPulseData.IsZipPulse() && fReadZipPulses) StorePulsesByDetCode(tempPulseData, zipListEndDetCode, vetoListEndDetCode);      
	     if(tempPulseData.IsNoiseMonitorPulse() && fReadNoiseMonitorPulses) StorePulsesByDetCode(tempPulseData, zipListEndDetCode, vetoListEndDetCode);      
	     
	   }
	 
       } //end while loop
     
     //Take the ordered pulseData list and fill a map for convenient access
     if(fReadZipPulses)  FillMapOfZipPulses();
     
     // set Modified flag to false
     if(fReadZipPulses) fIsZipPulsesModified = false;
   

   } else {


     // ==============================
     // ========= MIDAS data =========
     // ==============================
     
    
     // ------ Pulse record -------
     
     if(fdiagnosticPrints)
       cout << "RawDataReader::ReadRawDataRecord: INFO next trigger, number: " << fCurrentMidasEventTrigger+1 << endl;
     // Increment to next trigger 
     fCurrentMidasEventTrigger++;
        
     // Check if another "Midas Event"  needs to be read
     //   - Trigger Number > Total number of triggers
     //   - No trigger available
     
     if (fCurrentMidasEventTrigger==1 || 
	 fCurrentMidasEventTrigger>fNbMidasEventTriggers) {
       
       
       if(fdiagnosticPrints)
         cout << "RawDataReader::ReadRawDataRecord: INFO Need to read new event" << endl;

       // cout <<"RawDataReader::Current Midas Trigger: " << fCurrentMidasEventTrigger << endl;
       
       // get next Midas event (with trigger)
       int eventLoop = 1;
       while(eventLoop) 
	 {
	   //added Feb 21 2015. prevents GUI freezing even
	   //when there are very few triggers	   
	   if (fgSystem!=NULL)
	     fgSystem->ProcessEvents();
	   
	   if (!fIsMidasOnline) {
	     
	     // Read Midas Event Header
	     eventStatus = ReadMidasEventHeader(dispflag);  
	     if (eventStatus==0) return eventStatus;
             if(fdiagnosticPrints)
               cout << "RawDataReader::ReadRawDataRecord: INFO Reading event header (eventStatus=" << eventStatus << ")" << endl;
	     
	     // Read Midas Event Data
	     eventStatus = fMidasEvent.ReadDataRecord(fgzRawDataPtr);     
	     if (eventStatus==0) return eventStatus; 
             if(fdiagnosticPrints)
               cout << "RawDataReader::ReadRawDataRecord: INFO Reading raw data record (eventStatus=" << eventStatus << ")" << endl;
	     	     
	   } else {
	     
#ifdef HAVE_MIDAS
	     eventStatus = fMidasEvent.ReadEventOnline(fMidas);
	     if (eventStatus==0 || eventStatus==-1)  {
	       return eventStatus; 
	     }
#endif	     
	   }
	   
	   // Get number of triggers
	   fNbMidasEventTriggers = fMidasEvent.GetNbTriggers();
	   
	   //cout << "RawDataReader::Number of Trigger: " << fNbMidasEventTriggers << endl;
	   
	   if (fNbMidasEventTriggers==0)
	     continue;
	   else 
	     eventLoop = 0;
	   
	   // Get "Midas event" number
	   fMidasEventNumber = fMidasEvent.GetSerialNumber()+1;
	   
	   // Point to first trigger
	   fCurrentMidasEventTrigger = 1; // set to first trigger
	 }
     }
  
     //FIXME remove GetEventCategory
     //settled on a trigger, read Event Category
     fEventCategory = fMidasEvent.GetEventCategory(fCurrentMidasEventTrigger);

     // Now get pulse map
     
     // Let's keep in mind that the pulses could contain no data if they are
     //'stale' triggers (as implemented by Scott in Aug. 2014) so check that there
     // are real pulses
    
     if(fdiagnosticPrints)
       cout << "RawDataReader::ReadRawDataRecord: INFO Got the new pulse" << endl;
     int isRealZipPulse = 0;
    
     while(!isRealZipPulse){
       
       int check = fMidasEvent.IsStaleTrigger(fCurrentMidasEventTrigger);

       if(check==0){
         isRealZipPulse = 1;
         fListOfZipPulses =  fMidasEvent.GetPulseDataList(fCurrentMidasEventTrigger);
       }
       else if (check<0)
	 break;
       
       //if isRealZipPulse is still zero at this point, the pulse contains
       //no data.  In this case, stay in the while loop and go to the next
       //trigger.
       if(!isRealZipPulse){
         if(fdiagnosticPrints)
           cout << "RawDataReader::ReadRawDataRecord: INFO not real zip pulse" << endl;
	 if(fCurrentMidasEventTrigger>fNbMidasEventTriggers) 
	   break;
	 else
	   fCurrentMidasEventTrigger++;
       }
       
     }
     
     if(!isRealZipPulse&&fdiagnosticPrints)
       cout << "RawDataReader::ReadRawDataRecord: INFO This pulse is stale!" << endl;

     // Fill overall map
     FillMapOfZipPulses();
     fIsZipPulsesModified = false;
     
     
     // ------ Admin record -------

   
     /*
     // EventNumber = "MidasEvent" Number *1000 + Trigger Number
     uint32_t event = fMidasDataReader.GetMidasEventNumber()*1000 + fCurrentMidasEventTrigger;
     */

     // EventNumber
     fCurrentEventNumber++;
     uint32_t event = fCurrentEventNumber;
       

     // Time Between / livetime
     uint32_t timeBetween  = 0; //TEMP
     uint32_t liveTime = 0; // TEMP

     //EventTime (Time of "MidasEvent") 
     uint32_t eventTime =  fMidasEvent.GetTimeStamp();

     fAdminPtr->SetRawDataRecord(fMidasSeriesNumber,event,timeBetween, liveTime, eventTime);
     
     // all good, set status to 1
     eventStatus =1;

   }
   
   return eventStatus; 
}
   
   
   //////////////////////////////////////////////////////////////////////////////////////////////

// Skip reading of event records, then read data records
int RawDataReader::ReadRawDataRecord(const int nEventsToSkip) {
  
  int eventStatus = 0;

  // skip events
  int skipEventStatus = SkipRawEvents(nEventsToSkip);
  // read next event
  if (skipEventStatus) {
    Clear();

    eventStatus = ReadRawDataRecord();
  }

  return eventStatus;
}




//////////////////////////////////////////////////////////////////////////////////////////////

// Skip reading of event records and setup pointer for next event to be read
int RawDataReader::SkipRawEvents(const int nEventsToSkip)
{   
   bool dispflag = false; 
   int eventStatus = 0;
   
   if(fdiagnosticPrints)
     cout << "RawDataReader::SkipRawEvents: INFO Trying to skip " << nEventsToSkip << " events" << endl;
   if (nEventsToSkip==0) {
     // do nothing 
     eventStatus =1;
 
   } else if (nEventsToSkip>0) { 

     
     if (!fIsMidasData) {
       
       // loop through events
       for (int eventItr =0; eventItr<nEventsToSkip;eventItr++) {
	 eventStatus = ReadEventHeader(dispflag);  //fills fEventLength
	 if(eventStatus == 0) {  return eventStatus; }  //stop, since we're at the end of the file
       } // loop events

       
     } else {

       // MIDAS data
      
       int currentNbEventsSkipped = 0;
       while(currentNbEventsSkipped<nEventsToSkip) {
	 
	 // check if we need to read another midas event
	 if (fNbMidasEventTriggers==0 || 
	     fCurrentMidasEventTrigger+(nEventsToSkip-currentNbEventsSkipped)>fNbMidasEventTriggers) {
	  
	   //account for the ones you skipped by reading new event 
	   currentNbEventsSkipped += fNbMidasEventTriggers - fCurrentMidasEventTrigger;
     
	   // adjust EventNumber
           fCurrentEventNumber += fNbMidasEventTriggers - fCurrentMidasEventTrigger;
	  
           if(fdiagnosticPrints)
             cout << "RawDataReader::SkipRawEvents: INFO Reading New MIDAS Event" << endl;
	   // Read Midas Event Header
	   eventStatus = ReadMidasEventHeader(dispflag);  
	   if (eventStatus==0) return eventStatus;  
	    
	   
	   // TEMP: Reading full Midas Event Data (should be replaced by reading only bank 
	   // header (=number of triggers)
	   eventStatus = fMidasEvent.ReadDataRecord(fgzRawDataPtr);     
	   if (eventStatus==0) return eventStatus; 
	   
	  
	   // get number of triggers
	   fNbMidasEventTriggers = fMidasEvent.GetNbTriggers();

	   // check if you have to skip this whole MIDAS event
	   if (currentNbEventsSkipped+fNbMidasEventTriggers<nEventsToSkip){
	     // adjust EventNumber
             fCurrentEventNumber +=  fNbMidasEventTriggers;

             if(fdiagnosticPrints){
	       cout << "RawDataReader::SkipRawEvents: INFO event trigger was: " << fCurrentMidasEventTrigger << endl;
	       cout << "RawDataReader::SkipRawEvents: INFO event trigger is now: " << fCurrentMidasEventTrigger+(fNbMidasEventTriggers) << endl;
	     }
	     //set trigger pointer
	     currentNbEventsSkipped += fNbMidasEventTriggers;
	     fCurrentMidasEventTrigger = fNbMidasEventTriggers;
	   
	   } else {
             if(fdiagnosticPrints){
	       cout << "RawDataReader::SkipRawEvents: INFO (new event) Have: " << fNbMidasEventTriggers << " total triggers" << endl;
	       cout << "RawDataReader::SkipRawEvents: INFO (new event) Skipping: " << nEventsToSkip << " in this iteration" << endl;
	       cout << "RawDataReader::SkipRawEvents: INFO (new event) Skipped: " << currentNbEventsSkipped << " last iteration" << endl;
	     }
	     // adjust EventNumber
             fCurrentEventNumber +=  nEventsToSkip-currentNbEventsSkipped;

             if(fdiagnosticPrints){
	       cout << "RawDataReader::SkipRawEvents: INFO event trigger was: " << fCurrentMidasEventTrigger << endl;
	       cout << "RawDataReader::SkipRawEvents: INFO event trigger is now: " << fCurrentMidasEventTrigger+(nEventsToSkip-currentNbEventsSkipped) << endl;
	     }
	     //set trigger pointer
	     fCurrentMidasEventTrigger += nEventsToSkip-currentNbEventsSkipped;
	     currentNbEventsSkipped = nEventsToSkip;

	   }

	 } else {	   
           if(fdiagnosticPrints){
	     cout << "Have: " << fNbMidasEventTriggers << " total triggers" << endl;
	     cout << "Skipping: " << nEventsToSkip << " in this iteration" << endl;
	     cout << "Skipped: " << currentNbEventsSkipped << " last iteration" << endl;
	   }
	   // adjust EventNumber
           fCurrentEventNumber +=  nEventsToSkip - currentNbEventsSkipped;

           if(fdiagnosticPrints){
	     cout << "RawDataReader::SkipRawEvents: INFO event trigger was: " << fCurrentMidasEventTrigger << endl;
	     cout << "RawDataReader::SkipRawEvents: INFO event trigger is now: " << fCurrentMidasEventTrigger+(nEventsToSkip-currentNbEventsSkipped) << endl;
	   }
	   // Set trigger pointer
	   fCurrentMidasEventTrigger += nEventsToSkip - currentNbEventsSkipped;
	   currentNbEventsSkipped = nEventsToSkip;
	   eventStatus=1;

	 }
       }

     }
     
   }

   return eventStatus; 
}



//////////////////////////////////////////////////////////////////////////////////////////////
//Gets Next RecordID, meanwhile store position of ptr for next  
//recordID read and start from there.  Returns record ID
uint32_t RawDataReader::GetNextRecordID(bool dispflag)
{
   dispflag = false; //TEMP llh

   const int nread = 2;
   uint32_t header[2]; //store read values
   int tempFilePosition;

   //Position ptr at the start of the next block of data
   if(dispflag) cout <<"GetNextRecordID::position of pointer before move is = " << gztell(fgzRawDataPtr) << endl;
//   for(int tempItr=0; tempItr<1000; tempItr++)
   {
//      cout <<"Seeking!! "<< tempItr << endl;
      tempFilePosition = gzseek(fgzRawDataPtr, fNextRecordHeaderPosition - gztell(fgzRawDataPtr), SEEK_CUR); 
   }

   if(dispflag) cout <<"GetNextRecordID::position of pointer after move = " << gztell(fgzRawDataPtr) << endl;  
   
   //Read header info for this block
   int readCheck = gzread(fgzRawDataPtr, (char*)header, nread*sizeof(uint32_t));
   if(fFlipBytes)
   {
      header[0] = EndianHelper::Swap4ByteWord(header[0]); 
      header[1] = EndianHelper::Swap4ByteWord(header[1]); 
   }

   if(readCheck < 0) {
      cerr <<"RawDataReader::ERROR reading Record Header !" << endl;
      exit(1);
   }
   
   if(dispflag){
      printf("RawDataReader::Got RecordHeader \n");
      printf("    Record ID: %#x \n", header[0]);
      printf("    Record Length: %d \n", header[1]);
   }
   
   //Store the next record position
   fCurrentRecordPosition =  tempFilePosition + (uint32_t)readCheck; 
   fNextRecordHeaderPosition = fCurrentRecordPosition + header[1]; 

   if(dispflag) cout <<"GetNextRecordID::Next record position = " << fNextRecordHeaderPosition << endl;
   
   return (header[0]); //return headerID

}

//////////////////////////////////////////////////////////////////////////////////////////////
//Helper function for ReadRawDataRecord, storing pulses in order according to the detector code
//The primary function of doing this is to ensure that all the pulses belonging to one detector
//are stored consecutively in the fListOfZipPulses.  This makes finding pulses belonging to one
//detector easier later (e.g. to do cross-talk fits)
void RawDataReader::StorePulsesByDetCode(PulseData& tempPulseData, int& zipEndCode, int &vetoEndCode)
{
   //Insert the pulses in order according to the detector code
   int detCode = tempPulseData.GetDetectorCode();
   int detType = tempPulseData.GetDetectorType();

   //cout <<"detCode: " << detCode << endl;
   
   //check that its not a veto or noise monitor
   if( detType != BatRootTypes::kVetoDetType && 
       detType != BatRootTypes::kMonitorNoiseFast  &&  
       detType != BatRootTypes::kMonitorNoiseSlow)
      
   {
      
      //storing zip pulses in order according to detCode
      if(detCode > zipEndCode)
      {
	 //pulse is in anticipated order, so add to the end
	 fListOfZipPulses.push_back(tempPulseData);
	 zipEndCode = detCode; //detCode of last pulse in list
      }
      else
      {
	 //search backwards through the stored pulses to find the right place
	 //for this pulse according to its detector code
	 vector<PulseData>::iterator backItr = fListOfZipPulses.end() - 1; //to start at the last element
	 for( ; backItr != fListOfZipPulses.begin(); backItr--)
	 {
	    if(detCode > backItr->GetDetectorCode()) { break; }
	 }
	 
	 //store *after* the first pulse with DetCode < than current
	 if(detCode > backItr->GetDetectorCode()) { backItr++; }
	 
	 //now the pulse *after* the last position found
	 fListOfZipPulses.insert(backItr, tempPulseData);
      }
 
   } //done storing zip pulses 
   
   else if (detType == BatRootTypes::kVetoDetType)
   {
      
      //storing veto pulses in *temp list* order according to detCode
      if(detCode > vetoEndCode)
      {
	 //pulse is in anticipated order, so add to the end
	 fListOfVetoPulsesPtr->push_back(tempPulseData);
	 vetoEndCode = detCode;  //detCode of last pulse in list
      }
      else
      {
	 //search backwards through the stored pulses to find the right place
	 //for this pulse according to its detector code
	 vector<PulseData>::iterator backItr = fListOfVetoPulsesPtr->end() - 1; //to start at the last element
	 for( ; backItr != fListOfVetoPulsesPtr->begin(); backItr--)
	 {
	    if(detCode > backItr->GetDetectorCode()) { break; }
	 }
	 
	 //store *after* the first pulse with DetCode < than current
	 if(detCode > backItr->GetDetectorCode()) { backItr++; }
	 
	 //now the pulse *after* the last position found
	 fListOfVetoPulsesPtr->insert(backItr, tempPulseData);
      }
      
   //done sorting veto pulses 
 
   } else  { 


     // At this point,  should be noise Monitor, no special order needed
     fListOfNoiseMonitorPulsesPtr->push_back(tempPulseData);

   }


   return;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Helper function for ReadRawDataRecord, sort PulseData list into a map
void RawDataReader::FillMapOfZipPulses()
{
   //cout << "RawDataReader::FillMapOfZipPulses(): filling " << fListOfZipPulses.size() << " pulses" << endl;
   for(uint pulseItr=0; pulseItr < fListOfZipPulses.size(); pulseItr++)
   {
      PulseData aPulseData = fListOfZipPulses[pulseItr]; //makes a copy
      int detNum = aPulseData.GetDetectorNum();

      //retrive the vector of pulses for this zip, create new entry if it doesn't exist
      map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulsesPtr->find(detNum);
      if(mapItr != fMapOfZipPulsesPtr->end())
      {
	 (mapItr->second).push_back(aPulseData);
      }  
      else
      {
	 vector<PulseData> tempVector;
	 tempVector.push_back(aPulseData); //store this copy of PulseData
	 fMapOfZipPulsesPtr->insert(pair<int, vector<PulseData> >(detNum, tempVector));
      }	 

   }

   return;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Sets fFlipBytes for all readers
//Note: pulses are handled in RawDataReader::ReadRawDataRecord
void RawDataReader::SetEndianParam()
{
   //Pass this on to the other readers
   if(fReadDetectorConfig) fDetectorConfigPtr->SetFlipBytes(fFlipBytes);
   if(fReadAdmin)          fAdminPtr->SetFlipBytes(fFlipBytes);
   if(fReadHistory)        fHistoryPtr->SetFlipBytes(fFlipBytes);
   if(fReadTrigger)        fTriggerPtr->SetFlipBytes(fFlipBytes);
   if(fReadGPS)            fGPSPtr->SetFlipBytes(fFlipBytes);

   //PulseData flipBytes flag is set in ReadRawDataRecord

   return;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//Reset values of member data for next event 
void RawDataReader::Clear()
{

   //clear member data values for next event 
   if(fReadAdmin)   fAdminPtr->Reset();
   if(fReadHistory) fHistoryPtr->Reset();
   if(fReadTrigger) fTriggerPtr->Reset();
   if(fReadGPS)     fGPSPtr->Reset();

   if(fReadZipPulses)  fListOfZipPulses.clear();      //reset the list
   if(fReadZipPulses)  fMapOfZipPulsesPtr->clear();   //reset the map
   if(fReadVetoPulses) fListOfVetoPulsesPtr->clear(); //reset the list
   if(fReadNoiseMonitorPulses) fListOfNoiseMonitorPulsesPtr->clear(); //reset

   //EventHeader info
   fEventCategory = 0xffff;
   fEventType     = 0xffff;

   // set back "Modified" flag to false
   if(fReadZipPulses) fIsZipPulsesModified = false;
 

   return;
}






//////////////////////////////////////////////////////////////////////////////////////////////

// Function to Modidy Raw Data 
void RawDataReader::ModifyRawData(const map<int,string>& modificationMap)
{   

  // To modify raw data, fill the maps in section "user modifications". The modification
  // will then be applied automatically!
  //
  // Available maps:
  //
  //    1) map<OLD detCode, NEW detCode>  ("detCodeModificationMap"):  
  //   
  //       This map will be used to modify the configuration map AND PulseData map
  //     
  //        if "new" detCode = 0      => delete channel !
  //        if not defined in map     => keep unchanged !
  //
  //    2) map<NEW detCode, map<parameter name, NEW value> >    ("detConfigModificationMap")
  //       change configuration value such as Qbias, SampleRate, etc.
  //       (or add new parameter...)  




  // ========== USER MODIFICATIONS ==========


  // ---- Define modification maps ----
  // NOTE: if no information in maps -> NO change
  map<int,int> detCodeModificationMap;  // map<OLD detCode, NEW detCode> 
  map<int, map<string, double> >  detConfigModificationMap;  // map<NEW detCode, map<parameter name, NEW value>>


  // Loop Detectors
  map<int, string>::const_iterator detModItr = modificationMap.begin();
  for (;detModItr!=modificationMap.end(); detModItr++)
   {
     // Get detector number and modification type
     int detNum = detModItr->first;
     string modificationType = detModItr->second;

     //cout << "WARNING in RawDataReader::ModifyRawData: Modifying detector " << detNum << " for " << modificationType <<endl;


     // ---- Fill maps ----

     // Izip Type 11  to  CDMSlite Type 21 
     if (modificationType.compare("CDMSliteSoudanI") == 0) {
     
      // detector code base
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(11,detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(21,detNum);
 
      // detector code
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, detCodeBaseNew));   // QIS1 -> QI 
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+1));   // QOS1 -> QO
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));          // PAS1 -> delete
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+3));   // PBS1 -> PB
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));          // PCS1 -> delete
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // PDS1 -> PD
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+6, 0));          // QIS2 -> delete
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+7, 0));          // QOS2 -> delete
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+8, detCodeBaseNew+2));   // PAS2 -> PA
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+9, 0));          // PBS2 -> delete
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+10,detCodeBaseNew+4));   // PCS2 -> PC
      detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+11,0));          // PDS2 -> delete


      //detector configuration
      
      // change charge bias
      map<string, double> configMap;
      configMap.insert(pair<string,double>("chargeBias", 69));
   
      detConfigModificationMap.insert(pair<int, map<string, double> >(detCodeBaseNew, configMap)); // QI
      detConfigModificationMap.insert(pair<int, map<string, double> >(detCodeBaseNew+1, configMap)); // QO
     
     } //end block for Type 11 to CDMSlite Type 21



     // Modify for DCRC revC board readout for 100 mm iZIP (UMN data)
     if(modificationType.compare("UMNiZIP100mmDCRCrevC") == 0) 
     {

       if(detNum > 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNiZIP100mmDCRCrevC!"
	      <<"\n There are more zips than expected (3), please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(700, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, detCodeBaseNew));       // DIB1 QI ->  QIS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+1));   // DIB1 QO ->  QOS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+5));   // DIB1 PA ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+9));   // DIB1 PB ->  PFS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+6));   // DIB1 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+13));  // DIB1 PD ->  PDS2
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, detCodeBaseNew+2));     // DIB2 QI ->  QIS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+3));   // DIB2 QO ->  QOS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+12));  // DIB2 PA ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+10));  // DIB2 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+7));   // DIB2 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+4));   // DIB2 PD ->  PAS1
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Detete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+11));  // DIB3 PA ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+15));  // DIB3 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+14));  // DIB3 PC ->  PES2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+8));   // DIB3 PD ->  PES1
      }

     } //end block for DCRC revC 100 mm iZIP

     // Modify for DCRC revC board readout for special 100 mm iZIP G103a (UMN data)
     if(modificationType.compare("UMNiZIP100mmDCRCrevC_G103a") == 0) 
     {

       if(detNum > 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNiZIP100mmDCRCrevC_G103a!"
	      <<"\n There are more zips than expected (3), please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(700, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, detCodeBaseNew));       // DIB1 QI ->  QIS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+1));   // DIB1 QO ->  QOS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+5));   // DIB1 PA ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+9));   // DIB1 PB ->  PFS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+6));   // DIB1 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+13));  // DIB1 PD ->  PDS2
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, detCodeBaseNew+2));     // DIB2 QI ->  QIS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+3));   // DIB2 QO ->  QOS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+12));  // DIB2 PA ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+10));  // DIB2 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+7));   // DIB2 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+4));   // DIB2 PD ->  PAS1
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Detete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+11));  // DIB3 PA ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+15));  // DIB3 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+8));   // DIB3 PC ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+14));  // DIB3 PD ->  PES2
      }

     } //end block for DCRC revC special 100 mm iZIP G103a

     // Modify for DCRC revC board readout for standard iZIP (Queens data)a
     if(modificationType.compare("QueensiZIPDCRCrevC") == 0) 
     {
       int qmapzip[12]={1,1,1,1,1,1,2,2,2,2,2,2};
       int qmapch[12]={0,1,2,3,4,5,0,1,2,3,4,5};
       int qmapnewch[12]={0,1,2,3,4,5,6,7,8,9,10,11};
  
       if(detNum > 2)
       {
	 cerr <<"RawDataReader::ModifyRawData: ERROR while trying to modify for QueensiZIPDCRCrevC!"
	      <<"\n There are more zips than expected (2), please check raw data stream."
	      << endl;
	 continue;
	 //exit(1);
       }

       if(fverbosity>2)
         cout << "RawDataReader::MoifyRawData: INFO detNum: " << detNum << endl;
       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(11, 1);
 
      for(int chanCtr = 0; chanCtr< 6; chanCtr++)
      {
	  int oldCode = detCodeBaseOld + chanCtr;
	  int newCode=-1;
	  int seqnum = (detNum-1)*6 + chanCtr;
	  for(int searchItr=0;searchItr<12;searchItr++){
            if(qmapzip[searchItr]==detNum && qmapch[searchItr]==chanCtr){
	      newCode = detCodeBaseNew + qmapnewch[searchItr];
	      if(fverbosity>2)
                cout << "RawDataReader::ModifyRawData: INFO new base code: " << detCodeBaseNew << "   new channel: " << qmapnewch[searchItr] << endl;
	    }
          }

	  


	  //Only change the detector code and nothing else
	  if(fverbosity>2)
	    cout << "RawDataReader::ModifyRawData: INFO oldCode: " << oldCode << "  newCode: " << newCode << endl;
          if(newCode < 0)
          {
            cerr <<"RawDataReader::ModifyRawData: ERROR while trying to modify for QueensiZIPDCRCrevC!"
                 <<"new detector code invalid."
                 << endl;
            exit(1);
          }
	  detCodeModificationMap.insert(pair<int,int>(oldCode, newCode));  
      }

     } //end block for DCRC revC Queens iZIP



     // Modify for DCRC revC board readout for 100 mm HV prototype, UMN wiring #1 (used in Run 61)
     if(modificationType.compare("UMN100mmHVDCRCrevC_1") == 0) 
     {

       if(detNum > 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN100mmHVDCRCrevC_1!"
	      <<"\n There are more zips than expected (3), please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+10));  // DIB1 PD ->  PES2
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB2 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB2 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+8));   // DIB2 PA ->  Delete - no, pretend this is #8
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+6));   // DIB2 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+0));   // DIB2 PC ->  PAS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+9));   // DIB2 PD ->  Delete - no, pretend this is #9
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+11));  // DIB3 PA ->  Delete - no, pretend this is #11
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+7));   // DIB3 PB ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PFS1
      }

     } //end block for DCRC revC 100 mm HV prototype wiring #1


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #1 reading out Side 1 (used in Run 61)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_1") == 0) 
     {

       if(detNum != 1 && detNum != 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_1!"
	      <<"\n An unexpected zip number is present, there should only be zip 1 and zip 3, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));                  // DIB1 PD ->  Delete
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));                  // DIB3 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DIB3 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PF
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #1 


     // Modify for DCRC revC board readout for 100 mm HV prototype, UMN wiring #2 (used in Run 62)
     if(modificationType.compare("UMN100mmHVDCRCrevC_2") == 0) 
     {

       if(detNum == 3 || detNum > 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN100mmHVDCRCrevC_2!"
	      <<"\n An unexpected ZIP number was used, expecting 1,2,4. Please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+10));  // DIB1 PD ->  PES2
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB2 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB2 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+7));   // DIB2 PA ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+6));   // DIB2 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+0));   // DIB2 PC ->  PAS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+9));   // DIB2 PD ->  PDS2 (Not actually connected
      } else if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+8));   // DIB3 PA ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+11));  // DIB3 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PFS1
      }

     } //end block for DCRC revC 100 mm HV prototype wiring #2


     // Modify for DCRC revC board readout for 100 mm HV prototype, UMN wiring #3 (used in Run 75)
     if(modificationType.compare("UMN100mmHVDCRCrevC_3") == 0) 
     {

       if(detNum == 2 || detNum > 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN100mmHVDCRCrevC_3!"
	      <<"\n An unexpected ZIP number was used, expecting 1,3,4. Please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+10));  // DIB1 PD ->  PES2
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+8));   // DIB3 PA ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+11));  // DIB3 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PFS1
      } else if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB4 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB4 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+7));   // DIB4 PA ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+6));   // DIB4 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+0));   // DIB4 PC ->  PAS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+9));   // DIB4 PD ->  PDS2
      }

     } //end block for DCRC revC 100 mm HV prototype wiring #3


     // Modify for DCRC revC board readout for 100 mm HV prototype, UMN wiring #4 (used in Run 76)
     if(modificationType.compare("UMN100mmHVDCRCrevC_4") == 0) 
     {

       if(detNum == 2 || detNum > 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN100mmHVDCRCrevC_4!"
	      <<"\n An unexpected ZIP number was used, expecting 1,3,4. Please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(710, 1);
 

      if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PBS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+10));  // DIB1 PD ->  PES2
      } else if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+8));   // DIB3 PA ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+11));  // DIB3 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PCS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PFS1
      } else if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB4 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB4 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+7));   // DIB4 PA ->  PBS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+6));   // DIB4 PB ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+0));   // DIB4 PC ->  PAS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+9));   // DIB4 PD ->  PDS2
      }

     } //end block for DCRC revC 100 mm HV prototype wiring #4


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #2 reading out Side 1 (used in Run 62)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_2") == 0)
     // note the only change from HV Wiring #1 is that a different DCRC# (aka detNum) is used. 
     {

       if(detNum != 1 && detNum != 2)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_2!"
	      <<"\n An unexpected zip number is present, there should only be zip 1 and zip 2, please check raw data stream."
              <<"\n current detNum is" << detNum
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));                  // DIB1 PD ->  Delete
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));                  // DIB3 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DIB3 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PF
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #2 


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #3 reading out Side 1 (used in Run 69)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_3") == 0)
     // note the only change from HV Wirings #1,2 is that a different DCRC# (aka detNum) is used. 
     {

       if(detNum != 3 && detNum != 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_3!"
	      <<"\n An unexpected zip number is present, there should only be zip 3 and zip 4, please check raw data stream."
              <<"\n current detNum is" << detNum
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 3
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 3);
 

      if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB1 PA ->  PE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB1 PB ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB1 PC ->  PD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));                  // DIB1 PD ->  Delete
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));                  // DIB3 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DIB3 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB3 PC ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB3 PD ->  PF
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #3


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #4 reading out Side 1 (used in Run 70)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_4") == 0)
     // note the only change from HV Wirings #1,2,3 is that a different DCRC# (aka detNum) is used. 
     {

       if(detNum != 1 && detNum != 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_3!"
	      <<"\n An unexpected zip number is present, there should only be zip 1 and zip 4, please check raw data stream."
              <<"\n current detNum is" << detNum
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 1);
 

      if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB4 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB4 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+4));   // DIB4 PA ->  PE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+1));   // DIB4 PB ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+3));   // DIB4 PC ->  PD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));                  // DIB4 PD ->  Delete
      } else if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));                  // DIB1 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DIB1 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+2));   // DIB1 PC ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB1 PD ->  PF
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #4




     // Modify for DCRC revC board readout for 100 mm iZIP Pathfinder, iZIPv7, UMN wiring (used in Run 63)
     if(modificationType.compare("UMNiZIPv7DCRCrevC_1") == 0) 
     {

       if(detNum != 1 && detNum != 2 && detNum != 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNiZIPv7DCRCrevC_1!"
	      <<"\n An unexpected zip number is present, there should only be zip 1, 2, and 4, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(700, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+2));   // DIB1 QI ->  QIS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+3));   // DIB1 QO ->  QOS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+7));   // DIB1 PA ->  PDS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+9));   // DIB1 PB ->  PFS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+8));   // DIB1 PC ->  PES1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+11));  // DIB1 PD ->  PBS2
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+0));   // DIB2 QI ->  QIS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+1));   // DIB2 QO ->  QOS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+10));  // DIB2 PA ->  PAS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+12));  // DIB2 PB ->  PCS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+4));   // DIB2 PC ->  PAS1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+6));   // DIB2 PD ->  PCS1
      } else if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, 0));                  // DIB4 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB4 QO ->  Detete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+14));  // DIB4 PA ->  PES2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+15));  // DIB4 PB ->  PFS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, detCodeBaseNew+13));  // DIB4 PC ->  PDS2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+5));   // DIB4 PD ->  PBS1
      }

     } //end block for DCRC revC 100 mm iZIP Pathfinder, iZIPv7, UMN wiring (used in Run 63)



     // Modify for DCRC revC board readout of external trigger,(used in UMN Run 62)
     if(modificationType.compare("UMNExternalTrigger") == 0) 
     {

       if(detNum != 4)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNExternalTrigger!"
	      <<"\n An unexpected zip number is present, there should only be zip 4, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, leave the detector number unchanged
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(92, detNum);
 

      if (detNum==4) {  // DCRC 4
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, 0));			// DIB4 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));			// DIB4 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+0));	// DIB4 PA ->  P
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));			// DIB4 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));			// DIB4 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));			// DIB4 PD ->  Delete
      }

     } //end block for  DCRC revC board readout of external trigger,(used in UMN Run 62)


     // Modify for DCRC revC board readout of 150mm 5Q device default setup (used in UMN Run 65)
     if(modificationType.compare("UMN5Q_R65_1") == 0) 
     {

       if(detNum != 1 && detNum != 2 && detNum != 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN5Q_R65_1!"
	      <<"\n An unexpected zip number is present, there should only be zip 1, 2, and 3, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(800, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+0));   // DIB1 QI ->  QA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+2));   // DIB1 QO ->  QC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB1 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB1 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB1 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB1 PD ->  Delete
      } else if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, 0));   // DIB2 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+3));   // DIB2 QO ->  QD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB2 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB2 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB2 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB2 PD ->  Delete
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+1));   // DIB4 QI ->  QB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+4));   // DIB4 QO ->  QE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB4 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB4 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB4 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB4 PD ->  Delete
      }


     } //end block for DCRC revC board readout of 150mm 5Q device default setup,(used in UMN Run 65)


     // Modify for DCRC revC board readout of 150mm 5Q device alt setup (used in UMN Run 65)
	//DCRCs 1,2 are swapped from above
     if(modificationType.compare("UMN5Q_R65_2") == 0) 
     {

       if(detNum != 1 && detNum != 2 && detNum != 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMN5Q_R65_1!"
	      <<"\n An unexpected zip number is present, there should only be zip 1, 2, and 3, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(800, 1);
 

      if (detNum==2) {  // DCRC 2
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+0));   // DIB2 QI ->  QA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+2));   // DIB2 QO ->  QC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB2 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB2 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB2 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB2 PD ->  Delete
      } else if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, 0));   // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+3));   // DIB2 QO ->  QD
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB1 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB1 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB1 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB1 PD ->  Delete
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+0, detCodeBaseNew+1));   // DIB4 QI ->  QB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, detCodeBaseNew+4));   // DIB4 QO ->  QE
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, 0));   // DIB4 PA ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, 0));   // DIB4 PB ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   // DIB4 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, 0));   // DIB4 PD ->  Delete
      }


     } //end block for DCRC revC board readout of 150mm 5Q device alt setup,(used in UMN Run 65)


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #5 reading out Side 2 (used in Run 75)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_5") == 0) 
     {

       if(detNum != 1 && detNum != 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_5!"
	      <<"\n An unexpected zip number is present, there should only be zip 1 and zip 3, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 1);
 

      if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+2));   // DIB3 PA ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+5));   // DIB3 PB ->  PF
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   				// DIB3 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+4));   // DIB3 PD ->  Delete
      } else if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DIB1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DIB1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+1));   // DIB1 PA ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DIB1 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   				// DIB1 PC ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+3));   // DIB1 PD ->  PD
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #5 


     // Modify for DCRC revC and UMN HV board readout for 100 mm HV prototype, UMN HV wiring #6 reading out Side 2 (used in Run 76)
     if(modificationType.compare("UMNHVboard100mmHVDCRCrevC_6") == 0) 
     {

       if(detNum != 1 && detNum != 3)
       {
	 cerr <<"ERROR in RawDataReader::ModifyRawData while trying to modify for UMNHVboard100mmHVDCRCrevC_5!"
	      <<"\n An unexpected zip number is present, there should only be zip 1 and zip 3, please check raw data stream."
	      << endl;
	 exit(1);
       }

       // detector code base, mapping all zips to detector 1
      int detCodeBaseOld = ChannelMapHelper::CalcDetCodeBase(4, detNum);
      int detCodeBaseNew = ChannelMapHelper::CalcDetCodeBase(1710, 1);
 

      if (detNum==1) {  // DCRC 1
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DCRC1 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DCRC1 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+2));   // DCRC1 PA ->  PC
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+5));   // DCRC1 PB ->  PF
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   				// DCRC1 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+4));   // DCRC1 PD ->  PE
      } else if (detNum==3) {  // DCRC 3
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld, 0));                    // DCRC3 QI ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+1, 0));                  // DCRC3 QO ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+2, detCodeBaseNew+1));   // DCRC3 PA ->  PB
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+3, detCodeBaseNew+0));   // DCRC3 PB ->  PA
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+4, 0));   				// DCRC3 PC ->  Delete
	detCodeModificationMap.insert(pair<int,int>(detCodeBaseOld+5, detCodeBaseNew+3));   // DCRC3 PD ->  PD
      }

     } //end block for DCRC revC 100 mm HV prototype with UMN-HV board, wiring #6 


  }



 
 // ========== APPLY MODIFICATIONS  ==========
 

 //  ------ Apply changes to detector configuration -----

 if (fReadDetectorConfig && fDetectorConfigPtr->IsFilled() && !fDetectorConfigPtr->IsModified()) {
    
    // get current configMap from DetectorConfigData object
    map<int, map<string, double> >  detectorConfigMap = fDetectorConfigPtr->GetDetectorConfigMap();
      

    // 1. Change detector code using detCodeModificationMap (if needed)
   
    // loop through detCode modification map
    map<int, int>::iterator detCodeModMapItr = detCodeModificationMap.begin();
    for( ; detCodeModMapItr != detCodeModificationMap.end(); detCodeModMapItr++) {
  
      int detCodeOld = detCodeModMapItr->first;
      int detCodeNew = detCodeModMapItr->second;

      // find element in detectorConfigMap
      map< int, map<string, double>  >::iterator configMapItr = detectorConfigMap.find(detCodeOld);
      
      if (detCodeNew==0)
        detectorConfigMap.erase(configMapItr);
      else {
        detectorConfigMap.insert(configMapItr, pair< int, map<string, double> >(detCodeNew, configMapItr->second));
        detectorConfigMap.erase(configMapItr); 
      }
     }
   

    // 2. Change configuration using detConfigModificationMap

    // loop through detConfig modification map
    map<int, map<string, double> > ::iterator detConfigModMapItr = detConfigModificationMap.begin();
    for( ; detConfigModMapItr !=  detConfigModificationMap.end(); detConfigModMapItr++) {
  
      int detCode  = detConfigModMapItr->first;
      map<string,double> configModMap = detConfigModMapItr->second;

      // find element in detectorConfigMap
      map< int, map<string, double>  >::iterator configMapItr = detectorConfigMap.find(detCode);
      map<string,double> configMap = configMapItr->second;

      // loop configModMap to find parameter that need modifation
      map<string,double>::iterator parItr =  configModMap.begin();
      for( ; parItr !=  configModMap.end(); parItr++) 
           configMap[parItr->first] = parItr->second;  // replace value (or add parameter if doesn't exist...)
  
     // insert modified map detectorConfigMap
     detectorConfigMap[detCode] = configMap;
    }


   
    // 3.  Now set new confiMap in DetectorConfigData object
    fDetectorConfigPtr->SetDetectorConfigMap(detectorConfigMap,true);

    // display
    cout << "WARNING in RawDataReader::ModifyRawData: Configuration map has been modified!" << endl;

 }

 

 //  ----- Apply changes to PulseData map ----- 

 //note this parameter is not true if only the detector code
 //changes, but not the detetor number!
 bool isDetNumModified = false; 

 if (fReadZipPulses && !fIsZipPulsesModified) {

  // let's loop through each detector and replace vector of PulseData 
  // by a new one if needed (pulses have to be  in the correct order...)

  map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulsesPtr->begin();
  while(mapItr != fMapOfZipPulsesPtr->end()) {
     
      // Detector number 
      int detNum = mapItr->first;
      
      // check if there are any informations in detCodeModificationMap
      // for this detector
      bool isCodeModified = false;

      map<int, int>::iterator detCodeModMapItr = detCodeModificationMap.begin();
      for( ; detCodeModMapItr != detCodeModificationMap.end(); detCodeModMapItr++) {
         int detCodeOld = detCodeModMapItr->first;
         int detNumOld = ChannelMapHelper::GetDetNumFromCode(detCodeOld);
 
         if (detNumOld==detNum)
              isCodeModified = true;
      }

      // If modification not needed, continue detector loop 
      if (!isCodeModified) {
           ++mapItr;
          continue;
      }
 
      // Let's define new vector list, which will replace old one
      vector<PulseData> zipPulseListNew;

      // intialize zipEndCode and (needed to sort pulse vector)
      int zipEndCode = 0;

      //Get the pulse list for this zip
      vector<PulseData>* zipPulseList = &(mapItr->second);

      // loop pulses old vector
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
        {
	  PulseData aPulseData = (*zipPulseList)[pulseItr];
           
          // get detector code
 	  int detCode =  aPulseData.GetDetectorCode();
   
          // check if deCode in defined detCodeModificationMap
          map<int, int>::iterator detCodeModMapItr = detCodeModificationMap.find(detCode);

          // if new detector code available -> modify PulseData 
          int  detCodeNew = detCode;

          if(!(detCodeModMapItr == detCodeModificationMap.end())) 
            {
	      detCodeNew = detCodeModMapItr->second;
	      int detNumNew =  ChannelMapHelper::GetDetNumFromCode(detCodeNew);

	      if (detCodeNew!=0) 
	      {
		aPulseData.SetChannelConfig((uint32_t)detCodeNew);
		
		//flip the adc bins, but only for UMN DCRCrevC data! 
		map<int, string>::const_iterator searchModItr = modificationMap.find(detNum);
		string modificationType = searchModItr->second;

		//for MIDAS data this is done at a lower level
		if((modificationType.compare("UMNiZIP100mmDCRCrevC") == 0) && !fIsMidasData)
		{
		  aPulseData.FlipADCRawPulse();
		}

	      }

	      //now save flag if detector number changes
	      if(detNumNew != detNum)
	      {
		isDetNumModified = true;

		// cout <<"Found modified detector code! " 
		// 			 <<"\n old num = " << detNum
		// 			 <<"\n new num = " << detNumNew
		// 			 << endl;
	      }
            }

          // now let's store pulseData in the new vector in the right order
          if (detCodeNew==0) continue;  // if 0 -> NOT storing data

         
          //storing zip pulses in order according to detCode
          if(detCodeNew > zipEndCode)
            {
	       //pulse is in anticipated order, so add to the end
	       zipPulseListNew.push_back(aPulseData);
	       zipEndCode = detCodeNew; //detCode of last pulse in list
               
             } else {
	 
               //search backwards through the stored pulses to find the right place
	       //for this pulse according to its detector code
	       vector<PulseData>::iterator backItr = zipPulseListNew.end() - 1; //to start at the last element
	       for( ; backItr != zipPulseListNew.begin(); backItr--)
	          if(detCodeNew > backItr->GetDetectorCode()) { break; }
	           
	       //store *after* the first pulse with DetCode < than current
	       if(detCodeNew > backItr->GetDetectorCode()) { backItr++; }
	 
	       //now the pulse *after* the last position found
	        zipPulseListNew.insert(backItr, aPulseData);
             }
                  
        }  // end loop all pulses
     

    // replace vector or detele map if no pulses
    if (zipPulseListNew.empty()) 
    {
       fMapOfZipPulsesPtr->erase(mapItr++);
    } else 
    {
       *zipPulseList  = zipPulseListNew;
       ++mapItr;
    }


    // data modified on at least one detector
    fIsZipPulsesModified = true;
  
  } //end loop over fMapOfZipPulses


  //-----------------------------------------------------------
  
  //skip rest of this block if detector numbers for pulses don't change 
  //The followign was added by LLH to reorganized the pulses from
  //revC DCRC board being run at UMN

  if(isDetNumModified)
  {  
    // Loop second time over fMapOfZipPulses, reorganize by det number now that they have the correct codes

    mapItr = fMapOfZipPulsesPtr->begin();
    while(mapItr != fMapOfZipPulsesPtr->end()) {
     
      // Old detector number
      int detNumOldMap = mapItr->first;

      //Get the pulse list according to the old detector number
      vector<PulseData>* zipPulseList = &(mapItr->second);
      uint initialSize = zipPulseList->size();

      // loop pulses in existing vector, do it backwards because 
      // we're going to delete entries
      for(int pulseItr = initialSize-1; pulseItr >= 0; pulseItr--)
      {
	PulseData aPulseData = (*zipPulseList)[pulseItr];
        
	// get detector number stored for this pulse
	int detNumOfPulse =  aPulseData.GetDetectorNum();
	int detCodeOfPulse = aPulseData.GetDetectorCode();

	// check whether the detector number matches the key for the zip map 
	// and skip if the detector codes match...
	// ..otherwise, move the pulse into the correct pulse list for its zip
	if(detNumOfPulse == detNumOldMap)
	{
	  continue;
	}

	// 	cout <<"Found pulse associated with wrong detector in the zip map, reorganizing!" 
	// 	     <<"\ndetector number of pulse is: " << detNumOfPulse
	// 	     <<"\ndetector number of key is : " << detNumOldMap
	// 	     << endl;


	//remove it from the current list because it doesn't belong here (wrong detNum)
	zipPulseList->erase(zipPulseList->begin()+pulseItr);
	
	//now find the one it belongs in and insert it there
	map< int, vector<PulseData> >::iterator belongsHereMapItr = fMapOfZipPulsesPtr->find(detNumOfPulse);
	if(belongsHereMapItr != fMapOfZipPulsesPtr->end())
	{
	  
	  vector<PulseData>* belongsHereZipPulseList = &(belongsHereMapItr->second);

	  //search backwards through the stored pulses to find the right place
	  //for this pulse according to its detector code
	  vector<PulseData>::iterator backItr = belongsHereZipPulseList->end() - 1; //to start at the last element
	  for( ; backItr != belongsHereZipPulseList->begin(); backItr--)
	  { 
	    if(detCodeOfPulse > backItr->GetDetectorCode()) { break; }
	  }
         
	  //store *after* the first pulse with DetCode < than current
	  if(detCodeOfPulse > backItr->GetDetectorCode()) { backItr++; }
	 
	  //now insert the pulse *after* the last position found
	  belongsHereZipPulseList->insert(backItr, aPulseData);
	}  


      } //done inserting pulse into new pulse list !

      ++mapItr;

    } //end second loop over fMapOfZipPulses


    //Looping for a third time to remove zip entries with empty pulse lists
    mapItr = fMapOfZipPulsesPtr->begin();
    while(mapItr != fMapOfZipPulsesPtr->end()) {
           
      //Get the pulse list according to the old detector number
      vector<PulseData>* zipPulseList = &(mapItr->second);
      
      //remove if no pulses for this zip
      if(zipPulseList->size() == 0)
	fMapOfZipPulsesPtr->erase(mapItr++);
      else
	++mapItr;

    } //end third loop over fMapOfZipPulses

    //Looping for a fourth time to check the results of reorganization
    //commented out, but kept around for debugging purposes - LLH
//     mapItr = fMapOfZipPulsesPtr->begin();
//     while(mapItr != fMapOfZipPulsesPtr->end()) {
      
//       // Old detector number
//       int detNumOldMap = mapItr->first;
      
//       // Loop over pulse list
//       //Get the pulse list according to the old detector number
//       vector<PulseData>* zipPulseList = &(mapItr->second);
//       uint initialSize = zipPulseList->size();
      
//       for(int pulseItr = 0; pulseItr < initialSize; pulseItr++)
// 	{
// 	  PulseData aPulseData = (*zipPulseList)[pulseItr];
// 	  int detCodeOfPulse = aPulseData.GetDetectorCode();
	  
// 	  cout <<"\ndetector code of pulse is: " << detCodeOfPulse
// 	       << endl;
// 	}

//       ++mapItr;
      
//     } //end fourth loop over fMapOfZipPulses


  } //  --------------- end if isDetNumModified  --------------------


  // display warning
  if (fIsZipPulsesModified)
    cout << "WARNING in RawDataReader::ModifyRawData: Pulse data map has been modified!" << endl;

 } //end block for if zip pulses modified


 return;

}
bool RawDataReader::ParseSeriesAndDumpFromRawPath()
{
  cout << "The file is: " << fRawDataPath << endl;

  size_t pos = fRawDataPath.rfind("/");
  string filename;

  if(pos != string::npos)
    filename = fRawDataPath.substr(pos+1,fRawDataPath.size()-(pos+1));
  else
    filename = fRawDataPath;

  //cout << "The stripped filename is: " << filename << endl;
  
  //do some regex matching to parse out series and dump
  regex_t regex;
  int reti = regcomp(&regex,"([0-9]+)_([0-9]+)_F([0-9]+)",REG_EXTENDED);

  if(reti){
    cout << "RawDataReader::ParseSeriesAndDumpFromRawPath: WARNING could not create regex for series and dump, not getting them from filename, OK if the info is in the data stream" << endl;
    return false;
  }

  regmatch_t matchptr[4];
  reti = regexec(&regex,filename.c_str(),4,matchptr,0);



  if(!reti){
    //got a match

    //series number
    istringstream series(filename.substr(matchptr[1].rm_so,matchptr[1].rm_eo-matchptr[1].rm_so)+filename.substr(matchptr[2].rm_so,matchptr[2].rm_eo-matchptr[2].rm_so));
    series >> fMidasSeriesNumber;

    //dump number
    istringstream dump(filename.substr(matchptr[3].rm_so,matchptr[3].rm_eo-matchptr[3].rm_so));
    dump >> fMidasDumpNumber;

    //cout << "Midas Series: " << fMidasSeriesNumber << endl;
    //cout << "Midas Dump: " << fMidasDumpNumber << endl;
  }
  else if(reti == REG_NOMATCH){
    cout << "RawDataReader::ParseSeriesAndDumpFromRawPath: WARNING could not parse filename for series and dump, not getting them from filename, OK if the info is in the data stream" << endl;
    return false;
  }
  else {
    char msgbuf[100];
    regerror(reti,&regex,msgbuf,sizeof(msgbuf));
    string smsgbuf(msgbuf);
    cout << "RawDataReader::ParseSeriesAndDumpFromRawPath: WARNING could not parse filename for series and dump, not getting them from filename, OK if the info is in the data stream" << endl;
    cout << "RawDataReader::ParseSeriesAndDumpFromRawPath: WARNING regex failed with: " << smsgbuf << endl;
    return false;
  }


  return true;
}

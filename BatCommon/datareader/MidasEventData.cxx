///////////////////////////////////////////////////////////////////////
//
//  Class Name:         MidasEventData
// 
//  Author (s):         B. Serfass using 
//                           TMidasEvents (Konstantin Olchanski - TRIUMF)
//                           TTowerData (dcrcDisplay) (Thomas Lindner - TRIUMF)
//
//  Description:       C++ class representing one midas event. Data members are filled 
//                     by reading midas events from a file or by reading
//                     them from a midas shared memory buffer 
//
//
///////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "MidasEventData.h"

//wap: included for debugging
#include <bitset>


// =================================================================
// Constructor/Destructor
// =================================================================


MidasEventData::MidasEventData() :
   fdiagnosticPrints(false),
   fverbosity(0)
{
  fData = NULL;
 
  fBanksN = 0;
  fBankList = NULL;

  fEventHeader.fEventId      = 0;
  fEventHeader.fTriggerMask  = 0;
  fEventHeader.fSerialNumber = 0;
  fEventHeader.fTimeStamp    = 0;
  fEventHeader.fDataSize     = 0;

  fNbTrigger = 0;

  fCurrentEventNumber = 0;
  fCurrentTriggerData = NULL;
  fRequestId = -1;

  fDebugOn = false;
}

void MidasEventData::Copy(const MidasEventData& rhs)
{
  fEventHeader = rhs.fEventHeader;

  fData        = (char*)malloc(fEventHeader.fDataSize);
  assert(fData);
  memcpy(fData, rhs.fData, fEventHeader.fDataSize);
 
  fBanksN      = rhs.fBanksN;
  fBankList    = strdup(rhs.fBankList);
  assert(fBankList);
}

MidasEventData::MidasEventData(const MidasEventData &rhs)
{
  Copy(rhs);
}

MidasEventData::~MidasEventData()
{
  //Clear();
}

MidasEventData& MidasEventData::operator=(const MidasEventData &rhs)
{
  if (&rhs != this)
    Clear();

  this->Copy(rhs);
  return *this;
}

void MidasEventData::Clear()
{

  // clear bankList
  if (fBankList)
    free(fBankList);
  
  fBankList = NULL;
  fBanksN = 0;

  // clear record data
  if (fData)
    free(fData);
  fData = NULL;


  // clear header
  fEventHeader.fEventId      = 0;
  fEventHeader.fTriggerMask  = 0;
  fEventHeader.fSerialNumber = 0;
  fEventHeader.fTimeStamp    = 0;
  fEventHeader.fDataSize     = 0;
  
  
  // delete TriggerData
  fNbTrigger = 0;

  for(uint i = 0; i < fTriggerDataList.size(); i++)
    delete fTriggerDataList[i];
  
  fTriggerDataList.clear();
  fCurrentTriggerData = NULL;
  fRequestId = -1;
}

// =================================================================
// Get Functions
// =================================================================


TMidas_EVENT_HEADER * MidasEventData::GetEventHeader()
{
  return &fEventHeader;
}



bool MidasEventData::IsGoodSize() const
{
  return fEventHeader.fDataSize > 0 && fEventHeader.fDataSize <= 500 * 1024 * 1024;
}



vector<PulseData> MidasEventData::GetPulseDataList(int triggerNumber)
{
  
  vector<PulseData> pulseDataList;
  for(uint trigIt = 0; trigIt < fTriggerDataList.size(); trigIt++){
    MidasTriggerData* triggerData = fTriggerDataList[trigIt];
    if (triggerData->GetTriggerNumber()==triggerNumber-1) {
      pulseDataList = triggerData->fPulseDataList;
      break;
    }
    if(fverbosity>4)
      cout << "MidasEventData::GetPulseDataList INFO looking for trigger " << triggerNumber 
	   << " and found " << triggerData->GetTriggerNumber() << endl;
  }
  if(fdiagnosticPrints)
    cout << "MidasEventData::GetPulseDataList INFO returning " << pulseDataList.size() << " pulses" << endl;
    
  return pulseDataList;
}


int MidasEventData::GetEventCategory(int triggerNumber)
{
  for(uint trigIt = 0; trigIt < fTriggerDataList.size(); trigIt++){
    MidasTriggerData* triggerData = fTriggerDataList[trigIt];
    if (triggerData->GetTriggerNumber()==triggerNumber-1) {
      return triggerData->GetEventCategory();
    }
  }
   
  cerr << "MidasEventData::GetEventCategory: ERROR! could not find requested trigger" << endl;
  return -1;
}
  

int MidasEventData::IsStaleTrigger(int triggerNumber)
{

  //check if valid request
  if(fNbTrigger<triggerNumber){
    cerr << "MidasEventData::IsStaleTrigger: ERROR!  request status of nonexistant trigger number for this event" << endl;
    return -1;
  }
    
  // get pulse map for that trigger
  vector<PulseData> pulselist =  GetPulseDataList(triggerNumber);
  if(pulselist.size()==0)
    return 1;
     
  int stale = 1; //assume stale
  //check to see if the pulse list actually contains data
  for(uint i = 0; i < pulselist.size(); i++){
			 
    if((pulselist[i].GetRawPulse().size())>0){
      stale = 0;
      break;
    }
  }

  return stale;
       
}




// =================================================================
// Read Data (from File)
// =================================================================

int MidasEventData::ReadEventHeader(gzFile& localRawDataPtr)
{
  
   // MidasEvent Header
   //     EventID:  Begin run  = 0x8000
   //               End Run    = 0x8001
   //          
  
   // Returns -1 for error, and 0 for end of file
  int readCheck = gzread(localRawDataPtr, (char*)GetEventHeader(), sizeof(TMidas_EVENT_HEADER));
  
  if (readCheck <0 || readCheck != sizeof(TMidas_EVENT_HEADER))
    {
      cerr <<"MidasEventData::ReadEventHeader: ERROR problem reading midas file!"<<endl;
      exit(1);
    }
  
  if (readCheck==0)
     {
       cerr <<"MidasEventData::ReadEventHeader: ERROR:  End of midas file!"<<endl;
       exit(1);
     }
  
   
  // check endianness 
  uint32_t endian = 0x12345678;
  bool DoByteSwap = *(char*)(&endian) != 0x78;
  
  if (DoByteSwap) SwapBytesEventHeader();
  
  if (!IsGoodSize())
    { 
      cerr <<"MidasEventData::ReadEventHeader  ERROR:  Wrong MidasEvent header size!"<<endl;
      exit(1);
    }
  
  return readCheck;
}




int MidasEventData::ReadDataRecord(gzFile& localRawDataPtr)
{
  // Read data
  int readCheck = gzread(localRawDataPtr, GetData(), GetDataSize());
  
  if (readCheck != (int)GetDataSize())
    {
      cerr <<"MidasEventData::ReadRawDataRecord ERROR:  Wrong MidasEvent data size!"<<endl;
      exit(1);
    }
  
  // Endianness 
  SwapBytes(false);
  
  
  // Store data
  int eventId = fEventHeader.fEventId;
  
  if ((eventId & 0xFFFF) == 0x8000) {// Begin Run

    Print();
    
  } else if ((eventId & 0xFFFF) == 0x8001) {   // End Run
  
    Print();
  
    cout << "MidasEventData: Reached end of file!" << endl;
    return 0;

    
  } else {  // all other events
    
  
    // Set Bank
    SetBankList();
    
    // Handle data
    bool success= HandleBankData(); 
    
    if (~success) return -1;  
  }
  
  return readCheck;

}


// =================================================================
// Read Data (Online / SYSTEM BUFFER)
// =================================================================

#ifdef HAVE_MIDAS

// Read Event from buffer
int MidasEventData::ReadEventOnline(TMidasControl* midas)
{

  bool success = false;
  
  // clear previous event containers
  if(fTriggerDataList.size()>0){
    for(int i =0; i < fTriggerDataList.size(); i++){
      MidasTriggerData *myTrigger = fTriggerDataList[i];
      myTrigger->ClearPulseDataList();
      delete myTrigger;
    }
  }
  
  fTriggerDataList.clear();
  fNbTrigger = 0;
  fCurrentTriggerData = NULL;
    
  // check if midas instance available
  if (midas==0) {
    fprintf(stderr,"ERROR in MidasEventData: No MIDAS instance available \n");
    return -1;
  }
  
  // get Midas Event Limit
  double eventLimit = midas->GetEventLimit();
  
  // reqister event requests (use callback, request only once)
  if (fRequestId==-1)
    fRequestId = midas->eventRequest("SYSTEM",1,-1,(1<<2),true);

  if (fRequestId==-2) {
    fprintf(stderr,"ERROR in MidasEventData: Problem with Midas Event request. Sopping processing... \n");
    return -1;
  }

  // loop until we get an event
  while (1)
    {             
      int pevent_size = 25600000;//bytes
      char* pevent = new char[pevent_size];
        
      int size = midas->receiveEvent(fRequestId, pevent, pevent_size, true);
      
      // no event received
      if (size == 0)
	{
	  // check if shutdown or abort
	  // only poll for 1ms, because this is a time
	  //during which the GUI freezes.
	  if (!midas->poll(1)) {
	    delete[] pevent;
	    return -1;
	  }

	  // check if run stopped (in case stopped by hand using
	  // web interface) 
	  if (midas->GetRunState()==1) {
	    delete[] pevent;
	    return -1;
	  }

	  // check limit
	  if (eventLimit!=0 && fCurrentEventNumber>=eventLimit) {
	    delete[] pevent;
	    return -1;
	  }

          // FIXME: Add Time limit
	  delete[] pevent;
	  continue;
	}
      
      if (size <= 0) {
	delete[] pevent;
	return -1;
      } else
	fCurrentEventNumber++;
      
      // display
      //printf("Processing Midas Event %d\n",(int)fCurrentEventNumber);
      
    
      // Get pulses
      memcpy(GetEventHeader(), pevent, sizeof(TMidas_EVENT_HEADER));
      SetData(size, pevent+sizeof(TMidas_EVENT_HEADER));
      SetBankList();
      
      // Store pulses in map
      //Find
      success = HandleBankData();
      
      // cleanup
      delete[] pevent;
      
      if (success)
	break;
      else 
	return 0;
      
    }
  
  return 1;
}

#endif  



// =================================================================
// Handle/Store Bank Data 
// =================================================================
bool MidasEventData::HandleBankData()
{
  bool success = false;

  // Get Tower bank
  int bankLength = 0;
  int bankType = 0;
  void *pdata = 0;
  int found = FindBank("SCD0", &bankLength, &bankType, &pdata);
      
  if (found==0) {
    if (fdiagnosticPrints)
      cout <<"MidasEventData::HandleBankData WARNING:  No data record found!" << endl;
    return success;
  } 
  
  // reinterpret to unint32_t
  const uint32_t* buffer = reinterpret_cast<const uint32_t*>(pdata);

  bool lgcRevC = 0;
  bool lgcRevD = 0;
  //check whether RevC or RevD;
  if((buffer[0] & 0xf0000000) == 0xc0000000){
    lgcRevC = 1;
  }
  else if((buffer[0] & 0xf0000000) == 0x90000000){
    lgcRevD = 1;
  }
  else{
    cerr << "MidasEventData::ReadRawDataRecord ERROR: first header word is missing correct leading bits" << endl;
    exit(1);
  }
  
  if(lgcRevC){
    success = HandleBankDataRevCPreBackport(buffer);
  }
  else if(lgcRevD){
    success = HandleBankData(buffer);
  }
  else{
    success = true;
  }

  success=true;
  return success;
  
}


bool MidasEventData::HandleBankDataRevCPreBackport(const uint32_t* buffer)
{
  bool success = false;

  // %%%%% FIXME FIXME FIXME  %%%%%
  // Let hardcode a few parameters for now
  uint nbChannels  = 6;   // 6 channels only
  int detType = 4;  // Detector type
  uint32_t sampleDt = 800;
  int32_t triggerT0 = 0;  
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // Number of trigger
  int triggerRecordLength = buffer[0]& 0xfffffff;
    
  // Loop over triggers
  int index = 1;
  int trg_number =0;
  bool isstale=false;
  int nstale = 0;
  fCurrentTriggerData = NULL;

  for(int itRecord = 0 ; itRecord < triggerRecordLength; itRecord++){
    
    if((buffer[index] & 0xc0000000) != 0x40000000){
      cerr << "MidasEventData::HandleBankDataRevCPreBackport ERROR: trigger word is missing correct leading bits" << endl;
      exit(1);
    }
  
    // Get trigger information      
    int trg_src_tower = ((buffer[index+1]) & 0xffff)/ 8;
    int trg_src_dcrc = ((buffer[index+1]) & 0xffff) % 8;
    int trg_word = ((buffer[index+2]));
  
    //very often, only print at high verbosity and diagnostic prints on
    if(fverbosity>4 && fdiagnosticPrints)
      cout << "MidasEventData::HandleBankDataRevCPreBackport INFO got a trigger with: source tower = " << 
	trg_src_tower << ", source dcrc = " << 
	trg_src_dcrc << ", trigger word = " << 
	trg_word << endl;

    uint32_t trgoriginmask = (0x1<<20);
    //cout << "Trigger Type: " << ((trgoriginmask & trg_word)>>20) << endl;
    uint32_t EventCategory = ((trgoriginmask & (buffer[index+1]))>>20);
    
    // Check if triggerData already created
    bool createNewTrigger = true;
    if (fCurrentTriggerData!=NULL) 
      {
	if (TriggerMatch(fCurrentTriggerData,trg_src_tower,trg_src_dcrc,trg_word)) {
	  // same trigger as last time so no need to create one
	  createNewTrigger = false;
	} else {
	  // not same as last trigger, however double check it is not in the list
	  // earlier... (Is it necessary?)
	  if(fTriggerDataList.size()>0) {
	    for(uint trigIt = 0; trigIt < fTriggerDataList.size(); trigIt++){
	      if(TriggerMatch(fTriggerDataList[trigIt],trg_src_tower,trg_src_dcrc,trg_word)){
		// found trigger
		createNewTrigger = false;
		fCurrentTriggerData = fTriggerDataList[trigIt];
	      break;
	      }	     
	    }
	  }
	}
      }
    
    // create a new Trigger data if needed
    if (createNewTrigger) {
      //only push back the old one when you get a new one, avoid empty (stale) triggers
      if (fCurrentTriggerData!=NULL && !isstale){
        trg_number++; // incremente trigger number
        fTriggerDataList.push_back(fCurrentTriggerData);

        if((fCurrentTriggerData->fPulseDataList).size()==0)
          cerr << "MidasEventData::HandleBankDataRevCPreBackport ERROR (fatal): pushing back a trigger with zero size onto the trigger list" << endl;
      }
      fCurrentTriggerData = new MidasTriggerData(trg_src_tower,trg_src_dcrc,trg_word,trg_number);
      fCurrentTriggerData->SetEventCategory(EventCategory);
     
      //if we found this trigger to be stale and didn't push it back onto the fTriggerDataList, increment stale counter 
      if(isstale)
        nstale++;

      //reset stale-ness
      isstale=false;
    }

    // now get pulses....
    
    index += 3; // Skip the trigger header word, trigger source word,  and the trigger address word.
    
    if((buffer[index] & 0xe0000000) != 0x20000000){
      cerr << "MidasEventData::HandleBankDataRevCPreBackport ERROR: DCRC/tower identifier word is missing correct leading bits" << endl;
      exit(1);
    }	
    
    // Tower/DRCR Number -> detector number 
    int tower = ((buffer[index]) & 0xffff)/ 8;
    int dcrc = ((buffer[index]) & 0xffff) % 8;       
    int detNum = dcrc+6*(tower-1); // 6 dcrc per tower
      
    index += 1; // Skip the DCRC/tower identifier word
    
    // Now start loop over six channels
    for(uint ichan = 0; ichan < nbChannels; ichan++){
      
      if((buffer[index] & 0xc0000000) != 0x00000000 ||
	 ((buffer[index] & 0x00070000) >> 16) != ichan)
	cerr << "MidasEventData::HandleBankDataRevCPreBackport ERROR: channel word is missing correct leading bits" << endl;
      
      int numberWords = buffer[index] & 0xffff;

      //check for stale-ness
      if(numberWords==0){
	if(fdiagnosticPrints)
	  cout << "MidasEventData::HandleBankDataRevCPreBackport INFO: got an empty trigger" << endl;
        isstale=true;
      }
     
      // Now start loop over the samples.
      index++;
      vector<double> pulseVect;
      for(int j = 0; j < numberWords; j++){
	double lower_sample = (double) (buffer[index] & 0xffff);
	double upper_sample = (double) ((buffer[index] & 0xffff0000) >> 16);
	pulseVect.push_back(lower_sample);
	pulseVect.push_back(upper_sample);
	index++;	
      }      

      // Build detector code
      //very important to get this right!
      //channel map helper uses this
      //detector type needs to be 700 (kiZIPSNOLAB)
      //also look in BatRootType
      uint32_t detCode = (detType*1000000 + detNum*1000) + ichan;  // following CDMS numbering
      
      // Store in Pulse Data
      PulseData tempPulseData;
      if(ichan<2)
        sampleDt=400;

      tempPulseData.SetRawPulseRecord(detCode, pulseVect, sampleDt, triggerT0);
    
      //set sample dt back for phonons
      sampleDt=800;

      // Add pulse data in TriggerData
      (fCurrentTriggerData->fPulseDataList).push_back(tempPulseData);
    }
   
    if(fverbosity>3)
      cout << "MidasEventData::HandleBankDataRevCPreBackport INFO size of fCurrentTriggerData is " << (fCurrentTriggerData->fPulseDataList).size() << endl;

    if (isstale && fCurrentTriggerData!=NULL) {
      delete fCurrentTriggerData;
      fCurrentTriggerData=NULL;
    }

  }

  //gotta push back the last one if we're not stale
  if (fCurrentTriggerData!=NULL && !isstale){
     trg_number++; // incremente trigger number
     fTriggerDataList.push_back(fCurrentTriggerData);

     if((fCurrentTriggerData->fPulseDataList).size()==0)
       cerr << "MidasEventData::HandleBankDataRevCPreBackport ERROR (fatal): pushing back a trigger with zero size onto the trigger list" << endl;
  }

  //if the last one is stale, up the counter
  if(isstale) nstale++;

  // check Number of trigger 
  if (trg_number+nstale != (int)fTriggerDataList.size()+nstale)
    { 
      cout << "WARNING in MidasEventData::HandleBankDataRevCPreBackport: length of TriggerDataList " << endl;
      cout << "doesn't much number of trigger, raw data has " 
	   << trg_number << " triggers, " << nstale 
	   << " stale triggers, and pushed back " << fTriggerDataList.size() 
	   << " onto trigger list! " <<endl;
    }
  fNbTrigger = fTriggerDataList.size();  
  
  success=true;
  return success;

}

bool MidasEventData::HandleBankData(const uint32_t* buffer)
{
  
  //fDebugOn is set to false in the constructor but appears
  //to stay as true. manualy set false here
  fDebugOn =  false;
  
  //wap:print the first n lines of the buffer
  if(fDebugOn){
    for(int iii = 0; iii<500; iii++){
      std::bitset<32> x(buffer[iii]);
      cout << "buffer[" << iii << "] = " << x << endl;
    }
   }
  
  bool success = false;
  
  // %%%%% FIXME FIXME FIXME  %%%%%
  // Lets hardcode a few parameters for now
  uint32_t sampleDt = 800; //in ns sec. different between phonon/charge
  int32_t triggerT0 = 0;  
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  
  

  if((buffer[0] & 0xf0000000) != 0x90000000){
    cerr << "MidasEventData::HandleBankData ERROR: first header word is missing correct leading bits" << endl;
    //return success;
    exit(1);
  }	

  //the formatVersion is the 12th-27th bit of the header word
  int formatVersion = ((buffer[0]& 0xffff000) >> 12);
  if(fDebugOn)
    cout << "MidasEventData::HandleBankData: formatVersion =" << formatVersion << endl;
 
  
  //Number of trigger
  //wap: RevD data format the first 12 bits contain the number
  //of triggers. Latest readoutfe contain one trigger per event
  int triggerRecordLength = buffer[0]& 0xfff;
  if(fDebugOn)
    cout << "MidasEventData::HandleBankData: triggerRecordLength =" << triggerRecordLength << endl;
  
  // Loop over triggers
  int index = 1;
  int trg_number =0;
  for(int itRecord = 0 ; itRecord < triggerRecordLength; itRecord++){
    
    if((buffer[index] & 0xf0000000) != 0x50000000){
      cerr << "MidasEventData::HandleBankData ERROR: trigger word is missing correct leading bits" << endl;
      exit(1);
    }

    //wap : perhaps standard to have &0xffff here
    int triggerID = buffer[index+1];
    int triggerType  = buffer[index+2];
    // Get trigger information
    
    // Check if triggerData already created
    bool createNewTrigger = true;
    if (createNewTrigger) {

      //wap: longer term
      //this is a brute force use of MidasTriggerData
      //should create a new (overloaded) function or one that
      //is designed for one trigger per event
      fCurrentTriggerData = new MidasTriggerData(triggerID, triggerType, triggerType, trg_number);

      //use trigger type to set event category
      if(triggerType==2 || triggerType==3 || triggerType==4 || triggerType==5) //1:generic random; 2:BORR; 3:IRR; 4:EORR; 5:not used
        fCurrentTriggerData->SetEventCategory(1);  //RQs only know one kind of random
      else
        fCurrentTriggerData->SetEventCategory(triggerType);

      fTriggerDataList.push_back(fCurrentTriggerData);
      //fTriggerDataList.push_back(new MidasTriggerData(triggerID, triggerType, triggerType, trg_number));
      trg_number++; // increment trigger number
    }

    // now get pulses....
     
    index += 13; // Skip the trigger header word, trigger source word,  and the trigger address word.

    //get the number of detectors
    if((buffer[index] & 0xf0000000) != 0x30000000){
      std::bitset<32> x(buffer[index]);
      cerr << "MidasEventData::HandleBankData ERROR: Numer of detectors missing. buffer[" << index << "] = " << x << endl;
      exit(1);
    } 
    uint32_t ndets = (buffer[index] & 0xfffffff); 
    if(fDebugOn)
      cout << "MidasEventData::HandleBankData INFO: Numer of detectors: " << ndets << endl;

    index++;

    for(uint idet = 0; idet < ndets; idet++){
    
    //wap: why is this 0xe and not 0xf?
    if((buffer[index] & 0xe0000000) != 0x20000000){
      std::bitset<32> x(buffer[index]);
      cerr << "MidasEventData::HandleBankData ERROR: DCRC/tower identifier word is missing correct leading bits. buffer[" << index << "] = " << x << endl;
      exit(1);
    } 
    
    // Tower/DRCR Number -> detector number 
    int tower = ((buffer[index]) & 0xffff)/ 8;
    int dcrc = ((buffer[index]) & 0xffff) % 8;       
    
    // 0x3 = 11[bin]
    int detIndex = ((buffer[index] & 0x3)); 
    // 0x3c = [1111][1111][00][bin]
    int detID = ((buffer[index] & 0x3fc) >> 2); 
    
    // wap:should be 700 for SNOLAB iZIP
    // 0x3fffc00 = [1111][1111][1111][1111][0000][0000][00][bin]
    int detType = ((buffer[index] & 0x3fffc00) >> 10); 

    // pre-event-builder DCRC identifier is the detID
    int detNum = detID;
    
    if(fDebugOn)
      cout << "MidasEventData::HandleBankData: detIndex =" << detIndex << ". detID = " << detID << ".detType = " << detType << ". detNum = " << detNum << endl;
    
    index +=1; //index now is 16 to get dcrc serial number and version
    int dcrcSN1 = ((buffer[index] & 0xff000000) >> 23);
    int dcrcVersion1 = ((buffer[index] & 0xff0000) >> 15);
    int dcrcSN0 = ((buffer[index] & 0xff00) >> 7);
    int dcrcVersion0 = (buffer[index] & 0xff);
    
    if(fDebugOn)
      cout << "MidasEventData::HandleBankData: dcrcSN1 =" << dcrcSN1 << ". dcrcVersion1 = " << dcrcVersion1 << ". dcrcSN0 = " << dcrcSN0 << ". dcrcVersion0 = " << dcrcVersion0 << endl;

    // wap: SetRawPulseRecord function takes the channel
    // index RevC:0->5; RevD:0->15. This index is formed
    // from the channelType and channelNumber variables,
    // along with knowledge of how many charge channels
    // exist on a particular DCRC
    int nbChargeChannels = 0;
    bool lgcRevC = 0;
    if(dcrcVersion0 >= 30 && dcrcVersion0 < 40){
      nbChargeChannels = 2;
      lgcRevC = 1;
    }
    if(dcrcVersion0 >= 40 && dcrcVersion0 < 50){
      nbChargeChannels = 4;
    }

    index += 3;
    int nbChannels = (buffer[index] & 0xfffffff);
    if(fDebugOn){
      cout << "MidasEventData::HandleBankData : nbChannels =  " << nbChannels << endl;
    }      

    index += 1; //wap:skip from 19 down to 20 to get the number of samples
   
    // Now start loop over sixteen channels
    for(uint ichan = 0; ichan < nbChannels; ichan++){
      if((buffer[index] & 0xf0000000) != 0x10000000){
	std::bitset<32> x(buffer[index]);
	cerr << "MidasEventData::HandleBankData ERROR: channel word is missing correct leading bits. buffer[" << index << "] = " << x << endl;
      }
      int channelType = (buffer[index] & 0x3);
      int channelNumber = ((buffer[index] & 0x3c) >> 2);
      
      if(fDebugOn){
	cout << "MidasEventData::HandleBankData : channelType =  " << channelType << endl;
	cout << "MidasEventData::HandleBankData : channelNumber =  " << channelNumber << endl;
      }
      
      //wap: in the future this can be improved by
      //computing these sample numbers on once for the
      //for ionization and once for charge. calculating
      //16 times is redundant      
      index +=1;
      uint32_t nPrePulseSamples = buffer[index] & 0xffffffff;
      index +=1;
      uint32_t nOnPulseSamples = buffer[index] & 0xffffffff;
      index +=1;
      uint32_t nPostPulseSamples = buffer[index] & 0xffffffff;
      
      if(fDebugOn){
	cout << "MidasEventData::HandleBankData : nPrePulseSamples = " << nPrePulseSamples << endl;
	cout << "MidasEventData::HandleBankData : nOnPulseSamples = " << nOnPulseSamples << endl;
	cout << "MidasEventData::HandleBankData : nPostPulseSamples = " << nPostPulseSamples << endl;
      }

      //wap:numer of pre/post pulse samples simply added
      //to obtain the total waveform length.
      uint32_t nSamples = nPrePulseSamples + nOnPulseSamples + nPostPulseSamples;
     
      //check if the number of samples is odd.
      //if odd, then the last 16 bits of the last 32bit
      //word will be 0s. these must be ignored/skipped.
      //At the same time, the index variable must still be
      //incremented.
      int lgcOdd = nSamples%2;

      //this needs to be a ceiling division by 2
      //(e.g. if you have 7 samples, you need
      //to loop 4 times)
      uint32_t nLoop = (nSamples+1)/2;

      // Now start loop over the samples.
      // wap: the two sections, dependent on even or
      // odd samples, is clunky; but it prevents an
      // if statement check for every index
      index += 2;//wap: skip over extra words
      vector<double> pulseVect;

 
      if(lgcOdd==0){     
	for(uint32_t j = 0; j < nLoop; j++){
	  double lower_sample = (double) (buffer[index] & 0xffff);
	  double upper_sample = (double) ((buffer[index] & 0xffff0000) >> 16);
	  pulseVect.push_back(lower_sample);
	  pulseVect.push_back(upper_sample);
	  index++;	
	}      
      }
      else if(lgcOdd==1){
	//notice the loop is to nLoop-1
	for(uint32_t j = 0; j < nLoop-1; j++){
	  double lower_sample = (double) (buffer[index] & 0xffff);	 
	  double upper_sample = (double) ((buffer[index] & 0xffff0000) >> 16);	 
	  pulseVect.push_back(lower_sample);
	  pulseVect.push_back(upper_sample);
	  index++;  
	}
	//the lower 16 bits of the last 32bit word are stored in
	//the pulse vector.
	double lower_sample = (double) (buffer[index] & 0xffff);       
	pulseVect.push_back(lower_sample);
	index++;
      }
      
      int channelIndex = 0;
      
      if(channelType == 0){
	channelIndex = channelNumber;
      }
      else{
	channelIndex = nbChargeChannels + channelNumber;
      }
      if(fDebugOn){
	cout << "MidasEventData::HandleBankData : channel index =  " << channelIndex << endl;
      }
      
      // Build detector code
      uint32_t detCode = (detType*1000000 + detNum*1000) + channelIndex;  // following CDMS numbering
      
      // Store in Pulse Data
      PulseData tempPulseData;
      tempPulseData.SetRawPulseRecord(detCode, pulseVect, sampleDt, triggerT0);
      
      // Add pulse data in TriggerData
      (fCurrentTriggerData->fPulseDataList).push_back(tempPulseData);
    } //end loop over channels
    } //end loop over dets
  }

  // check Number of trigger 
  if (trg_number != (int)fTriggerDataList.size())
    { 
      cout << "WARNING in MidasEventData::ReadRawDataRecord: length of TriggerDataList " << endl;
      cout << "doesn't much number of trigger! " <<endl;
    }
  fNbTrigger = fTriggerDataList.size();
  
  success=true;
  return success;
}


// =================================================================
// Display Data
// =================================================================

void MidasEventData::Print(const char *option) const
{
  /// Print data held in this class.
  /// param [in] option If 'a' (for "all") then the raw data will be
  /// printed out too.
  ///
  
  time_t t = (time_t)fEventHeader.fTimeStamp;

  printf("Event start:\n");
  printf("  event id:       0x%04x\n", fEventHeader.fEventId);
  printf("  trigger mask:   0x%04x\n", fEventHeader.fTriggerMask);
  printf("  serial number:%8d\n", fEventHeader.fSerialNumber);
  printf("  time stamp:     %d, %s", fEventHeader.fTimeStamp, ctime(&t));
  printf("  data size:    %8d\n", fEventHeader.fDataSize);
  if ((fEventHeader.fEventId & 0xffff) == 0x8000)
    {
      printf("Begin of run %d\n", fEventHeader.fSerialNumber);
    }
  else if ((fEventHeader.fEventId & 0xffff) == 0x8001)
    {
      printf("End of run %d\n", fEventHeader.fSerialNumber);
    }
  else if (fBanksN <= 0)
    {
      printf("MidasEventData::Print: Use SetBankList() before Print() to print bank data\n");
    }
  else
    {
      printf("Banks: %s\n", fBankList);

      for (int i = 0; i < fBanksN * 4; i += 4)
	{
	  int bankLength = 0;
	  int bankType = 0;
	  void *pdata = 0;
	  int found = FindBank(&fBankList[i], &bankLength, &bankType, &pdata);
	  
	  printf("Bank %c%c%c%c, length %6d, type %2d\n",
		 fBankList[i], fBankList[i+1], fBankList[i+2], fBankList[i+3],
		 bankLength, bankType);

	  if (option[0] == 'a' && found)
	    switch (bankType)
	      {
	      case 4: // TID_WORD
		for (int j = 0; j < bankLength; j++)
		  printf("0x%04x%c", ((uint16_t*)pdata)[j], (j%10==9)?'\n':' ');
		printf("\n");
		break;
	      case 6: // TID_DWORD
		for (int j = 0; j < bankLength; j++)
		  printf("0x%08x%c", ((uint32_t*)pdata)[j], (j%10==9)?'\n':' ');
		printf("\n");
		break;
	      case 7: // TID_nd280 (like a DWORD?)
		for (int j = 0; j < bankLength; j++)
		  printf("0x%08x%c", ((uint32_t*)pdata)[j], (j%10==9)?'\n':' ');
		printf("\n");
		break;
	      case 9: // TID_FLOAT
		for (int j = 0; j < bankLength; j++)
		  printf("%.8g%c", ((float*)pdata)[j], (j%10==9)?'\n':' ');
		printf("\n");
		break;
	      case 10: // TID_DOUBLE
		for (int j = 0; j < bankLength; j++)
		  printf("%.16g%c", ((double*)pdata)[j], (j%10==9)?'\n':' ');
		printf("\n");
		break;
	      default:
		printf("MidasEventData::Print: Do not know how to print bank of type %d\n", bankType);
		break;
	      }
	}
    }
}



// ================================================================

bool MidasEventData::TriggerMatch(MidasTriggerData *lastTriggerData,int trg_src_tower,int trg_src_dcrc,int trg_word){
  
  if(lastTriggerData->GetTriggerSourceTower() == trg_src_tower &&
     lastTriggerData->GetTriggerSourceDCRC() == trg_src_dcrc && 
     lastTriggerData->GetTriggerWord() == trg_word)
    return true;

  
  return false;  

}





// =================================================================
// Bank Utility Functions
// =================================================================



bool MidasEventData::IsBank32() const
{
  return ((TMidas_BANK_HEADER *)fData)->fFlags & (1<<4);
}

int MidasEventData::LocateBank(const void *unused, const char *name, void **pdata) const
{
  /// See FindBank()

  int bktype, bklen;

  int status = FindBank(name, &bklen, &bktype, pdata);
  
  if (!status)
    {
      *pdata = NULL;
      return 0;
    }

  return bklen;
}

static const unsigned TID_SIZE[] = {0, 1, 1, 1, 2, 2, 4, 4, 4, 4, 8, 1, 0, 0, 0, 0, 0};
static const unsigned TID_MAX = (sizeof(TID_SIZE)/sizeof(TID_SIZE[0]));

int MidasEventData::FindBank(const char* name, int *bklen, int *bktype, void **pdata) const
{
  /// Find a data bank.
  /// \param [in] name Name of the data bank to look for.
  /// \param [out] bklen Number of array elements in this bank.
  /// \param [out] bktype Bank data type (MIDAS TID_xxx).
  /// \param [out] pdata Pointer to bank data, Returns NULL if bank not found.
  /// \returns 1 if bank found, 0 otherwise.
  ///

  const TMidas_BANK_HEADER *pbkh = (const TMidas_BANK_HEADER*)fData; 
  TMidas_BANK *pbk;
  //uint32_t dname;

  if (((pbkh->fFlags & (1<<4)) > 0)) {
#if 0
    TMidas_BANK32 *pbk32;
    pbk32 = (TMidas_BANK32 *) (pbkh + 1);
    memcpy(&dname, name, 4);
    do {
      if (*((uint32_t *) pbk32->fName) == dname) {
        *pdata = pbk32 + 1;
        if (TID_SIZE[pbk32->fType & 0xFF] == 0)
          *bklen = pbk32->fDataSize;
        else
          *bklen = pbk32->fDataSize / TID_SIZE[pbk32->fType & 0xFF];

        *bktype = pbk32->fType;
        return 1;
      }
      pbk32 = (TMidas_BANK32 *) ((char*) (pbk32 + 1) +
                          (((pbk32->fDataSize)+7) & ~7));
    } while ((char*) pbk32 < (char*) pbkh + pbkh->fDataSize + sizeof(TMidas_BANK_HEADER));
#endif

    TMidas_BANK32 *pbk32 = NULL;

    while (1) {
      IterateBank32(&pbk32, (char**)pdata);
      //printf("looking for [%s] got [%s]\n", name, pbk32->fName);
      if (pbk32 == NULL)
	break;

      if (name[0]==pbk32->fName[0] &&
	  name[1]==pbk32->fName[1] &&
	  name[2]==pbk32->fName[2] &&
	  name[3]==pbk32->fName[3]) {
	
	if (TID_SIZE[pbk32->fType & 0xFF] == 0)
	  *bklen = pbk32->fDataSize;
	else
	  *bklen = pbk32->fDataSize / TID_SIZE[pbk32->fType & 0xFF];
	
	*bktype = pbk32->fType;
	return 1;
      }
    }
  } else {
    pbk = (TMidas_BANK *) (pbkh + 1);
    do {
      if (name[0]==pbk->fName[0] &&
	  name[1]==pbk->fName[1] &&
	  name[2]==pbk->fName[2] &&
	  name[3]==pbk->fName[3]) {
        *pdata = pbk + 1;
        if (TID_SIZE[pbk->fType & 0xFF] == 0)
          *bklen = pbk->fDataSize;
        else
          *bklen = pbk->fDataSize / TID_SIZE[pbk->fType & 0xFF];

        *bktype = pbk->fType;
        return 1;
      }
      pbk = (TMidas_BANK *) ((char*) (pbk + 1) + (((pbk->fDataSize)+7) & ~7));
    } while ((char*) pbk < (char*) pbkh + pbkh->fDataSize + sizeof(TMidas_BANK_HEADER));
  }
  //
  // bank not found
  //
  *pdata = NULL;
  return 0;
}

void MidasEventData::SetData(uint32_t size, char* data)
{
  fEventHeader.fDataSize = size;
  assert(IsGoodSize());
  
  if (!fData) 
    AllocateData();
 
  fData = data;
  SwapBytes(false);
}


char* MidasEventData::GetData()
{
  if (!fData)
    AllocateData();
  return fData;
}

void MidasEventData::AllocateData()
{
  assert(IsGoodSize());
  fData = (char*)malloc(fEventHeader.fDataSize);
  assert(fData);
}



const char* MidasEventData::GetBankList() const
{
  return fBankList;
}

int MidasEventData::SetBankList()
{
  if (fEventHeader.fEventId <= 0)
    return 0;

  if (fBankList)
    return fBanksN;

  int listSize = 0;

  fBanksN = 0;

  TMidas_BANK32 *pmbk32 = NULL;
  TMidas_BANK *pmbk = NULL;
  char *pdata = NULL;

  while (1)
    {
      if (fBanksN*4 >= listSize)
	{
	  listSize += 400;
	  fBankList = (char*)realloc(fBankList, listSize);
	}

      if (IsBank32())
	{
	  IterateBank32(&pmbk32, &pdata);
	  if (pmbk32 == NULL)
	    break;
	  memcpy(fBankList+fBanksN*4, pmbk32->fName, 4);
	  fBanksN++;
	}
      else
	{
	  IterateBank(&pmbk, &pdata);
	  if (pmbk == NULL)
	    break;
	  memcpy(fBankList+fBanksN*4, pmbk->fName, 4);
	  fBanksN++;
	}
    }

  fBankList[fBanksN*4] = 0;

  return fBanksN;
}

int MidasEventData::IterateBank(TMidas_BANK **pbk, char **pdata) const
{
  /// Iterates through banks inside an event. The function can be used
  /// to enumerate all banks of an event.
  /// \param [in] pbk Pointer to the bank header, must be NULL for the
  /// first call to this function. Returns NULL if no more banks
  /// \param [in] pdata Pointer to data area of bank. Returns NULL if no more banks
  /// \returns Size of bank in bytes or 0 if no more banks.
  ///
  const TMidas_BANK_HEADER* event = (const TMidas_BANK_HEADER*)fData;

  if (*pbk == NULL)
    *pbk = (TMidas_BANK *) (event + 1);
  else
    *pbk = (TMidas_BANK *) ((char*) (*pbk + 1) + ((((*pbk)->fDataSize)+7) & ~7));

  *pdata = (char*)((*pbk) + 1);

  if ((char*) *pbk >=  (char*) event + event->fDataSize + sizeof(TMidas_BANK_HEADER))
    {
      *pbk = NULL;
      *pdata = NULL;
      return 0;
    }

  return (*pbk)->fDataSize;
}

int MidasEventData::IterateBank32(TMidas_BANK32 **pbk, char **pdata) const
{
  /// See IterateBank()

  const TMidas_BANK_HEADER* event = (const TMidas_BANK_HEADER*)fData;
  if (*pbk == NULL)
    *pbk = (TMidas_BANK32 *) (event + 1);
  else {
    uint32_t length = (*pbk)->fDataSize;
    uint32_t length_adjusted = (length+7) & ~7;
    //printf("length %6d 0x%08x, 0x%08x\n", length, length, length_adjusted);
    *pbk = (TMidas_BANK32 *) ((char*) (*pbk + 1) + length_adjusted);
  }

  TMidas_BANK32 *bk4 = (TMidas_BANK32*)(((char*) *pbk) + 4);

  //printf("iterate bank32: pbk 0x%p, align %d, type %d %d, name [%s], next [%s], TID_MAX %d\n", *pbk, (int)( ((uint64_t)(*pbk))&7), (*pbk)->fType, bk4->fType, (*pbk)->fName, bk4->fName, TID_MAX);

  if ((*pbk)->fType > TID_MAX) // bad - unknown bank type - it's invalid MIDAS file?
    {
      if (bk4->fType <= TID_MAX) // okey, this is a malformed T2K/ND280 data file
	{
	  *pbk = bk4;

	  //printf("iterate bank32: pbk 0x%p, align %d, type %d, name [%s]\n", *pbk, (int)(((uint64_t)(*pbk))&7), (*pbk)->fType, (*pbk)->fName);
	}
      else
	{
	  // truncate invalid data
	  *pbk = NULL;
	  *pdata = NULL;
	  return 0;
	}
    }

  *pdata = (char*)((*pbk) + 1);

  if ((char*) *pbk >= (char*)event  + event->fDataSize + sizeof(TMidas_BANK_HEADER))
    {
      *pbk = NULL;
      *pdata = NULL;
      return 0;
    }

  return (*pbk)->fDataSize;
}

typedef uint8_t BYTE;










// =================================================================
/// Byte swapping routine.
//  FIXME: eventually use EndianHelper.cxx
// =================================================================



#define QWORD_SWAP(x) { BYTE _tmp;       \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+7);     \
*(((BYTE *)(x))+7) = _tmp;               \
_tmp= *(((BYTE *)(x))+1);                \
*(((BYTE *)(x))+1) = *(((BYTE *)(x))+6); \
*(((BYTE *)(x))+6) = _tmp;               \
_tmp= *(((BYTE *)(x))+2);                \
*(((BYTE *)(x))+2) = *(((BYTE *)(x))+5); \
*(((BYTE *)(x))+5) = _tmp;               \
_tmp= *(((BYTE *)(x))+3);                \
*(((BYTE *)(x))+3) = *(((BYTE *)(x))+4); \
*(((BYTE *)(x))+4) = _tmp; }

/// Byte swapping routine.
///
#define DWORD_SWAP(x) { BYTE _tmp;       \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+3);     \
*(((BYTE *)(x))+3) = _tmp;               \
_tmp= *(((BYTE *)(x))+1);                \
*(((BYTE *)(x))+1) = *(((BYTE *)(x))+2); \
*(((BYTE *)(x))+2) = _tmp; }

/// Byte swapping routine.
///
#define WORD_SWAP(x) { BYTE _tmp;        \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+1);     \
*(((BYTE *)(x))+1) = _tmp; }

void MidasEventData::SwapBytesEventHeader()
{
  WORD_SWAP(&fEventHeader.fEventId);
  WORD_SWAP(&fEventHeader.fTriggerMask);
  DWORD_SWAP(&fEventHeader.fSerialNumber);
  DWORD_SWAP(&fEventHeader.fTimeStamp);
  DWORD_SWAP(&fEventHeader.fDataSize);
}

int MidasEventData::SwapBytes(bool force)
{
  TMidas_BANK_HEADER *pbh;
  TMidas_BANK *pbk;
  TMidas_BANK32 *pbk32;
  void *pdata;
  uint16_t type;

  pbh = (TMidas_BANK_HEADER *) fData;

  uint32_t dssw = pbh->fDataSize;

  DWORD_SWAP(&dssw);

  //printf("SwapBytes %d, flags 0x%x 0x%x\n", force, pbh->fFlags, pbh->fDataSize);
  //printf("evh.datasize: 0x%08x, SwapBytes: %d, pbh.flags: 0x%08x, pbh.datasize: 0x%08x swapped 0x%08x\n", fEventHeader.fDataSize, force, pbh->fFlags, pbh->fDataSize, dssw);

  //
  // only swap if flags in high 16-bit
  //
  if (pbh->fFlags < 0x10000 && ! force)
    return 0;

  if (pbh->fDataSize == 0x6d783f3c) // string "<xml..." in wrong-endian format
    return 1;

  if (pbh->fDataSize == 0x3c3f786d) // string "<xml..."
    return 1;

  if (dssw > fEventHeader.fDataSize + 100) // swapped data size looks wrong. do not swap.
    return 1;

  //
  // swap bank header
  //
  DWORD_SWAP(&pbh->fDataSize);
  DWORD_SWAP(&pbh->fFlags);
  //
  // check for 32-bit banks
  //
  bool b32 = IsBank32();

  pbk = (TMidas_BANK *) (pbh + 1);
  pbk32 = (TMidas_BANK32 *) pbk;
  //
  // scan event
  //
  while ((char*) pbk < (char*) pbh + pbh->fDataSize + sizeof(TMidas_BANK_HEADER)) {
    //
    // swap bank header
    //
    if (b32) {
      DWORD_SWAP(&pbk32->fType);
      DWORD_SWAP(&pbk32->fDataSize);
      pdata = pbk32 + 1;
      type = (uint16_t) pbk32->fType;
    } else {
      WORD_SWAP(&pbk->fType);
      WORD_SWAP(&pbk->fDataSize);
      pdata = pbk + 1;
      type = pbk->fType;
    }
    //
    // pbk points to next bank
    //
    if (b32) {
      assert(pbk32->fDataSize < fEventHeader.fDataSize + 100);
      pbk32 = (TMidas_BANK32 *) ((char*) (pbk32 + 1) +
                          (((pbk32->fDataSize)+7) & ~7));
      pbk = (TMidas_BANK *) pbk32;
    } else {
      assert(pbk->fDataSize < fEventHeader.fDataSize + 100);
      pbk = (TMidas_BANK *) ((char*) (pbk + 1) +  (((pbk->fDataSize)+7) & ~7));
      pbk32 = (TMidas_BANK32 *) pbk;
    }

    switch (type) {
    case 4:
    case 5:
      while (pdata < pbk) {
        WORD_SWAP(pdata);
        pdata = ((char*)pdata) + 2;
      }
      break;
    case 6:
    case 7:
    case 8:
    case 9:
      while (pdata < pbk) {
        DWORD_SWAP(pdata);
        pdata = ((char*)pdata) + 4;
      }
      break;
    case 10:
      while (pdata < pbk) {
        QWORD_SWAP(pdata);
        pdata = ((char*)pdata) + 8;
      }
      break;
    }
  }
  return 1;
}

// end

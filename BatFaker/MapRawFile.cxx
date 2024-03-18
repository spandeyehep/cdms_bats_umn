/** @file MapRawFile.cxx
    @author B. Loer
    @date 2016-12-05

    Create a map of location in bytes from the start of the file for each 
    event and pulse in a CDMS raw data file.  This will make seeking events
    and loading specific data parts much easier

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "RawDataReader.h"
#include "AdminData.h"
#include "PulseData.h"

#include "RawDataMap.h"



int main(int argc, const char** argv)
{
  if(argc != 2 && argc != 3){
    std::cerr<<"Usage: "<<argv[0]<<" <raw data file> [<map path>]"
	     <<std::endl;
    return 1;
  }

  //open up the input file 
  const std::string fin = argv[1];
  RawDataReader reader;
  reader.OpenRawDataFile("", fin);   //no need to check return since it exits
  
  //open up the map file
  std::string mapdir = argc > 2 ? argv[2] : "";
  std::string mapfile = RawDataMap::GetMapFileName(fin, mapdir);
  std::ofstream fout(mapfile.c_str());
  if(!fout){
    std::cerr<<"Error opening mapfile "<<mapfile<<std::endl;
    return 2;
  }
  //we need to register these so their blocks will get read
  AdminData admin;
  reader.RegisterAdminData(&admin);
  
  //write the mapfile header
  fout<< RawDataBlock::GetHeader() <<std::endl;
  //now read through the entire event and note all the required info
  reader.ReadFileHeader(false);
  

  z_off_t eventstart = reader.GetCurrentFilePosition();
  std::vector<RawDataBlock> recordvec;
  int neventsprocessed = 0;
  while(reader.ReadEventHeader(false)){
    ++neventsprocessed;
    //after reading event header, we should be at first record
    //but we don't know the event number yet, need admin data for that
    //so have to store everything temporarily
    long eventid=-1;
    int eventlength = reader.GetNextEventPosition() - eventstart;
    
    //event block
    recordvec.push_back(RawDataBlock(0,0,0,eventstart, eventlength));
    
    
    //now loop through all records within event
    while(reader.GetNextRecordHeaderPosition() < reader.GetNextEventPosition()){
      uint32_t recordID = reader.GetNextRecordID(false);
      uint32_t detectorcode = 0;
      z_off_t recordstart = reader.GetCurrentRecordPosition();
      int recordlength = reader.GetNextRecordHeaderPosition() - recordstart;
      
      if(recordID == BatRootTypes::kAdminRecordID || 
	 recordID == BatRootTypes::kAdminRecordID64 ){
	admin.ReadRawDataRecord(reader.GetRawDataPtr(), recordlength,
				recordID, false);
	eventid = admin.GetEvent();
      }
      else if(recordID == BatRootTypes::kPulseRecordID || 
	      recordID == BatRootTypes::kPulseRecordExpandedCodeID){
	//read the detector code from the pulse header
	static int detcodeword=4;
	gzseek(reader.GetRawDataPtr(), detcodeword*sizeof(uint32_t), SEEK_CUR);
	gzread(reader.GetRawDataPtr(),(char*)(&detectorcode),
	       sizeof(detectorcode));
	//this is ignorning endianness conversion!!	
      }
      
      recordvec.push_back(RawDataBlock(eventid, recordID, detectorcode,
				       recordstart, recordlength));
      
    }
    
    //once we get here we're read the event
    if(eventid < 0){
      std::cerr<<"Error: There was no AdminData record for this event!\n";
    }
    else{
      for(size_t i=0; i<recordvec.size(); ++i){
	recordvec[i].eventid = eventid;
	fout << recordvec[i] <<std::endl;
      }
    }
    
    //go to the beginnign of the next event
    //gzseek(reader.GetRawDataPtr(), reader.GetNextEventPosition(), SEEK_SET);
    eventstart = reader.GetNextEventPosition();
    reader.Clear();
    recordvec.clear();
  }
  std::cout<<"Processed "<<neventsprocessed<<" events."<<std::endl;
  
  return 0;
}

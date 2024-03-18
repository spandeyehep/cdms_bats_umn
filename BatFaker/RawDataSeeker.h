/** @file RawDataSeeker
    @author B. Loer
    @date 2016-12-06
    
    Utitlities for loading single events and Pulses from raw data

*/

#ifndef RAWDATASEEKER_h
#define RAWDATASEEKER_h

#include <string>
#include "PulseData.h"
#include "RawDataMap.h"

class RawDataSeeker { 
 public:

  ///Constructor, takes dir containing series dirs and optional place for maps
  RawDataSeeker(const std::string& topleveldir,
		const std::string& mappath="");

  ///Destructor, make sure we clean up file pointer
  ~RawDataSeeker();

  /******** utility functions *************/
  
  ///Get the dump number corresponding to an event
  static int GetDumpNumFromEvent(long event){ return event/10000; }

  ///Get the full path to a series and dump file
  static std::string GetRawFilePath(const std::string& topleveldir,
				    const std::string& series, int dump,
				    bool withsubdir=true);
  
  /************** data loading **************/
  ///Access to our internal raw memory buffer
  char* GetBuffer(){ return &(_buffer[0]); }

  ///Load an event into our memory buffer, return size, <1 on error
  int LoadRecord(const std::string& series, long event,
		 uint32_t recordtype=0, uint32_t detectorcode=0);

  ///Load the given Pulse from raw data
  PulseData LoadPulse(const std::string& series, long event, 
		      uint32_t detectorcode);


  ///Load the file header and detector config record (mostly for copying)
  int LoadPreEventHeader(const std::string& series, int dump); 

  
  ///Open a new raw data file, mostly for internal use
  bool LoadRawDataFile(const std::string& filename);
  
  ///seek to position of record in raw data file, mostly for internal use
  RawDataBlock SeekToRecord(const std::string& series, long event, 
			    uint32_t recordtype=0, uint32_t detectorcode=0);
  
  /************ query state ********************/
  
  ///Currently opened filename
  const std::string& GetCurrentFile() const { return _currentfile; }
  
  ///Currently loaded record
  const RawDataBlock& GetCurrentRecord() const { return _currentrecord; }
  
  ///Access to raw data map
  const RawDataMap& GetRawDataMap() const { return _eventmap; }

 private:
  std::string _topleveldir;    //< starting directory
  std::string _mappath;        //< place to look for and create datamaps
  
  std::string _currentfile;    //< currently loaded filename
  RawDataBlock _currentrecord; //< record currently loaded in memory buffer
  gzFile _fin;                 //< Currently opened file descriptor
  RawDataMap _eventmap;        //< associated data map for _currentfile

  std::vector<char> _buffer;   //< raw memory buffer for gzip reading
  

  

};


#endif

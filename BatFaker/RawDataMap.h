/** @file RawDataMap.h
    @author B. Loer
    @date 2016-12-05
    
    Defines the RawDataBlock class used to read/write raw data offset values
    and RawDataMap used to parse the files

*/
#ifndef RAWDATAMAP_h
#define RAWDATAMAP_h

#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "zlib.h" //for z_off_t type
#include "BatRootTypes.h" //for word type

struct RawDataBlock { 
  long eventid;
  uint32_t recordtype;
  uint32_t detectorcode;
  z_off_t offset;
  int length;
  
  RawDataBlock(long EventID=0, uint32_t RecordType=0, 
	       uint32_t DetectorCode=0, z_off_t Offset=0, int Length=0) : 
  eventid(EventID), recordtype(RecordType), detectorcode(DetectorCode),
    offset(Offset), length(Length) {}
  
  
  static std::string GetHeader(){
    return "EventID\tRecordType\tDetectorCode\tOffset\tLength\t";
  }
  
  //sorting operators for maps
  bool operator==(const RawDataBlock& right) const{
    return ( eventid == right.eventid &&
	     detectorcode == right.detectorcode && 
	     ( recordtype == right.recordtype || detectorcode != 0 )
	   );
  }
  
  bool operator<(const RawDataBlock& right) const{
    //items can be equal if detectorcode is equal by record type is nonzero
    if( *this == right )
      return false;
    if(eventid != right.eventid)
      return eventid < right.eventid;
    if(detectorcode != right.detectorcode)
      return detectorcode < right.detectorcode;
    if(recordtype != right.recordtype)
      return recordtype < right.recordtype;
    //if we get here they're equal
    return false;
  }

  
};

//read/write RawDataBlocks
inline std::ostream& operator<<(std::ostream& out, const RawDataBlock& blk)
{ 
  return out<<blk.eventid<<'\t'<<blk.recordtype<<'\t'<<blk.detectorcode
	    <<'\t'<<blk.offset<<'\t'<<blk.length;
}

inline std::istream& operator>>(std::istream& in, RawDataBlock& blk)
{
  return in>>blk.eventid>>blk.recordtype>>blk.detectorcode
	   >>blk.offset>>blk.length;
}


class RawDataMap { 
  /** @class RawDataMap
      @brief Read a raw data map file and do offset lookups
  */
  
  std::set<RawDataBlock> records;
  
 public:
  ///Constructor
  RawDataMap(const std::string& filename="");

  ///Load a map file, returns number of entries
  int LoadMapFile(const std::string& filename, bool printerror=true);
  
  ///Get number of loaded entries, also serves as status check
  int GetEntriesLoaded() const { return records.size(); }

  
  ///Get the offset and length of a record. eventid<0 on error
  RawDataBlock FindRecord(long EventID, 
			  uint32_t RecordType=0, 
			  uint32_t DetectorCode=0) const;

  
  ///See if a record is in this file
  bool IsRecordInCurrentFile(long EventID, 
			     uint32_t RecordType=0, 
			     uint32_t DetectorCode=0) const;


  
  RawDataBlock GetFirstRecord() const{
    return records.empty() ? RawDataBlock(-1) : *(records.begin());
  }

  ///Get the map filename given the raw file and optional separate directory
  static std::string GetMapFileName(const std::string& rawfilename,
				    const std::string& mapdir="");

};


#endif

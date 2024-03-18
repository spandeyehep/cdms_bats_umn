/** @file RawDataMap.cxx
    @author B. Loer
    @date 2016-12-06

    Implementation for the RawDataMap class and helpers
*/


#include "RawDataMap.h"
#include "TSystem.h"

std::string RawDataMap::GetMapFileName(const std::string& rawfilename,
				       const std::string& mapdir)
{
  const std::string suffix = ".eventmap";
  std::string mapfile = rawfilename + suffix;
  //replace the path with the supplied one if present
  if(mapdir != ""){
    mapfile = mapdir + "/" + gSystem->BaseName(mapfile.c_str());
  }
  return mapfile;
}



RawDataMap::RawDataMap(const std::string& filename)
{
  LoadMapFile(filename, filename!=""); //suppress errors on default load
}

int RawDataMap::LoadMapFile(const std::string& filename, bool printerror)
{
  //clear any existing data first
  records.clear();
  
  //try to read the file
  std::ifstream fin(filename.c_str());
  if(!fin){
    if(printerror)
      std::cerr<<"Error RawDataMap: Unable to open file "<<filename<<std::endl;
    return 0;
  }
  
  //read the header
  std::string header;
  std::getline(fin, header);
  //read lines
  RawDataBlock blk;
  while( fin >> blk )
    records.insert(blk);
  return records.size();
}
  
  ///Get the offset and length of a record. eventid<0 on error
RawDataBlock RawDataMap::FindRecord(long EventID, 
				    uint32_t RecordType, 
				    uint32_t DetectorCode) const
{
  RawDataBlock key(EventID, RecordType, DetectorCode);
  std::set<RawDataBlock>::const_iterator it = records.find(key);
  if( it != records.end() )
    return *it;
  //wasn't in the map!
  std::cerr<<"RawDataMap: Can't locate record "<<key<<std::endl;
  return RawDataBlock(-1);
}


bool RawDataMap::IsRecordInCurrentFile(long EventID, 
				       uint32_t RecordType,
				       uint32_t DetectorCode) const
{
  return records.count(RawDataBlock(EventID, RecordType, DetectorCode));
}

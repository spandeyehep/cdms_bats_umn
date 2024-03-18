#include "RawDataSeeker.h"


RawDataSeeker::RawDataSeeker(const std::string& topleveldir,
			     const std::string& mappath) :
  _topleveldir(topleveldir), _mappath(mappath),
  _currentfile(""), _currentrecord(-1), _fin(0)
{}


RawDataSeeker::~RawDataSeeker()
{
  if(_fin) gzclose(_fin);
}
///Get the full path to a series and dump file
std::string RawDataSeeker::GetRawFilePath(const std::string& topleveldir,
					  const std::string& series, 
					  int dump,
					  bool withsubdir) 
{
  static char fname[200];
  sprintf(fname, "%s%s_F%04d.gz", (withsubdir ? (series+"/").c_str() : ""), 
	  series.c_str(), dump);
  return topleveldir + (topleveldir=="" ? "" : "/") + fname;
}
  
///Load an event into our memory buffer, return size in bytes, <1 on error
int RawDataSeeker::LoadRecord(const std::string& series, long event,
			      uint32_t recordtype, uint32_t detectorcode)
{
  
  RawDataBlock recordblock = SeekToRecord(series, event, 
					  recordtype, detectorcode);
  if(recordblock.eventid < 0)
    return recordblock.eventid;
    
  //make sure our memory buffer is big enough
  if( _buffer.size() < (size_t)recordblock.length )
    _buffer.resize(recordblock.length + 20);
  
  //read the event
  int nread = gzread(_fin, GetBuffer(), recordblock.length);
  if(nread != recordblock.length){
    std::cerr<<"RawDataSeeker::LoadRecord error reading data for record "
	     << recordblock <<std::endl;
    //return error, or be OK with partial read????
  }

  _currentrecord = recordblock;

  return nread;
}

///Load the given Pulse from raw data
PulseData RawDataSeeker::LoadPulse(const std::string& series, long event, 
				   uint32_t detectorcode)
{
  PulseData tempPulseData;

  //see if we've already loaded this record or the whole event
  if(_currentrecord == RawDataBlock(event) || 
     _currentrecord == RawDataBlock(event,0,detectorcode) ){
    //event is loaded, so interpret the pulse straight from the memory block
    //Find the info for this record
    RawDataBlock recordblock = _eventmap.FindRecord(event, 0, 
						    detectorcode);
    if( recordblock.eventid > 0 ){
      int offsetbytes = recordblock.offset - _currentrecord.offset;
      tempPulseData.ReadRawPulseBuffer((uint32_t*)(GetBuffer()+offsetbytes),
				       recordblock.length,
				       recordblock.recordtype,
				       false);
    }
  }
  else{
    //queue up the record to be read
    RawDataBlock recordblock = SeekToRecord(series, event, 
					    0, detectorcode);
    if(recordblock.eventid > 0){
      tempPulseData.ReadRawPulseRecord(_fin, recordblock.length, 
				       recordblock.recordtype, false);
    }
  }
  //not sure what this will look like on error...

  return tempPulseData;
}


bool RawDataSeeker::LoadRawDataFile(const std::string& filename)
{
  
  //allow for no-op if it's already loaded
  if( filename == _currentfile )
    return true;
  
  //reset all the 'current' stuff
  if(_fin) gzclose(_fin);
  _fin = 0;
  _currentfile = "";
  _currentrecord = RawDataBlock(-1);
  _eventmap.LoadMapFile("",false);

  _fin = gzopen(filename.c_str(), "rb");
  if(!_fin){
    std::cerr<<"RawDataSeeker::LoadRawDataFile error opening file "<<filename
	     <<std::endl;
    return false;
  }
  //does the map file exist? 
  std::string mapfile = _eventmap.GetMapFileName(filename, _mappath);
  std::ifstream testfile(mapfile.c_str());
  if(!testfile.good()){
    std::cout<<"No .eventmap file found for "<<filename
	     <<"; attempting to generate now."<<std::endl;
    //assume CDMSBATSDIR is set or we're in cdmsbats directory
    std::string cdmsbatsdir=getenv("CDMSBATSDIR");
    if(cdmsbatsdir.empty()) cdmsbatsdir=".";
    std::string cmd = cdmsbatsdir+"/BUILD/bin/MapRawFile "+filename+" "+_mappath;
    std::cout<<"Command is "<<cmd<<std::endl;
    int ret = system(cmd.c_str());
    if(ret != 0){
      std::cerr<<"RawDataSeeker ERROR: Unable to generate map file\n";
      return false;
    }
  }
  //now load it
  if(_eventmap.LoadMapFile(_eventmap.GetMapFileName(filename, _mappath)) <=0 ){
    std::cerr<<"RawDataSeeker::LoadRawDataFile error loading eventmap for "
	     <<filename<<std::endl;
    return false;
  }
    
  //success!
  _currentfile = "";
  return true;
  
}

///seek to position of record in raw data file
RawDataBlock RawDataSeeker::SeekToRecord(const std::string& series,
					 long event, 
					 uint32_t recordtype, 
					 uint32_t detectorcode)
{
  //empty string for series means keep same series
  if(series != ""){
    //load the correct file
    std::string loadfile = GetRawFilePath(_topleveldir, series, 
					  GetDumpNumFromEvent(event));
    if( !LoadRawDataFile(loadfile) )
      return RawDataBlock(-1);
  }
  
  //Find the info for this record
  RawDataBlock recordblock = _eventmap.FindRecord(event, recordtype, 
						  detectorcode);
  if( recordblock.eventid < 0 ){
    std::cerr<<"RawDataSeeker::SeekToRecord error no record for event "<<event
	     <<std::endl;
    return RawDataBlock(-2);
  }
  
  //seek to the start of the record
  z_off_t newpos = gzseek(_fin, recordblock.offset, SEEK_SET);
  if(newpos != recordblock.offset){
    std::cerr<<"RawDataSeeker::LoadEvent error seeking to event start at "
	     <<recordblock.offset<<" for event "<<event<<"\n";
    return RawDataBlock(-3);
  }
  
  return recordblock;
}




int RawDataSeeker::LoadPreEventHeader(const std::string& series, int dump)
{
  //this is slight inefficient since we seek unneccarily, but it takes
  //care of all the file loading, so I'm going to go with it
  RawDataBlock ev1 = SeekToRecord(series, dump*10000+1 /*first event in dump*/,
				  0,0);
  gzrewind(_fin);
  _currentrecord = RawDataBlock(-1);
  //make sure buffer is big enough
  if(_buffer.size() < ev1.offset)
    _buffer.resize(ev1.offset);
  int nread = gzread(_fin, &(_buffer[0]), ev1.offset);
  if(nread != ev1.offset){
    std::cerr<<"RawDataSeeker::LoadPreEventHeader error reading data!\n";
    //nread should aleady have some sort of error I think...
  }
  return nread;
}

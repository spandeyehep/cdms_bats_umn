#include <fstream>
#include <stdlib.h>

#include "TemplateLoader.h"
#include "DetectorConfigManager.h"
#include "ChannelMapHelper.h"
#include "RawDataSeeker.h"

TemplateLoader::TemplateLoader(const std::string& seriestopdir,
			       const std::string& templatedir,
			       const std::string& detstatusfile)
  : _topleveldir(seriestopdir), 
    _templatedir(templatedir),
    _currentseries(""), _currentdump(-1)
{
  //see if CDMSBATSDIR is set
  std::string cdmsbatsdir = getenv("CDMSBATSDIR");
  if(cdmsbatsdir.empty())//if empty, assume it's pwd
    cdmsbatsdir = ".";
  
  //try to find the template directory intelligently
  if(_templatedir.empty()){
    _templatedir = cdmsbatsdir+"/PulseTemplates";
  }

  //set up the user data file with minimal settings
  _userdata.SetStringParameter("PATH_RAW_DATA","", true);
  _userdata.ProcessLine("DO_PROCESSING DETECTOR 1-15 = 1");
  _userdata.ProcessLine("READ DET_STATUS_FILE = 1");
  //read the detector status file
  std::string dsfile = detstatusfile;
  if(dsfile.empty()){
    dsfile = cdmsbatsdir+"/UserSettings/BatRootSettings/detector_status/detectorStatus.SuperCDMS";
  }
  
  //see if the file exists
  ifstream ftest(dsfile.c_str());
  if(!ftest){
    std::cerr<<"TemplateLoader ERROR: Unable to read detector status file "
	     <<dsfile<<"; will continue without, but templates may be wrong!"
	     <<std::endl;
  }
  else{
    _userdata.ReadFile(dsfile,true);
  }
  
}


//get everything ready for a single series
int TemplateLoader::LoadSeries(const std::string& series, int dump)
{
  //is it already loaded?
  if(series == _currentseries && 
     ( dump == _currentdump || dump == 0)){
    //already loaded, nothing to do
    return 0;
  }
  
  if(dump==0) dump=1;
  //load the detector config from the raw data file
  std::cout<<"TemplateLoader::LoadSeries loading series "<<series
	   <<" dump "<<dump<<std::endl;
  
  std::string rawfile = RawDataSeeker::GetRawFilePath(_topleveldir, 
						      series, dump);
  _detconfig = DetectorConfigManager(_userdata, rawfile);
  
  //get the correct detector statuses
  std::map<int,int> detMap = _detconfig.GetDetectorMap();
  _userdata.ConstructZipRQList(detMap);
  _userdata.DoCalcDetectorStatus(series);

  //finally we can load the templates
  _templatedata.ReadAllFiles(_templatedir, series, detMap,
			     _detconfig.GetDetectorChargePolarityMap(),
			     _userdata);
  
  //if we get here without exiting, everything is good!
  _currentseries = series;
  _currentdump = dump;
  return 0;
}


typedef TemplateLoader::PulseTemplate PulseTemplate;

PulseTemplate TemplateLoader::GetTemplateByCode(uint32_t detcode, 
						const std::string& suffix,
						const std::string& series,
						int dump)
{
  //figure out the channel name
  return GetTemplateByName(ChannelMapHelper::GetDetNumFromCode(detcode),
			   ChannelMapHelper::GetChannelName(detcode)+suffix,
			   series, dump);
  
}

PulseTemplate TemplateLoader::GetTemplateByName(int detNum,
						const std::string& channel,
						const std::string& series,
						int dump)
{
  //try to load all templates for this series
  if(LoadSeries(series,dump) < 0)
    return PulseTemplate();
  PulseTemplate pt = _templatedata.GetTemplate(detNum, channel);
  return pt;
}

double TemplateLoader::GetSampleRate(uint32_t detCode, 
				     const std::string& series)
{
  if(series != "")
    LoadSeries(series);
  return _detconfig.GetSampleRate(detCode);
}

double TemplateLoader::GetSampleRate(int detNum, const std::string& chanName,
				     const std::string& series)
{
  if(series != "")
    LoadSeries(series);
  return _detconfig.GetSampleRate(detNum, 
				  ChannelMapHelper::GetChannelType(chanName));
}

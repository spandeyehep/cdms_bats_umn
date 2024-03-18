/** @file TemplateLoader.h
    @author B. Loer
    @date 2016-12-07

    Utilities to load the correct template for a given series, channel
*/

#ifndef TEMPLATELOADER_h
#define TEMPLATELOADER_h

#include <string>
#include <vector>

#include "UserDataManager.h"
#include "DetectorConfigManager.h"
#include "TemplateDataManager.h"

///Stateful class to assist in loading templates
class TemplateLoader {

 public:
  ///Constructor, takes path to detector status file and series locations
  TemplateLoader(const std::string& seriestopdir,
		 const std::string& templatedir="",
		 const std::string& detstatusfile="");
  
  ///Queue up the templates for a given series/dump
  int LoadSeries(const std::string& series, int dump=0);

  
  typedef std::vector<double> PulseTemplate;
  
  ///Get the specified time template by full channel ID
  PulseTemplate GetTemplateByCode(uint32_t detCode, 
				  const std::string& suffix="",
				  const std::string& series="", int dump=0);
  
  ///Get the specified time template by detnum and channel name
  PulseTemplate GetTemplateByName(int detNum, const std::string& channel,
				  const std::string& series="", int dump=0);
  
  
  ///Get the sample rate for a given channel, current series by default
  double GetSampleRate(uint32_t detCode, const std::string& series="");
  double GetSampleRate(int detNum, const std::string& chanName,
		       const std::string& series); 
  //^require all 3 params to remove ambiguous calls
  
 private:

  std::string _topleveldir;
  std::string _templatedir; 

  std::string _currentseries;
  int _currentdump;
  
  UserDataManager _userdata;
  DetectorConfigManager _detconfig;
  TemplateDataManager _templatedata;
};

#endif

#ifndef TRACEWRITER_h
#define TRACEWRITER_h

/**
   @class TraceWriter
   @author Ben Loer
   @date Created 2013-08-09
   @brief This class saves raw traces into a ROOT file for basic analysis
*/

#include <string>
#include <vector>
#include <map>

class TFile;
class PulseData;
class AdminData;

class TraceSaver{
 public:
  TraceSaver(const std::string& filename="CdmsTraces.root");
  ~TraceSaver();

  std::string GetFileName() const { return fFilename; }
  void SetFileName(const std::string& fname) { fFilename = fname; }
  
  int SavePulse(const PulseData* pulse, AdminData* admin);
  int SaveDetector(const std::vector<PulseData>& pulses, AdminData* admin);
  int SaveDetectorMap(const std::map<int, std::vector<PulseData> >& pulsemap,
		      AdminData* admin);

 private:
  std::string fFilename;
  TFile* fOutFile;
  
};

#endif

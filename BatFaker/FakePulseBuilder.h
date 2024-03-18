/** @file FakePulseBuilder.h
    @author B. Loer
    @date 2016-12-07

    Construct a fake pulse from the sum of an arbitrary number of scaled 
    measured waveforms and templates

*/ 

#ifndef FAKEPULSEBUILDER_h
#define FAKEPULSEBUILDER_h

#include <string>
#include <vector>
#include "zlib.h"

#include "TemplateLoader.h"
#include "RawDataSeeker.h"


class FakePulseBuilder {
 public:
  ///Constructor, takes necessary args for TemplateLoader and RawDataSeeker
  FakePulseBuilder(const std::string& seriestopdir,
		   const std::string& eventmappath="",
		   const std::string& templatedir="",
		   const std::string& detstatusfile="");

  
  ///Destructor, make sure we clean up file
  ~FakePulseBuilder();
  
  typedef TemplateLoader::PulseTemplate Pulse; //vector<double>
  
  ///Transform a pulse with scale and delay, delay in nbins
  Pulse TransformPulse(const Pulse& pulse, double scale, double delay=0,
		       bool wrap=true);
  
  
  ///Read through a control file and produce fake output
  /** Parameters:
      controlfile: text file with pulse substitutions. See readme for details
      outdir: top-level directory in which to place fake files
      returns number of control lines parsed
  */
  int GenerateFakeDataFiles(const std::string& controlfile,
			    const std::string& outdir);
  
  ///parse a single line from the control file
  int ExecuteControlLine(const std::string& line);

  struct ControlFunction{
    std::string name;
    std::vector<std::string> args;
  };
  
  ///parse a single function from a control line
  Pulse EvaluateControlFunction(const ControlFunction& controlfunc);
 
  //get set flag to create subdirectories for series
  void SetCreateSeriesDirs(bool set){ _createseriesdirs = set; }
  bool GetCreateSeriesDirs() const { return _createseriesdirs; }
  
  //get/set flag to copy all events or just ones in control file
  void SetCopyAllEvents(bool set){ _copyallevents = set; }
  bool GetCopyAllEvents() const { return _copyallevents; }
  
  
 private:
  ///Split a line from the control file into separate actions
  std::vector<ControlFunction> ParseControlLine(const std::string& line);

  ///interpret a single control function from a string
  ControlFunction ParseControlFuncStr(std::string s);

  ///Make sure the control function has the right number of arguments
  bool ArgCheck(const ControlFunction& func, size_t min, size_t max=0);
  
  ///open a new output data file with some fake events, optionally create dir
  int OpenNewOutputFile(const std::string& series, int dump);
  
  ///properly close and write out the current file
  int CloseCurrentOutputFile();

  ///Write a single already-loaded event to output file
  int WriteCurrentEvent();
  
  ///Load an event for overwriting, and write all intermediate stuff 
  int PrepareNewTargetEvent(const std::string& series, long event);

  ///copy a constructed pulse into a memory buffer
  int CopyPulseToBuffer(const Pulse& pulse, uint32_t detcode);

  
 private:
  TemplateLoader _templates;
  RawDataSeeker _rawpulses;
  RawDataSeeker _targetevents;
  
  std::string _foutname;
  gzFile _fout;
  
  bool _createseriesdirs;
  bool _copyallevents;

  std::string _outdir;
};


#endif 

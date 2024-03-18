#include "FakePulseBuilder.h"
#include "PulseTools.h"

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <stdlib.h>

FakePulseBuilder::FakePulseBuilder(const std::string& seriestopdir,
				   const std::string& eventmappath,
				   const std::string& templatedir,
				   const std::string& detstatusfile) : 
  _templates(seriestopdir, templatedir, detstatusfile),
  _rawpulses(seriestopdir, eventmappath),
  _targetevents(seriestopdir, eventmappath),
  _foutname(""),
  _fout(NULL),
  _createseriesdirs(true), 
  _copyallevents(true)
{
  
}

FakePulseBuilder::~FakePulseBuilder()
{
  if(_fout)
    CloseCurrentOutputFile();
}

typedef FakePulseBuilder::Pulse Pulse;
typedef FakePulseBuilder::ControlFunction ControlFunction;



//TODO: Make the delay in seconds rather than bins
Pulse FakePulseBuilder::TransformPulse(const Pulse& pulse, 
				       double scale, double delay, bool wrap)
{
  //check for no-op conditions
  if(pulse.size() == 0 || (scale == 1 && delay == 0))
    return pulse;
  if(delay == 0)
    return PulseTools::Scale(pulse, scale);

  //otherwise we need to do the full algorithm
  Pulse pulseout(pulse);
  for(size_t i=0; i<pulse.size(); ++i){
    //in case delay is not whole bin, interpolate value linearly
    double delpoint = i - delay;
    int delbin = std::floor(delpoint);
    double delfrac = delpoint-delbin;
    if(wrap){
      delbin %= pulse.size(); //wrap pulse around edges
      if(delbin < 0)
	delbin = pulse.size() + delbin;
    }
    double outval = 0;
    if(delbin >=0 && delbin < (int)pulse.size()-1)
      outval = pulse[delbin] + delfrac*(pulse[delbin+1]-pulse[delbin]);
    else if(delbin == -1)
      outval = pulse[0]*delfrac;
    else if(delbin == (int)pulse.size() - 1){
      if(!wrap)
	outval = pulse[delbin]*(1.-delfrac);
      else
	outval = pulse[delbin] + delfrac*(pulse[0]-pulse[delbin]);
    }
    pulseout[i] = outval*scale;
  }
  return pulseout;
}

int FakePulseBuilder::GenerateFakeDataFiles(const std::string& controlfile,
					    const std::string& outdir)
{
  _outdir = outdir;
  //make sure the controlfile exists and is readable
  std::ifstream fin(controlfile.c_str());
  if(!fin){
    std::cerr<<"FakePulseBuilder ERROR: Unable to open control file "
	     <<controlfile<<std::endl;
    return -1;
  }
  
  int linesprocessed = 0;
  std::string line;
  //we'll find out if outdir exists while processing, so let's go!
  while(std::getline(fin, line)){
    int retval = ExecuteControlLine(line);
    //exit on bad return since errors might get lost in the noise
    if(retval < 0)
      return retval;
    ++linesprocessed;
  }
  
  //make sure the last file got written
  CloseCurrentOutputFile();
  
  std::cout<<"Successfully processed "<<linesprocessed<<" lines "<<std::endl;
  return linesprocessed;
}

uint16_t clampu16(double x)
{
  if(x < 0) return 0;
  if(x > 0xFFFF) return 0xFFFF;
  return x;
}

int FakePulseBuilder::ExecuteControlLine(const std::string& line)
{
  //make sure it's not just a comment
  if(line[0] == '#' || line.empty())
    return 0;
  
  std::vector<ControlFunction> funcs = ParseControlLine(line);
  //there shoudl be at minimum 1 argument, or else there was an error
  if(funcs.empty()){
    std::cerr<<"FakePulseMaker ERROR: ParseControlLine returned empty vector\n";
    return -1;
  }
  
  int status = 0;
  
  //first function should always be either Copy or Replace
  const ControlFunction& target = funcs[0];
  if(target.name != "Copy" && target.name != "Replace"){
    std::cerr<<"FakePulseBuilder ERROR: First call in line must be either "
	     <<"'Copy' or 'Replace'"<<std::endl;
    return -2;
  }
  //make sure we have the right number of args
  if( (target.name == "Copy" && !ArgCheck(target,2)) ||
      (target.name == "Replace" && !ArgCheck(target,3)) )
    return -3;
  
  //queue up the event to be copied
  const std::string& series = target.args[0];
  long event = std::stoi(target.args[1]);
  status = PrepareNewTargetEvent(series, event);
  if(status < 0)
    return status;

  //if there is a replace action, build the fake pulse
  if(target.name == "Replace"){
    //there better be more args
    if(funcs.size() < 2){
      std::cerr<<"FakePulseBuilder ERROR: Replace called with no input pulse"
	       <<std::endl;
      return -4;
    }
    Pulse sumpulse = EvaluateControlFunction(funcs[1]);
    for(size_t i=2; i<funcs.size(); ++i){
      Pulse pulse2 = EvaluateControlFunction(funcs[i]);
      if( pulse2.size() != sumpulse.size() ){
	std::cerr<<"FakePulseBuilder ERROR: Fake pulses generated with "
		 <<"different sample numbers!\n";
	return -5;
      }
      //sum in place
      std::transform(sumpulse.begin(), sumpulse.end(), pulse2.begin(),
		     sumpulse.begin(), std::plus<double>());
    }
    
    //make sure it fits in the target
    uint32_t detcode = stoul(target.args[2]);
    PulseData pd = _targetevents.LoadPulse(series, event, detcode);
    if(pd.GetRawPulse().size() == 0){
      std::cerr<<"FakePulseBuilder ERROR no target pulse "<<series
	       <<", "<<event<<", "<<detcode<<" found!\n";
      return -6;
    }
    if(pd.GetRawPulse().size() != sumpulse.size()){
      std::cerr<<"FakePulseBuilder ERROR: Fake pulse has different size "
	       <<"than target pulse!\n";
      return -7;
    }
    //copy it in
    const int PULSEDATAHEADSIZE = 12*sizeof(uint32_t);
    char* pulsestart = 
      _targetevents.GetBuffer() + 
      _targetevents.GetRawDataMap().FindRecord(event,0,detcode).offset - 
      _targetevents.GetCurrentRecord().offset + 
      PULSEDATAHEADSIZE;
    //plain copy doesn't deal with overflow or negatives well, so force clamp
    std::transform(sumpulse.begin(), sumpulse.end(), (uint16_t*)pulsestart,
		   clampu16);
    //this probably won't work, but we'll see
    //std::copy(sumpulse.begin(), sumpulse.end(), (uint16_t*)pulsestart);
  }

  return status;
}



Pulse FakePulseBuilder::EvaluateControlFunction(const ControlFunction& func)
{
  //checking correct number of arguments is done in ParseControlLine
  Pulse result;
  double scale = 1, delay=0;
  bool wrapdelay=false;
  if(func.name == "TemplateByName" && ArgCheck(func,4,6)){
    // args are series, dump, detnum, channel, scale, delay
    int detnum = stoi(func.args[2]);
    std::string chan = func.args[3];
    result = _templates.GetTemplateByName(detnum, chan,
					  func.args[0], stoi(func.args[1]));
    if(func.args.size() > 4)
      scale = stod(func.args[4]);
    if(func.args.size() > 5)
      delay = stod(func.args[5]) * _templates.GetSampleRate(detnum,chan,"");
  }
  else if(func.name == "TemplateByCode" && ArgCheck(func,4,6)){
    //args are series, dump, decode, suffix, scale, delay
    uint32_t detcode = stoul(func.args[2]);
    result = _templates.GetTemplateByCode(detcode, func.args[3],
					  func.args[0], stoi(func.args[1]));
    if(func.args.size() > 4)
      scale = stod(func.args[4]);
    if(func.args.size() > 5)
      delay = stod(func.args[5]) * _templates.GetSampleRate(detcode);
  }
  else if(func.name == "RawPulse" && ArgCheck(func,3,5)){
    //args are series, event, detcode, scale, delay
    PulseData pd = _rawpulses.LoadPulse(func.args[0], stoi(func.args[1]),
					stoul(func.args[2]));
    result = pd.GetRawPulse();
    if(func.args.size() > 3)
      scale = stod(func.args[3]);
    if(func.args.size() > 4) //convert delay in s to bins
      delay = stod(func.args[4]) / (1.e-9*pd.GetSampleDt()); //sampledt in ns
    wrapdelay=true;
  }
  else{
    std::cerr<<"FakePulseBuilder ERROR: Incorrect num args or unknown function "
	     <<func.name<<" passed to EvaluateControlFunction \n";
  }
  return TransformPulse(result, scale, delay, wrapdelay);
}
  
  
std::vector<ControlFunction> 
FakePulseBuilder::ParseControlLine(const std::string& line)
{
  /* Control files should have the following syntax:
     
     #comment (empty lines are OK too)
     Function(arg, arg, arg)
     #OR
     Function(arg, arg, arg) => Func(arg, arg, arg) + Func(arg, arg, arg) + ...

     Spaces between arguments are note required. Spaces around delimiters
     '=>' and '+' are required
  */

  //comments and empty lines should already be checked for
  std::vector<ControlFunction> result;
  //check for a replace delimiter
  size_t pos = line.find(" => ");
  if(pos == std::string::npos){
    //single function line only
    result.push_back(ParseControlFuncStr(line));
  }
  else{
    //first func comes before delimeter
    result.push_back(ParseControlFuncStr(line.substr(0,pos)));
    pos += 4;
    //there must be at least one more function 
    size_t pos2 = std::string::npos;
    while( (pos2 = line.find(" + ",pos)) != std::string::npos){
      result.push_back(ParseControlFuncStr(line.substr(pos, pos2-pos)));
      pos = pos2+3;
    }
    //should be one more at end
    result.push_back(ParseControlFuncStr(line.substr(pos)));
    //do some error checking???
  }
  return result;
}

const std::string& strip(std::string& s){
  while(s[0] == ' ' || s[0] == '\t' ||
	s[0] == '\'' || s[0] == '"')
    s.erase(0,1);
  while(s[s.size()-1] == ' ' || s[s.size()-1] == '\t' || 
	s[s.size()-1] == '\'' || s[s.size()-1] == '"')
    s.erase(s.size()-1,1);
  return s;
}

ControlFunction FakePulseBuilder::ParseControlFuncStr(std::string s)
{
  //remove leading and trailing whitespace
  strip(s);
  
  ControlFunction result;
  result.name="ERROR";
  
  //determine name
  size_t pos = s.find('(');
  if(pos == std::string::npos || s[s.size()-1] != ')'){
    std::cerr<<"FakePulseBuilder ERROR:  Control function must have form "
	     <<"'Name(arg, arg, arg)', got "<<s<<std::endl;
  }
  result.name = s.substr(0,pos);
  //now check for arguments
  size_t pos2 = std::string::npos;
  while( (pos2 = s.find(',', pos+1)) != std::string::npos){
    //we've found a new comma
    std::string subfunc = s.substr(pos+1,pos2-(pos+1));
    result.args.push_back(strip(subfunc));
    pos = pos2;
  }
  //found all commas, but there may be one more arg
  if( s.size() > pos+2 ){
    std::string subfunc = s.substr(pos+1, s.size()-1-(pos+1));
    result.args.push_back(strip(subfunc));
  }
  //any other consistency checks???
  return result;
}

bool FakePulseBuilder::ArgCheck(const ControlFunction& func, 
				size_t min, size_t max)
{
  bool pass = true;
  if(max < min) max = min;
  if(func.args.size() < min || func.args.size() > max){
    std::cerr<<"FakePulseBuilder ERROR function "<<func.name
	     <<"expects ";
    if(max == min)
      std::cerr<<max;
    else
      std::cerr<<"between "<<min<<" and "<<max;
    std::cerr<<" arguments; got "<<func.args.size();
    return pass = false;
  }

  return pass;
}


int FakePulseBuilder::OpenNewOutputFile(const std::string& series, int dump)
{
  //first close the current file
  int status=0;
  status = CloseCurrentOutputFile();
  if(status < 0)
    return status;
  
  
  
  //do we need to make a subdirectory? 
  std::string outfile = RawDataSeeker::GetRawFilePath(_outdir,series,dump,
						      _createseriesdirs);
  //make directory
  status = system((std::string("mkdir -p $(dirname ")+outfile+")").c_str());
  //this causes problems, so assume we succeeded and just fail at file creation
  /*if(status < 0){
    std::cerr<<"FakePulseBuilder::Unable to create output directory for file "
	     <<outfile<<std::endl;
    return status;
    }
  */
  
  //open file
  _fout = gzopen(outfile.c_str(),"wb");
  if(!_fout){
    std::cerr<<"FakePulseBuilder ERROR raised trying create output file "
	     <<outfile<<std::endl;
    return -1;
  }
  
  //copy the file header from the fake source
  int headsize = _targetevents.LoadPreEventHeader(series, dump);
  if( headsize <= 0){
    std::cerr<<"FakePulseBuilder ERROR loading file header for series "
	     <<series<<" dump "<<dump<<std::endl;
    return headsize;
  }
  
  int writesize = gzwrite(_fout, _targetevents.GetBuffer(), headsize);
  if(writesize != headsize){
    std::cerr<<"FakePulseBuilder ERROR copying file header!\n";
    return -2;
  }

  _foutname = outfile;

  return status;
  
}

int FakePulseBuilder::WriteCurrentEvent()
{
  int status = 0;
  RawDataBlock current = _targetevents.GetCurrentRecord();
  if(current.eventid > 0 && current.recordtype==0 && current.detectorcode==0){
    //a full event is loaded.  Not sure what to do if only part of event loaded
    std::cout<<"Copying event "<<current.eventid<<" to "<<_foutname<<std::endl;
    int nwrite = gzwrite(_fout, _targetevents.GetBuffer(), current.length);
    if(nwrite != current.length){
      std::cerr<<"FakePulseBuilder ERROR writing event "
	       <<current.eventid<<" to file \n";
      status = -1;
    }
  }
  return status;
}

int FakePulseBuilder::CloseCurrentOutputFile()
{
  //check for no-op
  if(!_fout)
    return 0;

  //first, copy any remaining events in the current file
  int status = WriteCurrentEvent();
  if(status < 0)
    return status;
  //do we need to write the remainder of the file??
  if(_copyallevents){
    long currevt = _targetevents.GetCurrentRecord().eventid;
    while(_targetevents.GetRawDataMap().IsRecordInCurrentFile(++currevt)){
      status = _targetevents.LoadRecord("", currevt);
      if(status < 0)
	return status;
      status = WriteCurrentEvent();
      if(status < 0)
	return status;
    }
  }
  //end of file written, close out
  gzclose(_fout);
  _fout = NULL;
  _foutname = "";

  return status;
  
}

int FakePulseBuilder::PrepareNewTargetEvent(const std::string& series, 
					    long event)
{
  int status = 0;
  int dump = RawDataSeeker::GetDumpNumFromEvent(event);
  //do we need to open a new file?
  std::string newfilename = RawDataSeeker::GetRawFilePath(_outdir,series,dump,
							  _createseriesdirs);
  int copyevent = -1;
  if(newfilename != _foutname){
    status = OpenNewOutputFile(series,dump);
    if(status < 0)
      return status;
  }
  
  int currentevt = _targetevents.GetCurrentRecord().eventid;
  if(currentevt != event){
    int copyevent = currentevt+1;
    if(currentevt > 0){ //an event is already loaded
      //if there is an event already open, we need to write it
      status = WriteCurrentEvent();
      if(status<0)
	return status;
    }
    else //no event loaded yet
      copyevent = _targetevents.GetRawDataMap().GetFirstRecord().eventid;
    
    if(_copyallevents){
      //we need to copy all events before ours
      while(copyevent < event){
	status = _targetevents.LoadRecord(series, copyevent);
	if(status < 0)
	  return status;
	status = WriteCurrentEvent();
	if(status < 0)
	  return status;
	++copyevent;
      }
    }
    //finally load our event
    status = _targetevents.LoadRecord(series,event);

  }
  
  return status;
}

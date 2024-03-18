///////////////////////////////////////////////////////////////////////////////// 
//main()
//Author:  B. Loer
//Description: The central routine for running BatViewer pulse viewer
//
//File Import By: B. Loer
//Creation Date: Dec. 19, 2011
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

//Standard Libaries
#include <iostream>
#include <list>
#include <sstream>
#include <iomanip>
#include "time.h"
#include <cctype>

//ROOT Libraries
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TSystem.h"

//CDMS Libraries
#include "CommandLineHelper.h"

#include "RawDataReader.h"
#include "AdminData.h"
#include "GPSData.h"
#include "HistoryData.h"
#include "PulseData.h"
#include "EventBuilder.h"
#include "TriggerData.h"

#include "UserDataManager.h"
#include "DmmDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "GpibDataManager.h"
#include "FilterDataManager.h"

#include "DetectorConfigManager.h"
#include "PulseTools.h"

#include "RootGraphix.h"
#include "EventPlotter.h"
#include "TraceSaver.h"

using namespace std;

/// Pause execution until user hits enter. Return whatever they entered  
string WaitForUser(const string& msg="Press <enter> to continue...")
{
  string dummy;
  if(!msg.empty())
    cout<<msg<<endl;
  getline(cin,dummy);
  return dummy;
}

/// Various utility functions to convert between event number w/without dump
const uint64_t DUMPMOD = 10000;

inline uint64_t GetDumpNumber(uint64_t eventnum)
{ return eventnum / DUMPMOD; }

inline uint64_t GetEventNumberWithinDump(uint64_t eventnum)
{ return eventnum % DUMPMOD; }

inline uint64_t BuildFullEventNumber(uint64_t dumpnum, uint64_t eventnum)
{ return dumpnum*DUMPMOD + eventnum; }

/// Create a filename from a series and dump number
std::string BuildFilename(const std::string& seriesname, uint64_t dumpnumber)
{
  if(dumpnumber == 0 ) dumpnumber = 1;
  std::stringstream filename;
  filename<<seriesname<<"_F"<<setw(4)<<setfill('0')<<dumpnumber;
  return filename.str();
}



/////////////////// BEGIN MAIN //////////////////////////////

int main(int argc, char** argv){
  
  /////////////// INITIALIZATION ///////////////////////////
  //check the command line arguments
  enum ARGS { RAWFILE=1, EVENT, FNAME, NARGCHOICES};
  
  CommandLineHelper cmd("BatViewer [options] <raw file/directory> [<event#>] [<outputfile>]");
  cmd.AddCommandSwitch(' ',"nohardcode",
		       "Don't assume 5 towers with 3 zips each");
  cmd.AddCommandSwitch('d',"detector",
		       "Plot only detector <index>","index");
  cmd.AddCommandSwitch(' ',"eventlist",
		       "Display/print all events in <listfile>", "listfile");
  cmd.AddCommandSwitch(' ',"slideshow",
		       "Auto increment to the next event after <s> seconds, without waiting for user input. Most useful with the eventlist option","s");
  cmd.AddCommandSwitch(' ',"scale",
		       "Size of root canvas as fraction of total screen size",
		       "size");
  cmd.AddCommandSwitch('s',"eventskip",
		       "Skip through <n> events with each view by default",
		       "n");
  cmd.AddCommandSwitch(' ',"tracefile",
		       "File in which to store raw traces (default CdmsTraces.root",
		       "file");
  cmd.AddCommandSwitch('a',"save-all-traces",
		       "Save all raw traces viewed to a file");
  cmd.AddCommandSwitch('n',"nodraw",
		       "Don't actually display anything; useful for saving traces");
  cmd.AddCommandSwitch(' ',"skip-empty",
		       "Skip all events with no valid ZIP waveforms");
  
  int nargs = cmd.ProcessCommandLine(argc,argv);
  if(nargs<1 || nargs>3)
    cmd.PrintSwitches(/*exit =*/ true);
  
  bool hardcodeplots = cmd.GetNCallsToOption("nohardcode") == 0;
  int detector_num = -1;
  if(cmd.GetNCallsToOption("detector") > 0){
    detector_num = atoi(cmd.GetArgumentCall("detector"));
  }
  std::string eventlistname="";
  if(cmd.GetNCallsToOption("eventlist") > 0){
    eventlistname = cmd.GetArgumentCall("eventlist");
  }
  bool nodraw = cmd.GetNCallsToOption("nodraw") > 0;
  
  std::ifstream eventlistfile;
  if(eventlistname != ""){
    eventlistfile.open(eventlistname.c_str());
    if(!eventlistfile.is_open()){
      std::cerr<<"ERROR: Unable to open event list file "<<eventlistname
	       <<std::endl;
      return 1;
    }
  }
  
  bool skipempty = cmd.GetNCallsToOption("skip-empty") > 0;
  
  int eventskip=1;
  if(cmd.GetNCallsToOption("eventskip") > 0)
    eventskip = atoi(cmd.GetArgumentCall("eventskip"));
  
  bool slideshow = cmd.GetNCallsToOption("slideshow") > 0;
  int slideshowwait = 0;
  if(slideshow)
    slideshowwait = atoi(cmd.GetArgumentCall("slideshow"));
  double canvasscale = 0;
  if(cmd.GetNCallsToOption("scale") > 0)
    canvasscale = atof(cmd.GetArgumentCall("scale"));
  
  //raw trace saver
  std::string tracesaverfile="CdmsTraces.root";
  if(cmd.GetNCallsToOption("tracefile") > 0)
    tracesaverfile = cmd.GetArgumentCall("tracefile");
  TraceSaver traceSaver(tracesaverfile);
  
  bool savealltraces = false;
  if(cmd.GetNCallsToOption("save-all-traces") > 0)
     savealltraces = true;
  
  long long seek_event = 0;

  if(argc > EVENT)
    seek_event = atoi(argv[EVENT]);
  else if(eventlistname != "")
    eventlistfile >> seek_event;

  

  bool saveimage = argc > FNAME;
  std::string imgfile = saveimage ?  argv[FNAME] : "";
  
  bool interactive = !saveimage && !slideshow && !nodraw;
  
  // find out if we're looking at a series or a single file (like one_event.dat)
  // first remove all trailing '/' characters
  string filearg = argv[RAWFILE];
  while( filearg.find_last_of('/') == filearg.size()-1 )
    filearg.resize(filearg.size()-1);
  //series directories have format [##]######_####
  //ignore first two that may or may not be there
  size_t pos = filearg.find_last_of('/');
  string series = filearg.substr( pos==string::npos ? 0 : pos+1);
  bool seriesgood = series.size() > 10;
  if(seriesgood){
    for(size_t i=series.size()-11; i<series.size(); ++i){
      if( (i==series.size()-5 && series[i] != '_') || 
	  (i!=series.size()-5 && !isdigit(series[i]) ) ){
	seriesgood = false;
	break;
      }
    }
  }
  if(!seriesgood){
    series = "";
    cout<<"Viewing events in file "<<filearg<<endl;
  }
  else{
    cout<<"Viewing series "<<series<<" in directory "<<filearg<<endl;
  }

  //create the filenames
    
  vector<PulseData> veto;
  map<int,vector<PulseData> > zips;
  vector<PulseData> noisemons;
  
  RawDataReader reader;
  reader.RegisterZipPulseMap(&zips);
  reader.RegisterVetoPulseVector(&veto);
  reader.RegisterNoiseMonitorPulseVector(&noisemons);
  AdminData admin;
  reader.RegisterAdminData(&admin);
  TriggerData trigger;
  reader.RegisterTriggerData(&trigger,50/*max towers*/); 
  EventPlotter plotter(hardcodeplots);
  if(canvasscale>0 && !nodraw)
    plotter.GetCanvas(canvasscale);
  //string to parse user input
  string response = "";
  string directoryname = filearg+"/";
  if(series == "")
    directoryname="";
  string filename = filearg;
  
  int status = 1;
  while(response != "q" && status > 0){
    // ************** determine what event to read next ***********
    if(response!="")
      seek_event = atoi(response.c_str());
    else if(seek_event < 0){
      break;
    }
    uint64_t seek_sub_event = GetEventNumberWithinDump(seek_event);
    if(seek_sub_event == 0){
      seek_sub_event += 1;
      seek_event += 1;
    }
    uint64_t seek_dump = GetDumpNumber(seek_event);
    uint64_t current_event = admin.GetEvent();
    //uint64_t current_sub_event = GetEventNumberWithinDump(current_event);
    uint64_t current_dump = GetDumpNumber(current_event);
    
    //handle several cases
    bool open_new_file = false;
    if(current_dump == 0){
      //this is the first loop
      open_new_file = true;
      if(seek_dump == 0){
	if(series != "") //go to first dump in series
	  seek_dump = 1;
	else{
	  //get dump number from filename
	  std::string filenamecopy = filename;
	  if(filenamecopy.substr(filename.size()-3) == ".gz")
	    filenamecopy.erase(filenamecopy.size()-3);
	  seek_dump = atoi(filenamecopy.substr(filenamecopy.size()-4).c_str());
	}
      }
      seek_event = BuildFullEventNumber(seek_dump, seek_sub_event);
    }
    else if(seek_dump == 0){
      //stay in same dump
      seek_dump = current_dump;
      seek_event = BuildFullEventNumber(seek_dump, seek_sub_event);
    }
    if(seek_dump != current_dump || seek_event < (long long)current_event)
      open_new_file = true;
    
    // ************** If we need to, open a new raw data file ********** 
    if(open_new_file){
      reader.CloseRawDataFile();
      if(series != "")
	filename = BuildFilename(series, seek_dump);
      reader.OpenRawDataFile(directoryname, filename);
      reader.ReadFileHeader(false);
    }
    // ************** Seek forward until we get to the requested event *****
    while(admin.GetEvent() !=  seek_event && status > 0){
      int skip_events = GetEventNumberWithinDump(seek_event) - 1 -
	( open_new_file ? 0 : GetEventNumberWithinDump(admin.GetEvent()) );
      reader.Clear();
      status = reader.ReadRawDataRecord(skip_events);
      if(open_new_file && GetEventNumberWithinDump(admin.GetEvent()) != 1){
	//cout<<"First event in this file is "<<admin.GetEvent()<<", not 1"<<endl;
	if(seek_sub_event == 1) //return it anyway
	  break;
      }
      open_new_file = false;
    }// end while loop seeking for event
    
    // ************* check the file status ***********************
    if(status < 0){
      cerr<<"An error occurred trying to read event "<<seek_event
	  <<" from file "<<filename<<"! Aborting.\n";
      break;
    }
    else if(status == 0){ //EOF
      std::cerr<<"Reached EOF for "<<filename<<std::endl;
      //see if the next file is available if we're in a series
      if(series=="")
	break;
      cout<<"Checking if there is another dump file available..."<<endl;
      seek_event = BuildFullEventNumber(++seek_dump,0);
      response = "";
      status = 1;
      continue;
    }
    

    seek_event = admin.GetEvent() + eventskip;
    if(eventlistname != ""){
      if( !(eventlistfile >> seek_event) ){
	std::cout<<"Reached end of event list file"<<std::endl;
	seek_event = -1;
      }
    }


    if(skipempty){
      if(zips.empty() || (detector_num >= 0 && zips.count(detector_num)==0)){
	cout<<"Event "<<admin.GetEvent()
	    <<" is empty for requested detectors! Skipping..."
	    <<endl;
	continue;
      }
    }
    
    if(!nodraw){
      // ************ Draw the event; prompt user for what to do next ********
      std::string triginfo = trigger.GetTriggerInfoTable();
      if(detector_num < 0){
	plotter.DrawOneEvent(zips,veto,noisemons, 
			     reader.GetEventType(), reader.GetEventCategory(),
			     &admin, interactive ,triginfo);
      }
      else{
	map<int,vector<PulseData> >::const_iterator it = zips.find(detector_num);
	if(it == zips.end()){
	  cerr<<"No detector "<<detector_num<<" in this event!\n";
	  return 1;
	}
	plotter.DrawOneDetector(it->second, detector_num, &admin);
	RootGraphix::Lock glock = RootGraphix::AcquireLock();
	//plotter.GetCanvas()->Modified();
	plotter.GetCanvas()->Update();
	
      }
    }
    
    cout<<"--------------------------------------------------------------------"
	<<endl;
    cout<<"Current event is series "<<admin.GetSeries()
	<<" Event "<<admin.GetEvent()<<endl;
    
    //cout<<triginfo<<flush;
    if(!nodraw && saveimage){
      RootGraphix::Lock glock = RootGraphix::AcquireLock();
      //make sure the canvas is fully updated
      plotter.GetCanvas()->SaveAs(imgfile.c_str());
      if(!slideshow)
	break;
    }
    //see if we need to save the raw traces
    if(savealltraces){
      if(detector_num < 0)
	traceSaver.SaveDetectorMap(zips, &admin);
      else
	traceSaver.SaveDetector(zips[detector_num], &admin);
    }
   
    
    if(slideshow){
      if(slideshowwait>0)
	gSystem->Sleep(slideshowwait*1000);
    }
    else{
      //do this in a loop to check for specific user commands
      while(true){
	cout<<"Enter: "<<"\t<#> to view that event (with or w/out dump prefix)"
	    <<"\n\t <enter> for event "<<seek_event
	    <<"\n\t 's' to save the raw traces for this event"
	    <<"\n\t 'q' to quit"
	    <<endl;
	response = WaitForUser("");
	
	if(response == "s"){
	  if(detector_num < 0)
	    traceSaver.SaveDetectorMap(zips, &admin);
	  else
	    traceSaver.SaveDetector(zips[detector_num], &admin);
	}
	else{
	  break;
	}
	
      }//end while query user loop
    }//end ifnot slideshow mode
  }//end while response != 'q' loop
  
  // ************** Clean up ****************
  reader.CloseRawDataFile();
  cout<<"BatViewer exiting..."<<endl;
  //make sure all gui events are destroyed in the right order
  RootGraphix::AcquireLock();
  return 0;
  
} //end main()   DONE!!!



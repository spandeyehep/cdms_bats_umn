#ifndef EVENTPLOTTER_h
#define EVENTPLOTTER_h

/**
   @class EventPlotter
   @author Ben Loer
   @brief This class plots the raw pulses for a single CDMS trigger
*/


#include <vector>
#include <map>
#include "PulseData.h"

class AdminData;
class TCanvas;
class TLegend;
class TPad;
class TMultiGraph; 

class EventPlotter{
 public:
  EventPlotter(bool hardcodeplots=true);
  ~EventPlotter();
  
  /// Draw the event; returns number of channels
  int DrawOneEvent(const map<int,vector<PulseData> >& zips,
		   const vector<PulseData>& veto,
		   const vector<PulseData>& noisemons,
		   uint32_t EventType,
		   uint32_t EventCategory,
		   AdminData* admin = 0,
		   bool interactive = true,
		   const std::string& trigtxt="");
  int DrawOneDetector(const vector<PulseData>& zip,
		      int detector_num, 
		      AdminData* admin = 0,
		      bool drawlegend = true);
  int DrawGenericPulseVec(const vector<PulseData>& pulses,
			  bool splitpad=false,
			  const std::string& title="",
			  bool legend_and_color = true,
			  double offsetincrement = 20);
  
  /// Get pointer to drawing canvas
  TCanvas* GetCanvas(double scale=0);

 private:
  bool _hardcodeplots; ///< Do we hardcode 15 zips for simplicity?
  ///Canvas on which we do our plotting
  TCanvas* fCanvas; 
  ///Utility function to draw the plot legend
  TLegend* AddLegend(TPad* pad, bool draw=true);
};



#endif /*EVENTPLOTTER_h*/

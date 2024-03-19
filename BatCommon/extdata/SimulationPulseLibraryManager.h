////////////////////////////////////////////////////////////////////////////////////
//
// Class Name: SimulationPulseLibraryManager
// Authors: A. Anderson
// Description:  This class reads the ROOT file containing the pulse library to be
// used for the pulse-based pulse simulation (as opposed to the template-based one)
//
// File Import By: A. Anderson
// Creation Date: 6 October 2013
//
////////////////////////////////////////////////////////////////////////////////////                                                                                        

#ifndef SIMULATIONPULSELIBRARYMANAGER_H
#define SIMULATIONPULSELIBRARYMANAGER_H

#include <TFile.h>
#include <TTree.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

class SimulationPulseLibraryManager
{
public:
    SimulationPulseLibraryManager();
    ~SimulationPulseLibraryManager();

    void setLibraryFile(vector<string> filename, map<int, int> detectorMap);
    int getLibraryFileNum(string filename);
    void readEvent(int jevt, int detNum, string filename);
    void readRandomEvent(int detNum, string filename);
    map<string, vector<double>* > getPulseMap(int detNum);
    map<string, double> getChanAmps();
    double getEventNum();
    double getSeriesNum();
    double getDMCXPosition();
    double getDMCYPosition();
    double getDMCZPosition();
    double getDMCRecoilEnergy();
    
private:
    map<string, double> fEvRQList;
    vector<TFile*> libraryFiles;
    vector<vector<TTree*> > treeVector;     // vector of vectors for trees => outer vector has 1 entry per input file
    map<int, map<string, vector<double>* > > templatePulseMap; // (detnum, (chanName, trace))
    map<string, double> ampMap; 
    vector<double>* chanAmps;
    vector<string>* chanNames;
    double eventNum;
    double seriesNum;
    double DMCXPosition, DMCYPosition, DMCZPosition;
    double DMCRecoilEnergy;
    stringstream ss;
};

#endif

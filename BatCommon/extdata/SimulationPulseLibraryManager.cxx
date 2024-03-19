////////////////////////////////////////////////////////////////////////////////////
//
// Class Name: SimulationPulseLibraryManager
// Authors: A. Anderson
// Description:  This class reads the ROOT file containing the pulse library to be
// used for the pulse-based pulse simulation (as opposed to the template-based one)
//
// File Import By: A. Anderson
// Creation Date: 6 October 2013
// Updates:   
//      2016/10/27 Jorge D. Morales  
//                 The tree vector now supports a detector map not starting in zip1
//                 (the vector is filled with a 0 to preserve vector size, and keep
//                 the other functions calling with the detNum compatible)
// 
////////////////////////////////////////////////////////////////////////////////////

#include "SimulationPulseLibraryManager.h"
#include "BatRootTypes.h"

#include <algorithm>
#include <iostream>

SimulationPulseLibraryManager::SimulationPulseLibraryManager()
{ }


SimulationPulseLibraryManager::~SimulationPulseLibraryManager()
{
    while(!libraryFiles.empty())
    {
	// close the TFiles if open
	if(libraryFiles.back()->IsOpen())
	    libraryFiles.back()->Close();
	
	// delete the TFiles
	delete libraryFiles.back();
	libraryFiles.pop_back();
    }
}


void SimulationPulseLibraryManager::setLibraryFile(vector<string> filename, map<int, int> detectorMap)
{
    int nfiles = filename.size();
    
    // reset and resize the vector of trees
    treeVector.clear();
    treeVector.resize(nfiles);

    // loop over vector of trees
    for(int jfile = 0; jfile < nfiles; jfile++)
    {
	// try to open the root file
	libraryFiles.push_back(new TFile(filename[jfile].c_str(), "READ"));

	cout << "**** Opening simulation pulse library: " << filename[jfile] << endl;
	
	// check if the file is corrupted and quit if problems
	if(libraryFiles[jfile]->IsZombie())
	{
	    std::cout << "SimulationPulseLibraryManager::setLibraryFile: ERROR! Problem reading file "
		      << filename[jfile] << "! May be corrupted or nonexistent." << std::endl;
	    exit(EXIT_FAILURE);
	}

	chanAmps = 0;
	chanNames = 0;
	eventNum = 0;
	seriesNum = 0;
	DMCXPosition = 0;
	DMCYPosition = 0;
	DMCZPosition = 0;
	DMCRecoilEnergy = 0;

	std::vector<std::string> mandatoryBranchNames(5);
	mandatoryBranchNames[0] = "chanAmps";
        mandatoryBranchNames[1] = "chanStr";
	mandatoryBranchNames[2] = "SIMEventNumber";
	mandatoryBranchNames[3] = "SIMSeriesNumber";
	mandatoryBranchNames[4] = "PTnorm";
	std::vector<std::string> optionalBranchNames(4);
	optionalBranchNames[0] = "SIMAvgX";
	optionalBranchNames[1] = "SIMAvgY";
	optionalBranchNames[2] = "SIMAvgZ";
	optionalBranchNames[3] = "SIMRecoilEnergy";
	
	// fill with 0's the Tree vector if first det is not zip1
	if( detectorMap.begin()->first > 1 ) {
	  for( int jj = 1 ; jj < detectorMap.begin()->first ; jj++ ) {
	    treeVector[jfile].push_back(0);
	  }
	}
        // setup file and trees for fake pulses
	map<int, int>::iterator it;
	for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
	{
	    int detNum = it->first;
	    string name = Form("zip%d",detNum);
	    treeVector[jfile].push_back((TTree*)libraryFiles[jfile]->Get(name.c_str()));
	    if(!treeVector[jfile][detNum-1])
	    std::cout << "ERROR in SimulationPulseLibraryManager::setLibraryFile: Cannot find "
	              << "tree " << Form("zip%d",detNum) << " in ROOT file!" << std::endl;
	    if(treeVector[jfile][detNum-1])
	    {
	      cout << "DETECTOR: zip" << detNum << "  JFILE: " << jfile << endl;
	      int nbranch = treeVector[jfile][detNum-1]->GetNbranches();

		map<string, vector<double>* > tempTraceMap;
		// loop over branches to create a temporary map of (chanName -> trace vector)
		// for this detector
		for(int jbranch = 0; jbranch < nbranch; jbranch++)
		{
		    string chanName = treeVector[jfile][detNum-1]->GetListOfBranches()->At(jbranch)->GetName();
		    
		    if(std::find(mandatoryBranchNames.begin(), mandatoryBranchNames.end(), chanName) == mandatoryBranchNames.end() &&
		       std::find(optionalBranchNames.begin(), optionalBranchNames.end(), chanName) == optionalBranchNames.end())
		    {
			// fill the pulse map on the first iteration
			if(tempTraceMap.count(chanName) == 0)
			  tempTraceMap.insert(std::pair<string, vector<double>* >(chanName, (vector<double>*)0));
		    }
		}
		
		// add the map of (chanName -> trace vector) for this detector to the map of
		// detNum -> (chanName -> trace vector)
		if(templatePulseMap.count(detNum) == 0)
		    templatePulseMap.insert(std::pair<int, map<string, vector<double>* > >(detNum, tempTraceMap));
		
		// loop over branches to set the address
		for(int jbranch = 0; jbranch < nbranch; jbranch++)
		{
		    string chanName = treeVector[jfile][detNum-1]->GetListOfBranches()->At(jbranch)->GetName();
		    
		    if(std::find(mandatoryBranchNames.begin(), mandatoryBranchNames.end(), chanName) == mandatoryBranchNames.end() &&
		       std::find(optionalBranchNames.begin(), optionalBranchNames.end(), chanName) == optionalBranchNames.end())
		    {
			treeVector[jfile][detNum-1]->SetBranchAddress(chanName.c_str(), &(templatePulseMap[detNum][chanName]));
		    }
		}
		
		// add other mandatory branches
		treeVector[jfile][detNum-1]->SetBranchAddress("chanAmps", &chanAmps);
		treeVector[jfile][detNum-1]->SetBranchAddress("chanStr", &chanNames);
		treeVector[jfile][detNum-1]->SetBranchAddress("SIMEventNumber", &eventNum);
		treeVector[jfile][detNum-1]->SetBranchAddress("SIMSeriesNumber", &seriesNum);

		// add other optional branches
		if(treeVector[jfile][detNum-1]->GetListOfBranches()->FindObject("SIMAvgX"))
		    treeVector[jfile][detNum-1]->SetBranchAddress("SIMAvgX", &DMCXPosition);
		else
		    DMCXPosition = -999999.;
		if(treeVector[jfile][detNum-1]->GetListOfBranches()->FindObject("SIMAvgY"))
		    treeVector[jfile][detNum-1]->SetBranchAddress("SIMAvgY", &DMCYPosition);
		else
		    DMCYPosition = -999999.;
		if(treeVector[jfile][detNum-1]->GetListOfBranches()->FindObject("SIMAvgZ"))
		    treeVector[jfile][detNum-1]->SetBranchAddress("SIMAvgZ", &DMCZPosition);
		else
		    DMCZPosition = -999999.;
		if(treeVector[jfile][detNum-1]->GetListOfBranches()->FindObject("SIMRecoilEnergy"))
		    treeVector[jfile][detNum-1]->SetBranchAddress("SIMRecoilEnergy", &DMCRecoilEnergy);
		else
		    DMCRecoilEnergy = -999999.;
	    }
	}
    }
}


void SimulationPulseLibraryManager::readEvent(int jevt, int detNum, string filename)
{
    // check that some files have been initialized
    if(libraryFiles.size() == 0)
    {
	std::cout << "SimulationPulseLibraryManager::readEvent: ERROR! You are trying to read an event but no files have been declared." << std::endl;
	exit(EXIT_FAILURE);
    }

    // loop over files to figure out which corresponds to the specified filename
    int nfiles = libraryFiles.size();
    int jfile = 0;
    ss.str(libraryFiles[jfile]->GetName());
    while(jfile < nfiles && filename != ss.str())
    {
	jfile++;
	ss.str(libraryFiles[jfile]->GetName());
    }

    // check that we actually found the filename
    if(filename != ss.str())
    {
	std::cout << "SimulationPulseLibraryManager::readEvent: ERROR! The file selected to read from was never initialized." << std::endl;
	exit(EXIT_FAILURE);
    }

    // get a random event index (uniform)
    int nentries = treeVector[jfile][detNum-1]->GetEntries();

    // actually get the event if this tree has nonzero entries
    if(nentries > 0){
      jevt = jevt % nentries;
      treeVector[jfile][detNum-1]->GetEntry(jevt);
    }else
    {   
	std::cout <<"SimulationPulseLibraryManager::readEvent: ERROR! Requested tree has no entries."<< std::endl;
	std::exit(EXIT_FAILURE);
    }

    // fill the amp map
    ampMap.clear();
    int nchan = chanNames->size();
    
    // check that vectors are the same size
    if(nchan != chanAmps->size())
	std::cout << "ERROR in SimulationPulseLibraryManager::readEvent: chanAmps is a different size than chanNames in input ROOT file" << std::endl;
    for(int jchan = 0; jchan < nchan; jchan++)
	ampMap[(*chanNames)[jchan]] = (*chanAmps)[jchan];
}


void SimulationPulseLibraryManager::readRandomEvent(int detNum, string filename)
{
    // check that some files have been initialized
    if(libraryFiles.size() == 0)
    {
	std::cout << "SimulationPulseLibraryManager::readEvent: ERROR! You are trying to read an event but no files have been declared." << std::endl;
        exit(EXIT_FAILURE);
    }

    // loop over files to figure out which corresponds to the specified filename
    int nfiles = libraryFiles.size();
    int jfile = 0;
    ss.str(libraryFiles[jfile]->GetName());
    while(jfile < nfiles && filename != ss.str())
    {
	jfile++;
	ss.str(libraryFiles[jfile]->GetName());
    }

    // check that we actually found the filename
    if(filename != ss.str())
    {
	std::cout << "SimulationPulseLibraryManager::readEvent: ERROR! The file selected to read from was never initialized." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // get a random event index (uniform)
    int nentries = treeVector[jfile][detNum-1]->GetEntries();

    if(nentries > 0)
    {
       int eventToPull = rand() % nentries;

       // get the event
       readEvent(eventToPull, detNum, filename);

       
    }
    else
    {
	std::cout <<"SimulationPulseLibraryManager::readRandomEvent: ERROR! Requested tree has no entries."<< std::endl;
	std::exit(EXIT_FAILURE);
    }
}


map<string, vector<double>* > SimulationPulseLibraryManager::getPulseMap(int detNum)
{
    return templatePulseMap[detNum];
}


map<string, double> SimulationPulseLibraryManager::getChanAmps()
{
    return ampMap;
}


double SimulationPulseLibraryManager::getEventNum()
{
    return eventNum;
}


double SimulationPulseLibraryManager::getSeriesNum()
{
    return seriesNum;
}

double SimulationPulseLibraryManager::getDMCXPosition()
{
    return DMCXPosition;
}

double SimulationPulseLibraryManager::getDMCYPosition()
{
    return DMCYPosition;
}

double SimulationPulseLibraryManager::getDMCZPosition()
{
    return DMCZPosition;
}

double SimulationPulseLibraryManager::getDMCRecoilEnergy()
{
    return DMCRecoilEnergy;
}

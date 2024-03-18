void merge_all(TString fileprefix, TString seriesNum, TString datatype, TString run, int fileMax)
{
   // ============= Setup ==================
   
   //Some constants
   Long64_t maxsize = 40000000000ULL; //40 gigs
   int basketsize = 32000;
   
   cout <<"files to loop over = " << fileMax << endl;
   
   //Construct the filename
   TString rootDir = "/micro/data3/cdmsbatsProd/"; 
   rootDir += run + "/dataReleases/";
   rootDir += fileprefix + "/unmerged/";   
   rootDir += datatype + "/"; 
   
   // ======= Check Number of Events per File and max zips - expects first file to be processed! ========
   
   cout <<"rootDir = " << rootDir << endl;

   TString testFileName = rootDir + seriesNum + "/" + fileprefix + "_" + seriesNum + "_F0001.root";

   cout <<"testFileName = " << testFileName << endl;
   TFile testFile(testFileName);
   TTree* testTree = testFile.Get("rqDir/eventTree");
   int nEventsPerFile = testTree->GetEntries();

   TTree* testConfigTree = testFile.Get("infoDir/userSettingsTree");
   int maxZips;
   testConfigTree->SetBranchAddress("MAX_ZIPS", &maxZips);
   testConfigTree->GetEntry(0);

   cout <<"\nNumber of events per file = " << nEventsPerFile <<"\nmaxzips = " << maxZips << endl;



   // ============= Merge Event Trees ==================

   //Chain files together to get a longer eventTree
   TChain chainedEventTree("rqDir/eventTree");
   chainedEventTree.SetMaxTreeSize(maxsize); //40 gigs

   //Loop over all files and chain
   for(int fileCtr=0; fileCtr < fileMax; fileCtr++)
   {
      TString dumpNum = Form("%d",fileCtr+1);
      int maxZeros = 4-dumpNum.Length();
      for(int letCtr=0; letCtr < maxZeros; letCtr++)   dumpNum = "0" + dumpNum;
      dumpNum = "F" + dumpNum;
      
      TString fileName = rootDir + seriesNum + "/" + fileprefix + "_" + seriesNum + "_" + dumpNum + ".root";
      cout <<"fileName = " << fileName << endl;
      
      //chaining event Tree
      chainedEventTree.AddFile(fileName);
      
  }
  
  int eventNumber = chainedEventTree.GetEntries();
  double discrepancy = fileMax*nEventsPerFile - eventNumber;

  //print the number of entries in this tree and check discrepancy w/ expected

  cout <<"\nNumber of entries in eventTree = " << eventNumber
       <<"\nNumber of merged files = " << fileCtr
       <<"\nexpected - found = " << discrepancy
       << endl;


  ofstream textFile;
  TString recordFile = "eventListSecondPass/" + run + "_" + datatype + "_eventList.txt";


  textFile.open(recordFile, ios_base::app);
  if(!textFile.fail())
  {
     textFile <<"\n" << seriesNum <<" events = " << eventNumber
	      <<", discrepancy = " << discrepancy;
     textFile.close();
  }

  //merge event tree
  TString outDir = "/micro/data3/cdmsbatsProd/";
  outDir += run + "/dataReleases/";
  outDir += fileprefix + "/merged/all/";
  outDir += datatype + "/";
  outDir += "merge_" + fileprefix + "_" + seriesNum + ".root";
  TFile* myFile = TFile::Open(outDir, "RECREATE");
  myFile->mkdir("infoDir");
  myFile->mkdir("detectorConfigDir");
  myFile->mkdir("rqDir");
  myFile->cd("rqDir");
  chainedEventTree.Merge(myFile, basketsize, "fast");

  // ============= Merge Zip Trees ==================  

  for(int zipNum=1; zipNum<=maxZips; zipNum++)
  {
     cout <<"Chaining zip = " << zipNum << endl;

     TChain chainedZipTree(Form("rqDir/zip%d", zipNum));
     chainedZipTree.SetMaxTreeSize(maxsize); //40 gigs
     
     //Loop over all files and chain
     for(int fileCtr=0; fileCtr < fileMax; fileCtr++)
     {
        TString dumpNum = Form("%d",fileCtr+1);
        int maxZeros = 4-dumpNum.Length();
        for(int letCtr=0; letCtr < maxZeros; letCtr++)   dumpNum = "0" + dumpNum;
        dumpNum = "F" + dumpNum;

        TString fileName =  rootDir + seriesNum + "/" + fileprefix + "_" + seriesNum + "_" + dumpNum + ".root";
//	cout <<"fileName = " << fileName << endl;

        //chaining event Tree
        chainedZipTree.AddFile(fileName);

     }

     if(chainedZipTree.GetEntries() != 0)
     {
	//merge zip tree
	TFile* myFileItr = TFile::Open(outDir, "UPDATE");
	myFileItr->cd("rqDir");
	chainedZipTree.Merge(myFileItr, basketsize, "fast");
     }

  }

  // ============= Merge Veto Trees ==================  

  if(datatype == "bg")
  {
     cout <<"Chaining veto " << endl;

     TChain chainedVetoTree("rqDir/vetoTree");
     chainedVetoTree.SetMaxTreeSize(maxsize); //40 gigs
     
     //Loop over all files and chain
     for(int fileCtr=0; fileCtr < fileMax; fileCtr++)
     {
        TString dumpNum = Form("%d",fileCtr+1);
        int maxZeros = 4-dumpNum.Length();
        for(int letCtr=0; letCtr < maxZeros; letCtr++)   dumpNum = "0" + dumpNum;
        dumpNum = "F" + dumpNum;

        TString fileName =  rootDir + seriesNum + "/" + fileprefix + "_" + seriesNum + "_" + dumpNum + ".root";
	//cout <<"fileName = " << fileName << endl;

        //chaining event Tree
        chainedVetoTree.AddFile(fileName);

     }

     //merge veto tree
     TFile* myFileItr = TFile::Open(outDir, "UPDATE");
     myFileItr->cd("rqDir");
     chainedVetoTree.Merge(myFileItr, basketsize, "fast");

  }// done with veto merge

  // ============= Store Info ==================  

  TString infoFileName =  rootDir + seriesNum + "/" + fileprefix + "_" + seriesNum + "_F0001.root";

  //Get the config trees
  TChain chainedConfigTree("infoDir/userSettingsTree");
  chainedConfigTree.AddFile(infoFileName);

  //update the file for config
  TFile* myFileItr1 = TFile::Open(outDir, "UPDATE");
  myFileItr1->cd("infoDir/");
  chainedConfigTree.Merge(myFileItr1, 32000, "fast");

  //Get the processingTree
  TChain chainedProcessingTree("infoDir/processingTree");
  chainedProcessingTree.AddFile(infoFileName);
  
  //update the file for processing
  TFile* myFileItr2 = TFile::Open(outDir, "UPDATE");
  myFileItr2->cd("infoDir/");
  chainedProcessingTree.Merge(myFileItr2, 32000, "fast");

  // ============= Get Zip Info  and DetectorConfig Trees ==================  
  
  for(int zipNum=1; zipNum<=maxZips; zipNum++)
  {
     
     //storing filterTrees
     TChain chainedInfoTree(Form("infoDir/filterTreeZip%d",zipNum));
     chainedInfoTree.AddFile(infoFileName);

     if(chainedInfoTree.GetEntries() != 0)
     {
	TFile* myFileItr3 = TFile::Open(outDir, "UPDATE");
	myFileItr3->cd("infoDir");
	chainedInfoTree.Merge(myFileItr3, basketsize, "fast");
     }

     //storing detectorConfig trees
     TChain chainedDetectorConfigTree(Form("detectorConfigDir/detectorConfigZip%d",zipNum));
     chainedDetectorConfigTree.AddFile(infoFileName);

     if(chainedInfoTree.GetEntries() != 0)
     {
	TFile* myFileItr3 = TFile::Open(outDir, "UPDATE");
	myFileItr3->cd("detectorConfigDir");
	chainedDetectorConfigTree.Merge(myFileItr3, basketsize, "fast");
     }

  }


  // ==============================================


  //Done

}

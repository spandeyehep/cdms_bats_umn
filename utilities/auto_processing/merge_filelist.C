#include "TList.h"
#include "TFile.h"
#include "TChain.h"
#include "TKey.h"
#include "TROOT.h"
#include <iostream>
#include <string>
using namespace std;

int merge_tree(const char* treepath, TString mtype, TList* files)
{
  cout<<"Merging tree "<<treepath<<" ..."<<endl;
  //assume we're already in the right path
  TFile* g=0;
  TIter nxfile(files);
  int nEvents;
  int tnEvents;
  while( g = ((TFile*)(nxfile())) ){
    TTree* tree = (TTree*)(g->Get(treepath));
    nEvents = tree->GetEntries();
    tnEvents = tnEvents + nEvents;
    //cout << "Number of events in current file: " << nEvents << endl;
    //cout << "Running total number of events: " << tnEvents << endl;
    delete tree;
  }
  TTree* outtree = 0;
  TIter nextfile(files);
  TFile* f=0;
  int nfiles;
  //int tsize;
  //int tzsize;
  if(mtype == "monobasket"){
    nfiles = files->GetSize();
    cout<<"Number Files to (sub)merge: "<<nfiles<<endl;
  }
  if(mtype == "polybasket"){
    nfiles = files->GetSize();
    cout<<"Number of Files to supermerge: "<<nfiles<<endl;
  }
  while( f = ((TFile*)(nextfile())) ){
    TTree* tree = (TTree*)(f->Get(treepath));
    if(mtype == "monobasket" || mtype == "polybasket"){
      //tsize = tree->GetTotBytes();
      //tzsize = tree->GetZipBytes();
      int basketsize;
      //Set these values to their defaults in case they are changed
      //should override any 'optimized' setting that comes with blinded files
      tree->SetAutoFlush(-30000000);
      tree->SetAutoSave(300000000);
      if(mtype == "monobasket"){
	//slow merge route...enforces a mono-basket.
	//Use this for submerged file generation
	basketsize = tnEvents * 9;
	tree->SetBasketSize("*",basketsize);
      }
      else if(mtype == "polybasket"){
	//slow merge but have only a few baskets
	//Use this for supermerging or remerging blinded data
	
	//int nbranch = tree->GetNbranches();
	//basketsize = tnEvents * (8 / 5) / (tsize / tzsize);
	basketsize = (tnEvents * (9.0 / 11.0));
	//int clustersize = tree->GetTotBytes();
	tree->SetBasketSize("*",basketsize);
	cout << "Total number of events is: " << tnEvents << endl;
	//cout << "Compression factor is: " << (tsize / tzsize) << endl;
        //cout << "Basket size is: " << basketsize << endl;
      }
      if(!tree){
	cerr<<"Unable to locate "<<treepath<<" in file "<<f->GetName()<<endl;
	return -1;
      }
      if(outtree==0){ //clone the first one to get the structure
	outtree = tree->CloneTree();
	if(!outtree){
	  cerr<<"There was a problem cloning "<<tree->GetName()<<" in file "
	      <<f->GetName()<<endl;
	  return -2;
	}
      }
      else{
	outtree->CopyAddresses(tree);
	if(outtree->CopyEntries(tree) <= 0){
	  cerr<<"There was an error copying entries from tree "
	      <<tree->GetName()<<" in file "<<f->GetName()<<endl;
	  return -3;
	}
      }
    //outtree->SetBasketSize("*",basketsize);
    }
    else{
      //Assume we're doing a fast merge if not slow or (slow) supermerge
      cout<<"Doing a fast sortbasketsbybranch merge"<<endl;
      if(!tree){
	cerr<<"Unable to locate "<<treepath<<" in file "<<f->GetName()<<endl;
	return -1;
      }
      if(outtree==0){ //clone the first one to get the structure
	outtree = tree->CloneTree(-1,"fast SortBasketsByBranch");
	if(!outtree){
	  cerr<<"There was a problem cloning "<<tree->GetName()<<" in file "
	      <<f->GetName()<<endl;
	  return -2;
	}
      }
      else{
	outtree->CopyAddresses(tree);
	if(outtree->CopyEntries(tree,-1,"fast SortBasketsByBranch") <= 0){
	  cerr<<"There was an error copying entries from tree "
	      <<tree->GetName()<<" in file "<<f->GetName()<<endl;
	  return -3;
	}
      }
    }	//end fast else
    delete tree;
    
  }	//end while
  if(outtree)
    
    outtree->Write(outtree->GetName(),TObject::kOverwrite);
  delete outtree;
  return 0;
}

int merge_directory(TDirectory* dir, TDirectory* parentOut, 
		    TList* files, const char* mtype)
{
  cout<<"Merging directory "<<dir->GetName()<<"..."<<endl;

  parentOut->cd();
  TIter next(dir->GetListOfKeys());
  TKey* key=0;
  while(key = ((TKey*)(next()))){
    TObject* obj = dir->Get(key->GetName());
    if(!obj){
      cerr<<"Error getting "<<key->GetName()<<" from directory "
	  <<dir->GetName()<<endl;
      return -2;
    }
    if(obj->InheritsFrom("TDirectory")){
      TDirectory* subdir = (TDirectory*)obj;
      TDirectory* subOutDir = 
	parentOut->mkdir(subdir->GetName(), subdir->GetTitle());
      if(!subOutDir){
	cerr<<"Unable to create directory "<<subdir->GetName()
	    <<" under path "<<parentOut->GetPath()<<endl;
	return -3;
      }
      int check = merge_directory(subdir, subOutDir, files, mtype);
      if(check<0)
	return check;
    }
    else if(obj->InheritsFrom("TTree")){
      //build the full path of the tree
      std::string path=dir->GetPath();
      size_t last = path.find_last_of(":/");
      if(last != std::string::npos){
	if(last<path.size()-1)
	  path = path.substr(last+1);
	else 
	  path="";
      }	
      parentOut->cd();
      int retval = merge_tree((path+"/"+obj->GetName()).c_str(), mtype, files);
      if(retval<0)
	return retval;  
    }
    else{
      cout<<"Skipping object "<<obj->GetName()<<"..."<<endl;
    }
  }
  parentOut->cd();
  //parentOut->Write(TObject::kOverwrite);
  cout<<"Done merging directory "<<dir->GetName()<<endl<<endl;
}

int merge_filelist(const char* outfile, const char* mtype)
{
  if(!gROOT->GetListOfFiles() || gROOT->GetListOfFiles()->GetSize()<1){
    cerr<<"No files provided for merging!"<<endl;
    return -2;
  }

  TList* files = new TList;
  files->AddAll(gROOT->GetListOfFiles());

  TFile* fout = new TFile(outfile,"RECREATE");
  //fout->SetCompressionSettings(0);
  if(!fout || !fout->IsOpen()){
    cerr<<"Unable to open output file "<<outfile<<"!\n";
    return -3;
  }
  //assume the first file is the template
  TFile* firstfile = (TFile*)(files->First());
  return merge_directory(firstfile, fout, files, mtype);
}

#ifndef __CINT__
int main(int argc, const char** argv)
{
  if(argc<3)
    return -1;
  //cout<<argc<<endl;
  //the first arg is the outfile
  for(int i=3; i<argc; ++i){
    TFile* file = new TFile(argv[i],"OPEN");
    if(!file || !file->IsOpen()){
      cerr<<"Unable to open file "<<argv[i]<<endl;
      return -1;
    }
  }
  return merge_filelist(argv[1], argv[2]);
}
#endif

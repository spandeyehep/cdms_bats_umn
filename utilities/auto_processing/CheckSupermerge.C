#include <fstream>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TChain.h"
#include <iostream>

using namespace std;

const UInt_t utorootoff = 788832000; // convert 1970 to 1995 
const Int_t syear  = 2013;
const Int_t smonth = 07;
const Int_t sday   = 15;

const string firstrun(Form("0113%02d%02d_1454",smonth,sday));
const string firstruncmp(Form("2%02d02d1454",smonth,sday));

const TDatime startday(syear,smonth,sday,0,0,0);
const UInt_t tl = startday.Convert()-utorootoff;
const TDatime today(2014, 6, 1, 0, 0 ,0 );
TDatime endday;
const Int_t NMAXDAYS = int(double(today.Convert()-startday.Convert())/24./3600.+0.5);
Int_t NDAYS = 0; 




void CheckSupermerge(TString job =""  ){
    TChain * t_calib =0;
    TChain * t_merge=0;
    std::ifstream infile(job);

    //Get the supermerged fill name from the input list
    TString base=job.ReplaceAll(".txt", "" ) ;
    base=base.Remove(0, job.Last( '/' )+1 );

    //get the data type from the input list
    TString type="ba";
    if(base.Contains("_cf") ) type="cf";
    if(base.Contains("_bg") ) type="bg";


    //Just to confirm that nothign funky is goind on
    cout<<"base = " <<base <<endl;
    
    // create a place to store the output histograms
    TFile * f_out = new TFile(base+".root", "RECREATE");

    TString series ;
    int series_n;
    int n_merge=0 , n_calib=0;
    float livetime =0;


    //endday is set to the top and you should set it according to today or a date corresponding to what you want ot compare to (eg: livetime from DQ files)
    if(NDAYS < int(double(endday.Convert()-startday.Convert())/24./3600.+0.5) )     NDAYS = int(double(endday.Convert()-startday.Convert())/24./3600.+0.5);
    //The livetime histograms have 1 bin per day
    TH1F * h_submerge = new TH1F("h_submerge", "submerge",  NDAYS, tl, tl+(NDAYS)*3600*24);
    TH1F * h_supermerge = new TH1F("h_supermerge", "supermerge",  NDAYS, tl, tl+(NDAYS)*3600*24);

    int length=0;
    n_merge=0 ; n_calib=0;
    Double_t EventTime=0,LiveTime=0,BiasFlashTime=0 ;

    //read in the series numbers from that input list
    while( infile >> series >>series_n){
        ////////outfile<< series <<endl;
        ////////outfile << series_n <<endl;
        if(series=="")continue; 
        t_calib=new TChain("rrqDir/calibevent");
        t_merge=new TChain("rqDir/eventTree");

        //add up all the unmerged files
        t_calib->Add("../unmerged/"+type+"/"+series+"/calib_Prodv5-3_"+series+"_F*.root");
        t_merge->Add("../unmerged/"+type+"/"+series+"/Prodv5-3_"+series+"_F*.root");
    
        //it's a problem if the RQ and the RRQs have different numbers of events
        if(t_calib->GetEntries() != t_merge->GetEntries()){
            cerr<<"Different RQ and RRQ tree lengths in submerged series: "<< series << " merge="<<t_merge->GetEntries()<<" calib="<< t_calib->GetEntries()  << endl;
            continue;
        }
        //we are keeping a running total of the number of events in the submerged files for later comparison against the number of events in the supermerged
        n_merge += t_merge->GetEntries();
        n_calib += t_calib->GetEntries();

        t_merge->SetBranchAddress("EventTime", &EventTime);
        t_merge->SetBranchAddress("LiveTime", &LiveTime);
                t_merge->SetBranchAddress("BiasFlashTime", &BiasFlashTime);

        //No entries? Let's report that
        if(t_merge->GetEntries() ==0 ) cout <<" No Entries in "<< series <<endl;

        for(int i=0;i<t_merge->GetEntries(); ++i){
            t_merge->GetEntry(i);
            //this is a basic cut that a series was neutralized. You can add more if needed
            if(BiasFlashTime>108000) continue;
            //Fill the histogram with the livetime, converted to days. There is an offset since ROOT's epoc is in 1995.
            h_submerge->Fill(EventTime - utorootoff, LiveTime/3600/24/1000 );
        }



    }
    //clean up the memory
    delete t_calib;
    delete t_merge;

    //Onto the supermerged files

    t_calib=new TChain("rrqDir/calibevent");
    t_merge=new TChain("rqDir/eventTree");

    if(type=="bg") type+="_restricted";

    //File names are a bit trickier bust still taken from teh input list
    t_calib->Add("../merged/all/"+type+"/calib_Prodv5-3-5_"+base.ReplaceAll("prodR134_","")+".root");
    cout<<"../merged/all/"<<type<<"/calib_Prodv5-3-5_"<<base.ReplaceAll("prodR134_","")<< ".root\n";
    t_merge->Add("../merged/all/"+type+"/merge_Prodv5-3-5_"+base.ReplaceAll("prodR134_","")+".root");

    if(t_calib->GetEntries() != t_merge->GetEntries()){
        cerr<<"Different RQ and RRQ tree lengths in supermerged : "<< base << " merge="<<t_merge->GetEntries()<<" calib="<< t_calib->GetEntries()  << endl;

        f_out->Close();
        return ;
    }

    //Let's compare the number of events between the supermerge and the submerged
    if(n_merge != t_merge->GetEntries() )       cerr<<"Different RQ and RRQ tree lengths in supermerged and submerged : "<< base << " super="<<t_merge->GetEntries()<<" sub="<< n_merge<< endl;
    if(n_calib != t_calib->GetEntries() )       cerr<<"Different RQ and RRQ tree lengths in supermerged and submerged : "<< base << " super="<<t_calib->GetEntries()<<" sub="<< n_calib  << endl;

    t_merge->SetBranchAddress("EventTime", &EventTime);
    t_merge->SetBranchAddress("LiveTime", &LiveTime);
        t_merge->SetBranchAddress("LiveTime", &LiveTime);

        if(t_merge->GetEntries() ==0 ) cout <<" No Entries in "<< series <<endl;

    for(int i=0;i<t_merge->GetEntries(); ++i){
        t_merge->GetEntry(i);
            if(BiasFlashTime>108000) continue;
        h_supermerge->Fill(EventTime - utorootoff, LiveTime/3600/24/1000 );
    }

    //Let's use our livetime histograms to compare the livetime between the supermerge and the submerged        
    if(h_submerge->Integral() != h_supermerge->Integral() ) cout<<"Series : "<< base << " has different livetimes!!   submerge ="<< h_submerge->Integral() <<"  supermerge="<< h_supermerge->Integral() <<" \n";

    //Write the histograms to disk, close the file and we're done
    h_submerge->Write();
    h_supermerge->Write();
    f_out->Close();
    return ;
}


int main(int argc, char* argv[]){

    //The program takes a supermerge series list as input
    CheckSupermerge( argv[1]);
    return 0;
}


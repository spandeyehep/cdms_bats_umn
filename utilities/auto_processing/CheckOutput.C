#include <fstream>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <iostream>

using namespace std;



void CheckOutput(TString job =""  ){
    TFile * f =0;
    TTree * t =0;
    std::ifstream infile(job);

    //format the input string to get a filename base
    TString base=job.ReplaceAll(".txt", "" ) ;
    base=base.Remove(0, job.Last( '/' )+1 );

    //we guess the type based on the input filename
    TString type="ba";
    if(base.Contains("_cf") ) type="cf";
    if(base.Contains("_bg") ) type="bg";


    cout<<"base = " <<base <<endl;
    //Open a file to dump a list of BASH commands to help you setup the rerunning in case some jobs have failed.
    std::ofstream outfile(base+"_commands.sh");
    outfile<<"#!/bin/bash \n";

    TString series ;
    int series_n;
    int n_merge=0 , n_calib=0;


    int length=0;
    //loop over the series in the input list
    while( infile >> series >>series_n){
        ////////outfile<< series <<endl;
        ////////outfile << series_n <<endl;
        if(series=="")continue;
        n_merge=0 ; n_calib=0;

        //checking noise file   
        f=0;
        f=TFile::Open("../noise/ROOT/Prodv5-3_Filter_"+series+".root");
        //if the file doesn't exist, we assume the job fails so we want to change the lock status
        if(f==0){ 
            outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".job\n";
        }else{
            //the file does exist but does it contain good data? Lets look at zip15 since it is last on the list and most likely to fail
            TH1D * noise = 0;
            noise = (TH1D*)f->Get("zip15/EventList");
            if(noise==0){ 
                //can't find the histogram, chalk it up to a failure
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".job\n";
            }else if(noise->GetEntries() == 0){ 
                //histogram exists but is empty, lets assume this failed as well
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage1/"<< series<< ".job\n";
            }
            //close the file since we are done with it. You will see this a lot thorughout the code
            f->Close();
        }

        //checking unmerged files
        //we'll check each dump individually
        for(int i=1;i<series_n+1;++i){
            TString dump = Form("%d",i );
            if(i<10) dump = "0"+dump;
            if(i<100) dump = "0"+dump;
            if(i<1000) dump = "0"+dump;

            f=0;
            f=TFile::Open("../unmerged/"+type+"/"+series+"/calib_Prodv5-3_"+series+"_F"+dump+".root");
            if(f==0){ 
                //file doesn't exist
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                //it failed, lets tag it and move on. You'll also see this quite a bit throughout the code 
                continue;
            }else{
                //file exists, let's check the zip15 TTree
                t=0;
                t=(TTree*)f->Get("rrqDir/calibzip15");
                if(t!=0) n_calib+= t->GetEntries();
                if(t==0){
                    //nope, not there, another failure
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                    f->Close();
                    continue;
                }else if(t->GetEntries() ==0){
                    //it is there but without any entries, lets assume this failed
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                    f->Close();
                    continue;
                }
            }
            f->Close();

            //same logic as above but now we look at the  calib files
            f=0;
            f=TFile::Open("../unmerged/"+type+"/"+series+"/Prodv5-3_"+series+"_F"+dump+".root");
            if(f==0){ 
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                continue;
            }else{
                t=0;
                t=(TTree*)f->Get("rqDir/zip15");
                if(t!=0) n_merge+= t->GetEntries();
                if(t==0){
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                    f->Close();
                    continue;
                }else if(t->GetEntries() ==0){
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage2/"<< series<<"_F"<<dump << ".job\n";
                    f->Close();
                    continue;
                }
            }

            f->Close();





        }

        //checking merged
        //same logic as above but now we look at the  submerged files
        f=0;
        f=TFile::Open("../submerged/all/"+type+"/calib_Prodv5-3_"+series+".root");
        if(f==0){ 
            outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series<< ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
            continue;
        }else{
            t=0;
            t=(TTree*)f->Get("rrqDir/calibzip15");
            if(t==0){
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                f->Close();
                continue;
            }else{
                if(t->GetEntries() ==0){
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                    f->Close();
                    continue;
                }
                if(t->GetEntries() != n_calib){
                    cout<<"File "<< "/submerged/all/"<<type<<"/calib_Prodv5-3_"<<series<<".root doesn't have the right number of entries. Redo the submerging :\n ";
                    cout<<"/data2/cdmsbatsProd/R134/dataReleases/Prodv5-3-5/merge.sh "<< series<<"  " << type << endl;
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                    f->Close();
                    continue;
                }

            }
        }
        f->Close();

        f=0;
        f=TFile::Open("../submerged/all/"+type+"/merge_Prodv5-3_"+series+".root");
        if(f==0){ 
            outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series<<".job\n";
            continue;
        }else{
            t=0;
            t=(TTree*)f->Get("rqDir/zip15");
            if(t==0){
                outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                f->Close();
                continue;
            }else {
                if(t->GetEntries() ==0){
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                    f->Close();
                    continue;
                }
                if(t->GetEntries() != n_merge){
                    cout<<"File "<< "/submerged/all/"<<type<<"/merge_Prodv5-3_"<<series<<".root doesn't have the right number of entries. Redo the submerging :\n ";
                    cout<<"/data2/cdmsbatsProd/R134/dataReleases/Prodv5-3-5/merge.sh "<< series<<"  " << type << endl;
                    outfile <<"mv /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".done /grid/data/cdms/processing/processing_R134/Prodv5-3_activejobs/"<< base <<"/stage3/"<< series << ".job\n";
                    f->Close();
                    continue;
                }

            }
        }

        f->Close();








    }
    outfile.close();





    return ;
}

int main(int argc, char* argv[]){
    CheckOutput( argv[1]);
    return 0;
}


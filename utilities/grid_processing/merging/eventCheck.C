void eventCheck(TString series, TString dataType, TString run)
{
//   TFile f("/pico/data3/cdmsbatsProd2009/R126/cf/180208_0804_merge_Prodv2-2.root");

   TString filename = "/pico/data3/cdmsbatsProd2009/" + run + "/" + dataType + "/merge_Prodv2-3_" + series + ".root";
   TFile f(filename);
   TTree* eventTree = f.Get("rqDir/eventTree");

   cout <<"Hello Entries = " << eventTree->GetEntries() << endl;

   double eventNumber;
   double priorEventNumber = 0;
   eventTree->SetBranchAddress("EventNumber", &eventNumber);

   int ctr = 0;
   while(eventTree->GetEntry(ctr))
   {
      ctr++;
      double diff = eventNumber-priorEventNumber;

      if( (diff > 1.) && diff != 9501.)
      {
	 cout <<"\nEvent difference > 1 for event=" << (int)eventNumber
	      <<"\nprior = " << (int)priorEventNumber <<" current = " << (int)eventNumber 
	      <<", diff = " << diff
	      <<endl;
      }

      priorEventNumber = eventNumber;
   }


}

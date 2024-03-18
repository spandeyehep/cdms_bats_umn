void doOverlay(int t, int z, string chanUp = "QI", 
	       string brRQ, string dpRQ, string filename="170319_1616_F0006", int dumpNum=6);

void zipRQOverlay_sat(string brRQ, string dpRQ, string filename="170319_1616_F0006", int dumpNum=6, int notCharge=0)
{

   for(int tCtr = 1; tCtr <= 5; tCtr++)
   {
      for(int zCtr = 1; zCtr <= 6; zCtr++)
      {
 	 if(notCharge==0)
 	 {
 	    doOverlay(tCtr, zCtr, "QI", brRQ, dpRQ, filename, dumpNum);
 	    doOverlay(tCtr, zCtr, "QO", brRQ, dpRQ, filename, dumpNum);
 	 }
	 
   	 doOverlay(tCtr, zCtr, "PA", brRQ, dpRQ, filename, dumpNum);
   	 doOverlay(tCtr, zCtr, "PB", brRQ, dpRQ, filename, dumpNum);
   	 doOverlay(tCtr, zCtr, "PC", brRQ, dpRQ, filename, dumpNum);
   	 doOverlay(tCtr, zCtr, "PD", brRQ, dpRQ, filename, dumpNum);
	 
      }
   }
   
   //for testing
   //doOverlay(2, 1, "PD", brRQ, dpRQ, filename, dumpNum);

}

void doOverlay(int t, int z, string chanUp, 
	       string brRQ, string dpRQ, string filename, int dumpNum)
{
   gStyle->SetOptStat(0000000);
   gStyle->SetCanvasColor(10);

   int zip = (t-1)*6 + z;   
   cout <<"The zip detector is " << zip << endl;
   
   //BatRoot file   
   TFile f_br(Form("../testOutput/test_%s.gz.root", filename.c_str()));
   TTree* brTree = f_br.Get(Form("zip%d", zip));
   TTree* evTree = f_br.Get("eventTree");
   brTree->AddFriend(evTree);

   //DarkPipe file
   TFile f_dp(Form("../testOutput/170319_1616_RRQ_even.root", t, z));
   TTree* dpTree = f_dp.Get(Form("zip%d", zip));
   TTree* dpevTree = f_dp.Get("EventTree");
   dpTree->AddFriend(dpevTree);

   //Make histograms
   TString brweight = "(EventNumber%2==0 && ";
   brweight += Form("%s%s == 0.)*-1.", chanUp.c_str(), brRQ.c_str(), chanUp.c_str(), brRQ.c_str());
   TString brplot = Form("eventTree.EventNumber>>hbr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   brTree->Draw(brplot, brweight);

   TString dpweight = Form("(%s%s == 0.)*-1", chanUp.c_str(), dpRQ.c_str(), chanUp.c_str(), dpRQ.c_str());
   TString dpplot = Form("EventTree.EventNumber>>hdp_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   dpTree->Draw(dpplot, dpweight); 
  
   //Compute fractional difference histos
   h_diff = (TH1F*)hbr_event->Clone("diff");
   h_diff->Add(hdp_event, -1.0);
   h_diff->Divide(hdp_event);
   
   //Format histograms
   hbr_event->SetLineColor(kMagenta);
   hbr_event->SetFillColor(kMagenta);
   
   hdp_event->SetLineColor(kBlue);
   hdp_event->SetFillColor(kBlue);   
   
   TCanvas* mycan = new TCanvas("mycan","mycan", 1000, 1000);
   mycan->SetHighLightColor(10);
   mycan->Divide(1,3);
   
   mycan->cd(1);
   hbr_event->SetTitle(Form("zip%d %s: DarkPipe over BatRoot", zip, chanUp));
   hbr_event->SetYTitle(Form("%s", brRQ.c_str()));
   hbr_event->SetXTitle("eventNumber");
   hbr_event->DrawCopy();
   hdp_event->DrawCopy("same");
   
   mycan->cd(2);
   hdp_event->SetTitle(Form("zip%d %s: BatRoot over DarkPipe", zip, chanUp));
   hdp_event->SetYTitle(Form("%s", brRQ.c_str()));
   hdp_event->SetXTitle("eventNumber");
   hdp_event->DrawCopy();
   hbr_event->DrawCopy("same");
   
   mycan->cd(3);
//   h_diff->SetMaximum(1e-9);
//   h_diff->SetMinimum(-1e-9);
   h_diff->SetTitle(Form("zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
   h_diff->SetYTitle(Form("%s fractional diff", brRQ.c_str()));
   h_diff->SetXTitle("eventNumber");
   h_diff->DrawCopy();
   
   mycan->Print(Form("plots/dpbr_%sover_t%dz%d_%s.gif", brRQ.c_str(), t, z, chanUp.c_str())); 
 
   return;
}

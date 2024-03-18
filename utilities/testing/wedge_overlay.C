void wedge_overlay(string chan)
{
   //initial setup
   TFile f("wedgedump.root");
   TTree* wedgeTree = f.Get("wedgeTree");
   int entry;
   if(chan == "PA") entry = 0;
   if(chan == "PB") entry = 1;
   if(chan == "PC") entry = 2;
   if(chan == "PD") entry = 3;
   if(chan != "PA" && chan != "PB" && chan != "PC" && chan != "PD") 
   { 
      cout <<"I don't understand this channel name: " << chan << endl; 
      exit;
   }

   //Get the baseline subtracted histogram
   string histoName = chan+"BaselineSubPulse";
   TH1D* h_baselinesub = (TH1D*)f.Get(histoName.c_str());
   			
   //For accessing WF parameters to make function
   double WFstart, WFend;
   double WFpar1, WFpar2, WFpar3;
   
   wedgeTree->SetBranchAddress("WFstart", &WFstart);
   wedgeTree->SetBranchAddress("WFend", &WFend);
   wedgeTree->SetBranchAddress("WFpar1", &WFpar1);
   wedgeTree->SetBranchAddress("WFpar2", &WFpar2);
   wedgeTree->SetBranchAddress("WFpar3", &WFpar3);

   //For accessing PF parameters to make function (not implemented yet)

   // ============== Construct fit functions ==================

   wedgeTree->GetEntry(entry);

   //Wedgefit function
   TF1* wedgeFunction = new TF1("wedgefunc", "[0]*(x*x-[2]*[2]) + [1]*(x-[2])", WFstart, WFend);  //last two parameters are the fit range, adjust as you like
   wedgeFunction->SetParameter(0, WFpar1);
   wedgeFunction->SetParameter(1, WFpar2);
   wedgeFunction->SetParameter(2, WFpar3);
   wedgeFunction->SetLineColor(kBlue);

   //for comparison with oleg's fit values
   TF1* wedgeFunction_oleg = new TF1("wedgefunc", "[0]*(x*x-[2]*[2]) + [1]*(x-[2])", 498, 560);
   wedgeFunction_oleg->SetParameter(0, 0.2483);
   wedgeFunction_oleg->SetParameter(1, -243.489);
   wedgeFunction_oleg->SetParameter(2, 503.2189);
   wedgeFunction_oleg->SetLineColor(kMagenta);

   // ============== Print Values ==================

   //wedge fit results
   wedgeTree->Scan("WFstart:WFend:WFpar1:WFpar2:WFpar3:WFchisq:WFeflag");

   //old and new rtftwalk
   wedgeTree->Scan("1.25e6*classic_r10:1.25e6*classic_r20:1.25e6*scott_r10:1.25e6*scott_r20");

   // =============== Now Plotting ==================

   //plot
   TCanvas* can = new TCanvas("mycan","mycan",1000, 400);
   can->SetHighLightColor(10);
   can->Divide(2,1);

   can->cd(1);
   h_baselinesub->DrawCopy();
   wedgeFunction->DrawCopy("same");

   can->cd(2);
   wedgeFunction->DrawCopy();
   wedgeFunction_oleg->DrawCopy("same");

   //store the plots
   can->Print("myplot.gif");

}

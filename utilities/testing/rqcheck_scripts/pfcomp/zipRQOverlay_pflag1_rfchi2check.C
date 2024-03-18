void doOverlay(int t, int z, string chan, string chanUp, 
	       string brRQ, string brRQ2, string pfRQ, string filename, int dumpNum, string brPulseFlag, string pfPulseFlag,string rfeflag);

void zipRQOverlay_pflag1_rfchi2check(string brRQ, string brRQ2, string pfRQ, string brPulseFlag, string pfPulseFlag, string rfeflag,string filename="170319_1616_F0006", int dumpNum=6, int notCharge=0)
{

   for(int tCtr = 1; tCtr <= 5; tCtr++)
   {
      for(int zCtr = 1; zCtr <= 6; zCtr++)
      {
	 if(!notCharge==0)
	 {
	   doOverlay(tCtr, zCtr, "qi", "QI", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPulseFlag,rfeflag);
	   doOverlay(tCtr, zCtr, "qo", "QO", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPUlseFlag,rfeflag);
	 }

  	 doOverlay(tCtr, zCtr, "pa", "PA", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPulseFlag,rfeflag);
  	 doOverlay(tCtr, zCtr, "pb", "PB", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPulseFlag,rfeflag);
  	 doOverlay(tCtr, zCtr, "pc", "PC", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPulseFlag,rfeflag);
  	 doOverlay(tCtr, zCtr, "pd", "PD", brRQ, brRQ2, pfRQ, filename, dumpNum,brPulseFlag,pfPulseFlag,rfeflag);
	 
      }
   }
   
   //for testing
//   doOverlay(2, 1, "pd", "PD", brRQ, pfRQ, filename, dumpNum);

}

void doOverlay(int t, int z, string chan, string chanUp, 
	       string brRQ, string brRQ2, string pfRQ, string filename, int dumpNum, string brPulseFlag, string pfPulseFlag, string rfeflag)
{
   gStyle->SetOptStat(0000000);
   gStyle->SetCanvasColor(10);
   gStyle->SetLabelSize(0.06, "Y");
   gStyle->SetTitleSize(0.06, "Y");
   gStyle->SetTitleSize(0.05, "X");
   gStyle->SetTitleOffset(0.75, "Y");
   gStyle->SetTitleOffset(0.75, "X");

   int zip = (t-1)*6 + z;   
   cout <<"The zip detector is " << zip << endl;
   
   //BatRoot file   
   TFile f_br(Form("/home/kos/cdmsbats/BatRoot/test_%sfull.root", filename.c_str()));
   TTree* brTree = f_br.Get(Form("zip%d", zip));
   TTree* evTree = f_br.Get("eventTree");
   brTree->AddFriend(evTree);

   //PipeRoot file
   string filename2 = "170319_1616_F0006_notrig";
   TFile f_pr(Form("/home/kos/PipeFitter/piperoot/%s.root.t%dz%d", filename2, t, z));
   TTree* prTree = f_pr.Get(Form("t%dz%d",t,z));

   //Make histograms
   //make cut on pulse flag
   TString brweight = Form("(%s%s == 1 && %s%s == 0)*(%s%s/%s%s)", chanUp.c_str(), brPulseFlag.c_str(), chanUp.c_str(), rfeflag.c_str(),chanUp.c_str(),brRQ.c_str(),chanUp.c_str(),brRQ2.c_str());
   TString brplot = Form("eventTree.EventNumber>>hbr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   brTree->Draw(brplot, brweight);

   //make cut on pulse flag
   TString prweight = Form("(%s.%s == 1 && %s.%s > -999)*(%s.%s)", chan.c_str(), pfPulseFlag.c_str(),chan.c_str(),pfRQ.c_str() ,chan.c_str(), pfRQ.c_str());
   TString prplot = Form("zevent>>hpr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   prTree->Draw(prplot, prweight); 
  
     //Compute fractional difference histos
     h_diff = (TH1F*)hbr_event->Clone("diff");
     h_diff->Add(hpr_event, -1.0);
     h_diff->Divide(hpr_event);

     //Format histograms
     hbr_event->SetLineColor(kMagenta);
     hbr_event->SetFillColor(kMagenta);

     hpr_event->SetLineColor(kBlue);
     hpr_event->SetFillColor(kBlue);   

     TCanvas* mycan = new TCanvas("mycan","mycan", 800, 800);
     mycan->SetHighLightColor(10);
     mycan->Divide(1,3);

     mycan->cd(1);
     TLatex* label1 = new TLatex(0.53, 0.9, Form("PulseFlag: 1 zip%d %s: PipeRoot over BatRoot", zip, chanUp));
     label1->SetNDC();
     label1->SetTextSize(0.06);
     hbr_event->SetTitle(""); 
     //hbr_event->SetTitle(Form("zip%d %s: PipeRoot over BatRoot", zip, chanUp));
     hbr_event->SetYTitle(Form("%s", brRQ.c_str()));
     hbr_event->SetXTitle("eventNumber");
     hbr_event->DrawCopy();
     hpr_event->DrawCopy("same");
     label1->Draw("same");

     mycan->cd(2);
     TLatex* label2 = new TLatex(0.53, 0.9, Form("PulseFlag: 1 zip%d %s: BatRoot over Darkpipe", zip, chanUp));
     label2->SetNDC();
     label2->SetTextSize(0.06);
     hpr_event->SetTitle("");
     //hpr_event->SetTitle(Form("zip%d %s: BatRoot over PipeRoot", zip, chanUp));
     hpr_event->SetYTitle(Form("%s", brRQ.c_str()));
     hpr_event->SetXTitle("eventNumber");
     hpr_event->DrawCopy();
     hbr_event->DrawCopy("same");
     label2->Draw("same");

     mycan->cd(3);
     TLatex* label3 = new TLatex(0.55, 0.9, Form("PulseFlag: 1 zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
     label3->SetNDC();
     label3->SetTextSize(0.06);
     h_diff->SetTitle("");
     h_diff->SetMaximum(0.02);
     h_diff->SetMinimum(-0.02);
     //h_diff->SetTitle(Form("zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
     h_diff->SetYTitle(Form("%s fractional diff", brRQ.c_str()));
     h_diff->SetXTitle("eventNumber");
     h_diff->DrawCopy();
     label3->Draw("same");

     mycan->Print(Form("plots_pflag1/pfbrpf1_%sover_t%dz%d_%s.gif", brRQ.c_str(), t, z, chanUp.c_str())); 
 
   return;
}

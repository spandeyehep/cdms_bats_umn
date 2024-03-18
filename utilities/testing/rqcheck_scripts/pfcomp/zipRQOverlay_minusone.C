void doOverlay(int t, int z, string chan = "qi", string chanUp = "QI", 
	       string brRQ, string prRQ, string filename="170319_1616_F0006", int dumpNum=6);

void zipRQOverlay_minusone(string brRQ, string prRQ, string filename="170319_1616_F0006", int dumpNum=6, int notCharge=0)
{

   for(int tCtr = 1; tCtr <= 5; tCtr++)
   {
      for(int zCtr = 1; zCtr <= 6; zCtr++)
      {
  	 if(notCharge==0 || notCharge==-1)
  	 {
  	    doOverlay(tCtr, zCtr, "qi", "QI", brRQ, prRQ, filename, dumpNum);
  	    doOverlay(tCtr, zCtr, "qo", "QO", brRQ, prRQ, filename, dumpNum);
  	 }
	 
	 if(notCharge==1 || notCharge==0)
	 {   
	    doOverlay(tCtr, zCtr, "pa", "PA", brRQ, prRQ, filename, dumpNum);
	    doOverlay(tCtr, zCtr, "pb", "PB", brRQ, prRQ, filename, dumpNum);
	    doOverlay(tCtr, zCtr, "pc", "PC", brRQ, prRQ, filename, dumpNum);
	    doOverlay(tCtr, zCtr, "pd", "PD", brRQ, prRQ, filename, dumpNum);
	 }
	 
      }
   }
   
   //for testing
   //doOverlay(2, 1, "PD", brRQ, prRQ, filename, dumpNum);

}

void doOverlay(int t, int z, string chan, string chanUp, 
	       string brRQ, string prRQ, string filename, int dumpNum)
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
   TFile f_br(Form("../testOutput/test_%s.gz.root", filename.c_str()));
   TTree* brTree = f_br.Get(Form("zip%d", zip));
   TTree* evTree = f_br.Get("eventTree");
   brTree->AddFriend(evTree);

   //PipeRoot file
   TFile f_pr(Form("../testOutput/test_%s.root.t%dz%d", filename, t, z));
   TTree* prTree = f_pr.Get(Form("t%dz%d",t,z));

   //Make histograms
   TString brweight = Form("(%sPFfpflag == 1 || %sPFfpflag ==2)*(%s%s + 1)", chanUp.c_str(), chanUp.c_str(), chanUp.c_str(), brRQ.c_str());
//   TString brweight = Form("(%sPFfpflag > 0)*(%s%s + 1)", chanUp.c_str(), chanUp.c_str(), brRQ.c_str());
   TString brplot = Form("eventTree.EventNumber>>hbr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   brTree->Draw(brplot, brweight);

   TString prweight = Form("(%s.pflag == 1 || %s.pflag == 2)*(%s.%s)", chan.c_str(), chan.c_str(), chan.c_str(), prRQ.c_str());
//   TString prweight = Form("(%s.pflag > 0)*(%s.%s)", chan.c_str(), chan.c_str(), prRQ.c_str());
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
//   gPad->SetLogy();
   TLatex* label1 = new TLatex(0.64, 0.9, Form("zip%d %s: PipeRoot over BatRoot", zip, chanUp));
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
//   gPad->SetLogy();
   TLatex* label2 = new TLatex(0.64, 0.9, Form("zip%d %s: BatRoot over Darkpipe", zip, chanUp));
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
   TLatex* label3 = new TLatex(0.64, 0.9, Form("zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
   label3->SetNDC();
   label3->SetTextSize(0.06);
   h_diff->SetTitle("");
   h_diff->SetMaximum(1e-4);
   h_diff->SetMinimum(-1e-4);
   //h_diff->SetTitle(Form("zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
   //h_diff->SetTitleOffset(1.);
   h_diff->SetYTitle(Form("%s fractional diff", brRQ.c_str()));
   h_diff->SetXTitle("eventNumber");
   h_diff->DrawCopy();
   label3->Draw("same");

   mycan->Print(Form("plots/prbr_%sover_t%dz%d_%s.gif", brRQ.c_str(), t, z, chanUp.c_str())); 
 
   return;
}

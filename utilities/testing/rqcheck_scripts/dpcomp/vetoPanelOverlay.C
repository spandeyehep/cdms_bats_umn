void doOverlay(int t, int z, string chanUp = "QI", 
	       string brRQ, string dpRQ, string filename="170319_0151_F0004", int dumpNum=4);

void vetoPanelOverlay(string brRQ, string dpRQ, string filename="170319_0151_F0004", int dumpNum=4)
{

   for(int pCtr = 1; pCtr <= 40; pCtr++)
   {
      string detNum;

      if(pCtr != 1 && pCtr != 20 && pCtr != 40) { continue; }

      if(pCtr < 10) detNum = Form("0%d", pCtr);
      else detNum = Form("%d", pCtr);

      doOverlay(pCtr, pCtr, detNum, brRQ, dpRQ, filename, dumpNum);
   }
   
}

void doOverlay(int t, int panelNum, string chanUp, 
	       string brRQ, string dpRQ, string filename, int dumpNum)
{
   gStyle->SetOptStat(0000000);
   gStyle->SetCanvasColor(10);
   gStyle->SetLabelSize(0.06, "Y");
   gStyle->SetTitleSize(0.06, "Y");
   gStyle->SetTitleSize(0.05, "X");
   gStyle->SetTitleOffset(0.75, "Y");
   gStyle->SetTitleOffset(0.75, "X");

   //BatRoot file   
   TFile f_br(Form("../testOutput/veto_%s.root", filename.c_str()));
   TTree* brTree = f_br.Get("vetoTree");
   TTree* evTree = f_br.Get("eventTree");
   brTree->AddFriend(evTree);

   //DarkPipe file
   TFile f_dp("../testOutput/170319_0151_veto.root");
   TTree* dpTree = f_dp.Get("vetoTree");
   TTree* dpevTree = f_dp.Get("EventTree");
   dpTree->AddFriend(dpevTree);

   //Make histograms
   TString brweight =Form("(%s%s != -999999.)*%s%s", brRQ.c_str(), chanUp.c_str(), brRQ.c_str(), chanUp.c_str());
   cout <<"brweight = " << brweight << endl;
   TString brplot = Form("eventTree.EventNumber>>hbr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   brTree->Draw(brplot, brweight);

   TString dpweight = Form("(%s%s != 9999)*%s%s",dpRQ.c_str(), chanUp.c_str(), dpRQ.c_str(), chanUp.c_str());
   cout <<"dpweight = " << dpweight << endl;
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
   
   TCanvas* mycan = new TCanvas("mycan","mycan", 800, 800);
   mycan->SetHighLightColor(10);
   mycan->Divide(1,3);
   
   mycan->cd(1);
//   gPad->SetLogy();
   TLatex* label1 = new TLatex(0.64, 0.9, Form("panelNum%d %s: DarkPipe over BatRoot", panelNum, chanUp));
   label1->SetNDC();
   label1->SetTextSize(0.06);
   hbr_event->SetTitle("");
   hbr_event->SetYTitle(Form("%s", brRQ.c_str()));
   hbr_event->SetXTitle("eventNumber");
   hbr_event->DrawCopy();
   label1->Draw("same");
   
   mycan->cd(2);
//   gPad->SetLogy();
   TLatex* label2 = new TLatex(0.64, 0.9, Form("panelNum%d %s: BatRoot over Darkpipe", panelNum, chanUp));
   label2->SetNDC();
   label2->SetTextSize(0.06);
   hdp_event->SetTitle("");
   hdp_event->SetYTitle(Form("%s", brRQ.c_str()));
   hdp_event->SetXTitle("eventNumber");
   hdp_event->DrawCopy();
   hbr_event->DrawCopy("same");
   label2->Draw("same");

   mycan->cd(3);
   TLatex* label3 = new TLatex(0.64, 0.9, Form("panelNum %d %s: %s", panelNum, chanUp.c_str(), filename.c_str()));
   label3->SetNDC();
   label3->SetTextSize(0.06);
   h_diff->SetTitle("");
   h_diff->SetMaximum(1);
   h_diff->SetMinimum(-1);
   //h_diff->SetTitle(Form("panelNum %d %s: %s", panelNum, chanUp.c_str(), filename.c_str()));
   //h_diff->SetTitleOffset(1.);
   h_diff->SetYTitle(Form("%s fractional diff", brRQ.c_str()));
   h_diff->SetXTitle("eventNumber");
   h_diff->DrawCopy();
   label3->Draw("same");

   mycan->Print(Form("plots/dpbr_%s%sover_veto.gif", brRQ.c_str(), chanUp.c_str())); 
 
   return;
}

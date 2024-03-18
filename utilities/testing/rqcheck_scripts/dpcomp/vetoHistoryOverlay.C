void doOverlay(int t, string rqName, double defVal, string filename="170319_0151_F0002", int dumpNum=2, int doLog=0);

void vetoHistoryOverlay(string rqName, int timeCtr=0, string filename="170319_0151_F0002", int dumpNum=2, int notCharge=0, int doLog=0)
{

    double defVal = (timeCtr > 20 ? 99999999. : -99999999.);
    if(timeCtr==0) defVal = 99999999.;
    string rqformed = Form("%s", rqName.c_str());
    doOverlay(rqformed, defVal, filename, dumpNum, doLog);
   
   //for testing


}

void doOverlay(string rqName, double defVal, string filename, int dumpNum, int doLog)
{
   gStyle->SetOptStat(0000000);
   gStyle->SetCanvasColor(10);
   gStyle->SetLabelSize(0.06, "Y");
   gStyle->SetTitleSize(0.06, "Y");
   gStyle->SetTitleSize(0.05, "X");
   gStyle->SetTitleOffset(0.75, "Y");
   gStyle->SetTitleOffset(0.75, "X");

   //temp
   int zip = 1;
   string chanUp = "nochan";

   //BatRoot file   
   TFile f_br(Form("../testOutput/%s.root", filename.c_str()));
   TTree* brTree = f_br.Get("eventTree");

   //DarkPipe file
   TFile f_dp("../testOutput/170319_0151_veto.root");
   TTree* dpTree = f_dp.Get("EventTree");

   //Make histograms
   TString brweight = Form("(%s != -999999)*%s", rqName.c_str(), rqName.c_str()); 
   cout <<"brweight = " << brweight << endl;
   TString brplot = Form("eventTree.EventNumber>>hbr_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   brTree->Draw(brplot, brweight);

   TString dpweight = Form("(%s != %.10e)*%s", rqName.c_str(), defVal, rqName.c_str());
   cout <<"dpweight = " << dpweight << endl;
   TString dpplot = Form("EventTree.EventNumber>>hdp_event(500, %d0000, %d0500)", dumpNum, dumpNum); 
   dpTree->Draw(dpplot, dpweight); 
  
   //Compute fractional difference histos
   h_diff = (TH1F*)hbr_event->Clone("diff");
   h_diff->Add(hdp_event, -1.0);
   h_diff->Divide(hdp_event);
   
   //Format histograms
   hbr_event->SetLineColor(kMagenta);
   hbr_event->SetLineWidth(0.5);
   hbr_event->SetFillColor(kMagenta);
   
   hdp_event->SetLineColor(kBlue);
   hdp_event->SetLineWidth(0.5);
   hdp_event->SetFillColor(kBlue);   
   
   TCanvas* mycan = new TCanvas("mycan","mycan", 1200, 800);
   mycan->SetHighLightColor(10);
   mycan->Divide(1,3);
   
   mycan->cd(1);
   if(doLog == 1) gPad->SetLogy();
   TLatex* label1 = new TLatex(0.64, 0.9, Form("DarkPipe over BatRoot"));
   label1->SetNDC();
   label1->SetTextSize(0.06);
   //   hbr_event->SetMinimum(0.1);
   //hdp_event->SetMinimum(0.1);
   hbr_event->SetTitle("");
   hbr_event->SetYTitle(Form("%s", rqName.c_str()));
   hbr_event->SetXTitle("eventNumber");
   hbr_event->DrawCopy();
   hdp_event->DrawCopy("same");
   label1->Draw("same");
   
   mycan->cd(2);
   cout <<"doLog = " << doLog << endl;
   if(doLog == 1) gPad->SetLogy();
   TLatex* label2 = new TLatex(0.64, 0.9, Form("BatRoot over Darkpipe"));
   label2->SetNDC();
   label2->SetTextSize(0.06);
   hdp_event->SetTitle("");
   hdp_event->SetYTitle(Form("%s", rqName.c_str()));
   hdp_event->SetXTitle("eventNumber");
   hdp_event->DrawCopy();
   hbr_event->DrawCopy("same");
   label2->Draw("same");

   mycan->cd(3);
   TLatex* label3 = new TLatex(0.64, 0.9, Form("%s", filename.c_str()));
   label3->SetNDC();
   label3->SetTextSize(0.06);
   h_diff->SetTitle("");
   h_diff->SetMaximum(1e-5);
   h_diff->SetMinimum(-1e-5);
   //h_diff->SetTitle(Form("zip %d %s: %s", zip, chanUp.c_str(), filename.c_str()));
   //h_diff->SetTitleOffset(1.);
   h_diff->SetYTitle(Form("%s fractional diff", rqName.c_str()));
   h_diff->SetXTitle("eventNumber");
   h_diff->DrawCopy();
   label3->Draw("same");

   mycan->Print(Form("plots/dpbr_%sover.gif", rqName.c_str())); 
 
   return;
}

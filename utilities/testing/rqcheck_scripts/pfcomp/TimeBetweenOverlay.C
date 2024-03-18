void TimeBetweenOverlay()
{
   gStyle->SetOptStat(0000000);
   gStyle->SetCanvasColor(10);

   //BatRoot file
   TFile f_br("../testOutput/test_170319_1616_F0006.gz.root");
   TTree* brTree = f_br.Get("eventTree");

   //PipeRoot file
   TFile f_pr("../testOutput/test_170319_1616_F0006.root.pipe");
   TTree* prTree = f_pr.Get("pipe");

   //Make histograms
   brTree->Draw("EventNumber>>hbr_event(500, 60000, 60500)", "TimeBetween");
   prTree->Draw("event>>hpr_event(500, 60000, 60500)", "timelev"); 
  
   //Compute fractional difference histos
   h_diff = (TH1F*)hbr_event->Clone("diff");
   h_diff->Add(hpr_event, -1.0);
   h_diff->Divide(hpr_event);
   
   //Format histograms
   hbr_event->SetLineColor(kMagenta);
   hbr_event->SetFillColor(kMagenta);
   
   hpr_event->SetLineColor(kBlue);
   hpr_event->SetFillColor(kBlue);   
   
    TCanvas* mycan = new TCanvas("mycan","mycan", 1000, 1000);
    mycan->SetHighLightColor(10);
    mycan->Divide(1,3);

    mycan->cd(1);
    gPad->SetLogy();
    hbr_event->SetTitle("PipeRoot over BatRoot");
    hbr_event->SetYTitle("TimeBetween");
    hbr_event->SetXTitle("eventNumber");
    hbr_event->DrawCopy();
    hpr_event->DrawCopy("same");

    mycan->cd(2);
    gPad->SetLogy();
    hbr_event->SetTitle("BatRoot over PipeRoot");
    hbr_event->SetYTitle("TimeBetween");
    hbr_event->SetXTitle("eventNumber");
    hbr_event->DrawCopy();
    hpr_event->DrawCopy("same");
    hbr_event->DrawCopy("same");

    mycan->cd(3);
    h_diff->SetMaximum(1.0e-9);
    h_diff->SetMinimum(-1.0e-9);
    h_diff->SetTitle("");
    h_diff->SetYTitle("TimeBetween fractional diff");
    h_diff->SetXTitle("eventNumber");
    h_diff->DrawCopy();

    mycan->Print("plots/TimeBetween.gif");
 
   return;
}

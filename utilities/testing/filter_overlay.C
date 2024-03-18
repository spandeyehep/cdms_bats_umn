void filter_overlay(int eventNumber, int detNumber, string chan)
{
   TFile f("dumppulses.root");

   //Get the baseline subtracted histogram
   string histoName = chan+Form("BaselineSubPulse_zip%d_event%d", detNumber, eventNumber);
   TH1D* h_baselinesub = (TH1D*)f.Get(histoName.c_str());
   
   //Get the filtered histogram
   histoName = chan+Form("NewFilteredPulse_zip%d_event%d", detNumber, eventNumber);
   TH1D* h_newfiltered = (TH1D*)f.Get(histoName.c_str());

   //Get the old filtered histogram
   histoName = chan+Form("OldFilteredPulse_zip%d_event%d", detNumber, eventNumber);
   TH1D* h_oldfiltered = (TH1D*)f.Get(histoName.c_str());
				
   //Set line styles
   h_newfiltered->SetLineColor(kBlue);
   h_newfiltered->SetLineWidth(2);

   h_oldfiltered->SetLineColor(kMagenta);
   h_oldfiltered->SetLineWidth(2);

   //plot
   h_baselinesub->DrawCopy();
   h_newfiltered->DrawCopy("same");
   h_oldfiltered->DrawCopy("same");

   //store the plots
   c1->Print("myplot.gif");

}

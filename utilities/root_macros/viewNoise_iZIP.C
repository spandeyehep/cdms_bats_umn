//This macro is used to view the stored noise spectra on a give detector
//It also creates a .png version of the histogram in the directory from which you run it
//To run this macro type: root 'viewNoise_iZIP.C(zip#, "seriesNumber")'
//You must first replace the directory path where the data is located

void viewNoise_iZIP(int zip, string seriesname)
{

  //  int FillColor = 10;
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  //Read the histograms - replace with your data directory
  string path = "/localhome/llhsu/nano_storage/test_noisefiles/testIT_Filter_";
  //filterfilename = path + seriesname + "_brDate01110207.root";
  filterfilename = path + seriesname + ".root";

  TFile f(filterfilename.c_str());
  TH1D* h_pas1 = f.Get(Form("zip%d/PAS1NoisePSD", zip));
  TH1D* h_pbs1 = f.Get(Form("zip%d/PBS1NoisePSD", zip));
  TH1D* h_pcs1 = f.Get(Form("zip%d/PCS1NoisePSD", zip));
  TH1D* h_pds1 = f.Get(Form("zip%d/PDS1NoisePSD", zip));
  TH1D* h_qis1 = f.Get(Form("zip%d/QIS1NoisePSD", zip));
  TH1D* h_qos1 = f.Get(Form("zip%d/QOS1NoisePSD", zip));
  TH1D* h_pas2 = f.Get(Form("zip%d/PAS2NoisePSD", zip));
  TH1D* h_pbs2 = f.Get(Form("zip%d/PBS2NoisePSD", zip));
  TH1D* h_pcs2 = f.Get(Form("zip%d/PCS2NoisePSD", zip));
  TH1D* h_pds2 = f.Get(Form("zip%d/PDS2NoisePSD", zip));
  TH1D* h_qis2 = f.Get(Form("zip%d/QIS2NoisePSD", zip));
  TH1D* h_qos2 = f.Get(Form("zip%d/QOS2NoisePSD", zip));

  //Format the histograms

  h_pas1->SetTitle(Form("z%d: side1 phonons", zip));
  h_pas2->SetTitle(Form("z%d: side2 phonons", zip));

  h_qis1->SetTitle(Form("z%d: side1 charge", zip));
  h_qis2->SetTitle(Form("z%d: side2 charge", zip));
  
  Double_t max = h_pas1->GetMaximum();
  if(h_pbs1->GetMaximum()>max) max = h_pbs1->GetMaximum();
  if(h_pcs1->GetMaximum()>max) max = h_pcs1->GetMaximum();
  if(h_pds1->GetMaximum()>max) max = h_pds1->GetMaximum();
  if(h_pas2->GetMaximum()>max) max = h_pas2->GetMaximum();
  if(h_pbs2->GetMaximum()>max) max = h_pbs2->GetMaximum();
  if(h_pcs2->GetMaximum()>max) max = h_pcs2->GetMaximum();
  if(h_pds2->GetMaximum()>max) max = h_pds2->GetMaximum();
  
  h_pas1->SetMaximum(2*max);
  h_pbs1->SetMaximum(2*max);
  h_pcs1->SetMaximum(2*max);
  h_pds1->SetMaximum(2*max);
  h_pas2->SetMaximum(2*max);
  h_pbs2->SetMaximum(2*max);
  h_pcs2->SetMaximum(2*max);
  h_pds2->SetMaximum(2*max);
  
  Double_t qmax = h_qis1->GetMaximum();
  if(h_qos1->GetMaximum()>qmax) qmax = h_qos1->GetMaximum();
  if(h_qis2->GetMaximum()>qmax) qmax = h_qis2->GetMaximum();
  if(h_qos2->GetMaximum()>qmax) qmax = h_qos2->GetMaximum();

  h_qis1->SetMaximum(2*qmax);
  h_qos1->SetMaximum(2*qmax);
  h_qis2->SetMaximum(2*qmax);
  h_qos2->SetMaximum(2*qmax);
  
  //format histograms
  formatHisto(h_pas1, 1, kRed);
  formatHisto(h_pbs1, 1, kMagenta);
  formatHisto(h_pcs1, 1, kBlue);
  formatHisto(h_pds1, 1, kCyan);
  formatHisto(h_qis1, 1, kGreen);
  formatHisto(h_qos1, 1, kBlack);

  //FIXME
  h_qis1->SetYTitle("V/#sqrt{hz}");
  h_qos1->SetYTitle("V/#sqrt{hz}");

  formatHisto(h_pas2, 2, kRed);
  formatHisto(h_pbs2, 2, kMagenta);
  formatHisto(h_pcs2, 2, kBlue);
  formatHisto(h_pds2, 2, kCyan);
  formatHisto(h_qis2, 2, kGreen);
  formatHisto(h_qos2, 2, kBlack);

  //FIXME
  h_qis2->SetYTitle("V / #sqrt{hz}");
  h_qos2->SetYTitle("V / #sqrt{hz}");
  
  // plot histograms

  TCanvas* can = new TCanvas("mycan","mycan",1200, 800);
  can->SetHighLightColor(10);		      
  can->SetTitle(Form("File %s",filterfilename.c_str()));
  //can->SetFillColor(FillColor);		      
  can->Divide(2,2);

  // === phonons side 1 ===

  can->cd(1);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();

  h_pas1->DrawCopy();
  h_pbs1->DrawCopy("same");
  h_pcs1->DrawCopy("same");
  h_pds1->DrawCopy("same");
  
  TLegend* legendP1 = new TLegend(0.75, 0.65, 0.90, 0.90);
  legendP1->AddEntry(h_pas1->GetName(), "PAS1","L");
  legendP1->AddEntry(h_pbs1->GetName(), "PBS1","L");
  legendP1->AddEntry(h_pcs1->GetName(), "PCS1","L");
  legendP1->AddEntry(h_pds1->GetName(), "PDS1","L");
  legendP1->Draw();
  
  // === charge side 1 ===

  can->cd(3);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();

  h_qis1->DrawCopy();
  h_qos1->DrawCopy("same");

  TLegend* legendQ1 = new TLegend(0.75, 0.75, 0.90, 0.90);
  legendQ1->AddEntry(h_qis2->GetName(), "QIS1","L");
  legendQ1->AddEntry(h_qos2->GetName(), "QOS1","L");
  legendQ1->Draw();


  // === phonons side 2 ===

  can->cd(2);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();

  h_pas2->DrawCopy();
  h_pbs2->DrawCopy("same");
  h_pcs2->DrawCopy("same");
  h_pds2->DrawCopy("same");

  TLegend* legendP2 = new TLegend(0.75, 0.65, 0.90, 0.90);
  legendP2->AddEntry(h_pas2->GetName(), "PAS2","L");
  legendP2->AddEntry(h_pbs2->GetName(), "PBS2","L");
  legendP2->AddEntry(h_pcs2->GetName(), "PCS2","L");
  legendP2->AddEntry(h_pds2->GetName(), "PDS2","L");
  legendP2->Draw();


  can->cd(4);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();

  h_qis2->DrawCopy();
  h_qos2->DrawCopy("same");

  TLegend* legendQ2 = new TLegend(0.75, 0.75, 0.90, 0.90);
  legendQ2->AddEntry(h_qis2->GetName(), "QIS2","L");
  legendQ2->AddEntry(h_qos2->GetName(), "QOS2","L");
  legendQ2->Draw();
  

  //store the plots

  can->Print(Form("plots/viewnoise_%s_z%d.gif", seriesname.c_str(), zip));

  f.Close();

}

void formatHisto(TH1D* histo, int lineStyle, int color)
{

  histo->SetAxisRange(1e3,625e3);
  //histo->SetAxisRange(2e2,625e3);
  histo->SetStats(kFALSE);
  histo->SetLineWidth(3);

  histo->SetLineStyle(lineStyle);
  histo->SetLineColor(color);

  histo->SetXTitle("frequency [hz]");
  histo->SetYTitle("amps / #sqrt{hz}");

  return;
}

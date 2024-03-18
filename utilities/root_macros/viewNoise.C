//This macro is used to view the stored noise spectra on a give detector
//It also creates a .png version of the histogram in the directory from which you run it
//To run this macro type: root 'viewNoise.C(zip#, "somedirectory/filterfile.root")'

void viewNoise(int zip, string filterfilename)
{

  int iZip = zip%6;
  if(iZip==0) iZip=6;
  int iTower = zip/6+1;
  if(iZip==6) iTower--;
  int FillColor = 19;

  cout<<"iZip  : "<<iZip<<endl;
  cout<<"iTower: "<<iTower<<endl;

  gROOT->Reset();

  //Read the histograms

  TFile f(filterfilename.c_str());
  TH1D* h_pa = f.Get(Form("zip%d/PANoisePSD", zip));
  TH1D* h_pb = f.Get(Form("zip%d/PBNoisePSD", zip));
  TH1D* h_pc = f.Get(Form("zip%d/PCNoisePSD", zip));
  TH1D* h_pd = f.Get(Form("zip%d/PDNoisePSD", zip));
  TH1D* h_qi = f.Get(Form("zip%d/QINoisePSD", zip));
  TH1D* h_qo = f.Get(Form("zip%d/QONoisePSD", zip));

  //Format the histograms

  h_pa->SetTitle(Form("T%dZ%d Phonon", iTower, iZip));
  h_qi->SetTitle(Form("T%dZ%d Charge", iTower, iZip));
  
  Double_t max = h_pa->GetMaximum();
  if(h_pb->GetMaximum()>max) max = h_pb->GetMaximum();
  if(h_pc->GetMaximum()>max) max = h_pc->GetMaximum();
  if(h_pd->GetMaximum()>max) max = h_pd->GetMaximum();
  
  h_pa->SetMaximum(2*max);
  h_pb->SetMaximum(2*max);
  h_pc->SetMaximum(2*max);
  h_pd->SetMaximum(2*max);
  
  Double_t qmax = h_qi->GetMaximum();
  if(h_qo->GetMaximum()>qmax) qmax = h_qo->GetMaximum();
  h_qi->SetMaximum(2*qmax);
  h_qo->SetMaximum(2*qmax);
  
  h_pa->SetAxisRange(1e3,625e3);
  h_pb->SetAxisRange(1e3,625e3);
  h_pc->SetAxisRange(1e3,625e3);
  h_pd->SetAxisRange(1e3,625e3);
  h_qi->SetAxisRange(1e3,625e3);
  h_qo->SetAxisRange(1e3,625e3);

  h_pa->SetStats(kFALSE);
  h_pb->SetStats(kFALSE);
  h_pc->SetStats(kFALSE);
  h_pd->SetStats(kFALSE);
  h_qi->SetStats(kFALSE);
  h_qo->SetStats(kFALSE);
  
  h_pa->SetLineWidth(3);
  h_pa->SetLineColor(kRed);
  h_pb->SetLineWidth(3);
  h_pb->SetLineColor(kMagenta);
  h_pc->SetLineWidth(3);
  h_pc->SetLineColor(kBlue);
  h_pd->SetLineWidth(3);
  h_pd->SetLineColor(kCyan);
  
  h_qi->SetLineWidth(3);
  h_qi->SetLineColor(kGreen);
  h_qo->SetLineWidth(3);
  h_qo->SetLineColor(kBlack);
  
  
  // plot histograms

  TCanvas* can = new TCanvas("mycan","mycan",800, 900);
  can->SetHighLightColor(10);		      
  can->SetTitle(Form("File %s",filterfilename.c_str()));
  can->SetFillColor(FillColor);		      
  can->Divide(1,2);
  
  can->cd(1);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();
  gPad->SetFillColor(FillColor);
  h_pa->DrawCopy();
  h_pb->DrawCopy("same");
  h_pc->DrawCopy("same");
  h_pd->DrawCopy("same");
  
  TLegend* legendP = new TLegend(0.75, 0.65, 0.90, 0.90);
  legendP->AddEntry(h_pa->GetName(), "PA","L");
  legendP->AddEntry(h_pb->GetName(), "PB","L");
  legendP->AddEntry(h_pc->GetName(), "PC","L");
  legendP->AddEntry(h_pd->GetName(), "PD","L");
  legendP->Draw();
  
  can->cd(2);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetGrid();
  gPad->SetFillColor(FillColor);
  h_qi->DrawCopy();
  h_qo->DrawCopy("same");
  
  TLegend* legendQ = new TLegend(0.75, 0.75, 0.90, 0.90);
  legendQ->AddEntry(h_qi->GetName(), "QI","L");
  legendQ->AddEntry(h_qo->GetName(), "QO","L");
  legendQ->Draw();
  

  //store the plots

  can->Print(Form("det%d.noisespec.png", zip));

  f.Close();

}


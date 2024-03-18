#include <vector>
#include <complex>

static const Int_t nBins = 2048; //Soudan, SUF

void doFFT(const std::vector<double>& pulsevector, 
	   std::vector<double>& fftRe, std::vector<double>& fftIm);

void doIFFT(std::vector<double>& inRe, std::vector<double>& inIm,
	    std::vector<double>& outRe, std::vector<double>& outIm);

//==============================================

void fftcheck()
{
   TFile f("../templates/NoiseAndTemplates_1703190151.root");
   TTree* myTree = f.Get("zip28");
   
   //Variables
   std::vector<double> pulsetempvec;
   std::vector<double> re_pulseFFT_vec;
   std::vector<double> im_pulseFFT_vec;
   Double_t pulsetemp[nBins]; 
   Double_t re_pulseFFT[nBins]; 
   Double_t im_pulseFFT[nBins]; 
   TH1D* h_pulsetemp = new TH1D("Pulse template", "Pulse template", nBins, 0.0, nBins);
   TH1D* h_pulsetemp_check = new TH1D("Pulse template (FFT&IFFT)", "Pulse template (FFT&IFFT)", nBins, 0.0, nBins);
   TH1D* h_pulseFFT_check = new TH1D("FFTPulse (IFFT)", "FFTPulse (IFFT)", nBins, 0.0, nBins);

   //Branch Addresses
   myTree->SetBranchAddress("PA_template_time", pulsetemp);
   myTree->SetBranchAddress("PA_template_fft_real", re_pulseFFT);
   myTree->SetBranchAddress("PA_template_fft_img", im_pulseFFT);
   myTree->GetEntry(0);
      
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_pulsetemp->SetBinContent(binItr+1, pulsetemp[binItr]);
      pulsetempvec.push_back(pulsetemp[binItr]);
      re_pulseFFT_vec.push_back(re_pulseFFT[binItr]);
      im_pulseFFT_vec.push_back(im_pulseFFT[binItr]);
   }

   //FFT Calculations
   std::vector<double> pulsetempfftRe, pulsetempfftIm;
   std::vector<double> pulsetempifftRe, pulsetempifftIm;
   std::vector<double> pulsetempInv;
   doFFT(pulsetempvec, pulsetempfftRe, pulsetempfftIm);
   doIFFT(pulsetempfftRe, pulsetempfftIm, pulsetempifftRe, pulsetempifftIm);
   doIFFT(re_pulseFFT_vec, im_pulseFFT_vec, pulsetempInv, pulsetempifftIm);

   //Copy results into histogram
   for(int fftItr=0; fftItr<nBins; fftItr++)
   {
      h_pulsetemp_check->SetBinContent(fftItr+1, pulsetempifftRe[fftItr]); 
      h_pulseFFT_check->SetBinContent(fftItr+1, pulsetempInv[fftItr]); 
   }

   //scaling
   double dt = 1.0/1.25e6;
   h_pulseFFT_check->Scale(1.0/sqrt(dt));

   //Plots
   TCanvas* c1 = new TCanvas("c1","",600,800);
   c1->Divide(1,2);
   c1->SetHighLightColor(10);
   c1->cd(1);
   h_pulsetemp->DrawCopy();
   h_pulsetemp_check->SetLineColor(kBlue);
   h_pulsetemp_check->SetLineStyle(3);
   h_pulsetemp_check->DrawCopy("same");
   c1->cd(2);  
   h_pulseFFT_check->SetLineColor(kMagenta);
   h_pulseFFT_check->DrawCopy();
   h_pulsetemp_check->DrawCopy("same");
  


   return;
}

//////////////////////////////////////////////////////////////////////

void doFFT(const std::vector<double>& pulsevector, 
	   std::vector<double>& fftRe, std::vector<double>& fftIm)
{
    double *pvector = new double[pulsevector.size()];
    double re,im;
    int i;
    int n = pulsevector.size();

   //copying std vector back into array to pass to FFT (to mirror what is done in PulseTools)
   for (i=0;i<(int)pulsevector.size();i++){
      pvector[i] = pulsevector[i];  
   }

   TVirtualFFT *fftr2c = TVirtualFFT::FFT(1,&n,"R2C ES");
   fftr2c->SetPoints(pvector);
   fftr2c->Transform();
   fftr2c->GetPointComplex(0,re,im);
   fftRe.push_back(re/sqrt(n)); //DC component
   fftIm.push_back(im/sqrt(n)); //DC component

   //I don't understand the max index here, shouldn't this just be n?
   //for(i=1; i<(int)((double)(n+1.0)/2.0) ;i++)
   for(i=1; i<n ;i++)
   {
      fftr2c->GetPointComplex(i,re,im);
      fftRe.push_back(re/sqrt(n));
      fftIm.push_back(im/sqrt(n));
   }

   cout <<"FFT Done!" << endl;

   return;

}

void doIFFT(std::vector<double>& inRe, std::vector<double>& inIm,
	    std::vector<double>& outRe, std::vector<double>& outIm)
{
    double *re_pvector = new double[inRe.size()];
    double *im_pvector = new double[inIm.size()];

    double re,im;
    int i;
    int n = inRe.size(); //both inRe and inIm should have same size
   
    //copying std vector back into array to pass to IFFT
    for (i=0;i<(int)inRe.size();i++){
       re_pvector[i] = inRe[i];  
       im_pvector[i] = inIm[i];  
    }
   
//    TVirtualFFT *ifftc2r = TVirtualFFT::FFT(1,&n,"C2R ES"); //this should be the opposite of FFT
//    TFFTRealComplex *ifftc2r = (TFFTRealComplex)TVirtualFFT::FFT(1,&n,"C2R ES"); //this should be the opposite of FFT
    TVirtualFFT *ifftc2r = TVirtualFFT::FFT(1,&n,"C2R ES"); //this should be the opposite of FFT
    ifftc2r->SetPointsComplex(re_pvector, im_pvector);
    ifftc2r->Transform();
    ifftc2r->GetPointComplex(0,re,im);
    outRe.push_back(re/sqrt(n)); //DC component
//    outIm.push_back(im); //DC component
   
//    //I don't understand the max index here, shouldn't this just be n?
//    //for(i=1; i<(int)((double)(n+1.0)/2.0) ;i++)
    for(i=1; i<n ;i++)
    {
       ifftc2r->GetPointComplex(i,re, im);
       outRe.push_back(re/sqrt(n));
//       outIm.push_back(im);
//       cout <<"Re = " << re <<", im = " << im << endl;

    }

   cout <<"IFFT Done!" << endl;

   return;
}

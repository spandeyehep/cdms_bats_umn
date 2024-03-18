#include <vector>
#include "math.h"

#include <TH1D>
#include <TComplex>

void getPulse(std::vector<double>& pulsevec, TH1D* h_pulse);
void getNoise(std::vector<double>& pulsetempvec, TH1D* h_noise);
void getPulseTemplate(std::vector<double>& pulsetempvec, TH1D* h_pulsetemp);

void getNoiseTemplate(std::vector<double>& re_temp_conj_noise_vec, std::vector<double>& im_temp_conj_noise_vec, 
		      std::vector<double>& re_temp_fft_vec, std::vector<double>& im_temp_fft_vec,		      
		      std::vector<double>& noise_fftsq_vec,
		      double& norm_fft, TH1D* h_noise_fftsq);

void doFFT(const std::vector<double>& pulsevector, 
	   std::vector<double>& fftRe, std::vector<double>& fftIm);

void doIFFT(std::vector<double>& inRe, std::vector<double>& inIm,
	    std::vector<double>& outRe, std::vector<double>& outIm);

void shiftHisto(TH1D* h, int idelay); 

static const Int_t nBins = 2048; //Soudan, SUF

//==============================================

void optfilt_dev(double fakeamp = 0.5e9, int fakedelay = 525)
{
   //================== Declarations =====================

   double dt = 1.0/1.25e6; //1/samplerate
   int trigTime = 500; //FIXME, this is just approximate based on Jeff's note
   int preTrigWindow = 500 - floor(50e-6/dt);
   int postTrigWindow = 500 + floor(200e-6/dt);

   //declare data containers
   std::vector<double> pulsevec; 
   std::vector<double> noisevec; 
   std::vector<double> fakepulsevec;

   //template containers
   double norm_fft; //denom of amp
   std::vector<double> pulsetempvec; //pulse template   
   std::vector<double> re_temp_conj_noise_vec; //s*/J
   std::vector<double> im_temp_conj_noise_vec; //s*/J
   std::vector<double> re_temp_fft_vec; //template fft
   std::vector<double> im_temp_fft_vec; //template fft
   std::vector<double> noise_fftsq_vec; //noise power spec

   //Histograms for checking
   TH1D* h_pulse = new TH1D("Pulse ", "Pulse ", nBins, 0.0, nBins);
   TH1D* h_noise = new TH1D("Noise ", "Noise ", nBins, 0.0, nBins);

   TH1D* h_pulsetemp = new TH1D("Pulse template", "Pulse template", nBins, 0.0, nBins);
   TH1D* h_noise_fftsq = new TH1D("Noise FFTsq", "Noise FFTsq", nBins, 0.0, nBins);

   TH1D* h_amp = new TH1D("Amp vs iDelay ", "Amp vs iDelay", nBins, 0.0, nBins);
   //================ Getting Data and Templates ===================

   getPulse(pulsevec, h_pulse);
   getNoise(noisevec, h_noise);

   getPulseTemplate(pulsetempvec, h_pulsetemp);
   getNoiseTemplate(re_temp_conj_noise_vec, im_temp_conj_noise_vec, 
		    re_temp_fft_vec, im_temp_fft_vec, 
		    noise_fftsq_vec, norm_fft, h_noise_fftsq);

   //Make a fake pulse from the template and the noise - no time shifts to begin with!
   TH1D* h_fakepulse = (TH1D*)h_pulsetemp->Clone("fakepulse");
   h_fakepulse->Scale(fakeamp); //setting amplitude
   shiftHisto(h_fakepulse, fakedelay); //setting delay (in bins)
   h_fakepulse->Add(h_noise, 1.0);

   for(int binItr=1; binItr <= h_fakepulse->GetNbinsX(); binItr++)
   {
     fakepulsevec.push_back(h_fakepulse->GetBinContent(binItr));
   }
   cout <<"fakepulsevec size = " << fakepulsevec.size() << endl;
   
   //================== Calculations =====================

   //more vectors for calculations
   std::vector<double> pulsefftRe, pulsefftIm;
   std::vector<double> pulseifftRe, pulseifftIm;
   std::vector<double> p_prod_Re, p_prod_Im;
   std::vector<double> p_prod_ifftRe, p_prod_ifftIm;

   //1. construct fitpulse fft
   //doFFT(fakepulsevec, pulsefftRe, pulsefftIm);
   doFFT(pulsevec, pulsefftRe, pulsefftIm);

   //2. construct p_prod = sqrt(dt) * pulse_fft * s*/J
   for(int binItr=0; binItr < pulsefftRe.size(); binItr++)
   {
     TComplex comp_zero(0.,0.);
     TComplex comp_pulse_fft(pulsefftRe[binItr], pulsefftIm[binItr]);
     TComplex comp_optfilt(re_temp_conj_noise_vec[binItr], im_temp_conj_noise_vec[binItr]);
     TComplex comp_p_prod = sqrt(dt)*comp_pulse_fft*comp_optfilt;

     if(binItr==0) comp_p_prod = comp_zero; //ignoring DC component

     //copy complex numbers into separate vectors
     p_prod_Re.push_back(comp_p_prod.Re());
     p_prod_Im.push_back(comp_p_prod.Im());

   } 

   //3. get max amplitude and delay
   std::vector<double> p_ahat; //vector of all amplitudes, eventually restrict the search window
   double maxAmp = 0;
   double amp0 = 0;
   int idelay = 0;

   doIFFT(p_prod_Re, p_prod_Im, p_prod_ifftRe);  //C2R - no imaginary component!    

   //pick out maximum amplitude and corresponding delay
   for(int binItr=0; binItr < pulsefftRe.size(); binItr++)
   {
     double amp =  p_prod_ifftRe[binItr]/norm_fft;
     if(amp > maxAmp) { maxAmp = amp; idelay = binItr; }
     if(binItr == trigTime) { amp0 = amp; }

     h_amp->SetBinContent(binItr+1, amp);
   }
   cout <<"Max amp = " << maxAmp <<", at bin = " << idelay 
	<<"\n amp0 = " << amp0 
	<< endl;

   //4. find delay and compute chisq (also will need to find amp0 - ask Bruno about trigger time 0?)
   //delay = idelay + preTrigWindow; //no need to shift since idelay is wrt to bin0 already
   delay = idelay;

   //Compute the chisq
   double chisq = 0;        //ignore DC component?
   for(int binItr=1; binItr < nBins; binItr++)
     {
       double pi = acos(-1); //why isn't this defined in math.h anymore?
       double theta = 2.0*pi*((double)binItr/(double)nBins)*(double)delay;

       TComplex phase_factor(cos(theta), sin(theta));
       TComplex temp_fft(re_temp_fft_vec[binItr], im_temp_fft_vec[binItr]);
       TComplex fit_fft( maxAmp*temp_fft/phase_factor );
       TComplex pulse_fft(pulsefftRe[binItr], pulsefftIm[binItr]);
       //TComplex difference = pulse_fft - fit_fft;

       chisq += pow(TComplex::Abs(pulse_fft - fit_fft), 2)/noise_fftsq_vec[binItr];

//        cout <<"phase factor = " << phase_factor
// 	    <<"\ntemp_fft = " << temp_fft
// 	    <<"\nfit_fft = " << fit_fft
// 	    <<"\np_fft = "   << pulse_fft
// 	    <<"\ndifference = " << pulse_fft - fit_fft
// 	    <<"\nabs = " << TComplex::Abs(pulse_fft - fit_fft)
// 	    <<"\nnoise_fftsq_vec = " << noise_fftsq_vec[binItr]
// 	    <<"\nchisq icrement = " << pow(abs(pulse_fft - fit_fft), 2)/noise_fftsq_vec[binItr]
// 	    <<"\npi = " << pi
// 	    << endl;
     }

   cout <<"\nchisq = " << chisq << endl;

   //=====================================================

   //Copy results into histograms

   //scale fit result
   TH1D* h_fit = (TH1D*)h_pulsetemp->Clone("fit result");
   h_fit->Scale(maxAmp);
   shiftHisto(h_fit, idelay); //shift fit template by idelay
   
   //=================== Plots ==========================

   //Formatting
   TLine l;
   
   h_fakepulse->SetTitle(Form("in amp = %.2E (fit amp = %.2E), in delay = %d (fit delay = %d)", fakeamp, maxAmp, fakedelay, idelay));
   h_fakepulse->SetStats(kFALSE);

   h_fit->SetLineColor(kBlue);
   h_fit->SetLineWidth(3);
   h_fit->SetStats(kFALSE);

   h_noise->SetLineColor(kRed);
   h_noise->SetStats(kFALSE);

   h_amp->SetStats(kFALSE);
   h_amp->SetLineColor(kMagenta);
   h_amp->GetYaxis()->SetTitle("amplitude");
   h_amp->GetXaxis()->SetTitle("idelay");
   
   //Drawing
   TCanvas* c1 = new TCanvas("c1","",600,800);
   c1->Divide(1,2);
   c1->SetHighLightColor(10);
   c1->cd(1);
   h_pulse->DrawCopy();
   //h_fakepulse->DrawCopy();
   h_fit->DrawCopy("same");
   
   l.DrawLine(0.0, 0.0, 2048, 0.0);

   c1->cd(2);
   h_amp->DrawCopy();

   l.DrawLine(0.0, 0.0, 2048, 0.0);

   return;

} //Done!

/////////////////////// Supporting Routines /////////////////////////////

void getPulse(std::vector<double>& pulsevec, TH1D* h_pulse)
{
   //Temp! using the template pulse as the data
   TFile f("170319_1616_F0002_pulsetrace.root");
   TTree* pulseTree = (TTree*)f.Get("pulseTree");

   //Set Branch Addresses
   float pulse[nBins];
   pulseTree->SetBranchAddress("bsnPulse", pulse);
   pulseTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_pulse->SetBinContent(binItr+1, pulse[binItr]);
      pulsevec.push_back((double)pulse[binItr]);
   }
   
   return;
}
void getNoise(std::vector<double>& noisevec, TH1D* h_noise)
{
   //Temp! using the template pulse as the data
   TFile f("170319_1616_F0002_noisetrace.root");
   TTree* pulseTree = (TTree*)f.Get("pulseTree");

   //Set Branch Addresses
   float pulse[nBins];
   pulseTree->SetBranchAddress("bsnPulse", pulse);
   pulseTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_noise->SetBinContent(binItr+1, pulse[binItr]);
      noisevec.push_back((double)pulse[binItr]);
   }
   
   return;
}

//////////////////////////////////////////////////////////////////////////////////////////

void getPulseTemplate(std::vector<double>& pulsetempvec, TH1D* h_pulsetemp)
{
   //Temp! using the template pulsetemp as the data
   TFile f("../templates/NoiseAndTemplates_1703191616.root");
   TTree* pulsetempTree = (TTree*)f.Get("zip28");

   //Set Branch Addresses
   double pulsetemp[nBins];
   pulsetempTree->SetBranchAddress("PA_template_time", pulsetemp);
   pulsetempTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_pulsetemp->SetBinContent(binItr+1, pulsetemp[binItr]);
      pulsetempvec.push_back(pulsetemp[binItr]);
   }
   
   return;
}

void getNoiseTemplate(std::vector<double>& re_temp_conj_noise_vec, std::vector<double>& im_temp_conj_noise_vec, 
		      std::vector<double>& re_temp_fft_vec, std::vector<double>& im_temp_fft_vec,
		      std::vector<double>& noise_fftsq_vec,
		      double& norm_fft, TH1D* h_noise_fftsq)
{
   //Temp! using the template pulsetemp as the data
   TFile f("../templates/NoiseAndTemplates_1703191616.root");
   TTree* noisetempTree = (TTree*)f.Get("zip28");

   //Set Branch Addresses
   double re_temp_conj_noise[nBins];
   double im_temp_conj_noise[nBins];
   double re_temp_fft[nBins];
   double im_temp_fft[nBins];
   double noise_fftsq[nBins];

   noisetempTree->SetBranchAddress("PA_template_conjnoise_fft_real", re_temp_conj_noise);
   noisetempTree->SetBranchAddress("PA_template_conjnoise_fft_img", im_temp_conj_noise);
   noisetempTree->SetBranchAddress("PA_template_fft_real", re_temp_fft);
   noisetempTree->SetBranchAddress("PA_template_fft_img", im_temp_fft);
   noisetempTree->SetBranchAddress("PA_noise_fftsq", noise_fftsq);
   noisetempTree->SetBranchAddress("PA_norm_fft", &norm_fft);
   noisetempTree->GetEntry(0);
   
   cout <<"norm fft = " << norm_fft << endl;

   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      //first bin is inf?
      if(binItr==0) noise_fftsq[binItr] = 0.0; 
      
      //histogramming
      h_noise_fftsq->SetBinContent(binItr+1, noise_fftsq[binItr]);
      
      //saving in vectors
      re_temp_conj_noise_vec.push_back(re_temp_conj_noise[binItr]);
      im_temp_conj_noise_vec.push_back(im_temp_conj_noise[binItr]);
      re_temp_fft_vec.push_back(re_temp_fft[binItr]);
      im_temp_fft_vec.push_back(im_temp_fft[binItr]);

      noise_fftsq_vec.push_back(noise_fftsq[binItr]);

   }


   return;
}

//////////////////////////////////////////////////////////////////////

//using darkpipe symmetric convention for normalization
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

//using darkpipe symmetric convention for normalization
void doIFFT(std::vector<double>& inRe, std::vector<double>& inIm,
	    std::vector<double>& outRe)
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
   
    TVirtualFFT *ifftc2r = TVirtualFFT::FFT(1,&n,"C2R ES"); //this should be the opposite of FFT
    ifftc2r->SetPointsComplex(re_pvector, im_pvector);
    ifftc2r->Transform();
    ifftc2r->GetPointComplex(0,re,im);
    outRe.push_back(re/sqrt(n)); //DC component
   
    for(i=1; i<n ;i++)
    {
       ifftc2r->GetPointComplex(i,re, im);
       outRe.push_back(re/sqrt(n));
//       cout <<"Re = " << re <<", im = " << im << endl;

    }

   cout <<"IFFT Done!" << endl;

   return;
}

////////////////////////////////////////////////

//for making overlays of template with pulse.
//Shifts histogram by idelay to the right, 
//zeroing out all bins before idelay
void shiftHisto(TH1D* h, int idelay)
{
  TH1D* aTempHisto = (TH1D*)h->Clone("aCopy");

  //make shifted histo, aTempHisto 
  for(int binItr = 0; binItr < h->GetNbinsX(); binItr++)
    {
      if(binItr<idelay) 
	aTempHisto->SetBinContent(binItr+1, 0.0); 
      else
	aTempHisto->SetBinContent(binItr+1, h->GetBinContent(binItr-idelay+1));
    }

  //copy contents of aTempHist into h
  for(int binItr = 0; binItr < h->GetNbinsX(); binItr++)
    {
      h->SetBinContent(binItr+1, aTempHisto->GetBinContent(binItr+1));
    }

  return;
}

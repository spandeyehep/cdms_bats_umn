#include <vector>
#include "math.h"

#include <TH1D>
#include <TComplex>

void getPulse(std::vector<double>& pulsevec, TH1D* h_pulse, string channelName="QI");

void getNoise(std::vector<double>& pulsetempvec, TH1D* h_noise, string channelName="QI");

void getPulseTemplate(std::vector<double>& pulsetempvec_qi, TH1D* h_pulsetemp_qi,
		      std::vector<double>& pulsetempvec_qo, TH1D* h_pulsetemp_qo,
		      std::vector<double>& pulsetempvec_qix, TH1D* h_pulsetemp_qix,
		      std::vector<double>& pulsetempvec_qox, TH1D* h_pulsetemp_qox);

void getNoiseTemplate(std::vector<double>& re_temp_conj_noise_vec, std::vector<double>& im_temp_conj_noise_vec, 
		      std::vector<double>& re_temp_fft_vec, std::vector<double>& im_temp_fft_vec,		      
		      std::vector<double>& noise_fftsq_vec,
		      double& norm_fft, TH1D* h_noise_fftsq, string channelName="QI");

void getQInverse(double* qinverse);

void doFFT(const std::vector<double>& pulsevector, 
	   std::vector<double>& fftRe, std::vector<double>& fftIm);

void doIFFT(std::vector<double>& inRe, std::vector<double>& inIm,
	    std::vector<double>& outRe, std::vector<double>& outIm);

void shiftHisto(TH1D* h, int idelay); 

static const Int_t nBins = 2048; //Soudan, SUF

//==============================================

void optfiltX_dev(double fakeampQI = 75, double fakeampQO = 35.0, int fakedelay = 25)
{
   //================== Declarations =====================

   double dt = 1.0/1.25e6; //1/samplerate
   int trigTime = 512; //FIXME, this is just approximate based on Jeff's note
   int preTrigWindow = 512 - floor(50e-6/dt);
   int postTrigWindow = 512 + floor(200e-6/dt);

   //declare data containers
   std::vector<double> pulsevec_qi; 
   std::vector<double> pulsevec_qo; 

   std::vector<double> noisevec_qi; 
   std::vector<double> noisevec_qo; 

   std::vector<double> fakepulsevec_qi;
   std::vector<double> fakepulsevec_qo;

   //template containers
   double qi_norm_fft;
   double qo_norm_fft;
   double qix_norm_fft;
   double qox_norm_fft;
   double qinverse[4];
   
   std::vector<double> pulsetempvec_qi; //pulse template   
   std::vector<double> pulsetempvec_qo; //pulse template   
   std::vector<double> pulsetempvec_qix; //pulse template   
   std::vector<double> pulsetempvec_qox; //pulse template   

   std::vector<double> qi_re_temp_conj_noise_vec; //s*/J -qi
   std::vector<double> qi_im_temp_conj_noise_vec; //s*/J -qi
   std::vector<double> qo_re_temp_conj_noise_vec; //s*/J -qo
   std::vector<double> qo_im_temp_conj_noise_vec; //s*/J -qo
   std::vector<double> qix_re_temp_conj_noise_vec; //s*/J -qix
   std::vector<double> qix_im_temp_conj_noise_vec; //s*/J -qix
   std::vector<double> qox_re_temp_conj_noise_vec; //s*/J -qox
   std::vector<double> qox_im_temp_conj_noise_vec; //s*/J -qox

   std::vector<double> qi_re_temp_fft_vec; //template fft -qi
   std::vector<double> qi_im_temp_fft_vec; //template fft -qi
   std::vector<double> qo_re_temp_fft_vec; //template fft -q
   std::vector<double> qo_im_temp_fft_vec; //template fft -qo
   std::vector<double> qix_re_temp_fft_vec; //template fft -qix
   std::vector<double> qix_im_temp_fft_vec; //template fft -qix
   std::vector<double> qox_re_temp_fft_vec; //template fft -qox
   std::vector<double> qox_im_temp_fft_vec; //template fft -qox

   std::vector<double> qi_noise_fftsq_vec; //noise power spec - qi
   std::vector<double> qo_noise_fftsq_vec; //noise power spec - qo
   std::vector<double> qix_noise_fftsq_vec; //noise power spec - qix
   std::vector<double> qox_noise_fftsq_vec; //noise power spec - qox

   //Histograms for checking
   TH1D* h_pulse_qi = new TH1D("Pulse QI", "Pulse QI", nBins, 0.0, nBins);
   TH1D* h_pulse_qo = new TH1D("Pulse QO", "Pulse QO", nBins, 0.0, nBins);
   
   TH1D* h_noise_qi = new TH1D("Noise QI", "Noise QI", nBins, 0.0, nBins);
   TH1D* h_noise_qo = new TH1D("Noise QO", "Noise QO", nBins, 0.0, nBins);

   TH1D* h_pulsetemp_qi = new TH1D("Pulse template QI", "Pulse template QI", nBins, 0.0, nBins);
   TH1D* h_pulsetemp_qo = new TH1D("Pulse template QO", "Pulse template QO", nBins, 0.0, nBins);
   TH1D* h_pulsetemp_qix = new TH1D("Pulse template QIX", "Pulse template QIX", nBins, 0.0, nBins);
   TH1D* h_pulsetemp_qox = new TH1D("Pulse template QOX", "Pulse template QOX", nBins, 0.0, nBins);

   TH1D* h_noise_fftsq_qi = new TH1D("Noise FFTsq QI", "Noise FFTsq QI", nBins, 0.0, nBins);
   TH1D* h_noise_fftsq_qo = new TH1D("Noise FFTsq QO", "Noise FFTsq QO", nBins, 0.0, nBins);
   TH1D* h_noise_fftsq_qix = new TH1D("Noise FFTsq QIX", "Noise FFTsq QIX", nBins, 0.0, nBins);
   TH1D* h_noise_fftsq_qox = new TH1D("Noise FFTsq QOX", "Noise FFTsq QOX", nBins, 0.0, nBins);

   TH1D* h_amp = new TH1D("Amp vs iDelay ", "Amp vs iDelay", nBins, 0.0, nBins);

   //================ Getting Data and Templates ===================

   //getting actual example pulse and noise
   getPulse(pulsevec_qi, h_pulse_qi, "QI");
   getNoise(noisevec_qi, h_noise_qi, "QI");

   getPulse(pulsevec_qo, h_pulse_qo, "QO");
   getNoise(noisevec_qo, h_noise_qo, "QO");

   //all pulse templates
   getPulseTemplate(pulsetempvec_qi, h_pulsetemp_qi, pulsetempvec_qo, h_pulsetemp_qo,
		    pulsetempvec_qix, h_pulsetemp_qix, pulsetempvec_qox, h_pulsetemp_qox);

   //noise templates for QI
   getNoiseTemplate(qi_re_temp_conj_noise_vec, qi_im_temp_conj_noise_vec, 
		    qi_re_temp_fft_vec, qi_im_temp_fft_vec, 
		    qi_noise_fftsq_vec, qi_norm_fft, h_noise_fftsq_qi, "QI");
   
   //noise templates for QO
   getNoiseTemplate(qo_re_temp_conj_noise_vec, qo_im_temp_conj_noise_vec, 
		    qo_re_temp_fft_vec, qo_im_temp_fft_vec, 
		    qo_noise_fftsq_vec, qo_norm_fft, h_noise_fftsq_qo, "QO");

   //noise templates for QIX
   getNoiseTemplate(qix_re_temp_conj_noise_vec, qix_im_temp_conj_noise_vec, 
		    qix_re_temp_fft_vec, qix_im_temp_fft_vec, 
		    qix_noise_fftsq_vec, qix_norm_fft, h_noise_fftsq_qix, "QIX");

   //noise templates for QOX
   getNoiseTemplate(qox_re_temp_conj_noise_vec, qox_im_temp_conj_noise_vec, 
		    qox_re_temp_fft_vec, qox_im_temp_fft_vec, 
		    qox_noise_fftsq_vec, qox_norm_fft, h_noise_fftsq_qox, "QOX");

//    //Temp! for debugging port
//    for(int ctr=0; ctr < 2048; ctr++)
//    { 
//       cout <<"\nnorm_fft[" << ctr <<"] = " << qox_norm_fft
// 	   <<"\nnoise_fftsq[" << ctr <<"] = " << qox_noise_fftsq_vec[ctr]
// 	   <<"\npulse_fft[" << ctr <<"] = (" << qox_re_temp_fft_vec[ctr] <<", " << qox_im_temp_fft_vec[ctr] <<") " 
// 	   <<"\npulse_fft[" << ctr <<"] = (" << qox_re_temp_conj_noise_vec[ctr] <<", " << qox_im_temp_conj_noise_vec[ctr] <<") " 
// 	   << endl;
//    }

   //get inverse of chisq matrix
   getQInverse(qinverse);

   //========== Make a fake pulse from template noise =============
   
   //constructing fake QI response
   TH1D* h_fakepulse_qi = (TH1D*)h_pulsetemp_qi->Clone("fakepulseQI");
   h_fakepulse_qi->Scale(fakeampQI); 
   shiftHisto(h_fakepulse_qi, fakedelay); 

   TH1D* h_fake_qix = (TH1D*)h_pulsetemp_qix->Clone("fakeQIX");
   h_fake_qix->Scale(fakeampQO); 
   shiftHisto(h_fake_qix, fakedelay); 

   h_fakepulse_qi->Add(h_noise_qi, 1.0);
   h_fakepulse_qi->Add(h_fake_qix, 1.0);

   //constructing fake QO response
   TH1D* h_fakepulse_qo = (TH1D*)h_pulsetemp_qo->Clone("fakepulseQO");
   h_fakepulse_qo->Scale(fakeampQO); 
   shiftHisto(h_fakepulse_qo, fakedelay); 

   TH1D* h_fake_qox = (TH1D*)h_pulsetemp_qox->Clone("fakeQOX");
   h_fake_qox->Scale(fakeampQI); 
   shiftHisto(h_fake_qox, fakedelay); 

   h_fakepulse_qo->Add(h_noise_qo, 1.0);
   h_fakepulse_qo->Add(h_fake_qox, 1.0);

   //storing fake pulses for fitting
   for(int binItr=1; binItr <= h_fakepulse_qi->GetNbinsX(); binItr++)
   {
     fakepulsevec_qi.push_back(h_fakepulse_qi->GetBinContent(binItr));
   }

   for(int binItr=1; binItr <= h_fakepulse_qo->GetNbinsX(); binItr++)
   {
     fakepulsevec_qo.push_back(h_fakepulse_qo->GetBinContent(binItr));

     //Temp for debugging port
     cout <<"fakepulsevec_qo[" << binItr-1 <<"]" << fakepulsevec_qo[binItr-1] << endl;
   }

   cout <<"fakepulsevec qo size = " << fakepulsevec_qi.size() << endl;
   
   //================== Calculations =====================

   //more vectors for calculations
   std::vector<double> qi_pulsefftRe, qi_pulsefftIm;
   std::vector<double> qo_pulsefftRe, qo_pulsefftIm;
   
   std::vector<double> qi_p_prod_Re, qi_p_prod_Im;
   std::vector<double> qo_p_prod_Re, qo_p_prod_Im;

   std::vector<double> qi_p_prod_ifftRe, qi_p_prod_ifftIm;
   std::vector<double> qo_p_prod_ifftRe, qo_p_prod_ifftIm;

   //1. construct fitpulse fft's
   doFFT(fakepulsevec_qi, qi_pulsefftRe, qi_pulsefftIm);
   doFFT(fakepulsevec_qo, qo_pulsefftRe, qo_pulsefftIm);
   
//     doFFT(pulsevec_qi, qi_pulsefftRe, qi_pulsefftIm);
//     doFFT(pulsevec_qo, qo_pulsefftRe, qo_pulsefftIm);

   //2. construct p_prod = sqrt(dt) * pulse_fft * s*/J
   for(int binItr=0; binItr < qi_pulsefftRe.size(); binItr++)
   {
      TComplex comp_zero(0.,0.);

      TComplex qi_comp_pulse_fft(qi_pulsefftRe[binItr], qi_pulsefftIm[binItr]);
      TComplex qo_comp_pulse_fft(qo_pulsefftRe[binItr], qo_pulsefftIm[binItr]);

      TComplex qi_comp_optfilt(qi_re_temp_conj_noise_vec[binItr], qi_im_temp_conj_noise_vec[binItr]);
      TComplex qo_comp_optfilt(qo_re_temp_conj_noise_vec[binItr], qo_im_temp_conj_noise_vec[binItr]);

      TComplex qi_comp_p_prod = sqrt(dt)*qi_comp_pulse_fft*qi_comp_optfilt;
      TComplex qo_comp_p_prod = sqrt(dt)*qo_comp_pulse_fft*qo_comp_optfilt;

      if(binItr==0) 
      {
	 //ignoring DC component
	 qi_comp_p_prod = comp_zero; 
	 qo_comp_p_prod = comp_zero; 
      }

      //copy complex numbers into separate vectors
      qi_p_prod_Re.push_back(qi_comp_p_prod.Re());
      qi_p_prod_Im.push_back(qi_comp_p_prod.Im());

      qo_p_prod_Re.push_back(qo_comp_p_prod.Re());
      qo_p_prod_Im.push_back(qo_comp_p_prod.Im());
      
    } 

    //3. get delay from max amplitude - at this point we are fitting qi and qo separately!!
    std::vector<double> qi_p_ahat; //vector of all amplitudes, eventually restrict the search window
    std::vector<double> qo_p_ahat; //vector of all amplitudes, eventually restrict the search window
    double maxAmp = 0;
    double ampQI = 0.0;
    double ampQO = 0.0;
    double amp0 = 0;
    int idelay = 0;

    doIFFT(qi_p_prod_Re, qi_p_prod_Im, qi_p_prod_ifftRe);  //C2R - no imaginary component!    
    doIFFT(qo_p_prod_Re, qo_p_prod_Im, qo_p_prod_ifftRe);  //C2R - no imaginary component!    

    //pick out maximum amplitude and corresponding delay
    //FIXME include case where qibias != qobias
    for(int binItr=0; binItr < qi_pulsefftRe.size(); binItr++)
    {
       //Note that there seems to be no division by norm_fftsq!!
       double amp =  qi_p_prod_ifftRe[binItr] + qo_p_prod_ifftRe[binItr];
	  
       if(amp > maxAmp) 
       { 
	  maxAmp = amp;
	  ampQI = qi_p_prod_ifftRe[binItr]/qi_norm_fft;
	  ampQO = qo_p_prod_ifftRe[binItr]/qo_norm_fft;
	  idelay = binItr; 
       }
       
       if(binItr == trigTime) { amp0 = amp; }
       
       h_amp->SetBinContent(binItr+1, amp);
    }

    cout <<"Max amp = " << maxAmp <<", at bin = " << idelay 
	 <<"\n amp0 = " << amp0 
	 <<"\n ampQI = " << ampQI
	 <<"\n ampQO = " << ampQO
	 << endl;

    //compute phase factor with this delay
    int delay = (idelay < nBins/2 ? idelay : (idelay - nBins)); 
    double pi = (double)acos(-1); //why isn't this defined in math.h anymore?

    //4. compute solution to system of two equations to get correct cross-talk contribution
    double rhs1 = 0.0;
    double rhs2 = 0.0;

    //FIXME, some repetition of step 2 which can be cleaned up!
     for(int binItr=0; binItr < qi_pulsefftRe.size(); binItr++)
     {
       double theta = 2.0*pi*((double)binItr/(double)nBins)*(double)delay;
       TComplex phase_factor(cos(theta), sin(theta));
       
       TComplex comp_zero(0.,0.);

//        cout <<"theta = " << theta 
// 	    <<"\nphase_factor = " << phase_factor
// 	    << endl;

       TComplex qi_comp_pulse_fft(qi_pulsefftRe[binItr], qi_pulsefftIm[binItr]);
       TComplex qo_comp_pulse_fft(qo_pulsefftRe[binItr], qo_pulsefftIm[binItr]);

       qi_comp_pulse_fft *= phase_factor;
       qo_comp_pulse_fft *= phase_factor;

       TComplex qi_comp_optfilt(qi_re_temp_conj_noise_vec[binItr], qi_im_temp_conj_noise_vec[binItr]);
       TComplex qo_comp_optfilt(qo_re_temp_conj_noise_vec[binItr], qo_im_temp_conj_noise_vec[binItr]);
       TComplex qix_comp_optfilt(qix_re_temp_conj_noise_vec[binItr], qix_im_temp_conj_noise_vec[binItr]);
       TComplex qox_comp_optfilt(qox_re_temp_conj_noise_vec[binItr], qox_im_temp_conj_noise_vec[binItr]);

       //until I can figure out how to do matrix algebra with complex numbers in the cint
       TComplex temp1 = qi_comp_pulse_fft*qi_comp_optfilt + qo_comp_pulse_fft*qox_comp_optfilt;
       TComplex temp2 = qo_comp_pulse_fft*qo_comp_optfilt + qi_comp_pulse_fft*qix_comp_optfilt;

       rhs1 += temp1.Re();
       rhs2 += temp2.Re();
     }

     //inverse of "chisq matrix" - this is a symmetric matrix
     double a1=qinverse[0];
     double a2=qinverse[1];
     double b1=qinverse[2];
     double b2=qinverse[3];

     //doing the matrix mult explicitely until I figure out how to do it w/ complex numbers
     double sol1 = (a1*rhs1 + a2*rhs2)*sqrt(dt);
     double sol2 = (b1*rhs1 + b2*rhs2)*sqrt(dt);

     cout <<"rhs1 = " << rhs1
	  <<"\nrhs2 = " << rhs2
	  <<"\n\nsol1 = " << sol1
	  <<"\nsol2 = " << sol2
	  <<"\ndelay = " << delay
	  << endl;

     //5. compute chisq 
    double chisq = 0;        //ignore DC component?

    //FIXME, again get rid of redundancy! - at the very least multiply by sqrt(dt) only once!
    //Do we want the DC component?
    for(int binItr=1; binItr < nBins; binItr++)
    {
      double theta = 2.0*pi*((double)binItr/(double)nBins)*(double)delay;
      TComplex phase_factor(cos(theta), sin(theta));

//       cout <<"theta = " << theta 
// 	   <<"\nphase_factor = " << phase_factor
// 	   << endl;

      TComplex qi_comp_pulse_fft(qi_pulsefftRe[binItr], qi_pulsefftIm[binItr]);
      TComplex qo_comp_pulse_fft(qo_pulsefftRe[binItr], qo_pulsefftIm[binItr]);

      qi_comp_pulse_fft *= phase_factor;
      qo_comp_pulse_fft *= phase_factor;

      TComplex qi_comp_temp_fft(qi_re_temp_fft_vec[binItr], qi_im_temp_fft_vec[binItr]);
      TComplex qo_comp_temp_fft(qo_re_temp_fft_vec[binItr], qo_im_temp_fft_vec[binItr]);
      TComplex qix_comp_temp_fft(qix_re_temp_fft_vec[binItr], qix_im_temp_fft_vec[binItr]);
      TComplex qox_comp_temp_fft(qox_re_temp_fft_vec[binItr], qox_im_temp_fft_vec[binItr]);

//       double add1 = pow(TComplex::Abs(qi_comp_pulse_fft*sqrt(dt) - (sol1*qi_comp_temp_fft + sol2*qix_comp_temp_fft)), 2)/qi_noise_fftsq_vec[binItr];
//       double add2 = pow(TComplex::Abs(qo_comp_pulse_fft*sqrt(dt) - (sol2*qo_comp_temp_fft + sol1*qox_comp_temp_fft)), 2)/qo_noise_fftsq_vec[binItr];

//       cout <<"\nchisq1 = " << add1
// 	   <<"\nchisq2 = " << add2
// 	   << endl;

      chisq += pow(TComplex::Abs(qi_comp_pulse_fft*sqrt(dt) - (sol1*qi_comp_temp_fft + sol2*qix_comp_temp_fft)), 2)/qi_noise_fftsq_vec[binItr];
      chisq += pow(TComplex::Abs(qo_comp_pulse_fft*sqrt(dt) - (sol2*qo_comp_temp_fft + sol1*qox_comp_temp_fft)), 2)/qo_noise_fftsq_vec[binItr];

    }

    cout <<"\nchisq = " << chisq << endl;

    //========== Copy results into histograms =============
   
    //constructing fake QI response
    TH1D* h_fitpulse_qi = (TH1D*)h_pulsetemp_qi->Clone("fitpulseQI");
    h_fitpulse_qi->Scale(sol1); 
    shiftHisto(h_fitpulse_qi, delay); 
    
    TH1D* h_fit_qix = (TH1D*)h_pulsetemp_qix->Clone("fitQIX");
    h_fit_qix->Scale(sol2); 
    shiftHisto(h_fit_qix, delay); 
    
    h_fitpulse_qi->Add(h_noise_qi, 1.0);
    h_fitpulse_qi->Add(h_fit_qix, 1.0);
   
    //constructing fit QO response
    TH1D* h_fitpulse_qo = (TH1D*)h_pulsetemp_qo->Clone("fitpulseQO");
    h_fitpulse_qo->Scale(sol2); 
    shiftHisto(h_fitpulse_qo, delay); 

    TH1D* h_fit_qox = (TH1D*)h_pulsetemp_qox->Clone("fitQOX");
    h_fit_qox->Scale(sol1); 
    shiftHisto(h_fit_qox, delay); 
    
    h_fitpulse_qo->Add(h_noise_qo, 1.0);
    h_fitpulse_qo->Add(h_fit_qox, 1.0);


//    //scale fit result
//    TH1D* h_fit = (TH1D*)h_pulsetemp_qi->Clone("fit result");
//    h_fit->Scale(maxAmp);
//    shiftHisto(h_fit, idelay); //shift fit template by idelay
   
   //=================== Plots ==========================

   //Formatting
   TLine l;
   
   h_fakepulse_qi->SetStats(kFALSE);
   h_fakepulse_qo->SetStats(kFALSE);
   
   h_fitpulse_qi->SetLineColor(kBlue);
   h_fitpulse_qi->SetLineWidth(3);
   h_fitpulse_qi->SetLineStyle(3);
   h_fitpulse_qi->SetStats(kFALSE);

   h_fitpulse_qo->SetLineColor(kBlue);
   h_fitpulse_qo->SetLineWidth(3);
   h_fitpulse_qo->SetLineStyle(3);
   h_fitpulse_qo->SetStats(kFALSE);

   h_noise_qi->SetLineColor(kRed);
   h_noise_qi->SetStats(kFALSE);

   h_noise_qo->SetLineColor(kRed);
   h_noise_qo->SetStats(kFALSE);

   h_amp->SetStats(kFALSE);
   h_amp->SetLineColor(kMagenta);
   h_amp->GetYaxis()->SetTitle("amplitude");
   h_amp->GetXaxis()->SetTitle("idelay");
   
   //Drawing
   TCanvas* c1 = new TCanvas("c1","",1000,800);
   c1->Divide(2,2);
   c1->SetHighLightColor(10);
   c1->cd(1);
   h_pulsetemp_qi->DrawCopy();
   //h_pulse->DrawCopy();
   //h_fakepulse_qi->DrawCopy();
   //h_fit->DrawCopy("same");
   
   l.DrawLine(0.0, 0.0, 2048, 0.0);

   c1->cd(2);
   h_pulsetemp_qo->DrawCopy();
   //h_amp->DrawCopy();

   l.DrawLine(0.0, 0.0, 2048, 0.0);

   c1->cd(3);
   h_pulsetemp_qix->DrawCopy();

   c1->cd(4);
   h_pulsetemp_qox->DrawCopy();

   TCanvas* c2 = new TCanvas("c2","",1200,500);
   c2->Divide(2,1);
   c2->SetHighLightColor(10);

//     c2->cd(1);
//     h_pulse_qi->DrawCopy();
//     h_fitpulse_qi->DrawCopy("same");
   
//     c2->cd(2);
//     h_pulse_qo->DrawCopy();
//     h_fitpulse_qo->DrawCopy("same");

   c2->cd(1);
   h_fakepulse_qi->SetTitle("Fake QI Fit");
   h_fakepulse_qi->DrawCopy();
   h_fitpulse_qi->DrawCopy("same");
   
   c2->cd(2);
   h_fakepulse_qo->SetTitle("Fake QO Fit");
   h_fakepulse_qo->DrawCopy();
   h_fitpulse_qo->DrawCopy("same");

   return;

} //Done!

/////////////////////// Supporting Routines /////////////////////////////

void getPulse(std::vector<double>& pulsevec, TH1D* h_pulse, string channelName)
{
   //Temp! using the template pulse as the data
   TFile f("170319_1616_F0002_qpulsetrace_z17.root");
   TTree* pulseTree = (TTree*)f.Get("pulseTree");

   //Set Branch Addresses
   float pulse[nBins];
   pulseTree->SetBranchAddress("bsnPulse", pulse);
   if(channelName == "QI")
   {
      pulseTree->GetEntry(0);
   }
   else
   {
      pulseTree->GetEntry(1);
   }

   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_pulse->SetBinContent(binItr+1, pulse[binItr]);
      pulsevec.push_back((double)pulse[binItr]);
   }
   
   return;
}

void getNoise(std::vector<double>& noisevec, TH1D* h_noise, string channelName)
{
   //Temp! using the template pulse as the data
   TFile f("170319_1616_F0002_qnoisetrace_z17.root");
   TTree* pulseTree = (TTree*)f.Get("pulseTree");

   //Set Branch Addresses
   float pulse[nBins];
   pulseTree->SetBranchAddress("bsnPulse", pulse);
   if(channelName == "QI")
   {
      pulseTree->GetEntry(0);
   }
   else
   {
      pulseTree->GetEntry(1);
   }

   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_noise->SetBinContent(binItr+1, pulse[binItr]);
      noisevec.push_back((double)pulse[binItr]);
   }
   
   return;
}

//////////////////////////////////////////////////////////////////////////////////////////

void getPulseTemplate(std::vector<double>& pulsetempvec_qi, TH1D* h_pulsetemp_qi,
		      std::vector<double>& pulsetempvec_qo, TH1D* h_pulsetemp_qo,
		      std::vector<double>& pulsetempvec_qix, TH1D* h_pulsetemp_qix,
		      std::vector<double>& pulsetempvec_qox, TH1D* h_pulsetemp_qox)
{
   TFile f("../templates/NoiseAndTemplates_1703191616.root");
   TTree* pulsetempTree = (TTree*)f.Get("zip17");

   //Set Branch Addresses
   double pulsetemp_qi[nBins];
   double pulsetemp_qo[nBins];
   double pulsetemp_qix[nBins];
   double pulsetemp_qox[nBins];

   pulsetempTree->SetBranchAddress("QI_template_time", pulsetemp_qi);
   pulsetempTree->SetBranchAddress("QO_template_time", pulsetemp_qo);
   pulsetempTree->SetBranchAddress("QIX_template_time", pulsetemp_qix);
   pulsetempTree->SetBranchAddress("QOX_template_time", pulsetemp_qox);

   pulsetempTree->GetEntry(0);
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < nBins; binItr++)
   {
      h_pulsetemp_qi->SetBinContent(binItr+1, pulsetemp_qi[binItr]);
      h_pulsetemp_qo->SetBinContent(binItr+1, pulsetemp_qo[binItr]);
      h_pulsetemp_qix->SetBinContent(binItr+1, pulsetemp_qix[binItr]);
      h_pulsetemp_qox->SetBinContent(binItr+1, pulsetemp_qox[binItr]);

      pulsetempvec_qi.push_back(pulsetemp_qi[binItr]);
      pulsetempvec_qo.push_back(pulsetemp_qo[binItr]);
      pulsetempvec_qix.push_back(pulsetemp_qix[binItr]);
      pulsetempvec_qox.push_back(pulsetemp_qox[binItr]);
   }
   
   return;
}

void getNoiseTemplate(std::vector<double>& re_temp_conj_noise_vec, std::vector<double>& im_temp_conj_noise_vec, 
		      std::vector<double>& re_temp_fft_vec, std::vector<double>& im_temp_fft_vec,
		      std::vector<double>& noise_fftsq_vec,
		      double& norm_fft, TH1D* h_noise_fftsq, string channelName)
{
   TFile f("../templates/NoiseAndTemplates_1703191616.root");
   TTree* noisetempTree = (TTree*)f.Get("zip17");

   //Set Branch Addresses
   double re_temp_conj_noise[nBins];
   double im_temp_conj_noise[nBins];
   double re_temp_fft[nBins];
   double im_temp_fft[nBins];
   double noise_fftsq[nBins];

   noisetempTree->SetBranchAddress(Form("%s_template_conjnoise_fft_real", channelName.c_str()), re_temp_conj_noise);
   noisetempTree->SetBranchAddress(Form("%s_template_conjnoise_fft_img", channelName.c_str()), im_temp_conj_noise);
   noisetempTree->SetBranchAddress(Form("%s_template_fft_real", channelName.c_str()), re_temp_fft);
   noisetempTree->SetBranchAddress(Form("%s_template_fft_img", channelName.c_str()), im_temp_fft);
   noisetempTree->SetBranchAddress(Form("%s_noise_fftsq", channelName.c_str()), noise_fftsq);
   noisetempTree->SetBranchAddress(Form("%s_norm_fft", channelName.c_str()), &norm_fft);
   noisetempTree->GetEntry(0);
   
//   cout <<"norm fft = " << norm_fft << endl;

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

void getQInverse(double* qinverse)
{
  TFile f("../templates/NoiseAndTemplates_1703191616.root");
  TTree* noisetempTree = (TTree*)f.Get("zip17");
  
  //Set Branch Addresses
  noisetempTree->SetBranchAddress("qinverse", qinverse);
  noisetempTree->GetEntry(0);
  
  //Copy template into vector and histogram
  for(int binItr = 0; binItr < 4; binItr++)
    {
      cout <<"qinverse = " << qinverse[binItr] << endl;
 
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
      //positive shift 
      if(idelay >= 0)
      {
	 if(binItr<idelay) 
	    aTempHisto->SetBinContent(binItr+1, 0.0); 
	 else
	    aTempHisto->SetBinContent(binItr+1, h->GetBinContent(binItr-idelay+1));
      }
      //negative shift
      else
      {
	 if( (binItr-idelay) < h->GetNbinsX() )
	    aTempHisto->SetBinContent(binItr+1, h->GetBinContent(binItr-idelay+1));
	 else
	    aTempHisto->SetBinContent(binItr+1, 0.0);
      }

    }

  //copy contents of aTempHist into h
  for(int binItr = 0; binItr < h->GetNbinsX(); binItr++)
    {
      h->SetBinContent(binItr+1, aTempHisto->GetBinContent(binItr+1));
    }

  return;
}

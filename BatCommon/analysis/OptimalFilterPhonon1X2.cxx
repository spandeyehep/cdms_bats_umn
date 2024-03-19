/////////////////////////////////////////////////////////////////////////////////
//  Class Name:     OptimalFilterPhonon1X2
//  Author:         Carlos Eduardo Martinez Amaya
//  Description:    This class perfoms an optimal filtering using two phonon
//                  templates (template1=Ptemplate and template2=Signal-Ptemplate)
//                  on phonon pulses.
//                  (based on the OF_Qxtalk.m MATLAB code)
//
//
//  File Import By: Carlos Eduardo Martinez Amaya
//  Creation Date:  Aug. 10, 2012
//
//  Modifications:
//////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TTree.h"
#include "TComplex.h"
#include "TMatrixD.h"

#include "OptimalFilterPhonon1X2.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterPhonon1X2::OptimalFilterPhonon1X2() :
fDoDelayInterpolation(-999999),
fPamps(-999999.),
fPamps0(-999999.),
fInterPamps(-999999.),
fDiscretePamps(-999999.),
fRamps(-999999.),
fRamps0(-999999.),
fInterRamps(-999999.),
fDiscreteRamps(-999999.),
fChisq(-999999.),
fDiscreteChisq(-999999.),
fInterChisq(-999999.),
fDchisqLF(-999999.),
fDelay(-999999.),
fDiscreteDelay(-999999.),
fInterDelay(-999999.),
fdT(-999999.),
fPxwindow1(-999999),
fPxwindow2(-999999),
fTemplatesLoaded(false),
fNormalizationsLoaded(false),
fNBinsTemplates(0)
{
  //     cout <<"Hello from OptimalFilterPhonon1X2()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "OptimalFilterPhonon1X2"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

OptimalFilterPhonon1X2::~OptimalFilterPhonon1X2()
{
  //   cout <<"Goodbye from OptimalFilterPhonon1X2()" << endl;
}

////////////////////////////////////////////////////////
//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void OptimalFilterPhonon1X2::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("OF1X2Pamps", initVal));
   fRQList.insert(pair<string,double>("OF1X2Pamps0", initVal));
   fRQList.insert(pair<string,double>("OF1X2Ramps", initVal));
   fRQList.insert(pair<string,double>("OF1X2Ramps0", initVal));
   fRQList.insert(pair<string,double>("OF1X2chisq", initVal));
   fRQList.insert(pair<string,double>("OF1X2delay", initVal));
   fRQList.insert(pair<string,double>("OF1X2DPamps", initVal));
   fRQList.insert(pair<string,double>("OF1X2DRamps", initVal));
   fRQList.insert(pair<string,double>("OF1X2Dchisq", initVal));
   fRQList.insert(pair<string,double>("OF1X2Ddelay", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

void OptimalFilterPhonon1X2::LoadTemplates(const vector<double>& templateSlow, const vector<double>& templateFast, const vector<TComplex>& templateSlowFFT,const vector<TComplex>& templateFastFFT)
{
    fNBinsTemplates = templateSlow.size();
    double  inBins  = 1/(double)fNBinsTemplates;
    
    //FFT Normalizations
    double  dnu     = 1/(fdT*(double)fNBinsTemplates);
    double  sqrtdnu     = sqrt(dnu);

    
    if ((int)templateFast.size() != fNBinsTemplates)
    {
        cerr << "OptimalFilterPhonon1X2::LoadTemplates ERROR! Template lengths do not match, check the input to LoadTemplates" << endl;
        exit(1);
    }
 
    fPulseTemplate      = templateSlow;
    fResidualTemplate   = templateFast;
    fPulseTemplateFFT   = templateSlowFFT;
    fResidualTemplateFFT= templateFastFFT;


    for (int binItr = 0; binItr < fNBinsTemplates; binItr++)
    {
     

        fPulseTemplateFFT[binItr] *= sqrtdnu;//Because of extra factors when processing
        fResidualTemplateFFT[binItr] *= sqrt(inBins);//BatRoot <=> Matlab normalization
    }
    
    fTemplatesLoaded = true;
    
    return;
}

void OptimalFilterPhonon1X2::LoadNormalizations(const vector<double>& noiseFFTSq)
{
    //FFT Normalizations
    double  dnu     = 1/(fdT*(double)fNBinsTemplates);
    
    
    //check that vector lengths match
    if((int)noiseFFTSq.size() != fNBinsTemplates)
    {
        cerr << "OptimalFilterPhonon1X2::LoadNormalizations ERROR!  Template lengths do not match, check the input to LoadNormalizations." << endl;
        exit(1);
    }
    
    fNoiseFFTSq = noiseFFTSq;
    
    for (int binItr = 0; binItr < fNBinsTemplates; binItr++)
    {
        fNoiseFFTSq[binItr] *= dnu;//BatRoot <=> Matlab normalization


    }
    
    fNormalizationsLoaded = true;
    
    return;
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//This is the main call for your analysis
void OptimalFilterPhonon1X2::DoCalc(const vector<double>& aPulse)
{
    int     nBins   = aPulse.size();
    double  inBins  = 1/(double)nBins;

    //check for null pulses
    if(aPulse.size() == 0)
    {
        cerr << "OptimalFilterPhonon1X2::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
        << endl;
        exit(1);
    }
    
    if(!fTemplatesLoaded || !fNormalizationsLoaded)
    {
        cerr << "OptimalFilterPhonon1X2::DoCalc ERROR!  attempting to run OptimalFilter without loading templates + normalizations!" << endl;
        exit(1);
    }
    
    //checking that the lengths of templates agree with the pulse lengths
    if(nBins != fNBinsTemplates)
    {
        cerr<< "OptimalFilterPhonon1X2::DoCalc ERROR! Number of bins in pulse does not match number of bins in templates!" << endl;
        exit(1);
    }
    
    //checking that the fit window is valid
    if(fPxwindow1 < 0 || fPxwindow2 < 0)
    {
        cerr << "OptimalFilterPhonon1X2::DoCalc ERROR! Fit windows appear to be uninitialized" << endl;
        exit(1);
    }

    //Added to check if sample time is initialized correctly 
//checking that the fit window is valid
    if(fdT < 0)
    {
        cerr << "OptimalFilterPhonon1X2::DoCalc ERROR! Sample Time  appears to be uninitialized" << endl;
        exit(1);
    }
    
    //================== Calculations =====================
    
    vector<TComplex>            pulseFFT;         //It will store the pulse FFT
    vector<vector<TComplex> >   pulseFFTOF;       //It will the store the pFFT*OF (Pulse first, Residual second)
    vector<vector<TComplex> >   templatesFFT;     //It will store all the templates FFT (Pulse first, Residual second)
    vector<vector<TComplex> >   OptimalFilter;    //It will store th OF for all templates FFT (Pulse first, Residual second)
    vector<vector<double> >     p_pulseOF_iFFT;   //It will store the the inverse FFT for all pFFT*OF (Pulse first, Residual second)
    vector<double>              p_pulseOF_iFFTre; //It will store the the inverse FFT of pulseFFT*OF
    vector<double>              p_residOF_iFFTre; //It will store the the inverse FFT of residualFFT*OF
    vector<double>              p_pOF_iFFTtemp;   //Temporary container
    int                         nTemplates = 2;   //Number of templates used
    
    //Templates FFT
    templatesFFT.push_back(fPulseTemplateFFT);
    templatesFFT.push_back(fResidualTemplateFFT);
    
    //Pulse FFT
    PulseTools::RealToComplexFFT(aPulse,pulseFFT);
    
    for (int binItr = 0; binItr < nBins; binItr++)
    {
        pulseFFT[binItr] *= sqrt(inBins);//BatRoot <=> Matlab normalization

    }
    
    //Lets build the OF
    //In our case instead of using a 2X2 matrix we only use the diagonals since we only have one pulse...
    vector<TComplex>    oftemp;
    vector<TComplex>    tFFTtemp;
    TComplex            tFFTtempConjugate;
    
    
    
    for (int ntem = 0 ; ntem < nTemplates; ntem++)
    {
        tFFTtemp = templatesFFT[ntem];
        
        for (int binItr = 0; binItr < nBins; binItr++)
        {
            oftemp.push_back(tFFTtempConjugate.Conjugate(tFFTtemp[binItr])/fNoiseFFTSq[binItr]);  

        }
        
        OptimalFilter.push_back(oftemp);//Store the calculated OF
        oftemp.clear();//Clear the temporary vector to avoid mix-ups
        tFFTtemp.clear();//Clear the temporary vector to avoid mix-ups
    }

    

    //Lets build the Weight matrix
    TMatrixD            wMatrix(2,2);
    
    for (int nrow = 0; nrow < nTemplates; nrow++)
    {
      

        tFFTtemp = templatesFFT[nrow];
        
        for (int ncol = 0; ncol < nTemplates; ncol++)
        {
	 

            oftemp = OptimalFilter[ncol];
            double wItem = 0;
            
            for (int binItr = 1; binItr < nBins; binItr++)//Avoid the DC component
            {
	     

                wItem = wItem + tFFTtemp[binItr]*oftemp[binItr];
            }
            
            wMatrix[nrow][ncol] = wItem;
        }
    }
    
    //Now we need to invert it
    TMatrixD iwMatrix = wMatrix;
    
    if (iwMatrix.Determinant() != 0)//This matrix should always be invertible, but it never hurts to check.
    {
        iwMatrix.Invert();
    } else
    {
        cout <<"ERROR! OptimalFilterPhonon1X2::DoCalc"
        <<"\nMatrix is not invertable!  This should not happen in this algorithm, check code!"
        << endl;
        
	//NM: This will happen if a channel is railed.  We don't want to sacrifice all processing for it.
	//Just store error values instead. 
        //exit(1);

  
    	//Next, store the results of this calculation as the RQ's.
    	//These values will be included in the output of BatRoot.
    	if(fStoreRQs) {
        	fRQList["OF1X2Pamps"]       = -999999;
        	fRQList["OF1X2Pamps0"]      = -999999;
		fRQList["OF1X2Ramps"]       = -999999;
		fRQList["OF1X2Ramps0"]      = -999999;
		fRQList["OF1X2chisq"]       = -999999;
		fRQList["OF1X2delay"]       = -999999;
		fRQList["OF1X2DPamps"]      = -999999;
		fRQList["OF1X2DRamps"]      = -999999;
		fRQList["OF1X2Dchisq"]      = -999999;
		fRQList["OF1X2Ddelay"]      = -999999;	
    	}
    
    	// ========== cleanup! ===========
    	fTemplatesLoaded = false;
    	fNormalizationsLoaded = false;
   
    }
    


    //Apply the optimum filter
    vector<TComplex> pFFTOFtemp;
    TComplex comp_zero(0.,0.);
    for (int ntem = 0; ntem < nTemplates; ntem++)
    {
     

        oftemp = OptimalFilter[ntem];
        
        for (int binItr = 0; binItr < nBins; binItr++)
        {
	 

            pFFTOFtemp.push_back(pulseFFT[binItr]*oftemp[binItr]);
        }

	pFFTOFtemp[0]= comp_zero;
        
        //Invert FFT
        PulseTools::ComplexToRealIFFT(pFFTOFtemp, p_pOF_iFFTtemp);
        
        //Normalize BatRoor <=> Matlab
        for (int binItr = 0; binItr < nBins; binItr++)
        {
	 

            p_pOF_iFFTtemp[binItr] *= sqrt((double)nBins);
        }
        
        pulseFFTOF.push_back(pFFTOFtemp);
        p_pulseOF_iFFT.push_back(p_pOF_iFFTtemp);
        
        pFFTOFtemp.clear();//Clear the temporary vector to avoid mix-ups
        p_pOF_iFFTtemp.clear();//Clear the temporary vector to avoid mix-ups
    }
    
    p_pulseOF_iFFTre = p_pulseOF_iFFT[0];
    p_residOF_iFFTre = p_pulseOF_iFFT[1];
    
    //=========================================================================
    // Use Qinverse to get the amplitudes at all times. 
    // Remember that Qinverse encompasses the mix
    //=========================================================================
   
    
    vector<vector<double> > a_to;//First vector is for PulseTemplate, second is for ResidualTemplate
    vector<double>          aP_totemp;
    vector<double>          aR_totemp;
    vector<double>          a_0Del;//0 delay amplitudes
    
    //--------------------------------------------------------------------------
    //--------------Lets build the amplitudes vector (in t0 space)--------------
    //--------------------------------------------------------------------------
    
    double tempSolP = 0;
    double tempSolR = 0;

    
    
    for (int binItr = 0; binItr < nBins; binItr++)
    {
      

        int tempdelay = binItr;
        
        //First do the matrix-vector multiplication and store
        tempSolP = iwMatrix[0][0]*p_pulseOF_iFFTre[binItr]+iwMatrix[0][1]*p_residOF_iFFTre[binItr];
        tempSolR = iwMatrix[1][0]*p_pulseOF_iFFTre[binItr]+iwMatrix[1][1]*p_residOF_iFFTre[binItr];
        
        aP_totemp.push_back(tempSolP);
        aR_totemp.push_back(tempSolR);
        
        fDelayPAmpMap.insert(pair<int,double>(tempdelay,tempSolP));
        fDelayRAmpMap.insert(pair<int,double>(tempdelay,tempSolR));

	
        
        tempSolP = 0;
        tempSolR = 0;
    }
    //Then store the amplitudes in the general amplitude vector
    a_to.push_back(aP_totemp);
    a_to.push_back(aR_totemp);


    
    //---------------------------------------------------------
    //--------------Now is time to build the Chi2--------------
    //---------------------------------------------------------
    
    //FIRST: Calculate the Chi2 part that is independent of t0 for each channel independently and then add
    
    double                      chi2Base     = 0;//Base Chi2, it is independent of t0
    double                      chi2Basetemp = 0;
    
    
    for (int binItr = 1; binItr < nBins; binItr++)//Avoid the DC component
    {
      chi2Basetemp = chi2Basetemp + pulseFFT[binItr].Rho2()/(fNoiseFFTSq[binItr]);
    }
    //Store the total value
    chi2Base = chi2Base+chi2Basetemp;
    
    //SECOND: Calculate the part that depends on t0
    vector<double>              chi2to;
    vector<double>              a1_to_temp;//Temp pulse amplitudes vector
    vector<double>              a2_to_temp;//Temp residuals amplitudes vector
    

    a1_to_temp       = a_to[0];
    a2_to_temp       = a_to[1];
    
    for (int binItr = 0; binItr < nBins; binItr++)
    {
      chi2to.push_back(a1_to_temp[binItr]*p_pulseOF_iFFTre[binItr] + a2_to_temp[binItr]*p_residOF_iFFTre[binItr]);
    }
 
   

    //THIRD: Generate the Chi2(t0) vector
    for(int binItr = 0; binItr < nBins; binItr++)
      {      
        int     tempdelay = binItr;
        double  tempchisq = 0;
        
        tempchisq = chi2Base-chi2to[binItr];
        
        fDelayChisqMap.insert(pair<int, double>(tempdelay, tempchisq));

	
    }
    
    //FOURTH: Search around the window specified (No DC component)
    
    double          minChisq    = numeric_limits<double>::infinity();
    double          chisq       = 0.0;
    double          ap          = 0.0;
    double          ar          = 0.0;
    double          ap0del      = 0.0;
    double          ar0del      = 0.0;
    int             delay       = 0;
    
     

    for (int binItr = 0; binItr < nBins; binItr++) 
    {
        int     tempdelay = binItr;
        double  chiMinTemp = 0;
        double  pMinTemp = 0;
        double  rMinTemp = 0;

        chiMinTemp  = fDelayChisqMap.find(tempdelay)->second;
        pMinTemp    = fDelayPAmpMap.find(tempdelay)->second;
        rMinTemp    = fDelayRAmpMap.find(tempdelay)->second;

	
        
        if (chiMinTemp < minChisq)
        {
	              
            minChisq    =   chiMinTemp;
            delay       =   tempdelay;
            chisq       =   minChisq;
            ap0del      =   fDelayPAmpMap.find(0)->second;
            ar0del      =   fDelayRAmpMap.find(0)->second;
            
            ap          =   pMinTemp;
            ar          =   rMinTemp;

        }
        
        //Search up to fQxwindow1, and after fQxwindow2
        if(binItr == fPxwindow1 - 1)
        {
	  binItr = fPxwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
        }
    }
    
    //Interpolated values

    if(fDoDelayInterpolation == 1)
      CalcDelayInterpolation(delay);
   
   
   
    //Now that the minimization has been done we need to store the quantities obtained
    fPamps0 = ap0del;
    fRamps0 = ar0del;
    
    //store the interpolated values, if they exist

    // Discrete values
    //store the discrete values
    fDiscreteDelay = (delay < nBins/2 ? delay : (delay - nBins))*fdT;
    fDiscreteChisq = chisq;
    fDiscretePamps = ap;
    fDiscreteRamps = ar;
    
    if(fInterDelay != -999999) {
        fDelay = fInterDelay*fdT;
        fChisq = fInterChisq;
        fPamps = fInterPamps;
        fRamps = fInterRamps;
    } else {
      fDelay = fDiscreteDelay;
      fChisq = fDiscreteChisq;
      fPamps = fDiscretePamps;
      fRamps = fDiscreteRamps;
    }
    
    
    //Next, store the results of this calculation as the RQ's.
    //These values will be included in the output of BatRoot.
    if(fStoreRQs) {
        fRQList["OF1X2Pamps"]       = fPamps;
        fRQList["OF1X2Pamps0"]      = fPamps0;
	fRQList["OF1X2Ramps"]       = fRamps*sqrt(fdT);     // need to scale normalizations accordingly
	fRQList["OF1X2Ramps0"]      = fRamps0*sqrt(fdT);    //   with  BatRoot <=> Matlab normalization
	fRQList["OF1X2chisq"]       = fChisq;
	fRQList["OF1X2delay"]       = fDelay;
	fRQList["OF1X2DPamps"]      = fDiscretePamps;
	fRQList["OF1X2DRamps"]      = fDiscreteRamps*sqrt(fdT);
	fRQList["OF1X2Dchisq"]      = fDiscreteChisq;
	fRQList["OF1X2Ddelay"]      = fDiscreteDelay;	
    }
    
    // ========== cleanup! ===========
    fTemplatesLoaded = false;
    fNormalizationsLoaded = false;
    
    return;
}




void OptimalFilterPhonon1X2::CalcDelayInterpolation(const int delay)
{    
    
    // Get the values corresponding to input "delay" and +/- 1 bins
    map<int, double>::const_iterator minIter = fDelayChisqMap.find((const int)delay); 
    map<int, double>::const_iterator lowIter = minIter;  
    map<int, double>::const_iterator highIter = minIter; 
    
    if(minIter == fDelayChisqMap.begin()) {
      lowIter = (--fDelayChisqMap.end()); 
      ++highIter; 
    } else if (minIter == (--fDelayChisqMap.end())) {
      --lowIter;
      highIter = fDelayChisqMap.begin();
    } else {
      --lowIter;
      ++highIter; 
    }
    
    
    // If the windowing constrained the system then there is a chance that
    //  the middle value is not the smallest value of the three!
    //    -> no interpolation
    if(!((*lowIter).second > (*minIter).second && 
	 (*highIter).second > (*minIter).second))  
      return;
    
    
    
    // To solve parabola equation, we define matrix using basis [-1 0 1] 
    // for delay
    TMatrixD xInverse(3,3);
    
    xInverse[0][0] = 0.5;
    xInverse[0][1] = -1.0;
    xInverse[0][2] = 0.5;
    
    xInverse[1][0] = -0.5;
    xInverse[1][1] = 0.0;
    xInverse[1][2] = 0.5;
    
    xInverse[2][0] = 0.0;
    xInverse[2][1] = 1.0;
    xInverse[2][2] = 0.0;
    
    
    
    //Solve for parabolic fit with inverse of the x value matrix
    
    double xmin = minIter->first;
    double ylow = lowIter->second;
    double ymin = minIter->second;
    double yhigh = highIter->second;
    
    double a = xInverse[0][0]*ylow + xInverse[0][1]*ymin + xInverse[0][2]*yhigh;
    double b = xInverse[1][0]*ylow + xInverse[1][1]*ymin + xInverse[1][2]*yhigh;
    double c = xInverse[2][0]*ylow + xInverse[2][1]*ymin + xInverse[2][2]*yhigh;
    
    //Store the interpolated values
    double InterpDelayInd = -b/(2*a);
    double InterpDelay = xmin + InterpDelayInd; //translate to find the actual delay
    fInterChisq = c - b*b/(4*a);
    fInterDelay = (InterpDelay < fNBinsTemplates/2 ? InterpDelay : (InterpDelay - fNBinsTemplates));
    
    
    
    // Amplitudes interpolation
    
    
    // SLOW pulse 
    
    
    // Get amp for -1,+0,+1 delay
    map<int, double>::const_iterator minIterPAmp = fDelayPAmpMap.find((const int)delay); 
    map<int, double>::const_iterator lowIterPAmp = minIterPAmp;  
    map<int, double>::const_iterator highIterPAmp = minIterPAmp; 
    
    if(minIterPAmp == fDelayPAmpMap.begin()) {
      lowIterPAmp = (--fDelayPAmpMap.end());
      ++highIterPAmp; 
    } else if (minIterPAmp == (--fDelayPAmpMap.end())) {
      --lowIterPAmp;
      highIterPAmp = fDelayPAmpMap.begin();
    } else {
      --lowIterPAmp;
      ++highIterPAmp; 
    }
    
    double ylowPAmp = lowIterPAmp->second;
    double yminPAmp = minIterPAmp->second;
    double yhighPAmp = highIterPAmp->second;
    
    
  
    //Solve for parabolic fit with inverse of the x value matrix
    double aPAmp = xInverse[0][0]*ylowPAmp + xInverse[0][1]*yminPAmp + xInverse[0][2]*yhighPAmp;
    double bPAmp = xInverse[1][0]*ylowPAmp + xInverse[1][1]*yminPAmp + xInverse[1][2]*yhighPAmp;
    double cPAmp = xInverse[2][0]*ylowPAmp + xInverse[2][1]*yminPAmp + xInverse[2][2]*yhighPAmp;
    
    
    //Store interpolated amplitudes
    fInterPamps = InterpDelayInd*InterpDelayInd*aPAmp+InterpDelayInd*bPAmp+cPAmp;
    
    
    
    
    // FAST pulse 
    
    
    // Get amp for -1,+0,+1 delay
    map<int, double>::const_iterator minIterRAmp = fDelayRAmpMap.find((const int)delay); 
    map<int, double>::const_iterator lowIterRAmp = minIterRAmp;  
    map<int, double>::const_iterator highIterRAmp = minIterRAmp; 
    
    if(minIterRAmp == fDelayRAmpMap.begin()) {
      lowIterRAmp = (--fDelayRAmpMap.end());
      ++highIterRAmp; 
    } else if (minIterRAmp == (--fDelayRAmpMap.end())) {
      --lowIterRAmp;
    highIterRAmp = fDelayRAmpMap.begin();
    } else {
      --lowIterRAmp;
      ++highIterRAmp; 
    }
    
    double ylowRAmp = lowIterRAmp->second;
    double yminRAmp = minIterRAmp->second;
    double yhighRAmp = highIterRAmp->second;
    
  
  
    //Solve for parabolic fit with inverse of the x value matrix
    double aRAmp = xInverse[0][0]*ylowRAmp + xInverse[0][1]*yminRAmp + xInverse[0][2]*yhighRAmp;
    double bRAmp = xInverse[1][0]*ylowRAmp + xInverse[1][1]*yminRAmp + xInverse[1][2]*yhighRAmp;
    double cRAmp = xInverse[2][0]*ylowRAmp + xInverse[2][1]*yminRAmp + xInverse[2][2]*yhighRAmp;
    
    
    //Store interpolated amplitudes
    fInterRamps = InterpDelayInd*InterpDelayInd*aRAmp+InterpDelayInd*bRAmp+cRAmp;
    
    
    return;
}

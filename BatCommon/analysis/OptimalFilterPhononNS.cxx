#include <iostream>

#include "OptimalFilterPhononNS.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
OptimalFilterPhononNS::OptimalFilterPhononNS() :
  fAmp(-999999.),
  fAmp0(-999999.),
  fChisq(-999999.),
  fDelay(-999999.),
  fAmpBig(-999999.),
  fChisqBig(-999999.),
  fDelayBig(-999999.),
  fdT(0.8e-6),
  fwindow1(-999999),
  fwindow2(-999999),
  fOFAmpsP(0),
  fOFDelayP(0),
  ffilterThresh(0.0),
  fchihalfwidth(-999999),
  fnscutoff(0),
  fSizeCOVpt(0),
  fSizeCOVbase(0),
  fNBinsTemplates(0),
  fVerbose(0),
  fVerboseN1(0),
  fVerboseN2(20),
  fDoDnCastCalc(false),
  fDoInverseCalc(false),
  fDoNSFilter(true),
  fTemplatesLoaded(false),
  fNormalizationsLoaded(false),
  fOFParamsLoaded(false),
  fThreshParamsLoaded(false),
  fIsRandom(false),
  fPrintRandom(true),
  fCalcSTDOF(false),
  fDoMaxAmp(true),
  fNormFFT(1.0),
  fSigToNoiseSq(1.0)
{
   //   cout <<"Hello from OptimalFilterPhononNS()" << endl;

   //for diagnostic purposes ONLY this should be commented OUT
   //unless you want only standard OF quantities calculated with the NF method [ANV]
   //SetCalcSTDOF(true);  

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "OptimalFilterPhononNS"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

OptimalFilterPhononNS::~OptimalFilterPhononNS()
{
//   cout <<"Goodbye from OptimalFilterPhononNS()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void OptimalFilterPhononNS::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("NFamps", initVal));
   fRQList.insert(pair<string,double>("NFamps0", initVal));
   fRQList.insert(pair<string,double>("NFchisq", initVal));
   fRQList.insert(pair<string,double>("NFdelay", initVal));
   //Lauren requests not to write out NFbig quantites because they are confusing [ANV]
   //fRQList.insert(pair<string,double>("NFbigamps", initVal));
   //fRQList.insert(pair<string,double>("NFbigchisq", initVal));
   //fRQList.insert(pair<string,double>("NFbigdelay", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//optional function, do it here and not in the constructor
// void OptimalFilterPhononNS::InitializeParameters()
// {
//
//    return;
// }

//This is the main call for your analysis
void OptimalFilterPhononNS::DoCalc(const vector<double>& aPulse)
{

  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"OptimalFilterPhononNS::DoCalc: ERROR!  Pulse passed into DoCalc(const vector<double>& aPulse) has length 0, please check that pulse is valid." << endl;
    exit(1);
  }

   //do your calculation here!

   int nBins = aPulse.size();
   if(fVerbose>0){
     cout << endl;
     cout << endl;
     cout << "OptimalFilterPhononNS::DoCalc: Pulse size is " << nBins << " bins " << endl;
   }
   //double sqrtdT = sqrt(fdT); //for efficiency

   //============== Some Preliminary Checks =================

   if(!fTemplatesLoaded) 
   {
     cerr <<"OptimalFilterPhononNS::DoCalc: ERROR!  attempting to run OptimalFilter without loading templates!" << endl;
     exit(1);
   }

   //check if normalizations loaded 
   if(!fNormalizationsLoaded) 
   { 
      cerr<<"OptimalFilterPhononNS::DoCalc: ERROR! normalizations not loaded properly!" << endl; 
      exit(1);
   }

   //check if OFparams loaded 
   if(!fOFParamsLoaded) 
   { 
      cerr<<"OptimalFilterPhononNS::DoCalc: ERROR! have not loaded standard OF parameters!" << endl; 
      exit(1);
   }

   //check if OFparams loaded 
   if(!fThreshParamsLoaded) 
   { 
      cerr<<"OptimalFilterPhononNS::DoCalc: ERROR! have not loaded threshold parameters!" << endl; 
      exit(1);
   }

   //checking that the lengths of templates agree with the pulse lengths
   if(nBins != fNBinsTemplates) 
   { 
      cerr<<"OptimalFilterPhononNS::DoCalc: ERROR! Number of bins in pulse does not match number of bins in templates!" << endl; 
      exit(1);
   }

   //checking that the fit window is valid
   if(fchihalfwidth < 0 ) 
   {
     cerr <<"OptimalFilterPhononNS::DoCalc: ERROR! Fit window appears to be uninitialized!" << endl; 
     exit(1);
   }

   //checking that the maxamp fit window is valid
   if(fwindow1 < 0 || fwindow2 < 0) 
   {
     cerr <<"OptimalFilterPhononNS::DoCalc: WARNING! Fit window appears to be uninitialized!  Max amplitude values will not be computed." << endl; 
     fDoMaxAmp = false;
   }

   if(fDoNSFilter){
     //is it too slick to do it in the conditional?
     //seems more transparent now that I said that
     if(!DoNSOF(aPulse,fDoInverseCalc,fDoDnCastCalc)){
        cerr <<"OptimalFilterPhononNS::DoCalc: ERROR! Optimal filter calculation failed!" << endl;
	exit(1);
     }
   }
   else{
     //eh, nothing really, since OFparams were loaded we should have the correct values
     //of fAmp and fDelay (inherited directly from the OF)
   }
   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      if(fVerbose>0){
        cout << "---------------------------------WRITE VALS-------------------------------" << endl;
        cout << "Writing NSOF: " << "fAmp = " << fAmp << " fAmp0 = " << fAmp0 << " fChisq = " << fChisq << " fDelay = " << fDelay << endl;
        cout << "Writing NSOF big: " << "fAmpBig = " << fAmpBig << " fAmp0 = " << fAmp0 << " fChisqBig = " << fChisqBig << " fDelayBig = " << fDelayBig << endl;
        cout << "--------------------------------------------------------------------------" << endl;
      }
      fRQList["NFamps"] = fAmp;
      fRQList["NFamps0"] = fAmp0;
      fRQList["NFchisq"] = fChisq;
      fRQList["NFdelay"] = fDelay;
      //Lauren requests not to write out NFbig quantites because they are confusing [ANV]
      //fRQList["NFbigamps"] = fAmpBig;
      //fRQList["NFbigchisq"] = fChisqBig;
      //fRQList["NFbigdelay"] = fDelayBig;
   }

   //============= Delete Templates to minimize copying  ================

   fPulseTemplateFFT.clear();
   fCOVpd.clear();
   fCOVbase.clear();
   
  
   //these are pretty informative, could store if necessary
   fOptimalFilter.clear();
   fChisquare.clear();

   // ========== cleanup! ===========
   fTemplatesLoaded = false;

   return;
}
void OptimalFilterPhononNS::LoadTemplates(const vector<TComplex>& pulseTemplateFFT)
{
  //copy the templates over
  fPulseTemplateFFT = pulseTemplateFFT;

  fNBinsTemplates = fPulseTemplateFFT.size();

  fTemplatesLoaded = true;

  return;
}
void OptimalFilterPhononNS::LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const SprseMatrix& COVpd, const SprseMatrix& COVbase)
{
  //copy the noise over
  fCOVpd = COVpd;
  fCOVbase = COVbase;

  //templates and matrix dimensions should be same (unless downcast but downcasting not implemented yet)
  //migrate to the commented lines, when overload + += - -= in SprseMatrix
  //for now use BLAS internal versions
  //fSizeCOVpt   = (int)fCOVpd.GetNrow();
  //fSizeCOVbase = (int)fCOVbase.GetNrow();
  fSizeCOVpt   = (int)fCOVpd.size1();
  fSizeCOVbase = (int)fCOVbase.size1();

  if((fNBinsTemplates != fSizeCOVpt) || (fNBinsTemplates != fSizeCOVbase))
  {
     cerr <<"OptimalFilterPhononNS::LoadTemplates(vector<TComplex>,SprseMatrix,SprseMatrix) Template lengths do not match, check the input to LoadTemplates." << endl;
     cerr <<"Template size: " << fNBinsTemplates << "  " << "COVpt row size: " << fSizeCOVpt << "  " << "COVbase row size: " << fSizeCOVbase << endl;
     exit(1);
  }

  fNormFFT = normFFT;
  fSigToNoiseSq = sigToNoiseSq;


  fNormalizationsLoaded=true;
  
  return;
}
void OptimalFilterPhononNS::LoadThresholds(const double& filterThresh, const int& nsofcutoff)
{
  //set the threshold and nscutoff
  //threshold value is in terms of ordinary OF amplitude
  ffilterThresh = filterThresh;
  fnscutoff = nsofcutoff;


  fThreshParamsLoaded = true;

  return;
}
void OptimalFilterPhononNS::LoadOFParams(const double& OFAmpsP, const double& OFDelayP)
{
  //set the OF parameters
  fOFAmpsP = OFAmpsP;
  fOFDelayP = OFDelayP;
  //check the OF threshold value if too low, just return OF parameters
  if(((OFAmpsP<=ffilterThresh) && !fIsRandom) && fThreshParamsLoaded){
    fDoNSFilter = false;
    //set amplitudes to OF values
    fAmp = OFAmpsP;
    fDelay = OFDelayP;
    //make chisquare and Amp0 flag values
  }

  fOFParamsLoaded = true;

  return;
}
bool OptimalFilterPhononNS::DoNSOF(const vector<double> &aPulse, bool &DoInverseCalc,bool &DoDnCastCalc) 
{

  //FIXME the implementation now only does the case where:
  //DoInverseCalc = false
  //DoDnCastCalc = false
  //irrespective of what these inputs actually are
  //implement this further but the DoInverseCalc = true
  //AND DoDnCastCalc = false should be EXPLICITLY DISALLOWED [ANV]

  //================== Calculations =====================
  double sqrtdT = sqrt(fdT); //for efficiency
  TComplex comp_zero(0.,0.);

  //vectors to hold intermediate calculations
  //names close to Matlab implementation
  double norm;
  vector<TComplex> pProd;
  vector<double> pAhat;
  vector<TComplex> pulseFFT;
  //vector<double> p_prod_ifftRe;

  //double parameters for calculation (special flag for diagnostic)
  double finalAmp=-999998.,amp0=-999998.,chisq=-999998.,delay=-999998.;
  double finalAmpbig=-999998.,chisqbig=-999998.,delaybig=-999998.;

  //get number of bins
  int nBins = aPulse.size();
  double fftscale = sqrt((double)nBins);

  //===== 1. construct fitpulse fft FIXME (generalize for downcasting) =====
  PulseTools::RealToComplexFFT(aPulse, pulseFFT);
  pulseFFT = PulseTools::pulseScale(pulseFFT,TComplex(sqrtdT,0));

  //FIXME slow, account for this in normalization
  //pulseFFT = PulseTools::pulseScale(pulseFFT,TComplex(sqrt((double)nBins),0));
  //pulseFFT = PulseTools::pulseScale(pulseFFT,TComplex(1.0/(double)nBins,0));
  //second level verbosity
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing pulseFFT: " << endl;
    for(int i=fVerboseN1; i<min((int)pulseFFT.size(),fVerboseN2); i++){
      cout << "[" << i << "](" << pulseFFT[i].Re() << "," << pulseFFT[i].Im() << ")   ";
    }
    cout << endl;
  }




  //===== 2. construct the full covariance matrix FIXME (generalize for downcasting) =====
  //FIXME dnu can also be a normalization at the end
  double dnu = 1/(fdT*(double)nBins);
  SprseMatrix covTotalSparse; 
  //covTotalSparse = (fCOVbase+fOFAmpsP*fOFAmpsP*fCOVpd)*dnu;
  //switch between a more time consuming version of standard OF calculation
  //and the full NSOF calculation, for diagnostic purposes fCalcSTDOF can
  //only be set by a private member method. 
  if(!fCalcSTDOF)
    covTotalSparse = (fCOVbase+fOFAmpsP*fOFAmpsP*fCOVpd);
  else
    covTotalSparse = (fCOVbase);

  if(fVerbose>1){
     cout << "OptimalFilterPhononNS::DoNSOF():  Printing (OFAmpsP,dnu): " <<endl;
     cout << "(" << fOFAmpsP << "," << dnu << ")" << endl;
     //diagnostic printing
     cout << "OptimalFilterPhononNS::DoNSOF():  Printing covTotalSparse: " << endl;
     for(int i=fVerboseN1;i<min(covTotalSparse.size1(),fVerboseN2);i++){
       for(int j=fVerboseN1;j<min(covTotalSparse.size2(),fVerboseN2);j++){
         complex<double> v = covTotalSparse(i,j);
         cout << "[" << i << "," << j << "]" << v << "  ";
       }
       cout << endl;
     }
     cout << "OptimalFilterPhononNS::DoNSOF():  Printing fCOVbase: " << endl;
     for(int i=fVerboseN1;i<min(fCOVbase.size1(),fVerboseN2);i++){
       for(int j=fVerboseN1;j<min(fCOVbase.size2(),fVerboseN2);j++){
         complex<double> v = fCOVbase(i,j);
         cout << "[" << i << "," << j << "]" << v << "  ";
       }
       cout << endl;
     }
     cout << "OptimalFilterPhononNS::DoNSOF():  Printing fCOVpd: " << endl;
     for(int i=fVerboseN1;i<min(fCOVpd.size1(),fVerboseN2);i++){
       for(int j=fVerboseN1;j<min(fCOVpd.size2(),fVerboseN2);j++){
         complex<double> v = fCOVpd(i,j);
         cout << "[" << i << "," << j << "]" << v << "  ";
       }
       cout << endl;
     }
  }


  
  //===== 3. solve linear system (or invert) FIXME (generalize for downcasting and inversion) =====
  //FIXME conversions need not be done if we have objects implemented well --> see BatMath directory [ANV]
  //if following OFamps normalization need this factor which normally goes into fOptimalFilter
  blasVec ptemplateFFT_blasVec = convVecTComplexToSTL(fPulseTemplateFFT);
  blasVec filterFFT_blasVec(ptemplateFFT_blasVec.size());
  if(fVerbose>0){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing ptemplateFFT_blasVec size is: " << ptemplateFFT_blasVec.size() << " bins " << endl;
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing filterFFT_blasVec size is: " << filterFFT_blasVec.size() << " bins " << endl;
  }
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing ptemplateFFT_blasVec: " << endl;
    for(int i=fVerboseN1; i<min((int)ptemplateFFT_blasVec.size(),fVerboseN2); i++){
      complex<double> z = ptemplateFFT_blasVec(i);
      cout << "[" << i << "]" << z <<  "   ";
    }
    cout << endl;
  }
  //solve FIXME the static_casts should go so that eventually NO MENTION of the blas classes needs
  //to be made (i.e. our wrapper classes are enough) need some C++ knowledge for this
  //basically how get external function to recognize an object which derives from the type it needs
  //I think we'd just have to write wrappers for these too in something like a LinAlgebraTools namespace
  umf::control_type<blasSprseMat::value_type > cControl;
  umf::info_type<blasSprseMat::value_type > cInfo;
  umf::symbolic_type<std::complex<double> > cSymbolic;
  umf::numeric_type<std::complex<double> > cNumeric;
  umf::symbolic(static_cast<blasSprseMat>(covTotalSparse), cSymbolic); 
  umf::numeric(static_cast<blasSprseMat>(covTotalSparse), cSymbolic, cNumeric, cControl, cInfo); 
  umf::solve(static_cast<blasSprseMat>(covTotalSparse), filterFFT_blasVec, ptemplateFFT_blasVec, cNumeric);
  if(cInfo.ptr[UMFPACK_STATUS]!=0){
     cerr <<"OptimalFilterPhononNS::DoNSOF(): ERROR! umf::solve returned non-zero exit status!" << endl; 

     if(cInfo.ptr[UMFPACK_STATUS]==UMFPACK_WARNING_singular_matrix)
         cerr <<"OptimalFilterPhononNS::DoNSOF(): ERROR! umf::solve detected singular matrix!" << endl;
     else 
         cerr <<"OptimalFilterPhononNS::DoNSOF(): ERROR! umf::solve detected unspecified error with UMF_STATUS = "<< cInfo.ptr[UMFPACK_STATUS] << " !" << endl;
     exit(1);
  }
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing filterFFT_blasVec: " << endl;
    for(int i=fVerboseN1; i<min((int)filterFFT_blasVec.size(),fVerboseN2); i++){
      complex<double> z = filterFFT_blasVec(i);
      cout << "[" << i << "]" << z <<  "   ";
    }
    cout << endl;
  }
  


  //===== 4. finish out the filter  FIXME (generalize for downcasting) =====
  //FIXME conversions need not be done if we have objects implemented well --> see BatMath directory [ANV]
  //might as well save the optimal filter, though it's pretty useless
  fOptimalFilter = convSTLToVecTComplex(filterFFT_blasVec);
  fOptimalFilter = conj(fOptimalFilter);
  pProd = PulseTools::pulseNSMult(pulseFFT,fOptimalFilter);
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing pProd: " << endl;
    for(int i=fVerboseN1; i<min((int)pProd.size(),fVerboseN2); i++){
      cout << "[" << i << "](" << pProd[i].Re() << "," << pProd[i].Im() << ")   ";
    }
    cout << endl;
  }


  //=======5. get amp0========================
  //FIXME check that this is analagous to the standardOF Amp0
  for(int binItr=0; binItr < nBins; binItr++){
    if(binItr != 0) amp0 += pProd[binItr].Re(); 
    else amp0=0.0;
  }

  amp0 /= fSigToNoiseSq;

  //remove DC component
  pProd[0] = comp_zero;

  //=======6. get other amplitudes========================
  //remove DC component
  fOptimalFilter[0] = comp_zero;
  norm = PulseTools::pulseDot(fOptimalFilter,fPulseTemplateFFT).Re();
  //FIXME  make a normalization at the end
  //pProd = PulseTools::pulseScale(pProd,TComplex(1/sqrt((double)nBins),0.0));
  PulseTools::ComplexToRealIFFT(pProd,pAhat);
  //FIXME make a normalization at the end
  //pAhat = PulseTools::pulseScale(pAhat,(double)nBins/norm);
  //need to normalize by norm for chisquare calculation
  pAhat = PulseTools::pulseScale(pAhat,fftscale/norm);
  if(fVerbose>1){
     cout << "OptimalFilterPhononNS::DoNSOF():  Printing (norm): " <<endl;
     cout << "(" << norm <<  ")" << endl;
  }
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing pAhat: " << endl;
    for(int i=fVerboseN1; i<min((int)pAhat.size(),fVerboseN2); i++){
      cout << "[" << i << "](" << pAhat[i] << "," << 0.0 << ")   ";
    }
    cout << endl;
  }


  //NOTE this is where the bulk of the time is spent I think [ANV]
  //Originally thought linear system solving was best but because of this
  //the inversion routine might be better (have the inverse to calculate more results) [ANV]
  //but you should NEVER try to invert a 8192x8192 matrix with these libraries so that's
  //where "downcasting" comes in. [ANV]
  //===== 7. get all the chisquares  FIXME (generalize for downcasting and inversion) =====
  vector<int> tdel_win;
  for(int j=0;j<(fchihalfwidth*2)+1;j++)
    tdel_win.push_back(-fchihalfwidth+j+(int)(fOFDelayP/fdT));

  fChisquare = getChiSquare(covTotalSparse,convVecTComplexToSTL(pulseFFT),ptemplateFFT_blasVec,tdel_win,pAhat,norm);

  int minbinchi;

  findMin(fChisquare,chisq,minbinchi);
  if(fVerbose>1){
    int length = fVerboseN2 - fVerboseN1;
    int modlength = min(length,(int)fChisquare.size());
    vector<int> index;
    for(int i=0;i<modlength;i++){
      int ind = (minbinchi-modlength/2+i);
      ind = (ind>=0 ? ind : (fChisquare.size() + ind));
      index.push_back(ind);
    }
    cout << "OptimalFilterPhononNS::DoNSOF():  Printing fChisquare: " << endl;
    for(int i=0; i<(int)index.size(); i++){
      if(index[i] != minbinchi)
        cout << "[" << index[i] << "](" << fChisquare[index[i]] << "," << 0.0 << ")   ";
      else
        cout << "[*" << index[i] << "*](" << fChisquare[index[i]] << "," << 0.0 << ")   ";
    }
    cout << endl;
  }


  finalAmp=pAhat[tdel_win[minbinchi]];
  tdel_win.clear();
  for(int j=0;j<(fchihalfwidth*2)+1;j++)
    tdel_win.push_back(-fchihalfwidth+j+(int)(fOFDelayP/fdT));
     
  delay=(double)tdel_win[minbinchi]*fdT;



  //===== 8. the "big" values not min chisquare but max amp =====
  if(fDoMaxAmp){
    int idelaybig=0;
    double maxAmp = -1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
    //pick out maximum amplitude and corresponding delay     
    for(int binItr=0; binItr < nBins; binItr++) {
     
     double amp =  pAhat[binItr];

     if(amp > maxAmp) { 
         maxAmp = amp; 
         finalAmpbig = pAhat[binItr];  
         idelaybig = binItr; }

      //search up to fwindow1, and after fwindow2
       if(binItr == fwindow1 - 1)  
          binItr = fwindow2 - 2; //-2 b/c for loop increments by 1 before next round, and c++ array convention adds -1
    }      

    //compute phase factor with this delay
    int idelaybig2 = (idelaybig < nBins/2 ? idelaybig : (idelaybig - nBins)); 
    delaybig = idelaybig2*fdT;
  
    //find index in pAhat of fOFDelayP which the chisquare vector is indexed about
    int iOFdel = ((int)(fOFDelayP/fdT) >= 0 ? (int)(fOFDelayP/fdT) : (nBins - (int)(fOFDelayP/fdT)));

    //index into the chisquare vector, this get's dicy (i.e. you probably won't understand this right away) [ANV]
    //get index of middle of chisquare range always has odd number by construction
    int mid = (fChisquare.size()/2); 
    //find index in fChisquare of idelaybig (account for wrapping)
    int iInChisquare;
    if(abs(idelaybig-iOFdel)<nBins/2){
      //has not wrapped
      iInChisquare = mid - (iOFdel-idelaybig);
    }
    else{
      //has wrapped
      iInChisquare = mid - (idelaybig-iOFdel-nBins/2);
    }
    //only set if it makes sense, i.e. in the window don't go computing more chisquareds
    if(iInChisquare>=0 && iInChisquare<(int)fChisquare.size()){
      chisqbig = fChisquare[iInChisquare];
    }
  }
  

  if(fVerbose>0){
    cout << "-----------------------------------------RESULT---------------------------" << endl;
    cout << "Results NSOF: " << "fAmp = " << finalAmp << " fAmp0 = " << amp0 << " fChisq = " << chisq << " fDelay = " << delay << endl;
    cout << "Results NSOF big: " << "fAmpBig = " << finalAmpbig << " fAmp0 = " << amp0 << " fChisqBig = " << chisqbig << " fDelayBig = " << delaybig << endl;
    cout << "--------------------------------------------------------------------------" << endl;
  }
  //===== 9. output values =====
  fAmp = finalAmp;
  fAmp0 = amp0; 
  fChisq = chisq;
  fDelay = delay;
  fAmpBig = finalAmpbig;
  fChisqBig = chisqbig;
  fDelayBig = delaybig;
  
  
  return true;
}
blasVec OptimalFilterPhononNS::convVecTComplexToSTL(vector<TComplex> &vec)
{
  blasVec out(vec.size());
  for(int i=0;i<(int)vec.size();i++){
    out[i]=complex<double>(vec[i].Re(),vec[i].Im());
  }

  return out;
}
vector<TComplex> OptimalFilterPhononNS::convSTLToVecTComplex(blasVec &vec)
{
  vector<TComplex> out;

  for(int i=0;i<(int)vec.size();i++){
    TComplex z=TComplex(std::real(vec[i]),std::imag(vec[i]));
    out.push_back(z);
  }

  return out;
}
vector<TComplex> OptimalFilterPhononNS::conj(vector<TComplex>& vec)
{
  vector<TComplex> out;
  TComplex temp;
  for(int i=0;i<(int)vec.size();i++){
    temp=vec[i];
    out.push_back(TComplex::Conjugate(temp));
  }

  return out;
}
blasVec OptimalFilterPhononNS::conj(blasVec& vec)
{
  blasVec out(vec.size());
  complex<double> temp;
  for(int i=0;i<(int)vec.size();i++){
    temp=vec(i);
    out(i)=(std::conj(temp));
  }

  return out;
}
blasVec OptimalFilterPhononNS::pulseNSMultBlas(const blasVec &veca,const blasVec &vecb) 
{
  blasVec out(veca.size());
  if(veca.size() != vecb.size())
    return out;
  for(int i=0;i<(int)veca.size();i++)
    out(i)=(veca(i)*vecb(i));

  return out;
}
void OptimalFilterPhononNS::findMax(vector<double> &list,double &max,double &time,TH1D *p)
{
  //max=0.0;
  //get max bin for template
  int maxbin = p->GetMaximumBin();
  max = -1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
  time=0;
  int itime=0;

  for(int i=0;i<(int)list.size();i++){
    if(list[i]>max){
      max=list[i];
      itime=i;
    }
  }

  //use number of bins in template, it has to be same as pulse
  if(itime>(fNBinsTemplates-maxbin))
    itime=(itime-fNBinsTemplates);

  //0.8 mu sec per bin, this should be generalized
  time=(double)(itime)*fdT;

  return;
}
void OptimalFilterPhononNS::findMin(vector<double> &list,double &min,int &itime)
{
  //max=0.0;
  //get max bin for template
  min = +1*numeric_limits<double>::infinity(); //maxAmp can be a negative number
  itime=0;

  for(int i=0;i<(int)list.size();i++){
    if(list[i]<min){
      min=list[i];
      itime=i;
    }
  }

  itime=itime;
  return;
}
vector<double> OptimalFilterPhononNS::getChiSquare(SprseMatrix &covTotalSparse,const blasVec &pulsefft,const blasVec &templatefft,vector<int> &tdel_win,vector<double> &amps ,double &norm)
{
  TComplex comp_zero(0.,0.);
  int n=tdel_win.size();
  int nsampP = pulsefft.size();
  vector<double> chisquare(n,0.0);
  if(fVerbose>1){
     //diagnostic printing
     cout << "OptimalFilterPhononNS::getChiSquare():  Printing covTotalSparse: " << endl;
     for(int i=fVerboseN1;i<min(covTotalSparse.size1(),fVerboseN2);i++){
       for(int j=fVerboseN1;j<min(covTotalSparse.size2(),fVerboseN2);j++){
         complex<double> v = covTotalSparse(i,j);
         cout << "[" << i << "," << j << "]" << v << "  ";
       }
       cout << endl;
     }
  }
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::getChiSquare():  Printing pulsefft: " << endl;
    for(int i=fVerboseN1; i<min((int)pulsefft.size(),fVerboseN2); i++){
      complex<double> z = pulsefft(i);
      cout << "[" << i << "]" << z <<  "   ";
    }
    cout << endl;
  }
  if(fVerbose>1){
    cout << "OptimalFilterPhononNS::getChiSquare():  Printing templatefft: " << endl;
    for(int i=fVerboseN1; i<min((int)templatefft.size(),fVerboseN2); i++){
      complex<double> z = templatefft(i);
      cout << "[" << i << "]" << z <<  "   ";
    }
    cout << endl;
  }

  //construct the omega vector
  blasVec omega(nsampP);
  double pimath = 3.14159265358979323846264338;
  for(int i=0;i<nsampP;i++)
    omega(i)=complex<double>(2.0*pimath*(double)i/(double)nsampP,0.0);

  for(int i=0;i<n;i++){
    blasVec trans(nsampP);  
      
    for(int j=0;j<nsampP;j++)
      trans(j)=exp(complex<double>(0.0,(double)(tdel_win[i]))*omega(j));

    //the negative value should be in the shift operator but the wrapped value
    //should be used for finding the amplitude from "amps"
    if(tdel_win[i]<0){
      tdel_win[i] = tdel_win[i] + nsampP; 
    }
    blasVec transP = pulseNSMultBlas(trans,pulsefft);
    if(fVerbose>2){
      cout << "OptimalFilterPhononNS::getChiSquare():  Printing trans with phase integer: " << tdel_win[i] <<  endl;
      for(int k=fVerboseN1; k<min((int)trans.size(),fVerboseN2); k++){
        complex<double> z = trans(k);
        cout << "[" << k << "]" << z <<  "   ";
      }
      cout << endl;
    }
    if(fVerbose>2){
      cout << "OptimalFilterPhononNS::getChiSquare():  Printing amplitude with phase integer: " << tdel_win[i] << "  " << amps[tdel_win[i]] <<  endl;
    }

    //subtract the template times normalization
    transP = transP - amps[tdel_win[i]]*templatefft;

    blasVec leftMult(nsampP);
    //solve
    umf::symbolic_type<std::complex<double> > cSymbolic;
    umf::numeric_type<std::complex<double> > cNumeric;
    umf::symbolic (static_cast<blasSprseMat>(covTotalSparse), cSymbolic); 
    umf::numeric (static_cast<blasSprseMat>(covTotalSparse), cSymbolic, cNumeric); 
    umf::solve(static_cast<blasSprseMat>(covTotalSparse), leftMult, transP, cNumeric);

    leftMult = conj(leftMult);
    //ignoring DC component
    vector<TComplex> leftMult_ROOT = convSTLToVecTComplex(leftMult);
    leftMult_ROOT[0] = comp_zero;

    //chisquare[i] = PulseTools::pulseDot(convSTLToVecTComplex(leftMult),convSTLToVecTComplex(transP)).Re();
    chisquare[i] = PulseTools::pulseDot(leftMult_ROOT,convSTLToVecTComplex(transP)).Re();
    //chisquare[i] = chisquare[i] - pow(amps[tdel_win[i]],2.0)*norm;

  }

  return chisquare;

}
void OptimalFilterPhononNS::SetVerbosityRange(int a, int b)
{ 
   if(a<0)
     a*=-1;
   if(b<0)
     b*=-1;

   if(a<b){
      fVerboseN1=a;
      fVerboseN2=b;
   }
   else{
      fVerboseN1=b;
      fVerboseN2=a;
   }
   return;
}

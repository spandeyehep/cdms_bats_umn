///////////////////////////////////////////////////////////////////////////////// 
//Class Name: F5ChargeX
//Authors: M. Kos
//Description: This class performs 2-parameter fit with delay ("F5") for charge channels.
// This fit should give good results even for sturating pulses.
//
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#include <iostream>

#include "F5ChargeX.h"
#include "PulseTools.h"
#include "TH1F.h"
#include "TFile.h"


//do not modify the signature of this constructor
F5ChargeX::F5ChargeX() :
  fQIVolts(-999999.),
  fQIChisq(-999999.),
  fQIBase(-999999.),
  fQOVolts(-999999.),
  fQOChisq(-999999.),
  fQOBase(-999999.),
  fSampleRate(-999999.),
  fOFDelay(-999999.),
  fSatDelay(-999999.),
  fTemplatesLoaded(false),
  fNBinsTemplates(0)
{
   //   cout <<"Hello from F5ChargeX()" << endl;

    //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "F5ChargeX";
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

F5ChargeX::~F5ChargeX()
{
//   cout <<"Goodbye from F5ChargeX()" << endl;
}

//if your RQ isn't here, it won't be stored in BatROOT output!!
//is there a way to choose what you do and don't want in the output?
void F5ChargeX::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("F5volts", initVal));
   fRQList.insert(pair<string,double>("F5chisq", initVal));
   fRQList.insert(pair<string,double>("F5base", initVal));
   fRQList.insert(pair<string,double>("F5satdelay",-123456.)); 

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//needed for this class because one instance belongs to more than one channel
void F5ChargeX::StoreAs(const string& chanType)
{
  if(chanType != "QI" && chanType != "QO")
    cout <<"F5Charge::WARNING! incorrect chanType passed to StoreAs function, storing QI values" << endl;

  if(chanType == "QO")
  {
    fRQList["F5volts"] = fQOVolts;
    fRQList["F5chisq"] = fQOChisq;
    fRQList["F5base"] = fQOBase;
  }
  else
  {
    fRQList["F5volts"] = fQIVolts;
    fRQList["F5chisq"] = fQIChisq;
    fRQList["F5base"] = fQIBase;
  }
  fRQList["F5satdelay"] = fSatDelay;
  return;

}

void F5ChargeX::LoadTemplates(const vector<double> &templateQI, const vector<double> &templateQIX, const vector<double>& templateQO, const vector<double> &templateQOX,
			      const double &templateMaxQI, const double &templateMaxQO)
{

  //these templates should all have the same lengths
  fNBinsTemplates = templateQI.size(); 
  fNBinsTemplatesQO = templateQO.size();
  fNBinsTemplatesQIX = templateQIX.size();
  fNBinsTemplatesQOX = templateQOX.size(); 
  
  if(fNBinsTemplates != (int)templateQIX.size() || fNBinsTemplates != (int)templateQO.size() || fNBinsTemplates != (int)templateQOX.size())
  {
    cerr <<"F5ChargeX::ERROR!  Template lengths do not match, check the input to LoadTemplates." << endl;
    exit(1);
  }

  fTemplateQI = templateQI;
  fTemplateQIX = templateQIX;
  fTemplateQO = templateQO;
  fTemplateQOX = templateQOX;
  
  fTemplateMaxQI = templateMaxQI;
  fTemplateMaxQO = templateMaxQO;

  fTemplatesLoaded = true;

  return;
}

void F5ChargeX::InitChisqFit(const vector<double>& templateQI, const vector<double>& templateQIX, const vector<double>& templateQO,  const vector<double>& templateQOX){
  int ibin,i,j;
 
  int nrows = 4;//4by4 matrix for LUDecomp
  int ncols = 4;

  //create new matrix and initialize it
  fpMns2 = new double * [nrows]; 
  for(i = 0;i<nrows;i++){
    fpMns2[i] = new double[ncols];
    for(j = 0;j<ncols;j++){
      fpMns2[i][j] = 0.;
    }
  }
  
  int Mbins = (int) templateQI.size();
  //cout << " template size " << Mbins << endl;
  for (ibin = 0; ibin < Mbins; ibin++) {				
    fpMns2[0][0] += pow(templateQI[ibin],2)/fnoiseQI + pow(templateQOX[ibin],2)/fnoiseQO;	
    fpMns2[0][1] += templateQI[ibin];
    fpMns2[0][2] += templateQI[ibin]*templateQIX[ibin]/fnoiseQI + templateQO[ibin]*templateQOX[ibin]/fnoiseQO;	
    fpMns2[0][3] += templateQOX[ibin];
    fpMns2[1][2] += templateQIX[ibin];
    fpMns2[2][2] += pow(templateQO[ibin],2)/fnoiseQO + pow(templateQIX[ibin],2)/fnoiseQI;	
    fpMns2[2][3] += templateQO[ibin];
  }
  fpMns2[0][1] /= fnoiseQI;
  fpMns2[0][3] /= fnoiseQO;
  fpMns2[1][2] /= fnoiseQI;
  fpMns2[2][3] /= fnoiseQO;
  fpMns2[1][0] = fpMns2[0][1];
  fpMns2[2][0] = fpMns2[0][2];
  fpMns2[3][0] = fpMns2[0][3];
  fpMns2[2][1] = fpMns2[1][2];
  fpMns2[3][2] = fpMns2[2][3];
  fpMns2[1][1] = Mbins/fnoiseQI;						
  fpMns2[3][3] = Mbins/fnoiseQO;
 
  //create new LU matrix and initialize it
  fpMnsLU2 = new double * [nrows]; 
  for(i = 0;i<nrows;i++){
    fpMnsLU2[i] = new double[ncols];
    for(j = 0;j<ncols;j++){
      fpMnsLU2[i][j] = fpMns2[i][j];
    }
  }						

  LUDecomp();
 
}

void F5ChargeX::LUDecomp(){
  vector<double> ivv;
  int matrixsize = 4;
  int i,j,k,imax=0;
  double lscale,tempscale,sum,dum;
  double tiny = 1.0e-20;
  
  for(i=0;i<matrixsize;i++) {findx.push_back(0.0); ivv.push_back(0.0);}
  fd = 1.0;
  /* Loop over rows to get the implicit scaling information */
  for (i=0;i<matrixsize;i++) {		
    lscale=0.0;
    for (j=0;j<matrixsize;j++) {
      if ((tempscale=fabs(fpMnsLU2[i][j])) > lscale) lscale=tempscale;
    }
    if (lscale == 0.0) {
      printf("Singular matrix in routine LUdecomp \r");
      fd = 0.;				/* Flag that matrix is singular */ 
      matrixsize  = 0;				/* prevent rest of routine from being done */
      break;
    }
    ivv[i]=1.0/lscale;		/* Save the scaling */
  }
  
  /* Loop over columns of Crout's method */
  for (j=0;j<matrixsize;j++) {
    for (i=0;i<j;i++) {
      sum=fpMnsLU2[i][j];
      for (k=0;k<i;k++) sum -= fpMnsLU2[i][k]*fpMnsLU2[k][j];
      fpMnsLU2[i][j]=sum;
    }
    /*Search for the largest pivot element */
    lscale=0.0;
    for (i=j;i<matrixsize;i++) {
      //ivv = vv+i; get ivv[i]
      sum=fpMnsLU2[i][j];
      for (k=0;k<j;k++) sum -= fpMnsLU2[i][k]*fpMnsLU2[k][j];
      fpMnsLU2[i][j]=sum;
      if ( (dum= ivv[i]*fabs(sum)) >=lscale) { //check this with numerical recipes [MK]
	lscale=dum;
	imax=i;
      }
    }
    /* Interchange rows if necessary */
    // jpos = vv + j;
    //imaxvv = vv+imax;
    if (j != imax) {
      for (k=0;k<matrixsize;k++) {
	dum = fpMnsLU2[imax][k];
	fpMnsLU2[imax][k]=fpMnsLU2[j][k];
	fpMnsLU2[j][k]=dum;
      }
      fd = -(fd);
      ivv[imax] = ivv[j];
    }
    findx[j]=imax;
    if (fpMnsLU2[j][j] == 0.0) fpMnsLU2[j][j]=tiny;
    /* Divide by the pivot element */
    if (j != matrixsize-1) { //check this with Numerical Recipes [MK]
      dum=1.0/(fpMnsLU2[j][j]);
      for (i=j+1;i<matrixsize;i++) fpMnsLU2[i][j] *= dum; //check this with Numerical Recipes [MK]
    }
  } //done with Crout's method
}

vector<double> F5ChargeX::LUBackSub(double **LUmatrix, int dim, const vector<double>& indx, const vector<double>& sol_vector){
  int i,ip,j;
  int ii = -1;
  double sum;
  vector<double> solution_vector;
  for(i=0;i<dim;i++) solution_vector.push_back(sol_vector[i]);
  /* Do the forward substitution, unscrambling the permutation along the way */
  for (i=0;i<dim;i++) {		
    ip=(int) indx[i];
    sum=solution_vector[ip];
    solution_vector[ip]=solution_vector[i];
    if (ii > -1)
      for (j=ii;j<=i-1;j++) sum -= LUmatrix[i][j]*solution_vector[j];
    else if (sum) ii=i; //check with numerical recipes
    solution_vector[i]=sum;
  }
  //now do the backward substitution
  for (i=dim-1;i>=0;i--) {		
    sum=solution_vector[i];
    for (j=i+1;j<dim;j++) sum -= LUmatrix[i][j]*solution_vector[j];
    solution_vector[i]=sum/LUmatrix[i][i];
  }
  return solution_vector;
}

vector<double> F5ChargeX::ImproveFit(double **MNSMatrix, double **LUmatrix,int dim,const vector<double>& indx,  const vector<double>& rhsVector, const vector<double>& SolVector){
	int j,i;
	double sdp;
        vector<double> SolutionVector;
        vector<double> r;
	for (i=0;i<dim;i++) {
	  sdp = -rhsVector[i];
	  for (j=0;j<dim;j++) sdp += MNSMatrix[i][j]*SolVector[j];
	  r.push_back(sdp);
	}
	SolutionVector = LUBackSub(LUmatrix,dim,indx,r);
	for (i=0;i<dim;i++) SolutionVector[i] = SolVector[i]-SolutionVector[i];
	return SolutionVector;
}

void F5ChargeX::ChisqFitX(){
  int dim = 4; //4 dimensions since it is a 4 parameter fit (2 scales + 2 baselines)  
  vector<double> rhs_vector;
  vector<double> sol_vector;
  int goodQStart = 0; 
  int goodQEnd =  fQIFit.size(); 
  //int Mbins = (int) fQIFit.size();
  int Mbins = goodQEnd - goodQStart;
  int ibin,fitpointsQI,fitpointsQO,i,j;
  for (i=0;i<dim;i++) rhs_vector.push_back(0.0);
  for (i=0;i<dim;i++) sol_vector.push_back(0.0);
  //cout << goodQStart << endl;
  //cout << goodQEnd << endl;
  //cout << Mbins << endl;
  /* Channel QI */
  fitpointsQI = 0;
  for (ibin = goodQStart; ibin < goodQEnd; ibin++) {
    if (fPulseQI[ibin]<fSatValue){
      fitpointsQI++;
      rhs_vector[0] += fScaleQI*fQIFit[ibin]*fTemplateQI[ibin]/fnoiseQI;
      rhs_vector[1] += fScaleQI*fQIFit[ibin]/fnoiseQI;
      rhs_vector[2] += fScaleQI*fQIFit[ibin]*fTemplateQIX[ibin]/fnoiseQI;
    }
  }

/* Channel QO */
  fitpointsQO = 0;
  for (ibin = goodQStart; ibin < goodQEnd; ibin++) {
    if (fPulseQO[ibin]<fSatValue){
      fitpointsQO++;
      rhs_vector[0] += fScaleQO*fQOFit[ibin]*fTemplateQOX[ibin]/fnoiseQO;
      rhs_vector[2] += fScaleQO*fQOFit[ibin]*fTemplateQO[ibin]/fnoiseQO;
      rhs_vector[3] += fScaleQO*fQOFit[ibin]/fnoiseQO;
    }
  }
    	    
  for(i=0;i<dim;i++) sol_vector[i] = rhs_vector[i];
  //Now check if there is enough fit points
  if (fitpointsQO + fitpointsQI < 10){
    cout<<"Not enough points to fit! "<<endl;
    fQIAmpl = 0.0;
    fQOAmpl = 0.0;
    fQIBase = 0.0;
    fQOBase = 0.0;
    fQIChisq = 10.0;
    fQOChisq = 10.0;
  }
  else if(fitpointsQO + fitpointsQI != 2*Mbins){
    for(ibin = goodQStart;ibin<goodQEnd;ibin++){
      if(fPulseQI[ibin]>=fSatValue){
	fpMns2[0][0] -= fTemplateQI[ibin]*fTemplateQI[ibin]/fnoiseQI;
	fpMns2[0][1] -= fTemplateQI[ibin]/fnoiseQI;
	fpMns2[0][2] -= fTemplateQI[ibin]*fTemplateQIX[ibin]/fnoiseQI;
        fpMns2[1][2] -= fTemplateQIX[ibin]/fnoiseQI;
        fpMns2[2][2] -= fTemplateQIX[ibin]*fTemplateQIX[ibin]/fnoiseQI;
        
      }
      if(fPulseQO[ibin]>=fSatValue){
	fpMns2[0][0] -= fTemplateQOX[ibin]*fTemplateQOX[ibin]/fnoiseQO;
	fpMns2[0][2] -= fTemplateQO[ibin]*fTemplateQOX[ibin]/fnoiseQO;
	fpMns2[0][3] -= fTemplateQOX[ibin]/fnoiseQO;
        fpMns2[2][2] -= fTemplateQO[ibin]*fTemplateQO[ibin]/fnoiseQO;
        fpMns2[2][3] -= fTemplateQO[ibin]/fnoiseQO;
        
      }
    }
    fpMns2[1][0] = fpMns2[0][1];
    fpMns2[2][0] = fpMns2[0][2];
    fpMns2[3][0] = fpMns2[0][3];
    fpMns2[2][1] = fpMns2[1][2];
    fpMns2[3][2] = fpMns2[2][3];
    fpMns2[1][1] = (double)fitpointsQI/fnoiseQI;
    fpMns2[3][3] = (double)fitpointsQO/fnoiseQO;
    
    for (i = 0; i<dim; i++) {
      for (j = 0; j<dim; j++) fpMnsLU2[i][j]=fpMns2[i][j];
    }
    LUDecomp();
    if (fd != 0.) { //check that matrix is not singular
      //cout << " Should get here for saturated pulses " << endl;
      fSolutionVector = LUBackSub(fpMnsLU2,dim,findx,sol_vector);
      fSolutionVector = ImproveFit(fpMns2,fpMnsLU2,dim,findx,rhs_vector,fSolutionVector); 
    }
  } else {           //LU backsub the matrix that has already been LUdecomposed in initChisqFit
    //cout << " Should not get here for saturated pulses " << endl;
     fSolutionVector = LUBackSub(fpMnsLU2,dim,findx,sol_vector);
  }

  //cout << fitpointsQO + fitpointsQO << endl;
  
  if (fd == 0){   //then matrix is singular
    fQIAmpl = 0.0;
    fQOAmpl = 0.0;
    fQIBase = 0.0;
    fQOBase = 0.0;
    fQIChisq = 11.0;
    fQOChisq = 11.0; 
    cout <<"Warning: Matrix is singular!"<<endl;
  }
  if (fd != 0){
    fQIAmpl = fSolutionVector[0];
    fQIBase = fSolutionVector[1];
    fQOAmpl = fSolutionVector[2];
    fQOBase = fSolutionVector[3];
    
    
    //now calculate the residuals and chis2
    fResSumQI = 0.0;
    fResSumQO = 0.0;
    for(ibin = goodQStart; ibin < goodQEnd; ibin++){
      if(fPulseQI[ibin]<fSatValue){ //voltsQI = fScaleQI*fQIFit[ibin]
	fResSumQI += (fScaleQI*fQIFit[ibin]-fSolutionVector[0]*fTemplateQI[ibin]-fSolutionVector[1]-fSolutionVector[2]*fTemplateQIX[ibin])*(fScaleQI*fQIFit[ibin]-fSolutionVector[0]*fTemplateQI[ibin]-fSolutionVector[1]-fSolutionVector[2]*fTemplateQIX[ibin]);
      }
      if(fPulseQO[ibin]<fSatValue){
	fResSumQO += (fScaleQO*fQOFit[ibin]-fSolutionVector[2]*fTemplateQO[ibin]-fSolutionVector[3]-fSolutionVector[0]*fTemplateQOX[ibin])*( fScaleQO*fQOFit[ibin]-fSolutionVector[2]*fTemplateQO[ibin]-fSolutionVector[3]-fSolutionVector[0]*fTemplateQOX[ibin]); 
      }
    }
    fQIChisq = TMath::Log10((fResSumQI/fnoiseQI)/(double)(fitpointsQI-dim/2));
    fQOChisq = TMath::Log10((fResSumQO/fnoiseQO)/(double)(fitpointsQO-dim/2));
  }
}


void F5ChargeX::DoCalc(const vector<double> &rawPulseQI, const vector<double> &rawPulseQO)
{   

  int nBins = rawPulseQI.size();
  int satdelay,satdelayQI,satdelayQO,i;
   
  //============== Some Preliminary Checks =================

  if(rawPulseQI.size() != rawPulseQO.size())
  {
    cerr <<"F5ChargeX::ERROR!  QI and QO pulses are not the same length, check input vectors!" << endl;
    exit(1);
  }

  if(!fTemplatesLoaded) 
  {
    cerr <<"F5ChargeX::ERROR!  attempting to run F5 without loading templates!" << endl;
    exit(1);
  }

  //checking that the lengths of templates agree with the pulse lengths
  if(nBins != fNBinsTemplates) 
  { 
    cerr<<"F5ChargeX::ERROR! Number of bins in pulse does not match number of bins in templates!" << endl; 
    exit(1);
  }

  //checking that the sample rate was initialized
  if(fSampleRate == -999999.) 
  {
    cerr <<"F5ChargeX::ERROR! Sample rate is uninitialized" << endl; 
    exit(1);
  }

  //Calculation goes here

  //cout << fScaleQI << endl;
  //cout << fScaleQO << endl;
  fScaleQI = 1.0/fScaleQI;
  fScaleQO = 1.0/fScaleQO;
  
  
  //[MK] This is calcZFitsC.m from darkpipe
  
  //check first if pulse is saturated
  
  for(i=0;i<(int)rawPulseQI.size();i++){
    fPulseQI.push_back(rawPulseQI[i]);
    fPulseQO.push_back(rawPulseQO[i]);
  }

  if (PulseTools::IsSaturated(fPulseQI,fSatValue) || PulseTools::IsSaturated(fPulseQO,fSatValue)){
   
    satdelayQI = (int) PulseTools::SaturationDelay(fPulseQI,fBSQI,fSTDQI,fTemplateStart,fSatValue);
    satdelayQO = (int) PulseTools::SaturationDelay(fPulseQO,fBSQO,fSTDQO,fTemplateStart,fSatValue);
    
    // both QI/QO saturated, take delay from pulse with later delay
    if (PulseTools::IsSaturated(fPulseQI,fSatValue) &&
	PulseTools::IsSaturated(fPulseQO,fSatValue)) {
    
      if (satdelayQI > satdelayQO) 
	satdelay =  satdelayQI+1; 
      else 
	satdelay = satdelayQO+1;

    } else if (PulseTools::IsSaturated(fPulseQI,fSatValue) && 
	       ~(PulseTools::IsSaturated(fPulseQO,fSatValue))) {
      // Only QI saturated
      satdelay =  satdelayQI+1; 

    } else {
      // Only QO saturated
      satdelay =  satdelayQO+1; 
    }
		 
    // Store RQ 
    fSatDelay = satdelay+1; //save this as an RQ for saturated pulses, if not sat it is -999999
  }
  
  else
    satdelay = (int) (fSampleRate*fOFDelay);
  
  
  satdelay = satdelay + 1;
  

  //Now prepare pulses with delay for fitting
  //first initialize them to be 0
  for(i = 0;i<fNBinsTemplates;i++){
    fQIFit.push_back(fBSQI);
    fQOFit.push_back(fBSQO);
    fPulseQI[i] = fBSQI;
    fPulseQO[i] = fBSQO;
    
  }

  //Now assign elements according to delay
  if (satdelay>0){
    for(i=0;i<fNBinsTemplates-satdelay;i++){
      fQIFit[i] = rawPulseQI[i+satdelay];
      fQOFit[i] = rawPulseQO[i+satdelay];
      fPulseQI[i] = rawPulseQI[i+satdelay];
      fPulseQO[i] = rawPulseQO[i+satdelay];
    }
  }
  else {
    for(i=0;i<fNBinsTemplates+satdelay;i++){
	fQIFit[i-satdelay] = rawPulseQI[i];
	fQOFit[i-satdelay] = rawPulseQO[i];
	fPulseQI[i-satdelay] = rawPulseQI[i];
	fPulseQO[i-satdelay] = rawPulseQO[i];
    }
  }
 
  
  //=================================================================================
  //do the actual fit here
  //First initialize chisq fit
  
  InitChisqFit(fTemplateQI,fTemplateQIX, fTemplateQO,fTemplateQOX);
  
  //Now actually do the fit
  ChisqFitX();

  //Check the fit results here
 
  // ========= done with fit! =================

  fQIVolts = fQIAmpl*fTemplateMaxQI;
  fQOVolts = fQOAmpl*fTemplateMaxQO;

  //cout << fQIVolts << endl;
  //cout << fQOVolts << endl;
  //cout << fQIBase << endl;
  //cout << fQOBase << endl;
  //cout << fQIChisq << endl;
  //cout << fQOChisq << endl;
 
  // ========== cleanup! ===========
  fTemplatesLoaded = false;

  //delete the 2-D matrices (4*4)
   // To clean up we must first delete the column arrays for
   // each row...
  for (int r = 0; r < 4; ++r )
    {
       delete [] fpMns2[r];
       delete [] fpMnsLU2[r];
    }

  // and then delete the row pointer array itself:
  delete [] fpMns2;
  delete [] fpMnsLU2;

}




void F5ChargeX::ConstructFakePulse(double normQI, double normQO, int delay)
{
   cout <<"Constructing fake pulses from templates!" << endl;

   if(!fTemplatesLoaded) { cerr <<"ERROR::Forgot to load templates!" << endl; exit(1);}
         
   // ===== Construct the pulse! ========
   
   //Copy template into vector and histogram
   for(int binItr = 0; binItr < fNBinsTemplates; binItr++)
   {
      //for a positive delay
      if(delay >= 0)
      {
	 if(binItr < delay)
	 {
	   fFakePulseQI.push_back(0.0); //no noise assumed
	   fFakePulseQO.push_back(0.0); //no noise assumed
	 }
	 if(binItr >= delay)
	 {
	   int shiftBin = binItr - delay;
	   fFakePulseQI.push_back(normQI*fTemplateQI[shiftBin] + normQO*fTemplateQIX[shiftBin]);
	   fFakePulseQO.push_back(normQO*fTemplateQO[shiftBin] + normQI*fTemplateQOX[shiftBin]);
	 }
	 
      }//endif positive delay
      
      //for a negative delay
      else
      {
	int shiftBin = binItr - delay;
	
	 if(shiftBin < 2048)
	  {
	     fFakePulseQI.push_back(normQI*fTemplateQI[shiftBin] + normQO*fTemplateQIX[shiftBin]);
	     fFakePulseQO.push_back(normQO*fTemplateQO[shiftBin] + normQI*fTemplateQOX[shiftBin]);
	  }
	 else
	 {
	    fFakePulseQI.push_back(0.0);  //no noise assumed
	    fFakePulseQO.push_back(0.0);  //no noise assumed
	 }
	 
      } //end if negative delay
      
      //      cout <<"fakePulseQO[" << binItr <<"] = " << fFakePulseQO[binItr] << endl;
      
   } //end loop over bins
      
   return;
}

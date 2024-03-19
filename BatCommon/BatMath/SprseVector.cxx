///////////////////////////////////////////////////////////////////////////////// 
//$Id$
//Class Name: SprseVector 
//Author: A.N. Villano 
//Description: This class is a representation of a vector.  It has a BLAS vector 
//             data member
// 
//
//File Import By: A.N. Villano 
//Creation Date: Nov. 12, 2012 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include "SprseVector.h"

//initialize parameters, object not useable until vector<TComplex> read
SprseVector::SprseVector() :
  isSet(false),
  fDimRow(0),
  fDimCol(0),
  fDimRowNative(0),
  fDimColNative(0),
  fNonZero(0),
  fnsofCutoff(0),
  fIsDnCast(false),
  fIsHConj(false),
  fIsConj(false),
  fVerbose(false)
{

   //cout <<"Hello from (NULL) SprseVector::SprseVector()" << endl;

}
SprseVector::SprseVector(vector<TComplex> *sprseVecTComplex, uint32_t N,bool hconj,bool conj) :
  isSet(false),
  fDimRow(0),
  fDimCol(0),
  fDimRowNative(0),
  fDimColNative(0),
  fNonZero(0),
  fnsofCutoff(0),
  fIsDnCast(false),
  fIsHConj(false),
  fIsConj(false),
  fVerbose(false)
{

  //set the cutoff?
  
  //cout <<"Hello from SprseVector::SprseVector()" << endl;
  //use column vectors by default
  uint32_t Nrow,Ncol;
  if(!hconj){
    Nrow=N;
    Ncol=1;
  }
  else{
    Nrow=1;
    Ncol=N;
  }

  if(SetNativeSize(Nrow,Ncol)){
    //set the vector<TComplex> (and keep a copy of it for the hell of it
    isSet = SetVector(sprseVecTComplex,hconj,conj);
    fIsConj=conj;
    fIsHConj=hconj;
  }
  else
    isSet = false;

}
/*SprseVector::~SprseVector()
{
   cout <<"Goodbye from SprseVector::SprseVector()" << endl;
}*/
bool SprseVector::SetVector(vector<TComplex> *sprseVecTComplex,bool hconj,bool conj)
{
 
  //make a copy so you can pass a root pointer in but root won't complain about what we do with the copy
  vector<TComplex> sprseVecTComplexCopy = *(sprseVecTComplex);

  blasVec SprseVec;
  isSet = convVecTComplexToSprseVector(sprseVecTComplexCopy,SprseVec,fNonZero,fnsofCutoff,hconj,conj);

  if(isSet){
    fElements=sprseVecTComplexCopy;
    (*this)=SprseVec;
    return true;
  }
  else
    return false;
}
bool SprseVector::ReSetVector(bool hconj,bool conj)
{
 
  //make a copy so you can pass a root pointer in but root won't complain about what we do with the copy

  blasVec SprseVec;
  isSet = convVecTComplexToSprseVector(fElements,SprseVec,fNonZero,fnsofCutoff,hconj,conj);

  if(isSet){
    (*this)=SprseVec;
    return true;
  }
  else
    return false;
}
bool SprseVector::convVecTComplexToSprseVector(vector<TComplex> &sprseVecTComplex,blasVec &out,uint32_t &size,uint32_t &nf_cutoff,bool hconj,bool conj)
{
  //**FIX ME** --> Can inadvertently read TGraphError with indicies LARGER than maximum dimension, this will crash program presently!!

  //I don't like that the Size had to be set explicitly by SetSize it should be read back

  //get the total number of elements
  size=(uint32_t) sprseVecTComplex.size(); //total number of elements
  if(size>fDimRow*fDimCol){
    cerr << "SprseVector::convVecTComplexToSprseVector(VecTComplex,blasVec): too many elements!" << endl;
    size=0; //so if fail no overall effect
    return false;
  }


  //now set up the BLAS sparce vector object
  out = blasVec(fDimRow*fDimCol);


  //high frequency cutoff for non-diagnoal components
  bool lgc_fast = (nf_cutoff>0 && nf_cutoff<fDimCol*fDimRow);
  int nfl,nfh;

  if(lgc_fast){ 
    if(nf_cutoff%2>0)
      nf_cutoff+=1;

    nfl = nf_cutoff/2;

    nfh = size - nfl; 
  }

 
  
  if(lgc_fast){
    for(int i=0;i<(int)size;i++){
      bool range = (i>=nfl && i<nfh);
      if(range){
        sprseVecTComplex[i] = TComplex(0.0,0.0);
      }
    }
  }

  complex<double> cval(0.0,0.0);

  for(int i=0;i<(int)size;i++){
    if((!conj && !hconj) || (conj && hconj))
      cval = complex<double>(sprseVecTComplex[i].Re(),sprseVecTComplex[i].Im());
    else 
      cval = complex<double>(sprseVecTComplex[i].Re(),-sprseVecTComplex[i].Im());

    out(i) = cval;
  }


  return true;
}
bool SprseVector::SetSize(uint32_t Nrow,uint32_t Ncol)
{
  //I don't like that the Size had to be set explicitly by SetSize it should be read back
  if(Nrow==0 || Ncol==0){
    cerr << "SprseVector::SetSize(uint32_t,uint32_t): one or more vector dimensions zero!" << endl;
    fDimRow=0;
    fDimCol=0;
    return false;
  }
  if(Nrow!=1 && Ncol!=1){
    cerr << "SprseVector::SetSize(uint32_t,uint32_t): one dimension should be unity!" << endl;
    fDimRow=0;
    fDimCol=0;
    return false;
  }
  fDimRow=Nrow;
  fDimCol=Ncol;

  return true;
}
bool SprseVector::SetNativeSize(uint32_t Nrow,uint32_t Ncol)
{
  if(SetSize(Nrow,Ncol)){
    fDimRowNative=Nrow;
    fDimColNative=Ncol;
    return true;
  }
  return false;
}
bool SprseVector::DnCastOut(uint32_t halfsize,bool recursive)
{
  //check if you are already downcast, recast if necessary
  if(!recursive && fIsDnCast){
    ReCast();
  }
  
  //check that this is an appropriate half size
  if(halfsize*2>fDimRow*fDimCol){
    cerr << "SprseVector::DnCastOut(uint32_t,bool): Downcast size too large" << endl;
    return false;
  }

  //count for new nonzero elements
  uint32_t nzeroelements=0;
  
  //recast *(this) object
  blasVec dncastVec(halfsize*2);
  typedef blasVec::const_iterator it1_t;

  bool condition_nshift,condition_shift;
  uint32_t i,ishft;
  ishft=fDimRow*fDimCol - (2*halfsize);
  for (it1_t it1 = this->begin(); it1 != this->end(); it1++)
  {
      i = it1.index();
      //make shift and delete conditions
      condition_shift = (i>=(halfsize+ishft));
      condition_nshift = (i<(halfsize));
      //do shift first then delete to be sure not to erase
      //std::cout << "(" << it2.index1() << "," << it2.index2() << ") = ";
      if(condition_shift && !condition_nshift){
        //cout << "inserting with shift" << endl;
        dncastVec.insert_element(i-ishft,(*this)(i));
	++nzeroelements;
      }
      else if(condition_nshift && !condition_shift){
        //cout << "inserting without shift" << endl;
        dncastVec.insert_element(i,(*this)(i));
	++nzeroelements;
      }
  }

  //swap this for the new vector 
  //cout << "got new BLAS size: " << dncastMat.size1() << " x " << dncastMat.size2() << endl;
  (*this).swap(dncastVec);

  //change fDimRow and fDimCol and nonzero
  if(!fIsHConj)
    SetSize((*this).size(),1);
  else
    SetSize(1,(*this).size());
  fNonZero=nzeroelements;

  //set the fIsDnCast bit
  fIsDnCast=true;

  return true;

}
bool SprseVector::DnCastIn(uint32_t halfsize,bool recursive)
{
  //check if you are already downcast, recast if necessary
  if(!recursive && fIsDnCast){
    ReCast();
  }
  
  //check that this is an appropriate half size
  if(halfsize*2>fDimRow*fDimCol){
    cerr << "SprseVector::DnCastOut(uint32_t,bool): Downcast size too large" << endl;
    return false;
  }

  //count for new nonzero elements
  uint32_t nzeroelements=0;
  
  //recast *(this) object
  blasVec dncastVec((fDimRow*fDimCol-halfsize*2));
  typedef blasVec::const_iterator it1_t;

  bool condition_shift;
  uint32_t i,ishft;
  ishft=halfsize;
  for (it1_t it1 = this->begin(); it1 != this->end(); it1++)
  {
      i = it1.index();
      //make shift and delete conditions
      condition_shift = ((i>=(halfsize))
        && (i<(fDimRow*fDimCol-halfsize)));
      //do shift first then delete to be sure not to erase
      //std::cout << "(" << it2.index1() << "," << it2.index2() << ") = ";
      if(condition_shift){
        //cout << "inserting with shift" << endl;
        dncastVec.insert_element(i-ishft,(*this)(i));
	++nzeroelements;
      }
  }

  //swap this for the new vector 
  //cout << "got new BLAS size: " << dncastMat.size1() << " x " << dncastMat.size2() << endl;
  (*this).swap(dncastVec);

  //change fDimRow and fDimCol and nonzero
  if(!fIsHConj)
    SetSize((*this).size(),1);
  else
    SetSize(1,(*this).size());
  fNonZero=nzeroelements;

  //set the fIsDnCast bit
  fIsDnCast=true;

  return true;

}
bool SprseVector::ReCast()
{
    if(fVerbose)
      cout << "SprseVector::DnCastOut(uint32_t,bool): Recasting to original!" << endl;
    SetSize(fDimRowNative,fDimColNative);
    ReSetVector(fIsHConj,fIsConj);
    fIsDnCast=false;
    return true;
}
/*SprseVector& SprseVector::operator=(const SprseVector &copy)
{

  if(this != &copy){
    //reference
    //http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html

    blasSprseMat a;
    a = static_cast<blasSprseMat>(copy);

    //give back the blasmat part in this awkward way
    (*this) = a;

    //set all the member data
    isSet = copy.IsSet();
    fElements = copy.GetGraph();
    fDimRow = copy.GetNrow();
    fDimCol = copy.GetNcol();
    fDimRowNative = copy.GetNrowNative();
    fDimColNative = copy.GetNcolNative();
    fNonZero = copy.GetN();
    fnsofCutoff = copy.GetNSOFCutoff();
    fIsDnCast = copy.IsDnCast();
    fIsHConj = copy.IsHConj();
  }
  return *this;
}*/

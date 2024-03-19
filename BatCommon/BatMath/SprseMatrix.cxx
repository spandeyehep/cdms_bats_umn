///////////////////////////////////////////////////////////////////////////////// 
//$Id$
//Class Name: SprseMatrix 
//Author: A.N. Villano 
//Description: This class is a representation of a matrix.  It has a BLAS sparse matrix
//             data member
// 
//
//File Import By: A.N. Villano 
//Creation Date: Apr. 22, 2012 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include "SprseMatrix.h"

//initialize parameters, object not useable until TGraphErrors read
SprseMatrix::SprseMatrix() :
  isSet(false),
  fDimRow(0),
  fDimCol(0),
  fDimRowNative(0),
  fDimColNative(0),
  fNonZero(0),
  fnsofCutoff(0),
  fIsDnCast(false),
  fIsHConj(false),
  fVerbose(false)
{

   //cout <<"Hello from (NULL) SprseMatrix::SprseMatrix()" << endl;

}
SprseMatrix::SprseMatrix(TGraphErrors *sprseTGraph,uint32_t Nrow, uint32_t Ncol,bool conj) :
  isSet(false),
  fDimRow(0),
  fDimCol(0),
  fDimRowNative(0),
  fDimColNative(0),
  fNonZero(0),
  fnsofCutoff(0),
  fIsDnCast(false),
  fIsHConj(false),
  fVerbose(false)
{

  //set the cutoff?
  
  //cout <<"Hello from SprseMatrix::SprseMatrix()" << endl;
  //set the size (currently TGraphErrors doesn't know the size in terms of matrix dimensions)
  if(SetNativeSize(Nrow,Ncol)){
    //set the TGraphErrors (and keep a copy of it for the hell of it
    isSet = SetMatrix(sprseTGraph,conj);
    fIsHConj=conj;
  }
  else
    isSet = false;

  

}
/*SprseMatrix::~SprseMatrix()
{
   cout <<"Goodbye from SprseMatrix::SprseMatrix()" << endl;
}*/
bool SprseMatrix::SetMatrix(TGraphErrors *sprseTGraph,bool conj)
{
 
  //make a copy so you can pass a root pointer in but root won't complain about what we do with the copy
  TGraphErrors sprseTGraphCopy = *(sprseTGraph);

  blasSprseMat SprseMat;
  isSet = convTGraphToSprseMatrix(sprseTGraphCopy,SprseMat,fNonZero,fnsofCutoff,conj);

  if(isSet){
    fElements=sprseTGraphCopy;
    (*this)=SprseMat;
    return true;
  }
  else
    return false;
}
bool SprseMatrix::ReSetMatrix(bool conj)
{
 
  //make a copy so you can pass a root pointer in but root won't complain about what we do with the copy

  blasSprseMat SprseMat;
  isSet = convTGraphToSprseMatrix(fElements,SprseMat,fNonZero,fnsofCutoff,conj);

  if(isSet){
    (*this)=SprseMat;
    return true;
  }
  else
    return false;
}
bool SprseMatrix::convTGraphToSprseMatrix(TGraphErrors &sprseTGraph,blasSprseMat &out,uint32_t &size,uint32_t &nf_cutoff,bool conj)
{
  //**FIX ME** --> Can inadvertently read TGraphError with indicies LARGER than maximum dimension, this will crash program presently!!

  //I don't like that the Size had to be set explicitly by SetSize it should be read back

  //get the total number of elements
  size=(uint32_t) sprseTGraph.GetN(); //total number of elements
  if(size>fDimRow*fDimCol){
    cerr << "SprseMatrix::convTGraphToSprseMatrix(TGraphErrors,blasSprseMat): too many elements!" << endl;
    size=0; //so if fail no overall effect
    return false;
  }


  //now set up the BLAS sparce matrix object
  out = blasSprseMat(fDimRow,fDimCol,size);

  //now read in all the elements
  int pointcount = size;

  //high frequency cutoff for non-diagnoal components
  bool lgc_fast = (nf_cutoff>0 && nf_cutoff<fDimRow);
  int nfl,nfh;

  if(lgc_fast){ 
    if(nf_cutoff%2>0)
      nf_cutoff+=1;

    nfl = nf_cutoff/2;

    nfh = size - nfl; 
  }

 
  int dumb=0; 
  
  if(lgc_fast){
    for(int i=0;i<size;i++){
      double x,y;
      sprseTGraph.GetPoint(i,x,y);
    
      bool xrange = ((int)x>=nfl && (int)x<nfh) && ((int)x != (int)y);
      bool yrange = ((int)y>=nfl && (int)y<nfh) && ((int)x != (int)y);
      if(xrange || yrange){
        sprseTGraph.RemovePoint(i);
	pointcount--;
      }
      else{
        if((int)x == 8188 && (int)y==8188)
	  dumb++;
      }
    }
  }

  //i don't know why but element 8188 has like 17,000 copies
  //cout << "8188 " <<  dumb << endl;
  
  complex<double> cval(0.0,0.0);

  for(int i=0;i<pointcount;i++){
    if(!conj)
      cval = complex<double>(sprseTGraph.GetErrorX(i),sprseTGraph.GetErrorY(i));
    else if(conj)
      cval = complex<double>(sprseTGraph.GetErrorX(i),-sprseTGraph.GetErrorY(i));
    double x,y;
    sprseTGraph.GetPoint(i,x,y);

    if(!conj)
      out((int)x,(int)y) = cval;
    else if(conj)
      out((int)y,(int)x) = cval;
  }


  size=pointcount;

  return true;
}
bool SprseMatrix::SetSize(uint32_t Nrow,uint32_t Ncol)
{
  //I don't like that the Size had to be set explicitly by SetSize it should be read back
  if(Nrow==0 || Ncol==0){
    cerr << "SprseMatrix::SetSize(uint32_t,uint32_t): one or more matrix dimensions zero!" << endl;
    fDimRow=0;
    fDimCol=0;
    return false;
  }
  if(Nrow != Ncol){
    cerr << "SprseMatrix::SetSize(uint32_t,uint32_t): only square matricies!" << endl;
    fDimRow=0;
    fDimCol=0;
    return false;
  }
  fDimRow=Nrow;
  fDimCol=Ncol;

  return true;
}
bool SprseMatrix::SetNativeSize(uint32_t Nrow,uint32_t Ncol)
{
  if(SetSize(Nrow,Ncol)){
    fDimRowNative=Nrow;
    fDimColNative=Ncol;
    return true;
  }
  return false;
}
bool SprseMatrix::DnCastOut(uint32_t halfsize,bool recursive)
{
  //check if you are already downcast, recast if necessary
  if(!recursive && fIsDnCast){
    ReCast();
  }
  
  //check that this is an appropriate half size
  if(halfsize*2>fDimRow){
    cerr << "SprseMatrix::DnCastOut(uint32_t,bool): Downcast size too large" << endl;
    return false;
  }

  //count for new nonzero elements
  uint32_t nzeroelements=0;
  
  //recast *(this) object
  blasSprseMat dncastMat(halfsize*2,halfsize*2,0);
  typedef blasSprseMat::const_iterator1 it1_t;
  typedef blasSprseMat::const_iterator2 it2_t;

  bool condition_nshift,condition_shift;
  uint32_t i,j,ishft,jshft;
  ishft=fDimRow - (2*halfsize);
  jshft=fDimCol - (2*halfsize);
  for (it1_t it1 = this->begin1(); it1 != this->end1(); it1++)
  {
    for (it2_t it2 = it1.begin(); it2 != it1.end(); it2++)
    {
      i = it2.index1();
      j = it2.index2();
      //make shift and delete conditions
      condition_nshift = (i<halfsize && j<halfsize); 
      condition_shift = ((i>=(halfsize+ishft)) && (j>=(halfsize+jshft)));
      //do shift first then delete to be sure not to erase
      //std::cout << "(" << it2.index1() << "," << it2.index2() << ") = ";
      if(!condition_nshift && condition_shift){
        //cout << "inserting with shift" << endl;
        dncastMat.insert_element(i-ishft,j-jshft,(*this)(i,j));
	++nzeroelements;
      }
      else if(condition_nshift && !condition_shift){
        //cout << "inserting without shift" << endl;
        dncastMat.insert_element(i,j,(*this)(i,j));
	++nzeroelements;
      }
    }
  }

  //swap this for the new matrix
  //cout << "got new BLAS size: " << dncastMat.size1() << " x " << dncastMat.size2() << endl;
  (*this).swap(dncastMat);

  //change fDimRow and fDimCol and nonzero
  SetSize((*this).size1(),(*this).size2());
  fNonZero=nzeroelements;

  //set the fIsDnCast bit
  fIsDnCast=true;

  return true;

}
bool SprseMatrix::DnCastIn(uint32_t halfsize,bool recursive)
{
  //check if you are already downcast, recast if necessary
  if(!recursive && fIsDnCast){
    ReCast();
  }
  
  //check that this is an appropriate half size
  if(halfsize*2>fDimRow){
    cerr << "SprseMatrix::DnCastOut(uint32_t,bool): Downcast size too large" << endl;
    return false;
  }

  //count for new nonzero elements
  uint32_t nzeroelements=0;
  
  //recast *(this) object
  blasSprseMat dncastMat((fDimRow-halfsize*2),(fDimCol-halfsize*2),0);
  typedef blasSprseMat::const_iterator1 it1_t;
  typedef blasSprseMat::const_iterator2 it2_t;

  bool condition_nshift,condition_shift;
  uint32_t i,j,ishft,jshft;
  ishft=halfsize;
  jshft=halfsize;
  for (it1_t it1 = this->begin1(); it1 != this->end1(); it1++)
  {
    for (it2_t it2 = it1.begin(); it2 != it1.end(); it2++)
    {
      i = it2.index1();
      j = it2.index2();
      //make shift and delete conditions
      condition_nshift = false; //everything must shift!
      condition_shift = ((i>=(halfsize)) && (j>=(halfsize)))
        && ((i<(fDimRow-halfsize)) && (j<(fDimCol-halfsize)));
      //do shift first then delete to be sure not to erase
      //std::cout << "(" << it2.index1() << "," << it2.index2() << ") = ";
      if(!condition_nshift && condition_shift){
        //cout << "inserting with shift" << endl;
        dncastMat.insert_element(i-ishft,j-jshft,(*this)(i,j));
	++nzeroelements;
      }
      else if(condition_nshift && !condition_shift){
        //cout << "inserting without shift" << endl;
        dncastMat.insert_element(i,j,(*this)(i,j));
	++nzeroelements;
      }
    }
  }

  //swap this for the new matrix
  //cout << "got new BLAS size: " << dncastMat.size1() << " x " << dncastMat.size2() << endl;
  (*this).swap(dncastMat);

  //change fDimRow and fDimCol and nonzero
  SetSize((*this).size1(),(*this).size2());
  fNonZero=nzeroelements;

  //set the fIsDnCast bit
  fIsDnCast=true;

  return true;

}
bool SprseMatrix::ReCast()
{
    if(fVerbose)
      cout << "SprseMatrix::DnCastOut(uint32_t,bool): Recasting to original!" << endl;
    SetSize(fDimRowNative,fDimColNative);
    ReSetMatrix(fIsHConj);
    fIsDnCast=false;
    return true;
}
/*SprseMatrix& SprseMatrix::operator=(const SprseMatrix &copy)
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

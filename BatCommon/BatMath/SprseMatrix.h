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

#ifndef HAVESprseMatrix 
#define HAVESprseMatrix 

//needed for uint32_t
#include <boost/cstdint.hpp>
//boost libraries needed for manipulation of sparse object
//#include <boost/numeric/bindings/traits/ublas_vector.hpp>
#include <boost/numeric/bindings/traits/ublas_sparse.hpp>
//#include <boost/numeric/bindings/umfpack/umfpack.hpp>

//namespace for BLAS objects
namespace ublas = boost::numeric::ublas;
//namespace umf = boost::numeric::bindings::umfpack;


using namespace std;
//BLAS objects
typedef ublas::compressed_matrix<complex<double>, ublas::column_major, 0,
ublas::unbounded_array<int>, ublas::unbounded_array<complex<double> > > blasSprseMat; 

//ROOT Objects
#include "Rtypes.h"
#include "TGraphErrors.h"


//this is a class which packages the BLAS matrix into an object easily used by cdmsbats
class SprseMatrix : public blasSprseMat
{

  public:
 
  //copy constructor (doesn't copy over derived data so OK)
  SprseMatrix(const SprseMatrix& c) : blasSprseMat(c) {}
  //constructor does empty object
  SprseMatrix();
  //constructor reads TGraphErrors
  SprseMatrix(TGraphErrors*,uint32_t,uint32_t,bool conj=false);

  //downcast outter
  bool DnCastOut(uint32_t,bool recursive=false);
  //downcast inner
  bool DnCastIn(uint32_t,bool recursive=false);
  //recast
  bool ReCast(void);

  //Sets
  //Set the verbosity
  void SetVerbose() { fVerbose=true; }
  void UnSetVerbose() { fVerbose=false; }
  //Set the sparse matrix (and save the TGraph?)
  bool convTGraphToSprseMatrix(TGraphErrors&,blasSprseMat&,uint32_t&,uint32_t&,bool conj=false);


  //Gets
  TGraphErrors GetGraph() { return fElements; }
  uint32_t GetNrow() { return fDimRow; }
  uint32_t GetNcol() { return fDimCol; }
  uint32_t GetNrowNative() { return fDimRowNative; }
  uint32_t GetNcolNative() { return fDimColNative; }
  uint32_t GetN() { return fNonZero; }
  uint32_t GetNSOFCutoff() { return fnsofCutoff; }
  bool IsDnCast() { return fIsDnCast; }
  bool IsHConj() { return fIsHConj; }
  bool GetVerbose() { return fVerbose; }

  //inherit the operators from the blasMatrix
  //do other inherits mess with this object? scaler mult, addition...
  //addition messes with fNonZero, potentially

  //Assignment and copy constructor seem to keep derived member data intact
  using blasSprseMat::operator=;
  //SprseMatrix& operator=(const SprseMatrix&);
  //Other operators like scalar mult (operator*()) and addition (operator+())
  //DON'T keep stuff intact. understandibly how would it know how to set what
  //it's returning as far as fNrows etc.  New objects probably also do NOT
  //have a copy of the TGraphErrors, but that's ok, just make a flag indicating
  //it.

  //using blasSprseMat::operator();
  //using blasSprseMat::operator+=;

  private:

  //protected Sets
  //nobody sets the matrix except the constructor
  bool SetMatrix(TGraphErrors*,bool conj=false);
  bool ReSetMatrix(bool conj=false);
  //Set the size of the matrix because it is not read in the TGraph
  //private so you can't reset it after reading matrix
  bool SetSize(uint32_t,uint32_t);
  bool SetNativeSize(uint32_t,uint32_t);
  bool SetNSCutoff(uint32_t);

  bool isSet;
  //blasSprseMat fSparse;
  TGraphErrors fElements;
  uint32_t fDimRow;
  uint32_t fDimCol;
  uint32_t fDimRowNative;
  uint32_t fDimColNative;
  uint32_t fNonZero;
  uint32_t fnsofCutoff;
  bool fIsDnCast;
  bool fIsHConj;
  bool fVerbose;


};

#endif /* HAVESprseMatrix */

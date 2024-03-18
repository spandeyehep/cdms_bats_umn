///////////////////////////////////////////////////////////////////////////////
//Class Name: RootGraphix
//Authors: B. Loer
//Description: Utility class used to draw interactive ROOT canvases within 
//a regular binary executable.
//
//File Import by: B. Loer
//Creation Date: Dec . 18, 2011
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ROOTGRAPHIX_h
#define ROOTGRAPHIX_h

#include "TMutex.h"
#include "TVirtualMutex.h"
#include "TThread.h"
#include "TApplication.h"
#include <memory>
class TCanvas;
class TPad;
class TVirtualPad;
/** @class RootGraphix 
    @brief Displays ROOT canvases, etc, outside the ROOT interactive environment
*/
// In ROOT 5, TMutex inherited from TObject.
// In ROOT 6, it does not, so we make this trivial class to add it back in.
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
class TMutexObj : public TMutex, public TObject {};
#else
typedef TMutex TMutexObj;
#endif

class RootGraphix{
  //constructor is private; all public interfaces are static
 public: 
  /** @typedef Lock
      @brief wrap a TLockGuard into a std::auto_ptr
  */
  typedef std::auto_ptr<TLockGuard> Lock;
  /// Lock the mutex protecting graphics updates to make changes safely
  static Lock AcquireLock();
  /// Utility function to Get a new canvas to draw objects on
  static TCanvas* GetCanvas(double scale=0.49, bool preventclose=true);
  /// Run a ROOT macro (mostly useful for setting the stylesheet)
  static void ProcessFile(const char* file);
  /// Divide a canvas into a set number of pads, up to 20
  static void DividePad(TPad* pad, int npads,bool getlock=true);
 public:
  ~RootGraphix();
 private:
  RootGraphix();
  static RootGraphix* GetInstance(); //< delay initialization until required
  void SetDefaultStyleSheet(); ///< make root not look ugly
  
  /// Get a pointer to the mutex. Should not be called directly; use AcquireLock
  TMutexObj* GetMutex() { return &fMutex; }
  TApplication* GetApplication(){ return &fApp; }
 private:
  TApplication fApp;
  TMutexObj fMutex;
  TThread fThread;
  static const UInt_t fKeepRunning = 0x100000;
  //thread entry point
  static void* RunRootGraphix(void* mutexptr);
};



#endif /*ROOTGRAPHIX_h*/

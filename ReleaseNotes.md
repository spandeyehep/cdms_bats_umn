Release: v5.6.5
===============
 
 
Overview: In several places for the crosstalk algorithm, both within BatRoot (EventBuilder) and
          BatNoise the code only does computations for detectors that are specifically known to
	  have two charge channels per side.  Since UMN is the first one to use the kiZIPSNOlab
	  detector type for large processings, it had not been previously included.  
 
Purpose:  Fix an issue related to kiZIPSNOlab detector type and charge crosstalk-correcting OF for
          iZIPs
 
Type:     hotfix 

Implementation: "hotfix" of master Git Flow protocol

BatCommon Release: v5.7.3
 
Notes: To include some of the more recent UMN processings the Z1_Templates.list file was also
       changed and one specialized UMN template file was included.  Other template files that are
       referenced in that list file have NOT been included.  But they should eventually. 



Release: v5.6.4
===============
 
 
Overview: added option to turn off noise when processing for DMC (scaling in-run-random event by 0). Also minor fix in auto_processing tools and calibration config files.
 
Purpose: Process DMC events without noise from data. 
 
Type:     hotfix 

Implementation: "hotfix" of master Git Flow protocol

BatCommon Release: v5.7.0
 
Notes:  



Release: v5.6.3
===============
 
 
Overview: hotfix to auto_processing tools, calibration config files, and processing list for Photoneutron and CDMSlite production
 
Purpose:  (1) The auto_procesisng tools allows processing without filesystem limitations (typically this is x10 faster in Brazos cluster), (2) the calibration config files are corrected, (3) and a few processing lists (CDMSliteR3) had duplucate series, now removed. 
 
Type:     hotfix 

Implementation: "hotfix" of master Git Flow protocol

BatCommon Release: v5.7.0
 
Notes:   The auto_processing package perhaps should become a separate repository, but this is already a major step towards full GRID capabilities. The rest are minor fixes for data processing (Photoneutron). The Process_dev branch may be deleted after this release.




Release: v5.6.2
===============
 
 
Overview: hotfix to fix two typos in the UserSettings calibration files.
 
Purpose:  correct two typos that were discovered after a test processing with v5.6.1. The typos affected Photoneutron data, specifically z4 and z5 calibration
 
Type:     hotfix 

Implementation: "hotfix" of master Git Flow protocol

BatCommon Release: v5.7.0
 
Notes:   


Release: v5.6.1
===============
 
 
Overview: hotfix to merge in some straggling ProdPN\_dev feature branch changes. 
 
Purpose:  the release of v5.6.0 included changes on develop and also features merged from the
          public feature branch ProdPN\_dev.  After that release the ProdPN\_dev continued
	  development instead of being deleted.  This hotfix serves to include the new features on
	  that feature branch. 
 
Type:     hotfix 

Implementation: "hotfix" of master Git Flow protocol

BatCommon Release: v5.7.0
 
Notes:  The public feature branch ProdPN_dev will be deleted after this merge, as should all
        feature branches once they have been merged into a release or hotfix.  

Release: v5.6.0
===============
 
 
Overview: First version of cdmsbats with BatFaker. 
 
Purpose:  BatFaker is another binary added to the cdmsbats package that is for constructing raw
          data output in the same format that the DAQ would create, but using simulated data. 
 
Type:     Minor increment 

Implementation: "release" of develop Git Flow protocol

BatCommon Release: v5.7.0
 
Notes:  Some C++11 features are included but only ones that can be accessed in gcc 4.4.6 compilers
        by using the -std=c++0x flag to gcc. 

Release: v5.5.2
===============
 
 
Overview: Jorge's DMC_dev branch updated some auto\_processing things. 
 
Purpose:  There was a bug in the autoprocessing scripts for DMC, the maxnumber
          in batcalib would short from the total number of events processed. 
 
Type:     bugfix

Implementation: "hotfix" in Git Flow protocol
 
Notes: 

Release: v5.5.1
===============
 
 
Overview: Made phonon relative calibrations in the new UMN input files unity, these are good starting values. 
 
Purpose: 
 
Type:     bugfix

Implementation: "hotfix" in Git Flow protocol
 
Notes: 

Release: v5.5.0
===============
 
 
Overview: Includes new detector types kHVSNOlab (710) and kHVUMN (1710) as well as detector
          modifications to use these data types with CDMS II cold hardware and RevC DCRCs.  Mostly
	  this is accomplished by upgrading to BC v5.6.0.
 
Purpose:  The kHVSNOlab detector type would have had to be introduced eventually and it was done
          now to accommodate UMN detector tests.  Because of the "special" one-sided readout of
	  high-voltage-biased detectors at UMN we also needed one detector type to represent an HV
	  detector with just one side of phonon channels read out, and the corresponding mapping. 
 
Type:     Minor increment (as opposed to bugfix or Major increment)
 
Notes:    Most upgrading happened for this change inside of BatCommon, in cdmsbats in some places
          like NoiseBuilder::ConstructMinMaxDistribution checks were included to ascertain if
	  charge channels are present.  Further in BatRoot a check in
	  BatOutputManager::GetChanList which exited if there was an empty channel list was
	  changed to a warning.  This was probably a redundant check anyway, because a channel not
	  being in the list will probably not result in any further problems, for some detectors
	  it may be suspicious if it has no charge channel, so that's probably why the check was
	  added.  Perhaps in the future this exception can be made detector-specific.

Compatibility:

  * BatCommon v5.6.0 is preferred but back-compatibility probably holds 

Release: v5.4.3
===============
 
 
Overview: Small bugfix for DMC processing in BatRoot.cxx otherwise configuration files and
          autoprocessing list changes. 
 
Purpose: 
 
Type:     bugfix  (as opposed to Minor or Major increment)
 
Notes: 

Releases < v5.4.3
=================

Overview: these releases did not have detailed notes.  We've recently updated to using
          [gitflow](https://github.com/nvie/gitflow) and trying to stick with the [semantic
	  versioning](http://semver.org/) scheme.

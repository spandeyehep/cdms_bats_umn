Release: v5.7.2 & v5.7.3
========================
 
 
Overview: Patch BatCommon to read all the zips packed into a single event, not just the first one
          in RevD-style reading with the SNOLAB raw data format.  Also extract the EventCategory
	  information from the data format, it wasn't before. 
 
Purpose:  The SNOLAB raw data format contains the possibility to pack many detectors' events into
          one raw data block.  With the adoption of the new Event Builder, we now make extensive
	  use of this feature and RawDataReader had a flaw where there was no "zip loop" so it
	  only read the first event. 
 
Type:     Minor increment 

Implementation: "hotfix" from develop in Git Flow protocol
 
Notes: added a "zip loop" events read out properly and extract the EventCategory from the raw data
       format.  v5.7.3 was to fix a diagnostic print statement that was not protected by a
       verbosity check. We now use the following code for EventCategory:

       0: full readout trigger
       1: generic random
       2: BOR random
       3: in-run random
       4: EOR random
       5: unused
       6: selective readout trigger

Release: v5.7.1
===============
 
 
Overview: Small modification for RevD SNOLAB-style HV channel ordering 
 
Purpose:  To get the channel ordering right for a SNOLAB-style HV detector read out by a RevD.1 DCRC 
 
Type:     Minor increment 

Implementation: "hotfix" from develop in Git Flow protocol
 
Notes: 

Release: v5.7.0
===============
 
 
Overview: First release of BatCommon with necessary changes for BatFaker, and small modification
          for data processing from the RevD style DAQ raw files. 
 
Purpose:  Prepare the code for the cdmsbats release with the first version of BatFaker and have a
          release for the cdmslite Run 3 processing. 
 
Type:     Minor increment 

Implementation: "release" from develop in Git Flow protocol
 
Notes: 

Release: v5.6.1
===============
 
 
Overview: Forgot to merge Jorge's DMC_dev branch before v5.6.0 release. 
 
Purpose:  Commit b5584f on Jorge's DMC_dev branch contained changes to SimulationPulseLibraryManager.cxx
          and should have been merged before the v5.6.0 release.  These changes included minor
	  error checking and print statements and a small functionality change to "support a
	  detector map not starting with zip1."
 
Type:     bugfix

Implementation: "hotfix" in Git Flow protocol
 
Notes: 

Release: v5.6.0
===============
 
 
Overview: Includes new detector types kHVSNOlab (710) and kHVUMN (1710) as well as detector
          modifications to use these data types with CDMS II cold hardware and RevC DCRCs. 
 
Purpose:  The kHVSNOlab detector type would have had to be introduced eventually and it was done
          now to accommodate UMN detector tests.  Because of the "special" one-sided readout of
	  high-voltage-biased detectors at UMN we also needed one detector type to represent an HV
	  detector with just one side of phonon channels read out, and the corresponding mapping. 
 
Type:     Minor increment (as opposed to bugfix or Major increment)
 
Notes: 

Release: v5.5.0
===============
 
 
Overview: Includes data-reading updates for the backport of the RevC DCRC DAQ code. 
 
Purpose: 
 
Type:     Minor increment (as opposed to bugfix or Major increment)
 
Notes: 

Releases < v5.5.0
=================

Overview: these releases did not have detailed notes.  We've recently updated to using
          [gitflow](https://github.com/nvie/gitflow) and trying to stick with the [semantic
	  versioning](http://semver.org/) scheme.

R134 Test-Processing list requirements as of 2014/01/11

Low Background  data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
Less than 70000 events in a series
DQ Diagnosis of not Bad (aka questionable or good)

Calibration data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
DQ Diagnosis of not Bad (aka questionable or good)

For the first test processing:
   * Used EOR randoms to construct noise templates for all series
   * Used calibration & OF templates from R133 Prod v5.3
   * Used detectorStatus.SuperCDMS cvs version 1.11

The lists for the first (& 2nd) test processing are as follows:

   1) Cf cabliration w/ & w/o CDMSlite:
      testR134_listA_cf_EORRand.txt --> Jul 25, 2013 (2 series)
      testR134_listB_cf_z14lite_EORRand.txt --> Dec 13, 2013 (3 series)
   2nd test processing:
	testR134_listC_cf_z14lite_EORRand.txt --> Feb 6-7, 2014 (7 series)
		w/ iT5Z2 as CDMSlite w/ 70 V bias
		w/ iT5Z3 in reverse-bias configuration

   2) post-Cf Bg w/ & w/o CDMSlite:
      testR134_postcf_listA_bg_EORRand.txt --> Jul 25-27, 2013 (12 series)
      testR134_postcf_listB_bg_z14lite_EORRand.txt --> Dec 13-15, 2013 (18 series)
         -Note that post-Cf list B includes a variety of CDMSlite voltages:
            72 V -> 75 V  -> 72 V  -> 60 V-> 72 V  -> 69 V
            18 hr-> 5.5 hr-> 3.5 hr-> 8 hr-> 1.5 hr-> 5 hr
   2nd test processing:
	testR134_postcf_listC_bg_z14lite_EORRand.txt --> Feb 7-10, 2014 (21 series)
		w/ iT5Z2 as CDMSlite w/ 70 V bias
		w/ iT5Z3 in reverse-bais configuration

   3) CDMSlite-only Bg data w/ T5Z2 as CDMSlite at 60 V
      testR134_cdmsliteonly_bg_z14lite_EORRand.txt
        -129 series from Dec 21, 2013 -> Jan 11, 2014
        -Special processing w/ no information for non-CDMSlite detectors
        -Intended to obtain early idea of gain stability
   2nd test processing:
	testR134_cdmsliteonly_listB_bg_z14lite_EORRand.txt --> Feb 7-15, 2014 (51 series)
		w/ iT5Z2 as CDMSlite w/ 70 V bias
		w/ iT5Z3 in reverse-bias configuration

   4) Reverse-bias Ba data w/ & w/o CDMSlite:
      testR134_reversebias_ba_EORRand.txt
        -7 series from July 30, 2013 to Nov 13, 2013
      testR134_reversebias_ba_z14lite_EORRand.txt
        -3 series from Jan 10, 2014 w/ T5Z2 60V CDMSlite

   5) Random sample of normal-bias Ba data w/ & w/o CDMSlite:
      testR134_randsample_ba_EORRand.txt
        -35 series from Jul 26 to Dec 2, 2013
        -Note that T3Z1&3 were reverted from reverse- to normal-bias operation on Oct 9, 2013
          * Thus, the first 17 series from 01130726_1135 to 01131007_1122 have reverse-bias T3Z1&3
          * And, the last 18 series from 01131011_1034 to 01131202_0817 are normal bias for all detectors
      testR134_randsample_ba_z14lite_EORRand.txt
        -35 series from Dec 26, 2013 to Mar 24, 2014
        -T5Z2 60 & 70 V CDMSlite


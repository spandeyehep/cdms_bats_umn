R135 Test-Processing list requirements as of 2014/09/25

Low Background  data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
Any DQ diagnosis

Ba data must satisfy:
All 15 iZIPs configured
More than 1000 events in a series
Not bad DQ diagnosis

For the first, 2nd & 3rd test processings:
   * Used EOR randoms to construct noise templates for all series
   * Use R134 cdmsbats production code & same detector Ch. statuses
  

The lists for the first test processing are as follows:

   1) post-Cf Bg w/ T5Z2 CDMSlite:
      testR135_postcf_listA_bg_z14lite_EORRand.txt --> Sep 23-25, 2014 (16 series)
         -Note that post-Cf list A includes T5Z2 CDMSlite at 70 V
	 -T5Z3 is in reverse bias mode (-/+2V polarity)

The lists for the 2nd test processing are (for R135 enegy calibration):

   2a) random ba sample:
       testR135_randsample_ba_EORRand.txt
         -All detectors in normal +/-2V mode
         -9 series from late Sep 2014 (after neutralization achieved)
   2b) random ab sample w/ T5Z2 CDMSlite:
       testR135_randsample_ba_z14lite_EORRand.txt
         -T5Z2 at 70 V; T5Z3 in reverse bias mode
         -10 series from Oct 2014
   

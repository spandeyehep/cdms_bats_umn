R135 Processing list requirements as of 2015/02/05

Calibration data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
DQ Diagnosis of not Bad (aka questionable or good)

Additionally, we will use EOR randoms by default; thus all lists will
end with "EORRand.txt"

NOTE:  these are the original lists.  Problems related to the production
       may result in some series being split out into additional lists.
       In particular, if there are EORRand issues, short split-off lists may
       be created to use the BORRand.

The Cf lists are:

	1) prodR135_1409a_cf_EORRand.txt
		- 20 series (no CDMSlite)
                - from Sep 16 to 19, 2014
	2) prodR135_1409b_cf_z14lite_EORRand.txt
		- 21 series w/ T5Z2 CDMSlite
                - all but last 3 have T5Z2 at 0 V bias
                - all series should have T5Z3 in reverse-bias polarity
	3) prodR135_1410a_cf_z14lite_EORRand.txt
		- 10 series w/ T5Z2 CDMSlite
		- special source-location calibration series
	4) prodR135_1411a_cf_z14lite_EORRand.txt
		- 24 series w/ T5Z2 CDMSlite
		- speical source-location calibration series
	5) prodR135_1501a_cf_z14lite_EORRand.txt
		- 16 series w/ T5Z2 CDMSlite
		- extra NRs for trig eff estimate
	6) prodR135_1502a_cf_z14lite_EORRand.txt
		- 16 series w/ T5Z2 CDMSlite
		- extra NRs for trig eff estimate

Cs list:

	1) prodR135_1410a_cs_z14lite_EORRand.txt
		- 9 series w/ T5Z2 CDMSlite
		- special source-location calibration series

Co list:

	1) prodR135_1410a_co_z14lite_EORRand.txt
		- 23 series w/ T5Z2 CDMSlite
		- special source-location calibration series

Ba lists:

	1) prodR135_1409a_ba_EORRand.txt
		- 9 series
		- includes early-run poor neutralizaiton
	2) prodR135_1410a_ba_z14lite_EORRand.txt
		- 9 series with T5Z2 CDMSlite
	3) prodR135_1412a_ba_EORRand.txt
		- 2 series
		- prior to high-stats selective readout change to DAQ
	4) prodR135_1412b_ba_EORRand.txt
		- 87 series
		- w/ higher-stats selective readout change to DAQ
	5) prodR135_1501a_ba_EORRand.txt
		- 80 series
		- w/ higher-stats selective readout change to DAQ

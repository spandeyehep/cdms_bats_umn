R134 Processing list requirements as of 2014/03/05

Calibration data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
DQ Diagnosis of not Bad (aka questionable or good)

Additionally, we will use EOR randoms by default; thus all lists will
end with "EORRand.txt"

NOTE:  these are the original lists.  Problems related to the production
       have resulted in some series being split out into additional lists.

The Cf lists are:

	1) prodR134_1307a_cf_EORRand.txt
		- 2 series (no CDMSlite)
		- 1st series w/ all dets in normal bias
		- 2nd series w/ T3Z1&3 in reverse bias
	2) prodR134_1312a_cf_z14lite_EORRand.txt
		- 3 series w/ T5Z2 at 60 V
	3) prodR134_1402a_cf_z14lite_EORRand.txt
		- 7 series w/ T5Z2 at 70 V & T5Z3 in reverse bias
	4) prodR134_1405a_cf_z14lite_EORRand.txt
		- 13 series w/ T5Z2 at 70 V & T5Z3 in reverse bias
	5) prodR134_1407a_cf_z14lite_EORRand.txt
		- 15 series w/ T5Z2 at 70 V
		- lots of cryocooler noise

The reverse-bias Ba lists are:

	1) prodR134_a_reversebias_ba_EORRand.txt
		- 7 series (no CDMSlite)
	2) prodR134_b_reversebias_ba_z14lite_EORRand.txt
		- 10 series w/ T5Z2 as CDMSlite 60 & 70 V
		- including up to 1140401_1205
	3) prodR134_c_reversebias_ba_z14lite_EORRand.txt
		- 2 series w/ T5Z2 as CDMSlite 70 V
		- series 01140430_0808, 01140430_1132
	4) prodR134_d_reversebias_ba_z14lite_EORRand.txt
		- 3 series w/ T5Z2 as CDMSlite 70 V

The normal-bias Ba lists are:

	1) prodR134_1307a_ba_EORRand.txt
		- 18 series
	2) prodR134_1308a_ba_EORRand.txt
		- 27 series
	3) prodR134_1309a_ba_EORRand.txt
		- 27 series
	4) prodR134_1310a_ba_EORRand.txt
		- 29 series
	5) prodR134_1311a_ba_EORRand.txt
		- 24 series
	6) prodR134_1312a_ba_EORRand.txt
		- 1 series
	7) prodR134_1312b_ba_z4lite_z14lite_EORRand.txt
		- 1 series w/ T2Z1 & T5Z2 as CDMSlite
	8) prodR134_1312c_ba_z14lite_EORRand.txt
		- 15 (16->15) series w/ T5Z2 as CDMSlite
	8.1) prodR134_1312d_ba_z14lite.txt
		- 1 series from 1312c_ba using BORRand w/ T5Z2 as CDMSlite	
	9) prodR134_1401a_ba_z14lite_EORRand.txt
		- 32 series w/ T5Z2 as CDMSlite
	10) prodR134_1402a_ba_z14lite_EORRand.txt
		- 19 series w/ T5Z2 as CDMSlite
	11) prodR134_1403a_ba_z14lite_EORRand.txt
		- 27 series w/ T5Z2 as CDMSlite
	12) prodR134_1404a_ba_z14lite_EORRand.txt
		- 12 series w/ T5Z2 as CDMSlite
		- including up to 1140416_1422
	13) prodR134_1404b_ba_z14lite_EORRand.txt
		- 14 series w/ T5Z2 as CDMSlite
		- including up to 1140430_1132
	14) prodR134_1405a_ba_z14lite_EORRand.txt
		- 5 series w/ T5Z2 as CDMSlite
		- including up to 1140509_0818
	15) prodR134_1405b_ba_z14lite_EORRand.txt
		- 12 series w/ T5Z2 as CDMSlite
		- from 1140512_0808 to 1140530_1113
	16) prodR134_1406a_ba_z14lite_EORRand.txt
		- 25 series w/ T5Z2 as CDMSlite
		- from 1140602_0828 to 1140630_1226
	17) prodR134_1407a_ba_z14lite_EORRand.txt
		- 8 series w/ T5Z2 as CDMSlite
		- from 1140702_0849 to 1140711_0918
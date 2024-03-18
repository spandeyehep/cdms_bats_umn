R134 Processing list requirements as of 2014/03/09

Low Background  data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
Less than 40000 events in a series (lowered from R133 limit of 70,000)
	An exception was made for May 2014 and onward due to worsening LF noise
	Thus there are series w/ >40k events in the later processing lists.
DQ Diagnosis of not Bad (aka questionable or good)

Additionally, we will use EOR randoms by default; all lists will end with "EORRand.txt"

Series are divided ~ equally between 2 lists per month, where appropriate.

NOTE: the following lists are the originals.  Problems during the production necessitated
      breaking several series out into additional lists.

Jul 2013:
	
	prodR134_1307a_bg_EORRand.txt
		- 80 series from 01130716_1912 to 01130731_2217

Aug 2013:

	prodR134_1308a_bg_EORRand.txt
		- 96 series from 01130801_0134 to 01130816_0705
	prodR134_1308b_bg_EORRand.txt
		- 95 series from 01130816_1333 to 01130831_2150

Sep 2013:

	prodR134_1309a_bg_EORRand.txt
		- 96 series from 01130901_0107 to 01130916_0257
	prodR134_1309b_bg_EORRand.txt
		- 95 series from 01130916_0614 to 01130930_2221

Oct 2013:

	prodR134_1310a_bg_EORRand.txt
		- 99 series from 01131001_0138 to 01131015_2000
	prodR134_1310b_bg_EORRand.txt
		- 98 series from 01131015_2318 to 01131031_2315

Nov 2013:

	prodR134_1311a_bg_EORRand.txt
		- 94 series from 01131101_0232 to 01131116_1406
	prodR134_1311b_bg_EORRand.txt
		- 93 series from 01131116_1723 to 01131130_2140

Dec 2013:

	prodR134_1312a_bg_EORRand.txt
		- 22 series from 01131201_0057 to 01131206_0710
	prodR134_1312b_bg_noZ10_noZ14_EORRand.txt
		- 19 series from 01131206_2057 to 01131209_0414
		- T4Z1 & T5Z2 not biased, but noise traces still readout like normal 4V operation
	prodR134_1312c_bg_z14lite_EORRand.txt
		- 5 series from 01131209_1748 to 01131210_0726
		- T5Z2 as CDMSlite w/ 0 V bias
	prodR134_1312d_bg_z10lite_z14lite_EORRand.txt
		- 4 series from 01131210_1838 to 01131211_0549
		- T4Z1 as CDMSlite w/ 0 V bias
		- T5Z2 as CDMSlite w/ 70 V bias
	prodR134_1312e_bg_z4lite_z14lite_EORRand.txt
		- 5 series from 01131211_2037 to 01131212_0706
		- T2Z1 as CDMSlite w/ 10, 15 & 0 V biases
		- T5Z2 as CDMSlite w/ 66, 63, 60 & 57 V biases
	prodR134_1312f_bg_z14lite_EORRand.txt
		- 55 (56->55) series from 01131212_2348 to 01131221_1109
		- T5Z2 as CDMSlite w/ many biases (at least 8 --> 51,57,60,63,66,69,72,75)
	prodR134_1312g_bg_z14lite_EORRand.txt
		- 66 series from 01131221_1324 to 01131231_2048
		- T5Z2 as CDMSlite w/ 60 V bias
	prodR134_1312h_bg_z14lite.txt
		- 1 series(01131216_0035) from list 1312f_bg using BORRand.
		- T5Z2 as CDMSlite

Jan 2014:
	
	prodR134_1401a_bg_z14lite_EORRand.txt
		- 96 series from 01140101_0005 to 01140116_1235
		- T5Z2 as CDMSlite w/ 60 V bias
	prodR134_1401b_bg_z14lite_EORRand.txt
		- 95 (96->95) series from 01140116_1552 to 01140131_2311
		- T5Z2 as CDMSlite w/ 60 V bias
	prodR134_1401c_bg_z14lite.txt
		- 1 series(01140121_0230) from list 1401b_bg using BORRand.
		- T5Z2 as CDMSlite

Feb 2014:

	prodR134_1402a_bg_z14lite_EORRand.txt
		- 87 series from 01140201_0228 to 01140215_0448
		- T5Z2 as CDMSlite w/ 60 & 70 V bias
	prodR134_1402b_bg_z14lite_EORRand.txt
		- 87 series from 01140215_0805 to 01140228_2120
		- T5Z2 as CDMSlite w/ 70 V bias

Mar 2014:

	prodR134_1403a_bg_z14lite_EORRand.txt
		- 99 series from 01140301_0038 to 01140316_1521
		- T5Z2 as CDMSlite w/ 70 V bias
	prodR134_1403b_bg_z14lite_EORRand.txt
		- 94 series from 01140316_1838 to 01140331_1813
		- T5Z2 as CDMSlite w/ 70 V bias

Apr 2014:

	prodR134_1404a_bg_z14lite_EORRand.txt	
		- 99 series from 01140401_0144 to 01140416_2334
		- T5Z2 as CDMSlite w/ 70 V bias
	prodR134_1404b_bg_z14lite_EORRand.txt
		- 83 series from 01140417_0251 to 01140430_2125
		- T5Z2 as CDMSlite w/ 70 V bias

May 2014:

	prodR134_1405a_bg_z14lite_EORRand.txt
		- 30 series from 01140501_0042 to 01140507_0447
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May
	prodR134_1405b_bg_z14lite_EORRand.txt
		- 76 series from 01140507_1647 to 01140519_0618
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May
	prodR134_1405d_bg_z14lite_EORRand.txt
		- 79 series from 01140519_1636 to 01140531_2314
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May
	prodR134_1406a_bg_z14lite_EORRand.txt
		- 87 series from 01140601_0231 to 01140615_2245
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May
	prodR134_1406b_bg_z14lite_EORRand.txt
		- 89 series from 01140616_0202 to 01140630_2342
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May
	prodR134_1407a_bg_z14lite_EORRand.txt
		- 88 series from 01140701_0259 to 01140714_0531
		- T5Z2 as CDMSlite w/ 70 V bias
		- 40,000 event max relaxed to study LF-noise outbreak during 1st week of May

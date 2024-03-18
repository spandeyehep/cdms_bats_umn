R135 Processing list requirements as of 2015/02/05

Low Background  data must satisfy the following:
All 15 iZIPs configured
More than 1000 events in a series
Less than 70k events in a series (same as R133 limit)
DQ Diagnosis of not Bad (aka questionable or good)

Additionally, we will use EOR randoms by default; all lists will end with "EORRand.txt"

Series are divided ~ equally between 2 lists per month, where appropriate.

NOTE: the following lists are the originals.  Problems during the production necessitated
      breaking several series out into additional lists.  When this happens, the offending 
      series are removed from the original list and placed into a new list w/o the "EORRand" 
      suffix (such that BORRand are used instead).

Sep 2014
	
	1) prodR135_1409a_bg_z14lite_EORRand.txt
		- 55 series from 01140922_1709 to 01140930_0435
		- T5Z2 as CDMSlite w/ T5Z3 in reverse polarity
		- Primarily useful for post-Cf activation lines
	2) prodR135_1409b_bg_EORRand.txt
		- 28 series from 01140910_1708 to 01140916_0641
		- chronologically out of order relative to list 1409a_bg
		- includes early-run poor neutralization data
		- No T5Z2 dib2 triggers

Oct 2014

	1) prodR135_1410a_bg_z14lite_EORRand.txt
		- 89 series from 01141001_2144 to 01141015_0256
		- T5Z2 as CDMSlite w/ T5Z3 in reverse polarity
		- Primarily useful for post-Cf activation lines
	2) prodR135_1410b_bg_z14lite_EORRand.txt
		- 37 series from 01141015_1649 to 01141031_0603
		- T5Z2 as CDMSlite w/ T5Z3 in reverse polarity
		- Primarily useful as additional post-Cf data, but may be used for CDMSlite WIMP search?
		- Includes some HV experimentation (ie, HV not always =70V)

Nov 2014

	1) prodR135_1411a_bg_z14lite_EORRand.txt
		- 34 series from 01141104_0945 to 01141110_0631
		- T5Z2 as CDMSlite w/ T5Z3 in reverse polarity
		- may include intentionally induced glitches & other electronic noise

Dec 2014

	1) prodR135_1412a_bg_EORRand.txt
		- 13 series from 01141217_1554 to 01141219_1009
		- subsequent to incident recovery?
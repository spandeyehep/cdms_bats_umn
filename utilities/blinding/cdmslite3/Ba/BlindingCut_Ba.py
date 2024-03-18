import sys
sys.path.append( '..' )

from BlindingCut import *


class BlindingCut_Ba( BlindingCut ):

    def load_data( self ):
        path = '/tera2/data1/cdmsbatsProd/R135/Prodv5-4_cdmslite/merged/all/ba/'
        
        if 0 < self.load_detector_data( 'RRQs', path+'calib_Prodv5-4_*.root:rrqDir/calibzip*', [ 'ptNF' ] ): return 1
        
        if 0 < self.load_global_data( 'RQs', path+'merge_Prodv5-4_*.root:rqDir/eventTree', [ 'EventCategory' ] ): return 2

        return 0

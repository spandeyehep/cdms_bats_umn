from math import *
from ROOT import *


from BlindingCut import *




class BlindingCut_Cf( BlindingCut ):

    def load_data( self ):
        self.load_detector_data( 'RQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/cf/merge_Prodv5-3_*.root:rqDir/zip*', [ 'DetType'          ,
                                                                                                                                  'PAOFamps'         ,
                                                                                                                                  'PBOFamps'         ,
                                                                                                                                  'PCOFamps'         ,
                                                                                                                                  'PDOFamps'         ,
                                                                                                                                  'PTglitch1OFchisq' ,
                                                                                                                                  'PTlfnoise1OFchisq',
                                                                                                                                  'PTOFchisq'        ,
                                                                                                                                  'QIS1bias'         ,
                                                                                                                                  'QIS2bias'         ,
                                                                                                                                  'QIS1status'       ,
                                                                                                                                  'QIS2status'       ,
                                                                                                                                  'QOS1status'       ,
                                                                                                                                  'QOS2status'        ] )

        self.load_detector_data( 'RRQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/cf/calib_Prodv5-3_*.root:rrqDir/calibzip*', [ 'pa2OF'       ,
                                                                                                                                         'pc1OF'       ,
                                                                                                                                         'precoilsumOF',
                                                                                                                                         'psumOF'      ,
                                                                                                                                         'ptNF'        ,
                                                                                                                                         'qi1OF'       ,
                                                                                                                                         'qi2OF'       ,
                                                                                                                                         'qo1OF'       ,
                                                                                                                                         'qo2OF'       ,
                                                                                                                                         'qsum1OF'     ,
                                                                                                                                         'qsum2OF'     ,
                                                                                                                                         'ysumOF'       ] )

        self.load_global_data( 'RQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/cf/merge_Prodv5-3_*.root:rqDir/eventTree', [ 'EventCategory',        
                                                                                                                                     'SeriesNumber' ,
                                                                                                                                     'VTTime20'      ] )
        
        self.load_global_data( 'RRQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/cf/calib_Prodv5-3_*.root:rrqDir/calibevent', [ 'ntrigp',
                                                                                                                                        'ntrigq' ] )

        return 0

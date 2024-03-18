from math import *
from ROOT import *


from BlindingCut import *
from Graph       import *




class BlindingCheck_randoms( BlindingCut ):

    def __init__( self, detector_names ):
        BlindingCut.__init__( self, detector_names )

        self.graphs = {}

        for parameter_name in self.noise_thresholds.keys(): self.graphs[ parameter_name ] = [ [ Graph() ] for detector_index in self.detector_indices ]

        for cut_name in [ 'cPostCf2'     ,
                          'cBiasPM'      ,
                          'cGroundDIB'   ,
                          'cPbSource'    ,
                          'cHasValidRRQs',
                          'cBlind_LT'     ]: self.graphs[ cut_name ] = [ [ Graph(),
                                                                           Graph() ] ]
        
        self.time_shift = ( 1.3306795-0.54173592 )*1.e9

        self.number_of_series = 0
        self.times = []
        
        self.counts = { 'all'        : [ [] ]                                            ,
                        'cPsignal'   : [ [] ]                                            ,
                        'cPsignalDet': [ [] for detector_index in self.detector_indices ] }


    def load_data( self ):

        # The original definition of this function was changed in order to not show explicitly the actual location of the restricted low-background data #
        # This version is loading low-background data already blinded instead                                                                            #
        # Elias Lopez Asamar, June 28th 2013                                                                                                             #

        self.load_detector_data( 'RQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/bg_permitted/merge_Prodv5-3_*.root:rqDir/zip*', [ 'DetType'          ,
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
        
        self.load_detector_data( 'RRQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/bg_permitted/calib_Prodv5-3_*.root:rrqDir/calibzip*', [ 'pa2OF'       ,
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
        
        self.load_global_data( 'RQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/bg_permitted/merge_Prodv5-3_*.root:rqDir/eventTree', [ 'EventCategory',
                                                                                                                                               'EventTime'    ,
                                                                                                                                               'SeriesNumber' ,
                                                                                                                                               'VTTime20'      ] )
                                                                                                                                                    
        self.load_global_data( 'RRQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/bg_permitted/calib_Prodv5-3_*.root:rrqDir/calibevent', [ 'ntrigp',
                                                                                                                                                  'ntrigq' ] )
        
        return 0


    def process_event( self, event_index ):
        time = self.global_data_chains[ 'RQs' ].EventTime-self.time_shift

        self.counts[ 'all' ][ 0 ][ -1 ] += 1

        decisions = self.cxPsignal_blind_133( event_index )
        if True in decisions: self.counts[ 'cPsignal' ][ 0 ][ -1 ] += 1

        for detector_index in self.detector_indices:
            if decisions[ detector_index ]: self.counts[ 'cPsignalDet' ][ detector_index ][ -1 ] += 1

        for detector_index in self.detector_indices:
            if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == -999999.: continue
            
            for parameter_name in self.noise_thresholds.keys(): self.graphs[ parameter_name ][ detector_index ][ 0 ].SetPoint( time                                                    ,
                                                                                                                               self.noise_thresholds[ parameter_name ][ detector_index ] )
            
            self.graphs[ 'cPostCf2' ][ 0 ][ int( not self.cPostCf2_blind_133( event_index ) ) ].SetPoint( time            ,
                                                                                                          detector_index+1 )
            
            self.graphs[ 'cGroundDIB' ][ 0 ][ int( not self.cGroundDIB_blind_133( event_index, detector_index ) ) ].SetPoint( time            ,
                                                                                                                              detector_index+1 )
            
            self.graphs[ 'cPbSource' ][ 0 ][ int( not self.cPbSource_blind_133( event_index, detector_index ) ) ].SetPoint( time            ,
                                                                                                                            detector_index+1 )
            
            self.graphs[ 'cHasValidRRQs' ][ 0 ][ int( self.cHasValidRRQs_blind_133( event_index, detector_index ) ) ].SetPoint( time            ,
                                                                                                                                detector_index+1 )
            
            if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == 21.: continue
            
            self.graphs[ 'cBiasPM' ][ 0 ][ int( self.cBiasPM_blind_133( event_index, detector_index ) ) ].SetPoint( time            ,
                                                                                                                    detector_index+1 )
            
            self.graphs[ 'cBlind_LT' ][ 0 ][ int( not self.cBlind_133_LT( event_index, detector_index ) ) ].SetPoint( time            ,
                                                                                                                      detector_index+1 )
        return 0


    def create_plots( self ):
        gROOT.SetStyle( 'Plain' )

        gStyle.SetTitleX( .4 )
        gStyle.SetTitleBorderSize( 0 )
        gStyle.SetOptStat( 0 )
                            
        canvas = TCanvas()
        canvas.SetBottomMargin( 1.4*canvas.GetBottomMargin() )        
        
        graph = Graph()

        for series_index in range( self.number_of_series ):
            if 0 < self.counts[ 'all' ][ 0 ][ series_index ]:
                graph.SetPoint( self.times[ series_index ], float( self.counts[ 'cPsignal' ][ 0 ][ series_index ] )/float( self.counts[ 'all' ][ 0 ][ series_index ] ) )

        graph.SetMarkerColor( 4 )
        
        if 0 < graph.GetN():
            graph.SetMarkerStyle( 20 )
            graph.SetMarkerSize( .2 )
            
            graph.SetTitles( 'Time (mm/dd/yyyy)', 'Fraction of events blinded' )
            
            graph.GetXaxis().SetTimeDisplay( 1 )
            graph.GetXaxis().SetTimeFormat( '%m/%d/%Y' )
            graph.GetXaxis().SetNdivisions( 5, 0, 0, False )
            graph.GetXaxis().SetLabelOffset( .018 )
            graph.GetXaxis().SetTitleOffset( 1.32 )
            
            graph.SetXAxisLimits( 540000000., 578000000. )
            graph.SetYAxisLimits( 0., .4 )
            
            graph.Draw( 'AP' )
            
            canvas.SaveAs( 'png/cPsignal.png' )

        for detector_index in self.detector_indices:
            graph = Graph()
            
            for series_index in range( self.number_of_series ):
                if 0 < self.counts[ 'all' ][ 0 ][ series_index ]:
                    graph.SetPoint( self.times[ series_index ], float( self.counts[ 'cPsignalDet' ][ detector_index ][ series_index ] )/ float( self.counts[ 'all' ][ 0 ][ series_index ] ) )

            graph.SetMarkerColor( 4 )
                
            if 0 < graph.GetN():
                graph.SetMarkerStyle( 20 )
                graph.SetMarkerSize( .2 )
                
                graph.SetTitle( 'IT'+str( ( detector_index/3 )+1 )+'Z'+str( ( detector_index%3 )+1 ) )
                graph.SetTitles( 'Time (mm/dd/yyyy)', 'Fraction of events blinded' )
                
                graph.GetXaxis().SetTimeDisplay( 1 )
                graph.GetXaxis().SetTimeFormat( '%m/%d/%Y' )
                graph.GetXaxis().SetNdivisions( 5, 0, 0, False )
                graph.GetXaxis().SetLabelOffset( .018 )
                graph.GetXaxis().SetTitleOffset( 1.32 )
                
                graph.SetXAxisLimits( 540000000., 578000000. )
                graph.SetYAxisLimits( 0., .08 )
                
                graph.Draw( 'AP' )
                
                canvas.SaveAs( 'png/cPsignalDet_'+str( detector_index+1 )+'.png' )
                
        for parameter_name in self.noise_thresholds.keys():
            for detector_index in self.detector_indices:
                graph = self.graphs[ parameter_name ][ detector_index ][ 0 ]
                graph.SetMarkerColor( 4 )
                
                if 0 < graph.GetN():
                    graph.SetMarkerStyle( 20 )
                    graph.SetMarkerSize( .2 )
                    
                    graph.SetTitle( 'IT'+str( ( detector_index/3 )+1 )+'Z'+str( ( detector_index%3 )+1 ) )
                    graph.SetTitles( 'Time (mm/dd/yyyy)', 'Energy ['+parameter_name.split( '_' ).__getitem__( 1 )+'] (keV)' )
                    
                    graph.GetXaxis().SetTimeDisplay( 1 )
                    graph.GetXaxis().SetTimeFormat( '%m/%d/%Y' )
                    graph.GetXaxis().SetNdivisions( 5, 0, 0, False )
                    graph.GetXaxis().SetLabelOffset( .018 )
                    graph.GetXaxis().SetTitleOffset( 1.32 )
                    
                    graph.SetXAxisLimits( 540000000., 578000000. )
                    graph.SetYAxisLimits( 0., 15. )
                    
                    graph.Draw( 'AP' )

                canvas.SaveAs( 'png/'+parameter_name+'_'+str( detector_index+1 )+'.png' )
                
        for cut_name in [ 'cPostCf2'     ,
                          'cBiasPM'      ,
                          'cGroundDIB'   ,
                          'cPbSource'    ,
                          'cHasValidRRQs',
                          'cBlind_LT'     ]:
            self.graphs[ cut_name ][ 0 ][ 0 ].SetMarkerColor( 3 )
            self.graphs[ cut_name ][ 0 ][ 1 ].SetMarkerColor( 2 )

            plot_status = 0
            
            for graph in self.graphs[ cut_name ][ 0 ]:
                if 0 < graph.GetN():
                    graph.SetMarkerStyle( 20 )
                    graph.SetMarkerSize( .2 )

                    if plot_status == 0:
                        graph.SetTitles( 'Time (mm/dd/yyyy)', 'Detector number' )
                        
                        graph.GetXaxis().SetTimeDisplay( 1 )
                        graph.GetXaxis().SetTimeFormat( '%m/%d/%Y' )
                        graph.GetXaxis().SetNdivisions( 5, 0, 0, False )
                        graph.GetXaxis().SetLabelOffset( .018 )
                        graph.GetXaxis().SetTitleOffset( 1.32 )

                        graph.SetXAxisLimits( 540000000., 578000000. )
                        graph.SetYAxisLimits( 0., 16. )
                        
                        graph.Draw( 'AP' )
                        plot_status = 1

                    else: graph.Draw( 'P'  )

            if 0 < plot_status: canvas.SaveAs( 'png/'+cut_name+'.png' )
            
        return 0


    def run( self, sample_names, step = 1 ):
        self.sample_names = sample_names

        if 0 < self.load_data(): return 1
        if 0 < self.check_data(): return 2

        first = True
        
        for event_index in range( 0, self.number_of_events, step ):
            if first:
                if 0 < self.read_event( event_index ): return 3
                if 0 < self.set_parameters(): return 4

                self.number_of_series += 1
                self.times.append( self.global_data_chains[ 'RQs' ].EventTime-self.time_shift )
                
                self.counts[ 'all'      ][ 0 ].append( 0 )
                self.counts[ 'cPsignal' ][ 0 ].append( 0 )
                
                for detector_index in self.detector_indices: self.counts[ 'cPsignalDet' ][ detector_index ].append( 0 )
                
                first = False

            else:
                series_number = self.global_data_chains[ 'RQs' ].SeriesNumber
                if 0 < self.read_event( event_index ): return 3
                                
                if not self.global_data_chains[ 'RQs' ].SeriesNumber == series_number:
                    if 0 < self.set_parameters(): return 4

                    self.number_of_series += 1
                    self.times.append( self.global_data_chains[ 'RQs' ].EventTime-self.time_shift )
                    
                    self.counts[ 'all'      ][ 0 ].append( 0 )
                    self.counts[ 'cPsignal' ][ 0 ].append( 0 )
                    
                    for detector_index in self.detector_indices: self.counts[ 'cPsignalDet' ][ detector_index ].append( 0 )

            if self.global_data_chains[ 'RQs' ].EventCategory == 1.:
                if 0 < self.process_event( event_index ): return 5

            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        return 0


    def cxPsignal_blind_133( self, event_index ):
        decisions = [ True for detector_index in self.detector_indices ]        
        
        # Checking detector-by-detector requirements #

        for detector_index in self.detector_indices:
            
            # Not blinding events from detectors operating in CDMSlite mode #
            
            if not self.cHasValidRRQs_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events below phonon energy threshold #
            
            if not self.cPmin_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events due to LF noise #

            if self.cLFnoise1_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue
                
            # Not blinding glitch-like events (chi-square based) #
        
            if self.cGlitch1_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding multiple-scattering events #

            if self.cPmult_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

        return decisions

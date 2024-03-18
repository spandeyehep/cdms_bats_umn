from math import *
from ROOT import *


from AlgorithmBase import *
from Graph         import *




class BlindingCheck_final( AlgorithmBase ):

    def __init__( self, detector_names ):
        AlgorithmBase.__init__( self, detector_names )

        self.time_shift = ( 1.3306795-0.54173592 )*1.e9
        
        self.number_of_series = 0
        self.times = []
        
        self.counts = { 'all'      : [ [] ]                                            ,
                        'cBlind'   : [ [] ]                                            ,
                        'cBlindDet': [ [] for detector_index in self.detector_indices ] }


    def load_data( self ):

        # The original definition of this function was changed in order to not show explicitly the actual location of the restricted low-background data #
        # This version is loading low-background data already blinded instead                                                                            #
        # Elias Lopez Asamar, June 28th 2013                                                                                                             #

        self.load_detector_data( 'cBlind_133', '/galbascratch/elias/blinding_decisions/cBlind_133_*.root:cutDir/cutzip*', [ 'cBlind_133' ] )

        self.load_global_data( 'RQs', '/galbadata/R133/dataReleases/Prodv5-3/merged/all/bg_permitted/merge_Prodv5-3_*.root:rqDir/eventTree', [ 'EventTime'    ,
                                                                                                                                               'SeriesNumber'  ] )

        self.load_global_data( 'cBlind_133', '/galbascratch/elias/blinding_decisions/cBlind_133_*.root:cutDir/cutevent', [ 'cBlind_133' ] )
        
        return 0


    def process_event( self, event_index ):
        self.counts[ 'all' ][ 0 ][ -1 ] += 1
        if self.global_data_chains[ 'cBlind_133' ].cBlind_133 == 1.: self.counts[ 'cBlind' ][ 0 ][ -1 ] += 1
        
        for detector_index in self.detector_indices:
            if self.detector_data_chains[ 'cBlind_133' ][ detector_index ].cBlind_133 == 1.: self.counts[ 'cBlindDet' ][ detector_index ][ -1 ] += 1

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
                graph.SetPoint( self.times[ series_index ], float( self.counts[ 'cBlind' ][ 0 ][ series_index ] )/float( self.counts[ 'all' ][ 0 ][ series_index ] ) )

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
            graph.SetYAxisLimits( 0., .2 )
            
            graph.Draw( 'AP' )
            
            canvas.SaveAs( 'png/cBlind.png' )

        for detector_index in self.detector_indices:
            graph = Graph()
            
            for series_index in range( self.number_of_series ):
                if 0 < self.counts[ 'all' ][ 0 ][ series_index ]:
                    graph.SetPoint( self.times[ series_index ], float( self.counts[ 'cBlindDet' ][ detector_index ][ series_index ] )/ float( self.counts[ 'all' ][ 0 ][ series_index ] ) )

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
                
                canvas.SaveAs( 'png/cBlindDet_'+str( detector_index+1 )+'.png' )

        return 0

    def run( self, sample_names, step = 1 ):
        self.sample_names = sample_names
        
        if 0 < self.load_data(): return 1
        if 0 < self.check_data(): return 2

        first = True
        
        for event_index in range( 0, self.number_of_events, step ):
            if first:
                if 0 < self.read_event( event_index ): return 3
                
                self.number_of_series += 1
                self.times.append( self.global_data_chains[ 'RQs' ].EventTime-self.time_shift )
                
                self.counts[ 'all'    ][ 0 ].append( 0 )
                self.counts[ 'cBlind' ][ 0 ].append( 0 )
                
                for detector_index in self.detector_indices: self.counts[ 'cBlindDet' ][ detector_index ].append( 0 )

                first = False

            else:
                series_number = self.global_data_chains[ 'RQs' ].SeriesNumber
                if 0 < self.read_event( event_index ): return 3
                                
                if not self.global_data_chains[ 'RQs' ].SeriesNumber == series_number:
                    self.number_of_series += 1
                    self.times.append( self.global_data_chains[ 'RQs' ].EventTime-self.time_shift )
                    
                    self.counts[ 'all'    ][ 0 ].append( 0 )
                    self.counts[ 'cBlind' ][ 0 ].append( 0 )
                    
                    for detector_index in self.detector_indices: self.counts[ 'cBlindDet' ][ detector_index ].append( 0 )
                    
            if 0 < self.process_event( event_index ): return 5
            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        if 0. < sum( self.counts[ 'all' ][ 0 ] ): print str( 100.*sum( self.counts[ 'cBlind' ][ 0 ] )/( sum( self.counts[ 'all' ][ 0 ] ) ) )+'%'

        for detector_index in self.detector_indices:
            if 0. < sum( self.counts[ 'all' ][ 0 ] ): print detector_index+1, str( 100.*sum( self.counts[ 'cBlindDet' ][ detector_index ] )/( sum( self.counts[ 'all' ][ 0 ] ) ) )+'%'
        
        return 0

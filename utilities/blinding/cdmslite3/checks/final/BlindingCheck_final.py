import sys
sys.path.append( '../..' )

from ROOT import gROOT, gStyle, TCanvas

from AlgorithmBase import AlgorithmBase
from Graph         import Graphs


class BlindingCheck_final( AlgorithmBase ):

    def __init__( self, detector_names ):
        AlgorithmBase.__init__( self, detector_names )

        self.time_shift = ( 1.3306795-0.54173592 )*1.0e9
        
        self.number_of_series = 0
        self.times = []
        
        self.counts = { 'all'   : [],
                        'cBlind': [] }


    def load_data( self ):
        path = '/tera2/data1/cdmsbatsProd/R135/Prodv5-4_cdmslite/merged/all/bg_restricted/'
        
        if 0 < self.load_global_data( 'RQs', path+'merge_Prodv5-4_*.root:rqDir/eventTree', [ 'EventTime '  ,
                                                                                             'SeriesNumber' ] ): return 1
        
        if 0 < self.load_global_data( 'cBlind_lite3', '../../root/cBlind_lite3_*.root:cutDir/cutevent', [ 'cBlind_lite3' ] ): return 2
        
        return 0


    def process_event( self, event_index ):
        self.counts[ 'all' ][ -1 ] += 1

        if self.global_data_chains[ 'cBlind_lite3' ].cBlind_lite3 == 1.0: self.counts[ 'cBlind' ][ -1 ] += 1
        
        return 0


    def create_plots( self ):
        gROOT.SetStyle( 'Plain' )

        gStyle.SetTitleX( 0.4 )
        gStyle.SetTitleBorderSize( 0 )
        gStyle.SetOptStat( 0 )
                            
        canvas = TCanvas()
        canvas.SetBottomMargin( 1.4*canvas.GetBottomMargin() )        
        
        graph = Graph()

        for series_index in range( self.number_of_series ):
            if 0 < self.counts[ 'all' ][ series_index ]:
                rate = float( self.counts[ 'cBlind' ][ series_index ] )/float( self.counts[ 'all' ][ series_index ] )
                graph.SetPoint( self.times[ series_index ], rate )

        graph.SetMarkerColor( 4 )
        
        if 0 < graph.GetN():
            graph.SetMarkerStyle( 20 )
            graph.SetMarkerSize( 0.4 )
            
            graph.SetTitles( 'Time (mm/dd/yyyy)', 'Fraction of events blinded' )
            
            graph.GetXaxis().SetTimeDisplay( 1 )
            graph.GetXaxis().SetTimeFormat( '%m/%d/%Y' )
            graph.GetXaxis().SetNdivisions( 5, 0, 0, False )
            graph.GetXaxis().SetLabelOffset( 0.018 )
            graph.GetXaxis().SetTitleOffset( 1.32 )
            
            #graph.SetXAxisLimits( 585000000.0, 612000000.0 )
            graph.SetYAxisLimits( 0.0, 0.2 )
            
            graph.Draw( 'AP' )
            
            canvas.SaveAs( 'png/cBlind.png' )

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
                
                first = False

            else:
                series_number = self.global_data_chains[ 'RQs' ].SeriesNumber
                if 0 < self.read_event( event_index ): return 3
                                
                if not self.global_data_chains[ 'RQs' ].SeriesNumber == series_number:
                    self.number_of_series += 1
                    self.times.append( self.global_data_chains[ 'RQs' ].EventTime-self.time_shift )
                    
                    self.counts[ 'all'    ][ 0 ].append( 0 )
                    self.counts[ 'cBlind' ][ 0 ].append( 0 )
                    
            if 0 < self.process_event( event_index ): return 5
            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        print self.counts[ 'all' ], self.counts[ 'cBlind' ]
        
        return 0

import sys
sys.path.append( '../..' )

from ROOT import gROOT, gStyle, TCanvas, TFile, TH1F

from BlindingCut import BlindingCut


class BlindingCheck_spectrum( BlindingCut ):

    def __init__( self, detector_names ):
        BlindingCut.__init__( self, detector_names )

        self.histograms = [ [ TH1F( 'cPmax_1_'+detector_name, '', 120, 0.0, 300.0 ),
                              TH1F( 'cPmax_2_'+detector_name, '', 120, 0.0, 300.0 ) ] for detector_name in detector_names ]
        
        self.counts = [ 0, 0, 0, 0 ]
        

    def load_data( self ):
        path = '/tera2/data1/cdmsbatsProd/R135/Prodv5-4_cdmslite/merged/all/bg_restricted/'
        
        if 0 < self.load_detector_data( 'RRQs', path+'calib_Prodv5-4_*.root:rrqDir/calibzip*', [ 'ptNF' ] ): return 1

        if 0 < self.load_global_data( 'RQs', path+'merge_Prodv5-4_*.root:rqDir/eventTree', [ 'EventCategory' ] ): return 2
        
        if 0 < self.load_global_data( 'cBlind_lite3', '../../root/cBlind_lite3_*.root:cutDir/cutevent', [ 'cBlind_lite3' ] ): return 3
        
        return 0


    def process_event( self, event_index ):
        decision = True in self.cxBlind_lite3( event_index )

        for detector_index in self.detector_indices:
            if int( decision ) == 0:
                self.histograms[ detector_index ][ 0 ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF )
        
                if self.global_data_chains[ 'RQs' ].EventCategory == 1.0: continue

                self.histograms[ detector_index ][ 1 ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF )
                
        self.counts[ int( decision ) ] += 1

        if not decision == self.global_data_chains[ 'cBlind_lite3' ].cBlind_lite3: self.counts[ 2+int( decision ) ] += 1
                
        return 0


    def create_plots( self ):
        gROOT.SetStyle( 'Plain' )

        gStyle.SetTitleX( 0.4 )
        gStyle.SetTitleBorderSize( 0 )
        gStyle.SetOptStat( 0 )
        
        canvas = TCanvas()
        canvas.SetLogy( 1 )
        canvas.SetBottomMargin( 1.4*canvas.GetBottomMargin() )        
        
        for detector_index in self.detector_indices:
            stream = TFile( 'objects.root', 'update' )

            for histogram in self.histograms[ detector_index ]:
                histogram.Write()

            stream.Close()

            for case_index in range( 2 ):
                histogram = self.histograms[ detector_index ][ case_index ]
                
                histogram.SetLineColor( 3 )
                histogram.SetTitle( 'IT'+str( ( detector_index/3 )+1 )+'Z'+str( ( detector_index%3 )+1 ) )
                histogram.GetXaxis().SetTitle( 'Phonon energy [ptNF] (keV)' )
                histogram.GetYaxis().SetTitle( 'Counts'                     )

                histogram.Draw( 'e' )
                
                canvas.SaveAs( 'png/'+histogram.GetName()+'.png' )

        canvas.SetLogy( 0 )
        
        return 0
        

    def run( self, sample_names, step = 1 ):
        status = BlindingCut.run( self, sample_names, step )
        print self.counts
            
        return status

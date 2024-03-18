from math import *
from ROOT import *


from BlindingCut import *
from Graph       import *




class BlindingCheck_Cf( BlindingCut ):

    def __init__( self, name, detector_names, mode_flag = 0 ):
        self.name = name

        BlindingCut.__init__( self, detector_names )
        self.mode_flag = mode_flag

        self.histograms = { 'cPmin'     : [ [ TH1F( 'cPmin_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ),
                                              TH1F( 'cPmin_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ) ] for detector_name in detector_names ]    ,
                            
                            'cLFnoise1' : [ [ TH1F( 'cLFnoise1_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, 0., 8. ),
                                              TH1F( 'cLFnoise1_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, 0., 8. ) ] for detector_name in detector_names ] ,
                            
                            'cGlitch1'  : [ [ TH1F( 'cGlitch1_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, 0., 16. ),
                                              TH1F( 'cGlitch1_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, 0., 16. ) ] for detector_name in detector_names ] ,
                            
                            'cPmult'    : [ [ TH1F( 'cPmult_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ),
                                              TH1F( 'cPmult_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ) ] for detector_name in detector_names ]   ,

                            'cBlind_LT' : [ [ TH1F( 'cBlind_LT_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ),
                                              TH1F( 'cBlind_LT_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, 0., 20. ) ] for detector_name in detector_names ],
                            
                            'cVT'       : [ [ TH1F( 'cVT_'+detector_name+'_0_'+name+'_'+str( mode_flag ), '', 200, -100., 100. ),
                                              TH1F( 'cVT_'+detector_name+'_1_'+name+'_'+str( mode_flag ), '', 200, -100., 100. ) ] for detector_name in detector_names ]   }
        
        self.graphs = {}

        for cut_name in [ 'cQin1'  ,
                          'cQin2'  ,
                          'cQsym'  ,
                          'cNR'    ,
                          'cGlitch' ]: self.graphs[ cut_name ] = [ [ Graph(),
                                                                     Graph() ] for detector_index in self.detector_indices ]
                          
        self.counts = [ 0, 0 ]
        

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


    def process_event( self, event_index ):
        if self.mode_flag == 0:
            decision = True in self.cxBlind_133( event_index )

            for detector_index in self.detector_indices:
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == 21.     : continue
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == -999999.: continue

                self.histograms[ 'cPmin' ][ detector_index ][ int( decision ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].psumOF )

                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTlfnoise1OFchisq
                self.histograms[ 'cLFnoise1' ][ detector_index ][ int( decision ) ].Fill( chisquare )
                
                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTglitch1OFchisq
                self.histograms[ 'cGlitch1' ][ detector_index ][ int( decision ) ].Fill( chisquare )

                self.histograms[ 'cBlind_LT' ][ detector_index ][ int( decision ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF )
                
                self.histograms[ 'cVT' ][ detector_index ][ int( decision ) ].Fill( self.global_data_chains[ 'RQs' ].VTTime20 )
                
                self.graphs[ 'cQin1' ][ detector_index ][ int( decision ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi1OF,
                                                                                      self.detector_data_chains[ 'RRQs' ][ detector_index ].qo1OF )

                self.graphs[ 'cQin2' ][ detector_index ][ int( decision ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi2OF,
                                                                                      self.detector_data_chains[ 'RRQs' ][ detector_index ].qo2OF )

                self.graphs[ 'cQsym' ][ detector_index ][ int( decision ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum1OF,
                                                                                      self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum2OF )
                
                self.graphs[ 'cNR' ][ detector_index ][ int( decision ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].precoilsumOF,
                                                                                    self.detector_data_chains[ 'RRQs' ][ detector_index ].ysumOF       )

                self.graphs[ 'cGlitch' ][ detector_index ][ int( decision ) ].SetPoint( self.global_data_chains[ 'RRQs' ].ntrigp,
                                                                                        self.global_data_chains[ 'RRQs' ].ntrigq )

            self.counts[ int( decision ) ] += 1
            
        elif self.mode_flag == 1: 
            decisions = self.cxBlind_133( event_index )
            
            for detector_index in self.detector_indices:
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == 21.     : continue
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == -999999.: continue

                self.histograms[ 'cPmin' ][ detector_index ][ int( decisions[ detector_index ] ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].psumOF )

                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTlfnoise1OFchisq
                self.histograms[ 'cLFnoise1' ][ detector_index ][ int( decisions[ detector_index ] ) ].Fill( chisquare )

                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTglitch1OFchisq
                self.histograms[ 'cGlitch1' ][ detector_index ][ int( decisions[ detector_index ] ) ].Fill( chisquare )

                decision = decisions.pop( detector_index ) 
                self.histograms[ 'cPmult' ][ detector_index ][ int( True in decisions ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].psumOF )
                decisions.insert( detector_index, decision ) 

                self.histograms[ 'cBlind_LT' ][ detector_index ][ int( decisions[ detector_index ] ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF )

                self.histograms[ 'cVT' ][ detector_index ][ int( decisions[ detector_index ] ) ].Fill( self.global_data_chains[ 'RQs' ].VTTime20 )

                self.graphs[ 'cQin1' ][ detector_index ][ int( decisions[ detector_index ] ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi1OF,
                                                                                                         self.detector_data_chains[ 'RRQs' ][ detector_index ].qo1OF )
                
                self.graphs[ 'cQin2' ][ detector_index ][ int( decisions[ detector_index ] ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi2OF,
                                                                                                         self.detector_data_chains[ 'RRQs' ][ detector_index ].qo2OF )
                
                self.graphs[ 'cQsym' ][ detector_index ][ int( decisions[ detector_index ] ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum1OF,
                                                                                                         self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum2OF )

                self.graphs[ 'cNR' ][ detector_index ][ int( decisions[ detector_index ] ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].precoilsumOF,
                                                                                                       self.detector_data_chains[ 'RRQs' ][ detector_index ].ysumOF       )
                
                self.graphs[ 'cGlitch' ][ detector_index ][ int( decisions[ detector_index ] ) ].SetPoint( self.global_data_chains[ 'RRQs' ].ntrigp,
                                                                                                           self.global_data_chains[ 'RRQs' ].ntrigq )

            self.counts[ int( True in decisions ) ] += 1

        elif self.mode_flag == 2:
            for detector_index in self.detector_indices:
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == 21.     : continue
                if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == -999999.: continue

                self.histograms[ 'cPmin' ][ detector_index ][ int( self.cPmin_blind_133( event_index, detector_index ) ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].psumOF )

                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTlfnoise1OFchisq
                self.histograms[ 'cLFnoise1' ][ detector_index ][ int( not self.cLFnoise1_blind_133( event_index, detector_index ) ) ].Fill( chisquare )

                chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTglitch1OFchisq
                self.histograms[ 'cGlitch1' ][ detector_index ][ int( not self.cGlitch1_blind_133( event_index, detector_index ) ) ].Fill( chisquare )

                self.histograms[ 'cBlind_LT' ][ detector_index ][ int( not self.cBlind_133_LT( event_index, detector_index ) ) ].Fill( self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF )

                self.histograms[ 'cVT' ][ detector_index ][ int( not self.cVT_blind_133( event_index ) ) ].Fill( self.global_data_chains[ 'RQs' ].VTTime20 )
                
                self.graphs[ 'cQin1' ][ detector_index ][ int( self.cQin1_blind_133( event_index, detector_index ) ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi1OF,
                                                                                                                                 self.detector_data_chains[ 'RRQs' ][ detector_index ].qo1OF )
                
                self.graphs[ 'cQin2' ][ detector_index ][ int( self.cQin2_blind_133( event_index, detector_index ) ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qi2OF,
                                                                                                                                 self.detector_data_chains[ 'RRQs' ][ detector_index ].qo2OF )
                
                self.graphs[ 'cQsym' ][ detector_index ][ int( self.cQsym_blind_133( event_index, detector_index ) ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum1OF,
                                                                                                                                 self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum2OF )

                self.graphs[ 'cNR' ][ detector_index ][ int( self.cNR_blind_133( event_index, detector_index ) ) ].SetPoint( self.detector_data_chains[ 'RRQs' ][ detector_index ].precoilsumOF,
                                                                                                                             self.detector_data_chains[ 'RRQs' ][ detector_index ].ysumOF       )
                
                self.graphs[ 'cGlitch' ][ detector_index ][ int( not self.cGlitch_blind_133( event_index ) ) ].SetPoint( self.global_data_chains[ 'RRQs' ].ntrigp,
                                                                                                                         self.global_data_chains[ 'RRQs' ].ntrigq )

        return 0


    def create_plots( self ):
        gROOT.SetStyle( 'Plain' )

        gStyle.SetTitleX( .4 )
        gStyle.SetTitleBorderSize( 0 )
        gStyle.SetOptStat( 0 )
                            
        canvas = TCanvas()
        canvas.SetLogy( 1 )
        canvas.SetBottomMargin( 1.4*canvas.GetBottomMargin() )        
        
        for cut_name in self.histograms.keys():
            if cut_name == 'cPmult' and self.mode_flag in [ 0, 2 ]: continue

            for detector_index in self.detector_indices:
                self.histograms[ cut_name ][ detector_index ][ 0 ].SetLineColor( 3 )
                self.histograms[ cut_name ][ detector_index ][ 1 ].SetLineColor( 2 )
                
                minimum_bin_content = min( self.histograms[ cut_name ][ detector_index ][ 0 ].GetMinimum(), self.histograms[ cut_name ][ detector_index ][ 1 ].GetMinimum() )
                maximum_bin_content = max( self.histograms[ cut_name ][ detector_index ][ 0 ].GetMaximum(), self.histograms[ cut_name ][ detector_index ][ 1 ].GetMaximum() )
                
                plot_status = 0
                
                for histogram in self.histograms[ cut_name ][ detector_index ]: 
                    if 0 < histogram.GetEntries():
                        if plot_status == 0:
                            histogram.SetTitle( 'IT'+str( ( detector_index/3 )+1 )+'Z'+str( ( detector_index%3 )+1 ) )

                            if cut_name == 'cPmin'    : histogram.GetXaxis().SetTitle( 'Phonon energy [psumOF] (keV)'        )
                            if cut_name == 'cLFnoise1': histogram.GetXaxis().SetTitle( '#Delta#chi^{2}'                      )
                            if cut_name == 'cGlitch1' : histogram.GetXaxis().SetTitle( '#Delta#chi^{2}'                      )
                            if cut_name == 'cPmult'   : histogram.GetXaxis().SetTitle( 'Phonon energy [psumOF] (keV)'        )
                            if cut_name == 'cBlind_LT': histogram.GetXaxis().SetTitle( 'Phonon energy [ptNF] (keV)'          )
                            if cut_name == 'cVT'      : histogram.GetXaxis().SetTitle( 'Time w.r.t. trigger [VTTime20] (ns)' )
                            
                            histogram.GetYaxis().SetTitle( 'Counts' )
                            
                            histogram.SetMinimum( .8*max( 1., minimum_bin_content ) )
                            histogram.SetMaximum( 1.4*maximum_bin_content )
                            
                            histogram.Draw( 'e' )
                            plot_status = 1
                        
                        else: histogram.Draw( 'e same' )
                    
                if 0 < plot_status: canvas.SaveAs( 'png/'+cut_name+'_'+str( self.mode_flag )+'_'+str( detector_index+1 )+'_'+self.name+'.png' )
                
        canvas.SetLogy( 0 )
        
        for cut_name in self.graphs.keys():
            for detector_index in self.detector_indices:
                self.graphs[ cut_name ][ detector_index ][ 0 ].SetMarkerColor( 3 )
                self.graphs[ cut_name ][ detector_index ][ 1 ].SetMarkerColor( 2 )

                plot_status = 0
                
                for graph in self.graphs[ cut_name ][ detector_index ]:
                    if 0 < graph.GetN():
                        graph.SetMarkerStyle( 20 )
                        graph.SetMarkerSize( .2 )

                        if plot_status == 0:
                            graph.SetTitle( 'IT'+str( ( detector_index/3 )+1 )+'Z'+str( ( detector_index%3 )+1 ) )

                            if cut_name == 'cQin1':
                                graph.SetTitles( 'Charge energy [qi1OF] (keV)', 'Charge energy [qo1OF] (keV)' )

                                graph.SetXAxisLimits( -10., 20. )
                                graph.SetYAxisLimits( -10., 20. )
                                
                            if cut_name == 'cQin2':
                                graph.SetTitles( 'Charge energy [qi2OF] (keV)', 'Charge energy [qo2OF] (keV)' )

                                graph.SetXAxisLimits( -10., 20. )
                                graph.SetYAxisLimits( -10., 20. )
                                
                            if cut_name == 'cQsym':
                                graph.SetTitles( 'Charge energy [qsum1OF] (keV)', 'Charge energy [qsum2OF] (keV)' )

                                graph.SetXAxisLimits( -10., 20. )
                                graph.SetYAxisLimits( -10., 20. )
                            
                            if cut_name == 'cNR':
                                graph.SetTitles( 'Recoil energy [precoilsumOF] (keV)', 'Ionization yield [ysumOF] (keV)' )

                                graph.SetXAxisLimits( 0., 100. )
                                graph.SetYAxisLimits( 0., 1.4 )
                                
                            if cut_name == 'cGlitch':
                                graph.SetTitles( 'Number of P-triggered detectors [ntrigp]', 'Number of Q-triggered detectors [ntrigq]' )

                                graph.SetXAxisLimits( 0., 16. )
                                graph.SetYAxisLimits( 0., 16. )
                                
                            graph.Draw( 'AP' )
                            plot_status = 1

                        else: graph.Draw( 'P'  )

                if 0 < plot_status: canvas.SaveAs( 'png/'+cut_name+'_'+str( self.mode_flag )+'_'+str( detector_index+1 )+'_'+self.name+'.png' )
                
        return 0
        

    def run( self, sample_names, step = 1 ):
        status = BlindingCut.run( self, sample_names, step )
        if self.mode_flag in [ 0, 1 ] and 0. < self.counts[ 0 ]+self.counts[ 1 ]: print str( 100.*self.counts[ 1 ]/( self.counts[ 0 ]+self.counts[ 1 ] ) )+'% of Cf data blinded'

        return status

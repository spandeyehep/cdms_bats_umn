import array

from ROOT import TChain, TFile, TTree

from AlgorithmBase import AlgorithmBase


class BlindingCut( AlgorithmBase ):

    def __init__( self, detector_names ):
        AlgorithmBase.__init__( self, detector_names )
        
        self.decisions = [ [] for detector_index in self.detector_indices ]
        

    def load_data( self ):
        path = '/tera2/data1/cdmsbatsProd/R135/Prodv5-4_cdmslite/merged/all/bg_restricted/'
        
        if 0 < self.load_detector_data( 'RRQs', path+'calib_Prodv5-4_*.root:rrqDir/calibzip*', [ 'ptNF' ] ): return 1

        if 0 < self.load_global_data( 'RQs', path+'merge_Prodv5-4_*.root:rqDir/eventTree', [ 'EventCategory' ] ): return 2
        
        return 0


    def load_detector_data( self, data_name, location, variable_names ):
        location_slices = location.split( ':' )

        if not len( location_slices ) == 2:
            print 'ERROR (AlgorithmBase::load_detector_data): Invalid location format >>', location
            return 1

        self.detector_data_chains[ data_name ] = []

        for detector_index in self.detector_indices:            
            self.detector_data_chains[ data_name ].append( TChain( location_slices[ 1 ].replace( '*', self.detector_names[ detector_index ] ) ) )
            
            for sample_name in self.sample_names:
                if self.detector_data_chains[ data_name ][ detector_index ].Add( location_slices[ 0 ].replace( '*', sample_name ), 0 ) == 0:
                    print 'ERROR (AlgorithmBase::load_detector_data): ROOT data not found >>', sample_name, detector_index+1
                    return 2

            self.detector_data_chains[ data_name ][ detector_index ].SetBranchStatus( '*', 0 )

            for variable_name in variable_names:
                if not self.detector_data_chains[ data_name ][ detector_index ].GetListOfBranches().Contains( variable_name ):
                    print 'ERROR (AlgorithmBase::load_detector_data): Variable not found in ROOT data >>', variable_name, detector_index+1
                    return 3

                else: self.detector_data_chains[ data_name ][ detector_index ].SetBranchStatus( variable_name, 1 )
                    
        return 0


    def process_event( self, event_index ):
        decisions = self.cxBlind_lite3( event_index )
        for detector_index in self.detector_indices: self.decisions[ detector_index ].append( decisions[ detector_index ] )

        return 0


    def run( self, sample_names, step = 1 ):
        self.sample_names = sample_names

        if 0 < self.load_data(): return 1
        if 0 < self.check_data(): return 2

        for event_index in range( 0, self.number_of_events, step ):
            if 0 < self.read_event( event_index ): return 3
            if 0 < self.process_event( event_index ): return 4

            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        return 0
                

    def save_decisions( self, file_name ):
        file_pointer = TFile( file_name, 'recreate' )
        file_pointer.mkdir( 'cutDir' )
        file_pointer.cd( 'cutDir' )

        number_of_events = 0

        first = True

        for detector_index in self.detector_indices:
            if first:
                number_of_events = len( self.decisions[ detector_index ] )
                first = False

            elif not len( self.decisions[ detector_index ] ) == number_of_events: return 1

        entry = array.array( 'd', [ 0.0 ] )

        tree = TTree( 'cutevent', '' )
        tree.Branch( 'cBlind_lite3', entry, 'cBlind_lite3/D'  )

        for event_index in range( number_of_events ):
            entry[ 0 ] = float( True in [ self.decisions[ detector_index ][ event_index ] for detector_index in self.detector_indices ] )
            tree.Fill()

        tree.Write()
            
        file_pointer.Close()        

        return 0
        

    def reset_decisions( self ):
        self.decisions = [ [] for detector_index in self.detector_indices ]

        return 0
 
        
    def cxBlind_lite3( self, event_index ):
        decisions = [ True for detector_index in self.detector_indices ]        
        
        # Checking detector-by-detector requirements #

        for detector_index in self.detector_indices:
            
            # Not blinding events with phonon energy above 85 keV #
            
            if not self.cPmax_blind_lite3( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

        # Not blinding random-triggered events #

        if self.cRandom_blind_lite3( event_index ):
            decisions = [ False for detector_index in self.detector_indices ]
            return decisions

        return decisions


    def cPmax_blind_lite3( self, event_index, detector_index ):
        energy = self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF
        
        return energy < 85.0


    def cRandom_blind_lite3( self, event_index ): return self.global_data_chains[ 'RQs' ].EventCategory == 1.0

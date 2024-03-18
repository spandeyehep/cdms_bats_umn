from ROOT import TChain

    
class AlgorithmBase:

    def __init__( self, detector_names ):
        self.detector_names = detector_names
        
        self.number_of_detectors = len( self.detector_names )
        self.detector_indices = range( self.number_of_detectors )

        self.sample_names = []
        
        self.detector_data_chains = {}
        self.global_data_chains = {}

        self.number_of_events = 0

        
    def load_data( self ): return 0
    

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
                    print 'ERROR (AlgorithmBase::load_detector_data): Variable not found in ROOT data >>', variable_name
                    return 3

                self.detector_data_chains[ data_name ][ detector_index ].SetBranchStatus( variable_name, 1 )

        return 0


    def load_global_data( self, data_name, location, variable_names ):
        location_slices = location.split( ':' )

        if not len( location_slices ) == 2:
            print 'ERROR (AlgorithmBase::load_global_data): Invalid location format >>', location
            return 1

        self.global_data_chains[ data_name ] = TChain( location_slices[ 1 ] )

        for sample_name in self.sample_names:
            if self.global_data_chains[ data_name ].Add( location_slices[ 0 ].replace( '*', sample_name ), 0 ) == 0:
                print 'ERROR (AlgorithmBase::load_global_data): ROOT data not found >>', sample_name
                return 2

        self.global_data_chains[ data_name ].SetBranchStatus( '*', 0 )
            
        for variable_name in variable_names:
            if not self.global_data_chains[ data_name ].GetListOfBranches().Contains( variable_name ):
                print 'ERROR (AlgorithmBase::load_global_data): Variable not found in ROOT data >>', variable_name
                return 3
            
            self.global_data_chains[ data_name ].SetBranchStatus( variable_name, 1 )
        
        return 0


    def check_data( self ):
        first = True
        
        for data_name in self.detector_data_chains.keys():
            for detector_index in self.detector_indices:
                if first:
                    self.number_of_events = self.detector_data_chains[ data_name ][ detector_index ].GetEntries()
                    first = False

                elif not self.detector_data_chains[ data_name ][ detector_index ].GetEntries() == self.number_of_events:
                    print 'ERROR (AlgorithmBase::check_data): Mismatching number of ROOT entries >>', data_name, detector_index+1
                    self.number_of_events = 0
                    return 1

        for data_name in self.global_data_chains.keys():
            if first:
                self.number_of_events = self.global_data_chains[ data_name ].GetEntries()
                first = False
                
            elif not self.global_data_chains[ data_name ].GetEntries() == self.number_of_events:
                print 'ERROR (AlgorithmBase::check_data): Mismatching number of ROOT entries >>', data_name
                self.number_of_events = 0
                return 1

        self.number_of_events = max( 0, self.number_of_events )

        return 0
    

    def read_event( self, event_index ):
        for data_name in self.detector_data_chains.keys():
            for detector_index in self.detector_indices:
                if self.detector_data_chains[ data_name ][ detector_index ].GetEntry( event_index ) == 0:
                    print 'ERROR (AlgorithmBase::read_event): Not able to read ROOT data >>', data_name, detector_index+1
                    return 1

        for data_name in self.global_data_chains.keys():
            if self.global_data_chains[ data_name ].GetEntry( event_index ) == 0:
                print 'ERROR (AlgorithmBase::read_event): Not able to read ROOT data >>', data_name
                return 1

        return 0
            
        
    def process_event( self, event_index ): return 0

    
    def run( self, sample_names, step = 1 ):
        self.sample_names = sample_names

        if 0 < self.load_data(): return 1
        if 0 < self.check_data(): return 2

        for event_index in range( 0, self.number_of_events, step ):
            if 0 < self.read_event( event_index ): return 3
            if 0 < self.process_event( event_index ): return 4

            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        return 0

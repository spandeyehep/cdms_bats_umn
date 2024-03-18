from math import *
from ROOT import *

import sqlite3
import array


from AlgorithmBase import *




class BlindingCut( AlgorithmBase ):

    def __init__( self, detector_names ):
        AlgorithmBase.__init__( self, detector_names )
        self.detector_flags = {}

        self.configuration_name = 'Prodv5-3'
        
        self.noise_thresholds = { 'min_psumOF' : [],  # used in cPthresh
                                  'max_psumOF' : [],  # used in cPmult
                                  'min_qi1OF'  : [],  # used in cQin1
                                  'max_qo1OF'  : [],  # used in cQin1
                                  'min_qi2OF'  : [],  # used in cQin2
                                  'max_qo2OF'  : [],  # used in cQin2
                                  'min_qsum1OF': [],  # used in cQsurf1
                                  'max_qsum1OF': [],  # used in cQsurf1
                                  'min_qsum2OF': [],  # used in cQsurf2
                                  'max_qsum2OF': [] } # used in cQsurf2
        
        self.band_parameters = [ { 'A_mu'    : [],
                                   'b_mu'    : [],
                                   'A_sigma' : [],
                                   'b_sigma' : [],
                                   'x0_sigma': [] } for calibration_index in range( 3 ) ]

        self.maximum_chisquares = { 'cGlitch1_v53' : [],
                                    'cLFnoise1_v53': [] }
        
        self.energy_scales = { 'PA1OFamps': [],
                               'PA2OFamps': [],
                               'PB1OFamps': [],
                               'PB2OFamps': [],
                               'PC1OFamps': [],
                               'PC2OFamps': [],
                               'PD1OFamps': [],
                               'PD2OFamps': [] }

        self.decisions = [ [] for detector_index in self.detector_indices ]
        

    def load_data( self ):

        ### RECALL THAT THIS VERSION OF THE BLINDING CUT ALGORITHM RUNS ON ALREADY BLINDED DATA ###

        path = '/galbadata/R133/dataReleases/Prodv5-3_June2013/merged/all/bg_permitted/'

        if 0 < self.load_detector_data( 'RQs', path+'merge_Prodv5-3_*.root:rqDir/zip*', [ 'DetType'          ,
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
                                                                                          'QOS2status'        ] ): return 1
        
        if 0 < self.load_detector_data( 'RRQs', path+'calib_Prodv5-3_*.root:rrqDir/calibzip*', [ 'pa2OF'       ,
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
                                                                                                 'ysumOF'       ] ): return 1
        
        if 0 < self.load_global_data( 'RQs', path+'merge_Prodv5-3_*.root:rqDir/eventTree', [ 'EventCategory',
                                                                                             'SeriesNumber' ,
                                                                                             'VTTime20'      ] ): return 2
        
        if 0 < self.load_global_data( 'RRQs', path+'calib_Prodv5-3_*.root:rrqDir/calibevent', [ 'ntrigp',
                                                                                                'ntrigq' ] ): return 2

        return 0


    def load_detector_data( self, data_name, location, variable_names ):
        location_slices = location.split( ':' )

        if not len( location_slices ) == 2:
            print 'ERROR (AlgorithmBase::load_detector_data): Invalid location format >>', location
            return 1

        self.detector_data_chains[ data_name ] = []

        for detector_index in self.detector_indices:            
            first = len( self.detector_flags[ 'CDMSlite' ] ) < self.number_of_detectors
            if first: self.detector_flags[ 'CDMSlite' ].append( 0 )
            
            self.detector_data_chains[ data_name ].append( TChain( location_slices[ 1 ].replace( '*', self.detector_names[ detector_index ] ) ) )
            
            for sample_name in self.sample_names:
                if self.detector_data_chains[ data_name ][ detector_index ].Add( location_slices[ 0 ].replace( '*', sample_name ), 0 ) == 0:
                    if first and data_name == 'RRQs':
                        print 'INFO (AlgorithmBase::load_detector_data): Found detector operating in CDMSlite mode >>', detector_index+1
                        self.detector_flags[ 'CDMSlite' ][ detector_index ] = 1
                        first = False

                    if not ( data_name == 'RRQs' and self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1 ):
                        print 'ERROR (AlgorithmBase::load_detector_data): ROOT data not found >>', sample_name, detector_index+1
                        return 2

            if self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1 and 'RQs' in self.detector_data_chains.keys():
                self.detector_data_chains[ 'RQs' ][ detector_index ].SetBranchStatus( '*', 0 )
                self.detector_data_chains[ 'RQs' ][ detector_index ].SetBranchStatus( 'DetType', 1 )
                
                for event_index in range( self.detector_data_chains[ 'RQs' ][ detector_index ].GetEntries() ):
                    self.detector_data_chains[ 'RQs' ][ detector_index ].GetEntry( event_index )
                    if not self.detector_data_chains[ 'RQs' ][ detector_index ].DetType in [ 21., -999999. ]:
                        print 'ERROR (AlgorithmBase::load_detector_data): ROOT data not found >>', sample_name, detector_index+1
                        return 2
                
            if data_name == 'RRQs' and self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1: continue
            self.detector_data_chains[ data_name ][ detector_index ].SetBranchStatus( '*', 0 )

            for variable_name in variable_names:
                if not self.detector_data_chains[ data_name ][ detector_index ].GetListOfBranches().Contains( variable_name ):
                    if variable_name in [ 'QIS1bias'  ,
                                          'QIS2bias'  ,
                                          'QIS1status',
                                          'QOS1status',
                                          'QIS2status',
                                          'QOS2status' ]:
                        if first and data_name == 'RQs':
                            print 'INFO (AlgorithmBase::load_detector_data): Found detector operating in CDMSlite mode >>', detector_index+1
                            self.detector_flags[ 'CDMSlite' ][ detector_index ] = 1
                            first = False 

                        if data_name == 'RQs' and self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1: continue

                    if variable_name in [ 'PAOFamps',
                                          'PBOFamps',
                                          'PCOFamps',
                                          'PDOFamps' ]:
                        if data_name == 'RQs' and self.detector_flags[ 'CDMSlite' ][ detector_index ] == 0:
                            if first: first = False
                            continue

                    print 'ERROR (AlgorithmBase::load_detector_data): Variable not found in ROOT data >>', variable_name, detector_index+1
                    return 3

                else: self.detector_data_chains[ data_name ][ detector_index ].SetBranchStatus( variable_name, 1 )
                    
        return 0


    def check_data( self ):
        first = True
        
        for data_name in self.detector_data_chains.keys():
            for detector_index in self.detector_indices:
                if self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1 and data_name == 'RRQs': continue

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
    

    def set_parameters( self ): 
        database_pointer = sqlite3.connect( '/localhome/cdms/temp/elias/DataBlinding/algorithm/parameters.db' )
        cursor = database_pointer.cursor()

        series_name = str( int( self.global_data_chains[ 'RQs' ].SeriesNumber ) )

        for parameter_name in self.noise_thresholds.keys():
            self.noise_thresholds[ parameter_name ] = []
            
            for detector_index in self.detector_indices:
                statement = 'SELECT value FROM noise_thresholds WHERE series_name = ? AND detector_name = ? AND configuration_name = ? AND parameter_name = ?'
                arguments = series_name, self.detector_names[ detector_index ], self.configuration_name, parameter_name,
                
                results = cursor.execute( statement, arguments )
                values = [ results[ 0 ] for results in results ]

                if len( values ) == 1: self.noise_thresholds[ parameter_name ].append( values[ 0 ] )
                else:
                    print 'ERROR (AlgorithmBase::set_parameters): Parameters not found >>', series_name, detector_index, parameter_name
                    database_pointer.close()
                    return 1

        for calibration_index in range( len( self.band_parameters ) ):
            for parameter_name in self.band_parameters[ calibration_index ].keys():
                self.band_parameters[ calibration_index ][ parameter_name ] = []
            
                for detector_index in self.detector_indices:
                    statement = 'SELECT value FROM band_parameters WHERE detector_name = ? AND configuration_name = ? AND calibration_index = ? AND parameter_name = ?'
                    arguments = self.detector_names[ detector_index ], self.configuration_name, calibration_index, parameter_name,

                    results = cursor.execute( statement, arguments )                
                    values = [ results[ 0 ] for results in results ]

                    if len( values ) == 1: self.band_parameters[ calibration_index ][ parameter_name ].append( values[ 0 ] )
                    else:
                        print 'ERROR (AlgorithmBase::set_parameters): Parameters not found >>', detector_index, calibration_index, parameter_name
                        database_pointer.close()
                        return 1
                    

        for parameter_name in self.maximum_chisquares.keys():
            self.maximum_chisquares[ parameter_name ] = []
            
            for detector_index in self.detector_indices:
                statement = 'SELECT value FROM maximum_chisquares WHERE detector_name = ? AND configuration_name = ? AND parameter_name = ?'
                arguments = self.detector_names[ detector_index ], self.configuration_name, parameter_name,

                results = cursor.execute( statement, arguments )                
                values = [ results[ 0 ] for results in results ]

                if len( values ) == 1: self.maximum_chisquares[ parameter_name ].append( values[ 0 ] )
                else:
                    print 'ERROR (AlgorithmBase::set_parameters): Parameters not found >>', detector_index, parameter_name
                    database_pointer.close()
                    return 1
                    
        for parameter_name in self.energy_scales.keys():
            self.energy_scales[ parameter_name ] = []
            
            for detector_index in self.detector_indices:
                statement = 'SELECT value FROM energy_scales WHERE detector_name = ? AND configuration_name = ? AND parameter_name = ?'
                arguments = self.detector_names[ detector_index ], self.configuration_name, parameter_name,

                results = cursor.execute( statement, arguments )                
                values = [ results[ 0 ] for results in results ]
                
                if len( values ) == 1: self.energy_scales[ parameter_name ].append( values[ 0 ] )
                else:
                    print 'ERROR (AlgorithmBase::set_parameters): Parameters not found >>', detector_index, parameter_name
                    database_pointer.close()
                    return 1

        database_pointer.close()

        return 0

        
    def read_event( self, event_index ):
        for data_name in self.detector_data_chains.keys():
            for detector_index in self.detector_indices:
                if self.detector_flags[ 'CDMSlite' ][ detector_index ] == 1: continue

                if self.detector_data_chains[ data_name ][ detector_index ].GetEntry( event_index ) == 0:
                    print 'ERROR (AlgorithmBase::read_event): Not able to read ROOT data >>', data_name, detector_index+1
                    return 1

        for data_name in self.global_data_chains.keys():
            if self.global_data_chains[ data_name ].GetEntry( event_index ) == 0:
                print 'ERROR (AlgorithmBase::read_event): Not able to read ROOT data >>', data_name
                return 1

        return 0
            
        
    def process_event( self, event_index ):
        decisions = self.cxBlind_133( event_index )
        for detector_index in self.detector_indices: self.decisions[ detector_index ].append( decisions[ detector_index ] )

        return 0


    def run( self, sample_names, step = 1 ):
        self.sample_names = sample_names

        self.detector_flags[ 'CDMSlite' ] = []
        
        if 0 < self.load_data(): return 1
        if 0 < self.check_data(): return 2

        for event_index in range( 0, self.number_of_events, step ):
            if 0 < event_index:
                series_number = self.global_data_chains[ 'RQs' ].SeriesNumber
                if 0 < self.read_event( event_index ): return 3

                if not self.global_data_chains[ 'RQs' ].SeriesNumber == series_number:
                    if 0 < self.set_parameters(): return 4

            else:
                if 0 < self.read_event( event_index ): return 3
                if 0 < self.set_parameters(): return 4

            if 0 < self.process_event( event_index ): return 5

            if event_index%10000 == 9999: print 'INFO (AlgorithmBase::run): Processing data >>', event_index+1, self.number_of_events

        return 0
                

    def save_decisions( self, file_name ):
        file_pointer = TFile( file_name, 'recreate' )
        file_pointer.mkdir( 'cutDir' )
        file_pointer.cd( 'cutDir' )

        number_of_events = 0

        first = True

        for detector_index in self.detector_indices:
            entry = array.array( 'd', [ 0. ] )
            
            tree = TTree( 'cutzip'+self.detector_names[ detector_index ], '' )
            tree.Branch( 'cBlind_133', entry, 'cBlind_133/D'  )

            if first:
                number_of_events = len( self.decisions[ detector_index ] )
                first = False

            elif not len( self.decisions[ detector_index ] ) == number_of_events: return 1

            for event_index in range( number_of_events ):
                entry[ 0 ] = float( self.decisions[ detector_index ][ event_index ] )
                tree.Fill()

            tree.Write()
            
        entry = array.array( 'd', [ 0. ] )
            
        tree = TTree( 'cutevent', '' )
        tree.Branch( 'cBlind_133', entry, 'cBlind_133/D'  )

        for event_index in range( number_of_events ):
            entry[ 0 ] = float( True in [ self.decisions[ detector_index ][ event_index ] for detector_index in self.detector_indices ] )
            tree.Fill()

        tree.Write()
            
        file_pointer.Close()        

        return 0
        

    def reset_decisions( self ):
        self.decisions = [ [] for detector_index in self.detector_indices ]

        return 0
 
        
    def cxBlind_133( self, event_index ):
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
                
            # Not blinding events in the outer guard ring, based on charge information #

            if not self.cQin1_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue
            
            if not self.cQin2_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue
            
            # Not blinding events near S1 or S2, based on charge information #

            if not self.cQsym_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events outside the NR band #

            if not self.cNR_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding multiple-scattering events #

            if self.cPmult_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events with recoil energy above 150 keV #

            if not self.cErecoilmax_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events from detectors used for the low-threshold WIMP search #

            if self.cBlind_133_LT( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events from T3Z1 and T3Z3 before June 1st 2012 #

            if self.cPbSource_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue
            
            # Not blinding events from detectors operating in reverse-bias mode #

            if not self.cBiasPM_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

            # Not blinding events from T3Z1 when S1 grounded on August 2012 #

            if self.cGroundDIB_blind_133( event_index, detector_index ):
                decisions[ detector_index ] = False
                continue

        # Not blinding events having veto hits within 25 microseconds before the trigger #
        
        if self.cVT_blind_133( event_index ):
            decisions = [ False for detector_index in self.detector_indices ]
            return decisions

        # Not blinding random-triggered events #

        if self.cRandom_blind_133( event_index ):
            decisions = [ False for detector_index in self.detector_indices ]
            return decisions

        # Not blinding glitch-like events (trigger-based definition) #

        if self.cGlitch_blind_133( event_index ):
            decisions = [ False for detector_index in self.detector_indices ]
            return decisions

        # Not blinding events for two days following Cf calibration #

        if self.cPostCf2_blind_133( event_index ):
            decisions = [ False for detector_index in self.detector_indices ]
            return decisions

        return decisions


    def cHasValidRRQs_blind_133( self, event_index, detector_index ):
        if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == 21.     : return False
        if self.detector_data_chains[ 'RQs' ][ detector_index ].DetType == -999999.: return False

        return True


    def cPmin_blind_133( self, event_index, detector_index ):
        energy = self.detector_data_chains[ 'RRQs' ][ detector_index ].psumOF
        
        if detector_index == 0: energy += -self.detector_data_chains[ 'RRQs' ][ detector_index ].pa2OF
        if detector_index == 8: energy += -self.detector_data_chains[ 'RRQs' ][ detector_index ].pc1OF
        
        return self.noise_thresholds[ 'min_psumOF' ][ detector_index ] < energy


    def cLFnoise1_blind_133( self, event_index, detector_index ):
        chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTlfnoise1OFchisq
        return self.maximum_chisquares[ 'cLFnoise1_v53' ][ detector_index ] < chisquare


    def cGlitch1_blind_133( self, event_index, detector_index ):
        chisquare = self.detector_data_chains[ 'RQs' ][ detector_index ].PTOFchisq-self.detector_data_chains[ 'RQs' ][ detector_index ].PTglitch1OFchisq
        return self.maximum_chisquares[ 'cGlitch1_v53' ][ detector_index ] < chisquare


    def cQin1_blind_133( self, event_index, detector_index ):
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QIS1status: return True

        if self.detector_data_chains[ 'RRQs' ][ detector_index ].qi1OF < self.noise_thresholds[ 'min_qi1OF' ][ detector_index ]:
            if self.noise_thresholds[ 'max_qo1OF' ][ detector_index ] < self.detector_data_chains[ 'RRQs' ][ detector_index ].qo1OF: return False

        return True
                

    def cQin2_blind_133( self, event_index, detector_index ):
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QIS2status: return True

        if self.detector_data_chains[ 'RRQs' ][ detector_index ].qi2OF < self.noise_thresholds[ 'min_qi2OF' ][ detector_index ]:
            if self.noise_thresholds[ 'max_qo2OF' ][ detector_index ] < self.detector_data_chains[ 'RRQs' ][ detector_index ].qo2OF: return False
                
        return True
    
        
    def cQsym_blind_133( self, event_index, detector_index ):
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QIS1status: return True
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QOS1status: return True
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QIS2status: return True
        if 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QOS2status: return True
        
        if self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum1OF < self.noise_thresholds[ 'min_qsum1OF' ][ detector_index ]:
            if self.noise_thresholds[ 'max_qsum2OF' ][ detector_index ] < self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum2OF: return False

        if self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum2OF < self.noise_thresholds[ 'min_qsum2OF' ][ detector_index ]:
            if self.noise_thresholds[ 'max_qsum1OF' ][ detector_index ] < self.detector_data_chains[ 'RRQs' ][ detector_index ].qsum1OF: return False

        return True


    def cNR_blind_133( self, event_index, detector_index ):
        for calibration_index in range( 3 ):
            energy = self.detector_data_chains[ 'RRQs' ][ detector_index ].precoilsumOF

            if 0. < energy:
                mean = self.band_parameters[ calibration_index ][ 'A_mu' ][ detector_index ]*pow( energy, self.band_parameters[ calibration_index ][ 'b_mu' ][ detector_index ] )
                
                energy = min( energy, self.band_parameters[ calibration_index ][ 'x0_sigma' ][ detector_index ] )
                width = self.band_parameters[ calibration_index ][ 'A_sigma' ][ detector_index ]*pow( energy, self.band_parameters[ calibration_index ][ 'b_sigma' ][ detector_index ] )

                if fabs( self.detector_data_chains[ 'RRQs' ][ detector_index ].ysumOF-mean ) < 3.*width: return True

        return False

                
    def cPmult_blind_133( self, event_index, detector_index ):
        for other_detector_index in self.detector_indices:
            if other_detector_index == detector_index: continue

            if self.detector_data_chains[ 'RQs' ][ other_detector_index ].DetType == -999999.: continue
            energy = 0.
            
            if self.detector_data_chains[ 'RQs' ][ other_detector_index ].DetType == 21.:
                energy += self.energy_scales[ 'PA1OFamps' ][ other_detector_index ]*self.detector_data_chains[ 'RQs' ][ other_detector_index ].PAOFamps
                energy += self.energy_scales[ 'PB1OFamps' ][ other_detector_index ]*self.detector_data_chains[ 'RQs' ][ other_detector_index ].PBOFamps
                energy += self.energy_scales[ 'PC1OFamps' ][ other_detector_index ]*self.detector_data_chains[ 'RQs' ][ other_detector_index ].PCOFamps
                energy += self.energy_scales[ 'PD1OFamps' ][ other_detector_index ]*self.detector_data_chains[ 'RQs' ][ other_detector_index ].PDOFamps

            else:
                energy += self.detector_data_chains[ 'RRQs' ][ other_detector_index ].psumOF
                
                if other_detector_index == 0: energy += -self.detector_data_chains[ 'RRQs' ][ other_detector_index ].pa2OF
                if other_detector_index == 8: energy += -self.detector_data_chains[ 'RRQs' ][ other_detector_index ].pc1OF

            if self.noise_thresholds[ 'max_psumOF' ][ other_detector_index ] < energy:
                if self.cLFnoise1_blind_133( event_index, detector_index ): continue
                if self.cGlitch1_blind_133( event_index, detector_index ): continue

                return True

        return False


    def cErecoilmax_blind_133( self, event_index, detector_index ): return self.detector_data_chains[ 'RRQs' ][ detector_index ].precoilsumOF < 150.


    def cBlind_133_LT( self, event_index, detector_index ):
        if detector_index in [ 0, 2, 3, 4, 10, 11, 13, 14 ]:
            if 11204111436. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11204211657.: return True
            if 11208221428. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11209091651.: return True

            if 11210010000. < self.global_data_chains[ 'RQs' ].SeriesNumber:
                maximum_energy = 10.*( 1.+( .0781633*fabs( self.detector_data_chains[ 'RQs' ][ detector_index ].QIS1bias-self.detector_data_chains[ 'RQs' ][ detector_index ].QIS2bias ) ) )
                return self.detector_data_chains[ 'RRQs' ][ detector_index ].ptNF < maximum_energy

        return False


    def cPbSource_blind_133( self, event_index, detector_index ):
        if ( detector_index == 6 or detector_index == 8 ) and self.global_data_chains[ 'RQs' ].SeriesNumber < 11206010911.: return True

        return False


    def cBiasPM_blind_133( self, event_index, detector_index ):
        if self.detector_data_chains[ 'RQs' ][ detector_index ].QIS1bias < 0. and 0. < self.detector_data_chains[ 'RQs' ][ detector_index ].QIS2bias: return False

        return True
    

    def cGroundDIB_blind_133( self, event_index, detector_index ):
        if detector_index == 6:
            if 11208170659. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11208211716.: return True                
            if 11208300605. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11208311600.: return True

        return False

    
    def cVT_blind_133( self, event_index ): return -25. < self.global_data_chains[ 'RQs' ].VTTime20 and self.global_data_chains[ 'RQs' ].VTTime20 < 0.


    def cRandom_blind_133( self, event_index ): return self.global_data_chains[ 'RQs' ].EventCategory == 1.


    def cGlitch_blind_133( self, event_index ):
        if 6. < self.global_data_chains[ 'RRQs' ].ntrigp-self.global_data_chains[ 'RRQs' ].ntrigq: return True
        if 1. < self.global_data_chains[ 'RRQs' ].ntrigq-self.global_data_chains[ 'RRQs' ].ntrigp: return True

        return False

        
    def cPostCf2_blind_133( self, event_index ):
        if 11204111436. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11204131603.: return True
        if 11208221428. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11208241656.: return True
        if 11208301401. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11209021718.: return True
        if 11301151609. < self.global_data_chains[ 'RQs' ].SeriesNumber and self.global_data_chains[ 'RQs' ].SeriesNumber < 11301171645.: return True

        return False

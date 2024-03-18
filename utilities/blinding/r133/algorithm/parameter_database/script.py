from math import *
from ROOT import *

import sqlite3


from energy_scales import *




database_pointer = sqlite3.connect( 'parameters.db' )
cursor = database_pointer.cursor()


cursor.execute( 'CREATE TABLE noise_thresholds ( series_name TEXT, detector_name TEXT, configuration_name TEXT, parameter_name TEXT, value REAL )' )

for sample_name in [ 'bg_restricted',
                     'cf'           ,
                     'ba'            ]:
    file_pointer = open( 'series_names/'+sample_name+'/out' )
    series_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
    file_pointer.close()

    series_names.sort()

    for detector_index in range( 15 ):

        # Saving psumOF thresholds to database #
        
        file_pointer = open( 'dat/noise_thresholds/phonon/blocks_'+str( detector_index+1 )+'_7_20_20-nsigmasOF_glitch1_lf_3_1mod_9mod.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()
        
        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break

            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'min_psumOF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'max_psumOF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
        
        # Saving qi1OF thresholds to database #
            
        file_pointer = open( 'dat/noise_thresholds/charge/blocks_1_'+str( detector_index+1 )+'_2_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'min_qi1OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        # Saving qo1OF thresholds to database #

        file_pointer = open( 'dat/noise_thresholds/charge/blocks_2_'+str( detector_index+1 )+'_8_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'max_qo1OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        # Saving qi2OF thresholds to database #

        file_pointer = open( 'dat/noise_thresholds/charge/blocks_3_'+str( detector_index+1 )+'_2_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'min_qi2OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        # Saving qo2OF thresholds to database #

        file_pointer = open( 'dat/noise_thresholds/charge/blocks_4_'+str( detector_index+1 )+'_8_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'max_qo2OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        # Saving qsum1OF thresholds to database #

        file_pointer = open( 'dat/noise_thresholds/charge/blocks_5_'+str( detector_index+1 )+'_2_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'min_qsum1OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        file_pointer = open( 'dat/noise_thresholds/charge/blocks_5_'+str( detector_index+1 )+'_8_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'max_qsum1OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        # Saving qsum2OF thresholds to database #

        file_pointer = open( 'dat/noise_thresholds/charge/blocks_6_'+str( detector_index+1 )+'_2_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'min_qsum2OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()
            
        file_pointer = open( 'dat/noise_thresholds/charge/blocks_6_'+str( detector_index+1 )+'_8_7_40_20.dat' )
        lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
        file_pointer.close()

        last_series_names = [ line.split( ' ' ).__getitem__( 0 ) for line in lines ]
        noise_thresholds = [ float( line.split( ' ' ).__getitem__( 1 ) ) for line in lines ]
        
        block_index = 0
        
        for series_name in series_names:
            if int( last_series_names[ block_index ] ) < int( series_name ): block_index += 1
            if block_index == len( last_series_names ): break
            
            arguments = series_name, str( detector_index+1 ), 'Prodv5-3', 'max_qsum2OF', noise_thresholds[ block_index ],
            cursor.execute( 'INSERT INTO noise_thresholds VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()


cursor.execute( 'CREATE TABLE band_parameters ( detector_name TEXT, configuration_name TEXT, calibration_index INT, parameter_name TEXT, value REAL )' )

calibration_names = [ 'April12'  ,
                      'August12' ,
                      'January13' ]

for calibration_index in range( len( calibration_names ) ):

    # Saving band mean parameters to database #
    
    file_pointer = open( 'dat/band_parameters/NRFitMean_precoilsumOF_ysumOF__Chi_'+calibration_names[ calibration_index ]+'.dat' )
    lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
    file_pointer.close()

    for detector_index in range( 15 ):
        band_parameters = { 'A_mu': float( lines[ detector_index ].split( '\t' ).__getitem__( 0 ) ),
                            'b_mu': float( lines[ detector_index ].split( '\t' ).__getitem__( 1 ) ) }
        
        for parameter_name in band_parameters.keys():
            arguments = str( detector_index+1 ), 'Prodv5-3', calibration_index, parameter_name, band_parameters[ parameter_name ],
            cursor.execute( 'INSERT INTO band_parameters VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()

    # Saving band sigma parameters to database #
    
    file_pointer = open( 'dat/band_parameters/NRFitSigma_precoilsumOF_ysumOF__Chi_'+calibration_names[ calibration_index ]+'.dat' )
    lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
    file_pointer.close()

    for detector_index in range( 15 ):
        band_parameters = { 'A_sigma' : float( lines[ detector_index ].split( '\t' ).__getitem__( 3 ) ),
                            'b_sigma' : float( lines[ detector_index ].split( '\t' ).__getitem__( 0 ) ),
                            'x0_sigma': float( lines[ detector_index ].split( '\t' ).__getitem__( 1 ) ) }
        
        for parameter_name in band_parameters.keys():
            arguments = str( detector_index+1 ), 'Prodv5-3', calibration_index, parameter_name, band_parameters[ parameter_name ],
            cursor.execute( 'INSERT INTO band_parameters VALUES ( ?, ?, ?, ?, ? )', arguments )
            database_pointer.commit()


cursor.execute( 'CREATE TABLE maximum_chisquares ( detector_name TEXT, configuration_name TEXT, parameter_name TEXT, value REAL )' )

for parameter_name in [ 'cGlitch1_v53' ,
                        'cLFnoise1_v53' ]:

    # Saving maximum allowed chi-square values to database #
    
    file_pointer = open( 'dat/maximum_chisquares/'+parameter_name+'.dat' )
    lines = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
    file_pointer.close()

    for detector_index in range( 15 ):
        arguments = str( detector_index+1 ), 'Prodv5-3', parameter_name, float( lines[ detector_index ].replace( '\n', '' ) ),
        cursor.execute( 'INSERT INTO maximum_chisquares VALUES ( ?, ?, ?, ? )', arguments )
        database_pointer.commit()


cursor.execute( 'CREATE TABLE energy_scales ( detector_name TEXT, configuration_name TEXT, parameter_name TEXT, value REAL )' )

energy_scales = {}

energy_scales[ 'PA1OFamps' ] = get_PA1OFamps_scale()
energy_scales[ 'PB1OFamps' ] = get_PB1OFamps_scale()
energy_scales[ 'PC1OFamps' ] = get_PC1OFamps_scale()
energy_scales[ 'PD1OFamps' ] = get_PD1OFamps_scale()
energy_scales[ 'PA2OFamps' ] = get_PA2OFamps_scale()
energy_scales[ 'PB2OFamps' ] = get_PB2OFamps_scale()
energy_scales[ 'PC2OFamps' ] = get_PC2OFamps_scale()
energy_scales[ 'PD2OFamps' ] = get_PD2OFamps_scale()

for detector_index in range( 15 ):
    for parameter_name in energy_scales.keys():
        arguments = str( detector_index+1 ), 'Prodv5-3', parameter_name, energy_scales[ parameter_name ][ detector_index ],
        cursor.execute( 'INSERT INTO energy_scales VALUES ( ?, ?, ?, ? )', arguments )
        database_pointer.commit()

database_pointer.close()

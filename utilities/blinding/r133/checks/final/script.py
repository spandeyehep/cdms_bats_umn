from math import *
from ROOT import *


from BlindingCheck_final import *




file_pointer = open( 'list.txt' )
sample_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

detector_names = [ str( detector_index+1 ) for detector_index in range( 15 ) ]


algorithm = BlindingCheck_final( detector_names )
algorithm.run( sample_names )
algorithm.create_plots()

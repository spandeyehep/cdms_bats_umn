from math import *
from ROOT import *


from BlindingCheck_randoms import *




file_pointer = open( 'list.txt' )
sample_tags = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

detector_tags = [ str( detector_index+1 ) for detector_index in range( 15 ) ]


algorithm = BlindingCheck_randoms( detector_tags )
algorithm.run( sample_tags )
algorithm.create_plots()

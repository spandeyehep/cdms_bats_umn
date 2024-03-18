from math import *
from ROOT import *


from BlindingCheck_Cf import *




file_pointer = open( 'list.txt' )
sample_tags = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

detector_tags = [ str( detector_index+1 ) for detector_index in range( 15 ) ]


for mode_flag in [ 0, 1, 2 ]:
    for sample_tag in sample_tags:        
        algorithm = BlindingCheck_Cf( sample_tag, detector_tags, mode_flag )
        algorithm.run( [ sample_tag ] )
        algorithm.create_plots()

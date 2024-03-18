from math import *
from ROOT import *


from BlindingCut_Cf import *


file_pointer = open( 'list.txt' )
sample_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()


for sample_name in sample_names:
    detector_names = [ str( detector_index+1 ) for detector_index in range( 15 ) ]
    
    algorithm = BlindingCut_Cf( detector_names )
    algorithm.run( [ sample_name ] )
    algorithm.save_decisions( '/data4/public/elias/blinding_decisions/Cf/cBlind_133_'+sample_name+'.root' )

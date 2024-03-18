from BlindingCut import BlindingCut


file_pointer = open( 'list.txt' )
sample_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

for sample_name in sample_names:
    algorithm = BlindingCut( [ '4' ] )
    algorithm.run( [ sample_name ] )
    algorithm.save_decisions( '/data3/public/eliasla/blinding/cdmslite3/root/cBlind_lite3_'+sample_name+'.root' )

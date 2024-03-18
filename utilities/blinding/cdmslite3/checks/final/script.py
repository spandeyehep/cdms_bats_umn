from BlindingCheck_final import BlindingCheck_final


file_pointer = open( 'list.txt' )
sample_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

algorithm = BlindingCheck_final( [ '4' ] )
algorithm.run( sample_names )
algorithm.create_plots()

from BlindingCheck_Ba import BlindingCheck_Ba


file_pointer = open( 'list.txt' )
sample_names = [ line.replace( '\n', '' ) for line in file_pointer.readlines() ]
file_pointer.close()

algorithm = BlindingCheck_Ba( [ '4' ] )
algorithm.run( sample_names )
algorithm.create_plots()

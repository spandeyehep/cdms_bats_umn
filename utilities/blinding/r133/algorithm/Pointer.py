import array


class Pointer:

    def __init__( self, variable_type ): self.pointer = array.array( variable_type, [ 0. ] )


    def ref( self ): return self.pointer


    def val( self ): return self.pointer[ 0 ]


    def set( self, value ): self.pointer[ 0 ] = value

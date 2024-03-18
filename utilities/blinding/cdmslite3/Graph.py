from ROOT import TGraph


class Graph( TGraph ):

    def __init__( self ): TGraph.__init__( self )


    def SetPoint( self, value_x_var, value_y_var ): TGraph.SetPoint( self, TGraph.GetN( self ), value_x_var, value_y_var )
    

    def SetXAxisLimits( self, limit_lo, limit_hi ): TGraph.GetXaxis( self ).SetLimits( limit_lo, limit_hi )


    def SetYAxisLimits( self, limit_lo, limit_hi ):
        TGraph.SetMinimum( self, limit_lo )
        TGraph.SetMaximum( self, limit_hi )
        

    def SetTitles( self, title_x, title_y ):
        TGraph.GetXaxis( self ).SetTitle( title_x )
        TGraph.GetYaxis( self ).SetTitle( title_y )

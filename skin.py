# (0.20615528128088/(abs(pi)*sqrt(x))) = (sqrt(2*1.7*10^(-8)/(x*2pi*4pi*10^(-7)))

def plot_single(folder_dict,rvals,l,c,Cdrpin):
    approximations = []
    for r in rvals:
        approximations.append(folder_dict[vals_to_key(r,l,c,Cdrpin,"Vout","ac")])


def vals_to_key(self,rin,lin,cin,Cdrpin,signal,typein):
    '''
    rin         a float or string
    lin         a float or string
    cin         a float or string
    Cdrpin      a float or string
    signal      a string (Vin, Vn001, Vn014, Vn026, Vout)
    typein      a string (ac or trans)
    '''
    return ('({r},{l},{c}),{cdrp},{sig},{tp}').format(r=rin, l=lin, c=cin, cdrp=Cdrpin, sig=signal, tp=typein)

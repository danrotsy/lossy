# (0.20615528128088/(abs(pi)*sqrt(x))) = (sqrt(2*1.68*10^(-8)/(x*2pi*4pi*10^(-7))) = R
import math
import bode
import num
import cPickle as pickle

def get_skin_transfer_func(folder_dict,rvals,l,c,Cdrpin,res,freq_domain):
    '''
    '''
    transfer_functions = get_r_functions(folder_dict,rvals,l,c,Cdrpin,res)
    return transpose(transfer_functions, rvlas, freq_domain)

def transpose(transfer_functions, rvals, freq_domain):
    '''
    '''
    skin_func = [freq_domain,[],[]]
    for i in range(0,freq_domain):
        f = freq_domain[i]
        r = get_r(f)
        r = num.find_closest(rvals, r)
        func = transfer_functions[r]
        skin_func[1][i] = bode.get_val(func[0],func[1], f)
        skin_func[2][i] = bode.get_val(func[0],func[2], f)
    return skin_func

def get_r_functions(folder_dict,rvals,l,c,Cdrpin,res):
    '''
    '''
    approximations = {}
    transfer_functions = {}
    for r in rvals:
        approximations[r] = folder_dict[vals_to_key(r,l,c,Cdrpin,"Vout","ac")]
        print approximation
    for r in approximations:
        transfer_functions[r] = bode.transfer_function(approximations[r],res)
    return transfer_functions

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

def get_r(frequency):
    '''
    '''
    return 0.20615528128088/(math.pi*math.sqrt(frequency))

def get_main_save():
    '''
    '''
    with open(r'main_save.p', 'rb') as open_file:
        main_save_list = pickle.load(open_file)
        folder_dict = main_save_list[0]
        rvals = main_save_list[1]
        lvals = main_save_list[2]
        cvals = main_save_list[3]
    return folder_dict, rvals, lvals, cvals

folder_dict, rvals, lvals, cvals = get_main_save()

# (0.20615528128088/(abs(pi)*sqrt(x))) = (sqrt(2*1.68*10^(-8)/(x*2pi*4pi*10^(-7))) = R
import math
import bode
import num
import cPickle as pickle

try:
    import matplotlib.pyplot as plt
except ImportError:
    print 'the "matplotlib" library is required for this application'

def get_skin_transfer_func(folder_dict,rvals,l,c,Cdrpin,res,freq_domain):
    '''
    '''
    transfer_functions = get_r_functions(folder_dict,rvals,l,c,Cdrpin,res)
    return transpose(transfer_functions, rvals, freq_domain)

def transpose(transfer_functions, rvals, freq_domain):
    '''
    '''
    skin_func = [freq_domain[1:],[],[]]
    for key in transfer_functions:
        print key
    for i in range(1,len(freq_domain)):
        f= freq_domain[i]
        r = get_r(f)
        r = num.find_closest(rvals, r)
        func = transfer_functions[round(r,2)]
        print f, get_r(f), bode.get_val(func[0],func[1],f)
        skin_func[1].append(bode.get_val(func[0],func[1], f))
        skin_func[2].append(bode.get_val(func[0],func[2], f))
    return skin_func

def get_r_functions(folder_dict,rvals,l,c,Cdrpin,res):
    '''
    '''
    approximations = {}
    transfer_functions = {}

    print rvals
    for r in rvals:
        approximations[round(r,2)] = folder_dict[vals_to_key(r,l,c,Cdrpin,"Vout","ac")]
        print approximations[round(r,2)]
    for r in rvals:
        transfer_functions[round(r,2)] = bode.transfer_function(approximations[round(r,2)],res)
    return transfer_functions

def vals_to_key(rin,lin,cin,Cdrpin,signal,typein):
    '''
    rin         a float or string
    lin         a float or string
    cin         a float or string
    Cdrpin      a float or string
    signal      a string (Vin, Vn001, Vn014, Vn026, Vout)
    typein      a string (ac or trans)
    '''
    r_tuple = (rin,lin,cin)
    return (r_tuple,Cdrpin,signal,typein)

def get_r(frequency):
    '''
    '''
    p = 1.7e-8
    d = get_depth(frequency)
    w = 100e-6
    h = 18e-6
    return p/((2*w*d)+(2*h*d)-(4*d*d))

def get_depth(frequency):
    '''
    '''
    p = 1.7e-8
    u = 4*math.pi*1e-7
    return math.sqrt((2*p)/(frequency*u))

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
skin_transfer_func = get_skin_transfer_func(folder_dict, rvals, lvals[0], cvals[0], 1, 2048, range(0,1000000000,10000000))
plt.plot(skin_transfer_func[0],skin_transfer_func[1])
plt.show()

# ==============================================================================
# ------------------------------------------------------------------------------
# Imports
# ------------------------------------------------------------------------------
try:
    import math
except ImportError:
    print 'the "math" library is required for this application'
try:
    import bode
except ImportError:
    print 'cannot find "skin.py" in the application directory'

try:
    import num
except ImportError:
    print 'cannot find "num.py" in the application directory'

try:
    import cPickle as pickle
except ImportError:
    print 'the "cPickle" library is required for this application'

try:
    import matplotlib.pyplot as plt
except ImportError:
    print 'the "matplotlib" library is required for this application'
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Skin Effect Single Transfer Functions
# ---------------   ---------------------------------------------------------------
# the general function that returns a specific transfer function for given l,c
def get_skin_transfer_func(folder_dict,rvals,l,c,Cdrpin,signal,drops,lenline,pulse,bit_time,res,freq_domain,width,thickness):
    '''
    the general function that returns a specific transfer function for given l,c
    '''
    transfer_functions = get_r_functions(folder_dict,rvals,l,c,Cdrpin,signal,drops,lenline,pulse,bit_time,res)
    return transpose(transfer_functions, rvals, freq_domain,width,thickness)
# overlays transfer functions of different r's to simulate skin effect
def transpose(transfer_functions, rvals, freq_domain,width,thickness):
    '''
    overlays transfer functions of different r's to simulate skin effect
    '''
    skin_func = [freq_domain[1:],[],[]]
    approx_rvals = []
    for j in rvals:
        approx_rvals.append(float('%2.1f'%j))
    print 'NEW RVALS:', approx_rvals
    for i in range(1,len(freq_domain)):
        f= freq_domain[i]
        r = get_r(f,width,thickness)
        print "get_r",r
        if r < 2:
            try:
                func = transfer_functions[float('%2.1f'%r)]
            except KeyError:
                func = transfer_functions[float('%2.1f'%r)+ 0.1]
        elif r > 19:
            try:
                func = transfer_functions[float('%2.1f'%r)]
            except KeyError:
                func = transfer_functions[19.0]
        else:
            try:
                func = transfer_functions[float('%2.1f'%r)]
            except KeyError:
                func = transfer_functions[float('%2.1f'%float('%2.0f'%r))]
        print 'key accessed:',r
        skin_func[1].append(bode.get_val(func[0],func[1], f))
        skin_func[2].append(bode.get_val(func[0],func[2], f))
    return skin_func
# gets all transfer functions with given l and c varying through r
def get_r_functions(folder_dict,rvals,l,c,Cdrpin,signal,drops,lenline,pulse,bit_time,res):
    '''
    gets all transfer functions with given l and c varying through r
    '''
    approximations = {}
    transfer_functions = {}
    for r in rvals:
        approximations[float('%2.1f'%r)] = folder_dict[vals_to_key(r,l,c,Cdrpin,signal,"ac",drops,lenline,pulse,bit_time)]
        print str(folder_dict[vals_to_key(r,l,c,Cdrpin,signal,"ac",drops,lenline,pulse,bit_time)]),'expanded to key','%2.1f'%r
    for r in rvals:
        transfer_functions[float('%2.1f'%r)] = bode.transfer_function(approximations[float('%2.1f'%r)],res)
    return transfer_functions
# given values, finds the key for folder_dict for the file with the given values
def vals_to_key(rin,lin,cin,Cdrpin,signal,typein,drops,lenline,pulse,bit_time):
    '''
    rin         a float or string
    lin         a float or string
    cin         a float or string
    Cdrpin      a float or string
    signal      a string (Vin, Vn001, Vn014, Vn026, Vout)
    typein      a string (ac or trans)
    '''
    r_tuple = (floatToSci(rin),lin,cin)
    return (r_tuple,Cdrpin,signal,typein,drops,lenline,pulse,bit_time)
# finds the resistance due to skin effect at a given frequency
def get_r(frequency,width,thickness):
    '''
    finds the resistance due to skin effect at a given frequency
    '''
    p = 1.68e-8
    d = get_depth(frequency)
    w = width
    h = thickness
    print 'Frequency',frequency, "r", p/((2*w*d)+(2*h*d)-(4*d*d))
    return p/((2*w*d)+(2*h*d)-(4*d*d))
# finds the skin depth due the skin effect at a given frequency
def get_depth(frequency):
    '''
    finds the skin depth due the skin effect at a given frequency
    '''
    p = 1.7e-8
    u = 4*math.pi*1e-7
    return math.sqrt((2*p)/(2*math.pi*frequency*u))
# gets folder_dict, rvals, lvals, and cvals from main_save.p (from main.py)
def get_main_save():
    '''
    gets folder_dict, rvals, lvals, and cvals from main_save.p (from main.py)
    '''
    with open(r'main_save.p', 'rb') as open_file:
        main_save_list = pickle.load(open_file)
        folder_dict = main_save_list[0]
        rvals = main_save_list[1]
        lvals = main_save_list[2]
        cvals = main_save_list[3]
    return folder_dict, rvals, lvals, cvals
# returns a string with the same value as the float in scientific notation
def floatToSci(num):
    '''
    num:  a float

    returns a string with the same value as the float in scientific notation
    '''
    conversion = {0:'',3:'m',6:'u',9:'n',12:'p',15:'f'}
    num = float(num)
    for m in range(0,16,3):
        if num*(10**m)>=1:
            num = '%3.1f' %(num*(10**m))
            return str(num) + conversion[m]
    # returns a float of the same value as the original scientific notation
def sciToFloat(sci_string):
    '''
    sci_string: a string in scientific notation

    returns a float of the same value as the original scientific notation
    '''
    conversion = {"":0,"m":-3,"u":-6,"n":-9,"p":-12,"f":-15}
    multiplier = ''
    num = ''
    for c in sci_string:
        if c in '0123456789' or c=='.':
            num += c
        else:
            multiplier += c
    out = float(num)*(10**conversion[multiplier])
    return out
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Testing
# ------------------------------------------------------------------------------
# folder_dict, rvals, lvals, cvals = get_main_save()
# skin_transfer_func = get_skin_transfer_func(folder_dict, rvals, lvals[0], cvals[0], 1, 2048, range(0,1000000000,10000000))
# plt.plot(skin_transfer_func[0],skin_transfer_func[1])
# plt.show()
# ==============================================================================

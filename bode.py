# ==============================================================================
# ------------------------------------------------------------------------------
# Imports
# ------------------------------------------------------------------------------
try:
    import numpy as np
except ImportError:
    print 'the "numpy" library is required for this application'

try:
    import matplotlib.pyplot as plt
except ImportError:
    print 'the "matplotlib" library is required for this application'

try:
    import cmath
except ImportError:
    print 'the "cmath" library is required for this application'

try:
    import math
except ImportError:
    print 'the "math" library is required for this application'

try:
    import num as num
except ImportError:
    print 'cannot find "num.py" in application directory'
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Transfer Functions
# ------------------------------------------------------------------------------
# applies a filter to a frequency amplitude domain
def apply_transfer_func(freq,amp,transfer_func):
    '''
    applies a filter to a frequency amplitude domain
    '''
    new_amp = []
    for i in range(0,len(amp)):
        current_freq = freq[i]
        # if freq < 0, negate angle (for conjugate)
        if current_freq < 0:
            # get the attenuation from the transfer function
            attenuation = get_val(transfer_func[0],transfer_func[1],abs(current_freq))
            # get the negative of the transfer funciton's angle
            angle = -(get_val(transfer_func[0],transfer_func[2],abs(current_freq)))
        # if freq >= 0, leave angle the same
        else:
            # get the attenuation from the transfer function
            attenuation = get_val(transfer_func[0],transfer_func[1],current_freq)
            # get the transfer funciton's angle
            angle = (get_val(transfer_func[0],transfer_func[2],current_freq))
        # convert angle to complex number
        phase_shift = cmath.exp(complex(0,math.radians(angle)))
        # apply attenuation and phase shift to old amp to get new amp
        new_amp.append(attenuation*phase_shift*amp[i])
    return new_amp
# returns the invert_transfer_function of a file at a given resolution
def invert_transfer_function(file_path,res):
    '''
    generates the inverse transfer function over a frequency domain given the file_path and res (resolution)
    '''
    # opens file
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_func_inv = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-1]
            line_split = line_split.split(',')
            # convert from dB to 1/H. 1/H = 10^(-dB/20)
            line_split[1] = pow(10,(float(line_split[1])/-20.0))
            # negate angle
            line_split[2] = float(line_split[2])*(-1.0)
            # add frequency in first slot
            transfer_func_inv[0].append(float(line_split[0]))
            # add 1/H in second slot
            transfer_func_inv[1].append(line_split[1])
            # add negated angle in third slot
            transfer_func_inv[2].append(line_split[2])
            # new line if method for saving transfer functions is added
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    return transfer_func_inv
# returns the transfer funciton of a file at a give resolution
def transfer_function(file_path,res):
    '''
    generates the transfer function of a frequency domain given the file_path and res (resolution)
    '''
    # opens file
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_function = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-1]
            line_split = line_split.split(',')
            # convert from dB to H. H = 10^(dB/20)
            line_split[1] = pow(10,(float(line_split[1])/20.0))
            # add frequency in first slot
            transfer_function[0].append(float(line_split[0]))
            # add H in second slot
            transfer_function[1].append(line_split[1])
            # add angle in third slot
            transfer_function[2].append(float(line_split[2]))
            # new line if method for saving transfer functions is added
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    return transfer_function
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Filters
# ------------------------------------------------------------------------------
# high pass filter
def high_pass_filter(bot_freq, top_freq):
    '''
    highpass filter         (top_freq) /--------------- 1.0
                                      /
                    0.0 -------------/ (bot_freq)
    '''
    return [[0.0,bot_freq,top_freq,1.0e15],[0.0,0.0,1.0,1.0],[0.0,0.0,0.0,0.0]]
# low pass filter
def low_pass_filter(top_freq,bot_freq):
    '''
    lowpass filter  1.0 -------------\ (top_freq)
                                      \
                            (bot_freq) \--------------- 0.0
    '''
    return [[0.0,top_freq,bot_freq,1.0e15],[1.0,1.0,0.0,0.0],[0.0,0.0,0.0,0.0]]
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Fourier
# ------------------------------------------------------------------------------
# returns the amplitude and frequency of a transient plot, with or without shift
def trans_fourier(file_path, shift, res):
    '''
    returns the amplitude and frequency of a transient plot, with or without shift
    '''
    data = np.genfromtxt(file_path, delimiter=',', names=['t', 'Vout'])
    t = []
    v = []
    for datum in data[1:]:
        t.append(datum[0])
        v.append(datum[1])
    tnew = genrange(1e-7,res)
    vnew = []
    for val in tnew:
        left,right,left_index,right_index = num.find_closest_two(t,val)
        deltav = float(v[right_index]) - float(v[left_index])
        deltat = float(right) - float(left)
        if deltat == 0:
            vnew.append(v[left_index])
        else:
            vnew.append(deltav*((val-t[left_index])/deltat)+v[left_index])
    amp = np.fft.fft(vnew)
    freq = np.fft.fftfreq(res,(1e-7/float(res)))
    if shift:
        freq = np.fft.fftshift(freq)
    return t,v,amp,freq
# combines amplitudes across different frequencies and applies them to the time domain
def inv_fourier(freq,amp):
    '''
    returns the inverse fourier transform
    '''
    return np.fft.ifft(amp)
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# Linear Interpolation
# ------------------------------------------------------------------------------
# returns a constrained jump set of a function (represented the sets x1 and x2) over a given resolution
def fill_func(x1,x2,n):
    '''
    generates a constrained jump set of a function (represented the sets x1 and x2) over a given resolution
    '''
    rng = max(x1)-min(x1)
    xnew = genrange(max(x1)-(rng/float(n)),n)
    ynew = []
    for val in xnew:
        left,right,left_index,right_index = num.find_closest_two(x1,val)
        deltav = float(x2[right_index]) - float(x2[left_index])
        deltat = float(right) - float(left)
        if deltat == 0:
            ynew.append(x2[left_index])
        else:
            ynew.append(deltav*((val-x1[left_index])/deltat)+x2[left_index])
    return xnew, ynew
# returns the value of a set function at a value undefined over the set's limited domain
def get_val(func_x,func_y,x_in):
    '''
    given a set represented function, returns the best approximation for a val in its undefined domain
    '''
    if x_in > max(func_x):
        return func_y[-1]
    else:
        left,right,left_index,right_index = num.find_closest_two(func_x,x_in)
        deltav = float(func_y[right_index]) - float(func_y[left_index])
        deltat = float(right) - float(left)
        if deltat < 1:
            return float(func_y[left_index])
        return deltav*((x_in-func_x[left_index])/deltat)+func_y[left_index]
# returns a range of floats
def genrange(interval, n):
    '''
    generates a range of floats
    '''
    assert type(interval) == float
    assert type(n) == int
    n = float(n)
    inc = interval/n
    r = []
    cur = 0
    for i in range(0, int(n)):
        r.append(cur)
        cur += inc
    return r
# ==============================================================================


# ==============================================================================
# ------------------------------------------------------------------------------
# TESTING
# ------------------------------------------------------------------------------
# res = 4096
#
# t,v,amp,freq = trans_fourier(r"RCtest\RC_tran_Vn002.csv", False, res)
# transfer_func_inv = invert_transfer_function(r"RCtest\RC_ac_Vn002.csv",res)
# transfer_func = transfer_function(r"RCtest\RC_ac_Vn002.csv",res)
#
# t,v,amp,freq = trans_fourier(r"RCtest\RC_Vc_tran.csv", False, res)
# transfer_func_inv = invert_transfer_function(r"RCtest\RC_Vc_ac.csv",res)
# transfer_func = transfer_function(r"RCtest\RC_Vc_ac.csv",res)
# low_pass_filter1 = low_pass_filter(3e8,5e8)
# low_pass_filter2 = low_pass_filter(8e8,1e9)
# print get_val(giga_hz_filter[0],giga_hz_filter[1],8.0e8)
#
# new_amp = apply_transfer_func(freq,amp,transfer_func_inv)
# new_amp = apply_transfer_func(freq,new_amp,transfer_func_inv)
# new_amp = apply_transfer_func(freq,new_amp,low_pass_filter2)
#
# new_v = inv_fourier(freq, new_amp)
#
#
# plt.subplot(2,1,1)
# plt.plot(genrange(1e-7,len(new_v)),new_v.real,color='red',linestyle='-')
# plt.plot(t,v,color='blue',linestyle='-')
#
# plt.subplot(2,1,2)
# plt.plot(transfer_func[0],transfer_func[1],color='red',linestyle='-')
# plt.plot(transfer_func[0],transfer_func[2],color='red',linestyle='-')
# plt.plot(transfer_func_inv[0],transfer_func_inv[1],color='blue',linestyle='-')
# plt.plot(transfer_func_inv[0],transfer_func_inv[2],color='blue',linestyle='-')
#
# plt.show()
# ==============================================================================

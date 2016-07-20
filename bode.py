import numpy as np
import num as num
import cmath
import math
import matplotlib.pyplot as plt

def apply_transfer_func(freq,amp,transfer_func): #freq,amp,transfer_func
    '''
    '''
    new_amp = [] # can this be a list?
    for i in range(0,len(amp)):
        current_freq = freq[i]
        if current_freq < 0:
            attenuation = get_val(transfer_func[0],transfer_func[1],abs(current_freq))
            angle = -(get_val(transfer_func[0],transfer_func[2],abs(current_freq)))
        else:
            attenuation = get_val(transfer_func[0],transfer_func[1],current_freq)
            angle = (get_val(transfer_func[0],transfer_func[2],current_freq))
        #print 'angle', angle
        phase_shift = cmath.exp(complex(0,math.radians(angle)))
        print 'f:',current_freq, ',','att:',attenuation,',','phi:', angle,',',phase_shift
        new_amp.append(attenuation*phase_shift*amp[i]) # *attenuation *phase_shift
    return new_amp

# returns the inverse transfer function of a file at a given resolution
def invert_transfer_function(file_path,res):
    '''
    generates the inverse transfer function over a frequency domain given the file_path and res (resolution)
    '''
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_func_inv = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-1]
            line_split = line_split.split(',')
            line_split[1] = pow(10,(float(line_split[1])/-20.0))
            line_split[2] = float(line_split[2])*(-1.0)
            transfer_func_inv[0].append(float(line_split[0]))
            transfer_func_inv[1].append(line_split[1])
            transfer_func_inv[2].append(line_split[2])
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    # freq_filled, mag_filled = fill_func(transfer_func_inv[0],transfer_func_inv[1],res)
    # place_holder, phase_filled = fill_func(transfer_func_inv[0],transfer_func_inv[2],res)
    # transfer_func_inv_filled = [freq_filled,mag_filled,phase_filled]

    return transfer_func_inv

# returns the transfer funciton of a file at a give resolution
def transfer_function(file_path,res):
    '''
    generates the transfer function of a frequency domain given the file_path and res (resolution)
    '''
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_function = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-1]
            line_split = line_split.split(',')
            line_split[1] = pow(10,(float(line_split[1])/20.0))
            transfer_function[0].append(float(line_split[0]))
            transfer_function[1].append(line_split[1])
            transfer_function[2].append(float(line_split[2]))
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    # freq_filled, mag_filled = fill_func(transfer_function[0],transfer_function[1],res)
    # place_holder, phase_filled = fill_func(transfer_function[0],transfer_function[2],res)
    # transfer_function_filled = [freq_filled,mag_filled,phase_filled]

    return transfer_function

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
        # print deltat, x_in, ((x_in-func_x[left_index])/deltat), func_y[left_index], deltav, deltav*((x_in-func_x[left_index])/deltat)+func_y[left_index]
        if deltat < 1:
            return float(func_y[left_index])
        return deltav*((x_in-func_x[left_index])/deltat)+func_y[left_index]

# returns the amplitude and frequency of a transient plot, with or without shift
def trans_fourier(file_path, shift):
    '''
    returns the amplitude and frequency of a transient plot, with or without shift
    '''
    data = np.genfromtxt(file_path, delimiter=',', names=['t', 'Vout'])
    t = []
    v = []
    for datum in data[1:]:
        t.append(datum[0])
        v.append(datum[1])
    tnew = genrange(1e-7,4096)
    vnew = []
    for val in tnew:
        left,right,left_index,right_index = num.find_closest_two(t,val)
        deltav = float(v[right_index]) - float(v[left_index])
        deltat = float(right) - float(left)
        if deltat == 0:
            vnew.append(v[left_index])
        else:
            vnew.append(deltav*((val-t[left_index])/deltat)+v[left_index])
        # print val,left, right, trel
    # print t
    # plt.plot(t, v, color='blue', linestyle='-')
    amp = np.fft.fft(vnew)
    freq = np.fft.fftfreq(4096,(1e-7/4096.0))
    if shift:
        freq = np.fft.fftshift(freq) # is this necessary?
    # plt.plot(freq, np.absolute(amp)**2, color='red', linestyle='-')
    # plt.plot(freq, amp.real, color='red', linestyle='-')
    # plt.plot(freq, amp.imag, color='blue', linestyle='-')
    # plt.show()
    return t,v,amp,freq

def inv_fourier(freq,amp):
    return np.fft.ifft(amp)

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

t,v,amp,freq = trans_fourier(r"C:\Users\HEP\Desktop\RC_tran_Vn002.csv", False)
transfer_func_inv = invert_transfer_function(r"C:\Users\HEP\Desktop\RC_ac_Vn002.csv",4096)
transfer_func = transfer_function(r"C:\Users\HEP\Desktop\RC_ac_Vn002.csv",4096)

new_amp = apply_transfer_func(freq,amp,transfer_func_inv)
# print type(amp),type(new_amp)
# print len(t),len(new_amp)
# for i in range(0,len(amp)):
#     print freq[i],amp[i]
new_v = inv_fourier(freq, new_amp)

# for i in range(0, len(transfer_func[0])):
#     print_tuple = (round(transfer_func[0][i],4),
#                     "amp:", round(transfer_func[1][i],4),round(transfer_func_inv[1][i],4),
#                     round(transfer_func[1][i]*transfer_func_inv[1][i],4),
#                     "phase:",round(transfer_func[2][i],4),round(transfer_func_inv[2][i],4),
#                     round(transfer_func[2][i]+transfer_func_inv[2][i],4))
#     print_string = ''
#     for entry in print_tuple:
#         print_string += str(entry) + ','
#     print print_string

# print get_val(transfer_func_inv[0],transfer_func_inv[1], 4.4e8)

plt.subplot(2,1,1)
plt.plot(genrange(1e-7,len(new_v)),new_v.real,color='red',linestyle='-')
plt.plot(t,v,color='blue',linestyle='-')

plt.subplot(2,1,2)
plt.plot(transfer_func[0],transfer_func[1],color='red',linestyle='-')
plt.plot(transfer_func[0],transfer_func[2],color='red',linestyle='-')
plt.plot(transfer_func_inv[0],transfer_func_inv[1],color='blue',linestyle='-')
plt.plot(transfer_func_inv[0],transfer_func_inv[2],color='blue',linestyle='-')

plt.show()

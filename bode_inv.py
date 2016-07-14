import numpy as np
import matplotlib.pyplot as plt

def invert_transfer_function(file_path):
    '''
    '''
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_func_inv = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-2]
            line_split = line_split.split(',')
            line_split[1] = pow(10,(float(line_split[1])*(-1.0)/20.0))
            line_split[2] = float(line_split[2])*(-1.0)
            transfer_func_inv[0].append(float(line_split[0]))
            transfer_func_inv[1].append(line_split[1])
            transfer_func_inv[2].append(line_split[2])
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    print "transfer_func_inv"
    for i in range(0,20):
        print transfer_func_inv[0][i],transfer_func_inv[1][i],transfer_func_inv[2][i]

    return transfer_func_inv

def transfer_function(file_path):
    '''
    '''
    with open(file_path, 'r') as file:
        data = file.readlines()

    transfer_function = [[],[],[]]

    for line in data[2:]:
        if len(line)>2:
            line_split = line[:-2]
            line_split = line_split.split(',')
            line_split[1] = pow(10,(float(line_split[1])*(1.0)/20.0))
            transfer_function[0].append(float(line_split[0]))
            transfer_function[1].append(line_split[1])
            transfer_function[2].append(float(line_split[2]))
            line = str(line_split[0]) + ',' + str(line_split[1]) + ',' + str(line_split[2]) + r'\n'

    freq_filled,mag_filled = fill_func(transfer_function[0],transfer_function[1],120)
    place_holder, phase_filled = fill_func(transfer_function[0],transfer_function[2],120)
    transfer_function_filled = [freq_filled,mag_filled,phase_filled]

    print "transfer_function"
    for i in range(0,20):
        print transfer_function[0][i],transfer_function[1][i],transfer_function[2][i]

    print "transfer_function filled"
    for i in range(0,20):
        print transfer_function_filled[0][i],transfer_function_filled[1][i],transfer_function_filled[2][i]

    plt.plot(transfer_function[0], transfer_function[2], color='red', linestyle='-')
    plt.plot(transfer_function_filled[0], transfer_function_filled[2], color='blue', linestyle='-')
    plt.show()

    return transfer_function

def fill_func(x1,x2,n,domain=0):
    if domain==0:
        rng = max(x1)-min(x1)
        xnew = genrange(max(x1)-(rng/float(n)),n)
    else:
        xnew = genrange(domain,n)
    ynew = []
    for val in xnew:
        left,right,left_index,right_index = findClosestTwo(x1,val)
        deltav = x2[right_index] - x2[left_index]
        deltat = right - left
        ynew.append(deltav*((val-x1[left_index])/deltat)+x2[left_index])
    return xnew, ynew

def trans_fourier(file_path, shift):
    data = np.genfromtxt(file_path, delimiter=',', names=['t', 'Vout'])
    t = []
    v = []
    for datum in data[1:]:
        t.append(datum[0])
        v.append(datum[1])
    tnew = genrange(9e-8,110)
    vnew = []
    for val in tnew:
        left,right,left_index,right_index = findClosestTwo(t,val)
        deltav = v[right_index] - v[left_index]
        deltat = right - left
        vnew.append(deltav*((val-t[left_index])/deltat)+v[left_index])
        # print val,left, right, trel
    # print t
    # plt.plot(t, v, color='blue', linestyle='-')
    amp = np.fft.fft(vnew)/110
    freq = np.fft.fftfreq(110,(9e-8/110.0))
    if shift:
        freq = np.fft.fftshift(freq) # is this necessary?
    # plt.plot(freq, np.absolute(amp)**2, color='red', linestyle='-')
    # plt.plot(freq, amp.real, color='red', linestyle='-')
    # plt.plot(freq, amp.imag, color='blue', linestyle='-')
    # plt.show()
    return amp,freq

def findClosestTwo(in_list,val):
    '''
    list:      the list to be searched
    val:       the value to be compared

    returns the element in list closest to val
    '''
    ref = min(in_list, key=(lambda x:abs(x-val)))
    if ref >= val:
        right = in_list[in_list.index(ref)+1]
        left = ref
        right_index = in_list.index(ref)+1
        left_index = in_list.index(ref)
    else:
        left = in_list[in_list.index(ref)-1]
        right = ref
        right_index = in_list.index(ref)
        left_index = in_list.index(ref)-1
    return left, right, left_index, right_index

def genrange(interval, n):
    '''
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

#trans_fourier(r"MultidropTransient\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42p.csv", False)
invert_transfer_function(r"MultidropBode\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42p.csv")
transfer_function(r"MultidropBode\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42p.csv")

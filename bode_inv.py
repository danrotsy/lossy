import numpy as np
import matplotlib.pyplot as plt

def invert_bode(old_path, new_path):
    '''
    '''
    with open(old_path, 'r') as file:
        data = file.readlines()

    data[0] = "Freq.,V,Deg."

    for line in data[1:]:
        if line != r"\n":
            line_split = line[:-2]
            line_split = line_split.split(',')
            line_split[1] = line_split[1]*(-1)
            line_split[2] = line_split[2]*(-1)
            line = line_split[0] + ',' + line_split[1] + ',' + line_split[2] + r'\n'

    with open(new_path, 'w') as file:
        file.writelines(data)

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
    amp = np.fft.fft(vnew)
    freq = np.fft.fftfreq(110,(9e-8/110.0))
    if shift:
        freq = np.fft.fftshift(freq) # is this necessary?
    # plt.plot(freq, np.absolute(amp)**2, color='red', linestyle='-')
    plt.plot(freq, amp.real, color='red', linestyle='-')
    plt.plot(freq, amp.imag, color='blue', linestyle='-')

    # n = 100
    # dx = 5.0
    # x = dx*np.arange(0,n)
    # w1 = 100.0
    # w2 = 20.0
    # fx = np.sin(2*np.pi*x/w1) + 2*np.cos(2*np.pi*x/w2)
    # Fk = np.fft.fft(fx)/n
    # nu = np.fft.fftfreq(n,dx)
    # plt.plot(nu, Fk.real, color='red', linestyle='-')
    plt.show()

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

trans_fourier(r"MultidropTransient\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42p.csv", True)

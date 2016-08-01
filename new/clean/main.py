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
    from Tkinter import Tk,BOTH,BooleanVar, StringVar, IntVar
except ImportError:
    print 'the "Tkinter" library is required for this application'

try:
    from ttk import Frame, Button, Scale, OptionMenu, Label, Checkbutton
except ImportError:
    print 'the "ttk" library is required for this application'

try:
    import bode
except:
    print 'cannot find "bode.py" in application directory'

try:
    import skin
except:
    print 'cannot find "skin.py" in application directory'

try:
    import cPickle as pickle
except:
    print 'the "cPickle" library is required for this application'

try:
    import os
except:
    print 'the "os" library is required for this application'

from numpy import arange
# ==============================================================================





class MainMenu(Frame):

    # ==========================================================================
    # --------------------------------------------------------------------------
    # GUI creation methods
    # --------------------------------------------------------------------------
    # instatiation method of MainMenu
    def __init__(self, parent):
        '''
        instatiates main menu as a frame with Frame as parent and calls initUI()
        '''

        self.R_low1 = self.sciToFloat("0.02")
        self.R_high1 = self.sciToFloat("2.00")
        self.R_low2 = self.sciToFloat("3.00")
        self.R_high2 = self.sciToFloat("20.0")
        # self.R_high = self.sciToFloat("1.8")
        self.R_step1 = self.sciToFloat("0.2")
        self.R_step2 = self.sciToFloat("1.00")

        self.L_low = self.sciToFloat("0.079n")
        self.L_high = self.sciToFloat("7.9n")
        # self.L_high = self.sciToFloat("7.11n")
        self.L_step = self.sciToFloat("0.79n")

        self.C_low = self.sciToFloat("0.22p")
        self.C_high = self.sciToFloat("22p")
        # self.C_high = self.sciToFloat("19.8p")
        self.C_step = self.sciToFloat("2.2p")

        #each key will be formatted as such: (r, l, c), Cdrp, signal, ac/trans
        #Ex. folder_dict[[(1.02, 1.659e-9, 2.42e-12), 1, "Vout", "trans"]] =
        #r"MultidropTransient\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42pRun1491331.csv"
        #self.<X>vals lists will be in floats
        # self.folder_dict = {}
        self.Rvals = []
        self.Rskinvals = []
        self.Lvals = []
        self.Cvals = []
        #determine the Rvals, Lvals, and Cvals from a folder in the home directory
        for r in arange(self.R_low1, self.R_high1, self.R_step1):
            self.Rvals.append(r)
        for r in arange(self.R_low2, self.R_high2, self.R_step2):
            self.Rskinvals.append(r)
        for l in arange(self.L_low, self.L_high, self.L_step):
            self.Lvals.append(l)
        for c in arange(self.C_low, self.C_high, self.C_step):
            self.Cvals.append(c)
        Frame.__init__(self, parent)
        self.parent = parent
        self.initUI()
        self.createFileDict()
        for key in self.folder_dict:
            print
            print 'KEY:', key
            print 'DIR:', self.folder_dict[key]
            print
        print 'FOLDER_DICT SIZE:', len(self.folder_dict)
        print
    # sets up the GUI itself
    def initUI(self):
        '''
        sets a title
        packs the frame
        instantiates plot button
        instantiates r, l and c lock buttons
        instantiates r, l and c sliders
        instantiates option menu
        '''
        self.parent.title("Transmission Lines")
        self.pack(fill=BOTH, expand=1)

        plotButton = Button(self, text="Plot", command=self.gui_to_plots, width= 20)
        plotButton.place(x=5, y=95)

        self.rlockvar = BooleanVar()
        rlock = Checkbutton(self, text="R", variable=self.rlockvar)
        rlock.place(x=5,y=5)

        self.llockvar = BooleanVar()
        llock = Checkbutton(self, text="L", variable=self.llockvar)
        llock.place(x=5,y=35)

        self.clockvar = BooleanVar()
        clock = Checkbutton(self, text="C", variable=self.clockvar)
        clock.place(x=5,y=65)

        self.rlabelvar = StringVar()
        self.rlabelvar.set('20mOhm/m')
        self.rlabel = Label(self, textvariable=self.rlabelvar)
        self.rlabel.place(x=235,y=5)
        self.rscale = Scale(self, from_=0.02, to_=2.00, length=185, command=self.update_rlabel) #0.2
        self.rscale.place(x=50,y=5)

        self.llabelvar = StringVar()
        self.llabelvar.set('100pH/m')
        self.llabel = Label(self, textvariable=self.llabelvar)
        self.llabel.place(x=235,y=35)
        self.lscale = Scale(self, from_=0.079*(10**-9), to_=7.9*(10**-9), length=185, command=self.update_llabel) #0.79n
        self.lscale.place(x=50,y=35)

        self.clabelvar = StringVar()
        self.clabelvar.set('200fF/m')
        self.clabel = Label(self, textvariable=self.clabelvar)
        self.clabel.place(x=235,y=65)
        self.cscale = Scale(self, from_=0.22*(10**-12), to_=22*(10**-12), length=185, command=self.update_clabel) #2.2p
        self.cscale.place(x=50,y=65)

        self.vilockvar = BooleanVar()
        vilock = Checkbutton(self, text="Vin", variable=self.vilockvar)
        vilock.place(x=320,y=5)

        self.dilockvar = BooleanVar()
        dilock = Checkbutton(self, text="First Drop", variable=self.dilockvar)
        dilock.place(x=370,y=5)

        self.dmlockvar = BooleanVar()
        dmlock = Checkbutton(self, text="Mid Drop", variable=self.dmlockvar)
        dmlock.place(x=370,y=35)

        self.dflockvar = BooleanVar()
        dflock = Checkbutton(self, text="Last Drop", variable=self.dflockvar)
        dflock.place(x=370,y=65)

        self.volockvar = BooleanVar()
        volock = Checkbutton(self, text="Vout", variable=self.volockvar)
        volock.place(x=320,y=35)

        self.cdscalevar = IntVar()
        self.cdscalevar.set(1)
        self.cdlabelvar = StringVar()
        self.cdlabelvar.set('1pF')
        self.cdlabelstatic = Label(self, text='Cdrp')
        self.cdlabelstatic.place(x=265, y=98)
        self.cdlabel = Label(self,textvariable=self.cdlabelvar)
        self.cdscale = Scale(self, from_=1, to_=5, variable = self.cdscalevar,command=self.update_cd)
        self.cdscale.place(x=310, y=98)
        self.cdlabel.place(x=420,y=98)

        self.lenscalevar = IntVar()
        self.lenscalevar.set(1)
        self.lenlabelvar = StringVar()
        self.lenlabelvar.set('1cm')
        self.lenlabelstatic = Label(self, text='Length')
        self.lenlabelstatic.place(x=265, y=128)
        self.lenlabel = Label(self,textvariable=self.lenlabelvar)
        self.lenscale = Scale(self, from_=1, to_=9, variable = self.lenscalevar,command=self.update_len)
        self.lenscale.place(x=310, y=128)
        self.lenlabel.place(x=420,y=128)

        self.errormsgvar = StringVar()
        self.errormsg = Label(self, textvariable=self.errormsgvar, foreground='red')
        self.errormsg.place(x=5,y=125)

        self.var = StringVar(self)
        self.var.set('Transient')
        self.libary = OptionMenu(self, self.var,'', 'Transient', 'Bode', 'Inverse', 'Skin Depth', 'Skin Depth Bode', 'Skin Depth Inv.')
        self.libary.place(x=145,y=95)

        self.loadvar = StringVar(self)
        self.loadvar.set('10 Loads')
        self.loadlibary = OptionMenu(self, self.loadvar,'', '10 Loads', '14 Loads', '28 Loads')
        self.loadlibary.place(x=145,y=125)
    # updates r slider label
    def update_rlabel(self, val):
        #self.rlabelvar.set(round(float(val),1))
        self.rlabelvar.set(str(self.floatToSci(val)+'Ohm/m'))
    # updates l slider label
    def update_llabel(self, val):
        #self.llabelvar.set(round(float(val),10))
        self.llabelvar.set(str(self.floatToSci(val)+'H/m'))
    # updates c slider label
    def update_clabel(self, val):
        #self.clabelvar.set(round(float(val),13))
        self.clabelvar.set(str(self.floatToSci(val)+'F/m'))
    # updates Cdrp slider label
    def update_cd(self,val):
        self.cdlabelvar.set(str(int(round(float(val))))+'pF')
        self.cdscalevar.set(int(round(float(val))))
    #
    def update_len(self,val):
        self.lenlabelvar.set(str(int(round(float(val))))+'cm')
        self.lenscalevar.set(int(round(float(val))))
    # ==========================================================================


    # ==========================================================================
    # --------------------------------------------------------------------------
    # File-GUI interaction
    # --------------------------------------------------------------------------
    # finds and plots all files given info on the gui
    def gui_to_plots(self):
        '''
        finds and plots all files given info on the gui
        '''
        #
        self.errormsgvar.set('')
        #
        out,to_process,signal,length,drops,x_axis_title,y_axis_title = self.gui_out()
        #
        title = self.var.get().upper() + ' ' + signal.upper()+ '  Length: ' + str(length) + 'cm  Loads: ' + str(drops) + '\n'+ 'Cdrp=' + str(self.cdlabelvar.get())  + '  ' + self.get_name_tag()
        #
        todo = self.get_todo(out,signal)
        #
        self.convert_and_run(todo)
        #
        print "found files"
        if to_process == False:
            #
            self.plot_simple(todo, title, x_axis_title, y_axis_title)
        else:
            #
            self.plot_complex(todo, title, x_axis_title, y_axis_title)
    # returns the output of the gui in a comprehensible format (out,process)
    def gui_out(self):
        '''
        returns the output of the gui in a comprehensible format (out,process)
        '''
        out = []
        x_axis_title = ''
        y_axis_title = ''
        if self.rlockvar.get():
            r = []
            r.append(self.findClosest(self.Rvals, self.rscale.get()))
            out.append(r)
        else:
            out.append(self.Rvals)
        if self.llockvar.get():
            l = []
            l.append(self.findClosest(self.Lvals, self.lscale.get()))
            out.append(l)
        else:
            out.append(tuple(self.Lvals))
        if self.clockvar.get():
            c =[]
            c.append(self.findClosest(self.Cvals, self.cscale.get()))
            out.append(c)
        else:
            out.append(tuple(self.Cvals))
        out.append(self.cdscale.get())
        changed = 0
        signal = ''
        if self.vilockvar.get():
            signal = 'Vin'
            changed += 1
        if self.dilockvar.get():
            signal = 'Vf'
            changed += 1
        if self.dmlockvar.get():
            signal = 'Vm'
            changed += 1
        if self.dflockvar.get():
            signal = 'Vd'
            changed += 1
        if self.volockvar.get():
            signal = 'Vout'
            changed += 1
        if changed == 0:
            self.errormsgvar.set('must enter a signal')
            return None
        if changed > 1:
            self.errormsgvar.set('cannot scan more than one node')
            self.vilockvar.set(False)
            self.dilockvar.set(False)
            self.dmlockvar.set(False)
            self.dflockvar.set(False)
            self.volockvar.set(False)
            return None
        if signal == 'Vin':
            self.errormsgvar.set('the "vin" libraries are coming soon...')
            self.vilockvar.set(False)
            self.dilockvar.set(False)
            self.dmlockvar.set(False)
            self.dflockvar.set(False)
            self.volockvar.set(False)
            return None
        if self.cdscale.get() == 0:
            self.errormsgvar.set('the bus libraries are coming soon...')
            return None
        out.append(signal)
        atype = []
        to_process = False
        if self.var.get() == 'Bode':
            atype.append('ac')
            x_axis_title = 'Frequency(Hz)'
            y_axis_title = 'Gain(dB)'
        if self.var.get() == 'Transient':
            atype.append('trans')
            x_axis_title = 'Time(s)'
            y_axis_title = 'Voltage(V)'
        if (self.var.get() == 'Inverse') or (self.var.get() == 'Skin Depth') or (self.var.get() == 'Skin Depth Inv.'):
            to_process = True
            atype.append('ac')
            atype.append('trans')
            x_axis_title = 'Time(s)'
            y_axis_title = 'Voltage(V)'
        if (self.var.get() == 'Skin Depth Bode'):
            to_process = True
            atype.append('ac')
            atype.append('trans')
            x_axis_title = 'Frequency(Hz)'
            y_axis_title = 'Gain(dB)'
        if to_process and (self.var.get() != 'Inverse'):
            out[0] = self.Rvals
            for val in self.Rskinvals:
                out[0].append(val)
        out.append(atype)
        drops = None
        if self.loadvar.get() == '10 Loads':
            drops = 10
        if self.loadvar.get() == '14 Loads':
            drops = 14
        if self.loadvar.get() == '28 Loads':
            drops = 28
        out.append(drops)
        length = self.lenscale.get()
        out.append(length)
        return out,to_process,signal,drops,length,x_axis_title,y_axis_title
    # creates a list of r,l and c's to inspect and plot
    def get_todo(self, out, signal):
        '''
        creates a list of r,l and c's to inspect and plot
        '''
        todo = []
        for r in out[0]:
            for l in out[1]:
                for c in out[2]:
                    for a in out[5]:
                        todo.append(((self.floatToSci(r),self.floatToSci(l),self.floatToSci(c)),out[3],signal,a,out[6],out[7]))
        print
        print 'TODO:'
        for item in todo:
            print item
        print
        return todo
    #
    def convert_and_run(self,todo):
        '''
        '''
        key_list = todo
        for key in key_list:
            if key not in self.folder_dict.keys():
                filename = 'transmission_line_{}_dropoffs'.format(key[4])
                if(key[3] == 'trans'):
                    analysis = 'tran'
                    path = 'Transient'
                else:
                    analysis = 'ac'
                    path = 'Bode'
                cdrp = '{}p'.format(key[1])
                lenline = key[5]
                cmd = 'Auto_LTSpice_Now.exe {} {} {} {} {} {} {} {}'.format(key[4], analysis, cdrp, lenline, key[0][0], key[0][1], key[0][2], key[2])
                os.system(cmd)
                self.folder_dict[key] = '../data/{}/Multidrop{}/{}_Rline={}_Lline={}_Cline={}_Cdrp={}_Lenline={}_{}.csv'.format(filename, path, analysis, key[0][0], key[0][1],key[0][2],cdrp,lenline,key[2]) #self.floatToSci(key[0][0]), self.floatToSci(key[0][1]), self.floatToSci(key[0][2]),cdrp, lenline, key[2])
                print "COMMAND:", cmd
                print "KEY:", key
                print "FOLDER_DICT", self.folder_dict[key]
    # plots simple transient or bode plots
    def plot_simple(self, todo, title, x_axis_title, y_axis_title):
        '''
        plots simple transient or bode plots
        '''
        instructions = []
        color_step = 1.0/len(todo)
        color = 0
        for item in todo:
            instruction = [0,0,0]
            instruction[0] = self.folder_dict[item]
            instruction[1] = item[2]
            instruction[2] = (color,0,1-color)
            instructions.append(instruction)
            color+=color_step
        self.new_figure(title, instructions, x_axis_title, y_axis_title)
    # plots transient or bode plots after transforms are applied
    def plot_complex(self, todo, title, x_axis_title, y_axis_title):
        '''
        plots transient or bode plots after transforms are applied
        '''
        res = 4096
        selected_ac = []
        selected_trans = []
        low_pass_filter = bode.low_pass_filter(8e8,1e9)
        t_list = []
        v_list = []
        amp_list = []
        freq_list = []
        inv_list = []
        std_list = []
        x_list = []
        y_list = []
        for item in todo:
            if item[3] == 'ac':
                selected_ac.append(self.folder_dict[item])
            if item[3] == 'trans':
                selected_trans.append(self.folder_dict[item])
        for trans in selected_trans:
            t,v,amp,freq = bode.trans_fourier(trans, False, res)
            t_list.append(t)
            v_list.append(v)
            amp_list.append(amp)
            freq_list.append(freq)
        for ac in selected_ac:
            inv_list.append(bode.invert_transfer_function(ac, res))
            std_list.append(bode.transfer_function(ac,res))
        color_step = 1.0/len(t_list)
        color = 0
        color_list = []
        #
        if (self.var.get() == 'Inverse'):
            self.errormsgvar.set('this may take a moment...')
            for i in range(0, len(t_list)):
                t,v,amp,freq = t_list[i],v_list[i],amp_list[i],freq_list[i]
                color_list.append((color,0,1-color))
                color += color_step
                new_amp = bode.apply_transfer_func(freq, amp, inv_list[i])
                new_amp = bode.apply_transfer_func(freq, new_amp, inv_list[i])
                new_amp = bode.apply_transfer_func(freq, new_amp, low_pass_filter)
                new_v = bode.inv_fourier(freq,new_amp)
                x_list.append(bode.genrange(1e-7,len(new_v)))
                y_list.append(new_v)
            self.errormsgvar.set('')
            self.new_figure_no_file(title, x_list, y_list, x_axis_title, y_axis_title, color_list)
        if (self.var.get() == 'Skin Depth'):
            for i in range(0, len(t_list)):
                t,v,amp,freq = t_list[i],v_list[i],amp_list[i],freq_list[i]
                color_list.append((color,0,1-color))
                color += color_step
                skin_depth_transfer_func = skin.get_skin_transfer_func(self.folder_dict, self.Rvals, todo[0][0][1], todo[0][0][2], todo[0][1], res, bode.genrange(1.0e9,100))
                new_amp = bode.apply_transfer_func(freq, amp, inv_list[i])
                new_amp = bode.apply_transfer_func(freq, new_amp, skin_depth_transfer_func)
                new_v = bode.inv_fourier(freq, new_amp)
                x_list.append(bode.genrange(1e-7,len(new_v)))
                y_list.append(new_v)
            self.errormsgvar.set('')
            self.new_figure_no_file(title, x_list, y_list, x_axis_title, y_axis_title, color_list)
        if (self.var.get() == 'Skin Depth Bode'):
            color_list.append((color,0,1-color))
            skin_depth_transfer_func = skin.get_skin_transfer_func(self.folder_dict, self.Rvals, todo[0][0][1], todo[0][0][2], todo[0][1], res, bode.genrange(1.0e9,100))
            skin_depth_transfer_func = bode.to_dB(skin_depth_transfer_func)
            x_list.append(skin_depth_transfer_func[0])
            y_list.append(skin_depth_transfer_func[1])
            self.errormsgvar.set('')
            self.new_figure_no_file(title, x_list, y_list, x_axis_title, y_axis_title, color_list)
        if (self.var.get() == 'Skin Depth Inv.'):
            for i in range(0, len(t_list)):
                t,v,amp,freq = t_list[i],v_list[i],amp_list[i],freq_list[i]
                color_list.append((color,0,1-color))
                color += color_step
                skin_depth_transfer_func = skin.get_skin_transfer_func(self.folder_dict, self.Rvals, todo[0][0][1], todo[0][0][2], todo[0][1], 2048, bode.genrange(1.0e9,100))
                skin_depth_transfer_func_inv = bode.invert_known_transfer_function(skin_depth_transfer_func)
                new_amp = bode.apply_transfer_func(freq, amp, inv_list[i])
                new_amp = bode.apply_transfer_func(freq, new_amp, skin_depth_transfer_func_inv)
                new_amp = bode.apply_transfer_func(freq, new_amp, low_pass_filter)
                new_v = bode.inv_fourier(freq, new_amp)
                x_list.append(bode.genrange(1e-7,len(new_v)))
                y_list.append(new_v)
            self.errormsgvar.set('')
            self.new_figure_no_file(title, x_list, y_list, x_axis_title, y_axis_title, color_list)
    # returns r,l,c description for title
    def get_name_tag(self):
        '''
        makes a r,l,c description for plot title from the GUI
        '''
        const = []
        var = []
        if self.rlockvar.get():
            const.append('R=' + self.rlabelvar.get() + ' ')
        else:
            var.append('R ')
        if self.llockvar.get():
            const.append('L=' + self.llabelvar.get() + ' ')
        else:
            var.append('L ')
        if self.clockvar.get():
            const.append('C=' + self.clabelvar.get() + ' ')
        else:
            var.append('C ')
        tag = ''
        for c in const:
            tag += c + '  '
        tag += 'Var: '
        for v in var:
            tag += v + '  '
        return tag
    # returns a float of the same value as the original scientific notation
    def sciToFloat(self, sci_string):
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
    # returns a string with the same value as the float in scientific notation
    def floatToSci(self, num):
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
            #     return str(round((num*(10**m)),2)) + conversion[m]
            # else:
            #     return str(num*(10**m)) + conversion[m]
                # if m<10:
                # if num*(10**m)>=10.0 and (num*(10**m)<100):
                #     return str(int(num*(10**m))) + '.' + conversion[m]
                # elif (num*(10**m)>=100):
                #     return str(int(num*(10**m))) + conversion[m]
                # else:
                #     return str(round(((num*(10**m))),1)) + conversion[m]
                # else:
                #     return str(round((num*(10**m)),2)) + conversion[m]
                # else:
                #     if num*(10**m)>=25:
                #         return str(int(num*(10**m))) + conversion[m]
                #     else:
                #         return str(round((num*(10**m)),3)) + conversion[m]
            #return str(round((num*(10**m)),2)) + conversion[m]
    #
    def shorten(self, num):
        '''
        '''
        strnum = self.floatToSci(num)
        return self.sciToFloat(strnum)
    # takes values and returns a key for self.folder_dict
    def vals_to_key(self,rin,lin,cin,Cdrpin,signal,typein, lenin, dropsin):
        '''
        rin         a float or string
        lin         a float or string
        cin         a float or string
        Cdrpin      a float or string
        signal      a string (Vin, Vn001, Vn014, Vn026, Vout)
        typein      a string (ac or trans)
        '''
        return ('({r},{l},{c}),{cdrp},{sig},{tp},{len},{drops}').format(r=rin, l=lin, c=cin, cdrp=Cdrpin, sig=signal, tp=typein, len = lenin, drops = dropsin)
    # returns the element in list closest to val
    def findClosest(self,list,val):
        '''
        list:      the list to be searched
        val:       the value to be compared

        returns the element in list closest to val
        '''
        return min(list, key=(lambda x:abs(x-val)))
    # returns a list of keys for every file in the folder, and a list of all the Rs, Ls and Cs
    def createFileDict(self):
    	'''
    	folder_name:the path of the folder to be scanned

    	returns a list of keys for every file in the folder, and a list of all the Rs, Ls and Cs
    	'''
    	folder_dict = {}
    	dict_key = [0, 0, 0, 0]
    	dict_rlc_tuple = (0,0,0)
    	###this changes the current directory to folder_name, so there wouldn't be directory to list named folder_name###
    	###os.chdir(folder_name)###
    	file_list = []
    	var = []
    	for a in [10, 14, 28]:
    		for b in ["Bode", "Transient"]:
    			path = "../data/transmission_line_{}_dropoffs/Multidrop{}".format(a, b)
    			for f in os.listdir(path):
    				if f[-3:] == "csv":
    					var = []
    					file_list.append(os.path.join("../data", f))
    					for d in f.split('=')[1:]:
    						curr_index = 0
    						for char in d:
    							if char == "_":
    								break
    							curr_index += 1
    						var.append( d[:curr_index] )

    					if f[0] == "t":
    						analy = "tran"
    					else:
    						analy = "ac"
    					if f[-7:-4] == "out":
    						signal = "Vout"
    					elif f[-7:-4] == "rst":
    						signal = "Vfirst"
    					elif f[-7:-4] == "ast":
    						signal = "Vlast"
    					elif f[-7:-4] == "dle":
    						signal = "Vmiddle"
    					else:
    						signal = "Vin"
    					tuple = (var[0], var[1], var[2])
    					tuple2 = (tuple, var[3], signal, analy, a, var[4])
    					folder_dict[tuple2] = file_list[-1]
        self.folder_dict = folder_dict
    # saves folder_dict, Rvals, Lvals, and Cvals in a pickle file
    def save(self):
        '''
        saves folder_dict, Rvals, Lvals, and Cvals in a pickle file
        '''
        with open(r"main_save.p", 'wb') as open_file:
            pickle.dump([self.folder_dict,self.Rvals,self.Lvals,self.Cvals],open_file)
    #
    def load(self):
        '''
        loads folder_dict, Rvals, Lvals, and Cvals from a pickle file
        '''
        with open(r"main_save.p", 'rb') as open_file:
            load_list = pickle.load(open_file)
            self.folder_dict = load_list[0]
            self.Rvals = load_list[1]
            self.Lvals = load_list[2]
            self.Cvals = load_list[3]
    # ==========================================================================


    # ==========================================================================
    # --------------------------------------------------------------------------
    # Plotting Functionality
    # --------------------------------------------------------------------------
    # generates new figure and shows it
    def new_figure_no_file(self, title, x_axis_list, y_axis_list, x_axis_title, y_axis_title, color_list):
        '''
        '''
        fig = self.create_new_figure(title)
        for i in range(0, len(x_axis_list)):
            self.plot_axes(fig, x_axis_list[i], y_axis_list[i], color_list[i], x_axis_title, y_axis_title)
        plt.show()
    #
    def new_figure(self, title, plot_info, x_axis_title, y_axis_title):
        '''
        title:      plot
        plot_info:  list of lists: [[r'file_path1', name1, plot_color1], [r'file_path2', name2, plot_color2], ... ]
        x_axis_title: title on x axis
        y_axis_title: title on y axis

        generates new figure and shows it
        '''
        fig = self.create_new_figure(title)
        for i in plot_info:
            self.plot_file(fig, i[0], i[1], i[2], x_axis_title, y_axis_title)
        plt.show()
    # adds a file to the figure as a plot
    def plot_file(self, fig,file_path, name, plot_color, x_axis_title, y_axis_title):
        '''
        fig:        figure which plot will be on
        file_path:  r'(file directory)'
        name:       '(name of line)'
        plot_color: '(a valid color)' (blue, green, red, cyan, magenta, yellow, black, white)
                    RGB tuple (0,1,0)
                    Hexstring ('#008000')
        x_axis_title: title on x axis
        y_axis_title: title on y axis

        plots a file as a line on a given figure
        '''
        ax = fig.add_subplot(111)
        ax.set_xlabel(x_axis_title)
        ax.set_ylabel(y_axis_title)
        data = np.genfromtxt(file_path, delimiter=',', names=['t', name])
        ax.plot(data['t'], data[name], color=plot_color, linestyle='-')
    #
    def plot_axes(self, fig, x_axis_list, y_axis_list, plot_color, x_axis_title, y_axis_title):
        '''
        '''
        ax = fig.add_subplot(111)
        ax.set_xlabel(x_axis_title)
        ax.set_ylabel(y_axis_title)
        ax.plot(x_axis_list, y_axis_list, color=plot_color, linestyle='-')
    # instantiates a new figure for new plots
    def create_new_figure(self, title):
        '''
        title:      '(figure title)''

        creates a new figure
        '''
        fig = plt.figure()
        fig.suptitle(title, fontsize = 12, fontweight='bold')
        return fig
    # ==========================================================================





if __name__ == '__main__':
    root = Tk()
    root.geometry("460x160+300+300")
    mainmenu = MainMenu(root)
    root.mainloop()
    mainmenu.save()

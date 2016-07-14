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

from numpy import arange

import bode_rename as br
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

        self.R_low = self.sciToFloat("0.02")
        self.R_high = self.sciToFloat("2")
        # self.R_high = self.sciToFloat("1.8")
        self.R_step = self.sciToFloat("0.2")

        self.L_low = self.sciToFloat("0.079n")
        self.L_high = self.sciToFloat("7.9n")
        # self.L_high = self.sciToFloat("7.11n")
        self.L_step = self.sciToFloat("0.79n")

        self.C_low = self.sciToFloat("0.22p")
        self.C_high = self.sciToFloat("22p")
        # self.C_high = self.sciToFloat("19.8p")
        self.C_step = self.sciToFloat("2.2p")

        Frame.__init__(self, parent)
        self.plot = False
        self.parent = parent
        self.initUI()
        self.createFileDict('')
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

        plotButton = Button(self, text="Plot", command=self.gui_to_vals, width= 20)
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
        self.rlabelvar.set('20m')
        self.rlabel = Label(self, textvariable=self.rlabelvar)
        self.rlabel.place(x=250,y=5)
        self.rscale = Scale(self, from_=0.02, to_=2, length=190, command=self.update_rlabel) #0.2
        self.rscale.place(x=50,y=5)

        self.llabelvar = StringVar()
        self.llabelvar.set('100p')
        self.llabel = Label(self, textvariable=self.llabelvar)
        self.llabel.place(x=250,y=35)
        self.lscale = Scale(self, from_=0.079*(10**-9), to_=7.9*(10**-9), length=190, command=self.update_llabel) #0.79n
        self.lscale.place(x=50,y=35)

        self.clabelvar = StringVar()
        self.clabelvar.set('200f')
        self.clabel = Label(self, textvariable=self.clabelvar)
        self.clabel.place(x=250,y=65)
        self.cscale = Scale(self, from_=0.22*(10**-12), to_=22*(10**-12), length=190, command=self.update_clabel) #2.2p
        self.cscale.place(x=50,y=65)

        self.vilockvar = BooleanVar()
        vilock = Checkbutton(self, text="Vin", variable=self.vilockvar)
        vilock.place(x=300,y=5)

        self.dilockvar = BooleanVar()
        dilock = Checkbutton(self, text="First Drop", variable=self.dilockvar)
        dilock.place(x=360,y=5)

        self.dmlockvar = BooleanVar()
        dmlock = Checkbutton(self, text="Mid Drop", variable=self.dmlockvar)
        dmlock.place(x=360,y=35)

        self.dflockvar = BooleanVar()
        dflock = Checkbutton(self, text="Last Drop", variable=self.dflockvar)
        dflock.place(x=360,y=65)

        self.volockvar = BooleanVar()
        volock = Checkbutton(self, text="Vout", variable=self.volockvar)
        volock.place(x=300,y=35)

        self.cdscalevar = IntVar()
        self.cdlabelvar = StringVar()
        self.cdlabelvar.set(0)
        self.cdlabelstatic = Label(self, text = 'Cdrp')
        self.cdlabelstatic.place(x=250, y=98)
        self.cdlabel = Label(self,textvariable=self.cdlabelvar)
        self.cdscale = Scale(self, from_=0, to_=5, variable = self.cdscalevar,command=self.update_cd)
        self.cdscale.place(x=300, y=98)
        self.cdlabel.place(x=425,y=98)

        self.errormsgvar = StringVar()
        self.errormsg = Label(self, textvariable=self.errormsgvar, foreground='red')
        self.errormsg.place(x=5,y=125)

        self.var = StringVar(self)
        self.var.set('Transient')
        self.libary = OptionMenu(self, self.var,'', 'Transient', 'Bode')
        self.libary.place(x=145,y=95)
    # updates r slider label
    def update_rlabel(self, val):
        #self.rlabelvar.set(round(float(val),1))
        self.rlabelvar.set(self.floatToSci(val))
    # updates l slider label
    def update_llabel(self, val):
        #self.llabelvar.set(round(float(val),10))
        self.llabelvar.set(self.floatToSci(val))
    # updates c slider label
    def update_clabel(self, val):
        #self.clabelvar.set(round(float(val),13))
        self.clabelvar.set(self.floatToSci(val))
    # updates Cdrp slider label
    def update_cd(self,val):
        self.cdlabelvar.set(int(round(float(val))))
        self.cdscalevar.set(int(round(float(val))))
    # ==========================================================================


    # ==========================================================================
    # --------------------------------------------------------------------------
    # File-GUI interaction
    # --------------------------------------------------------------------------
    # finds and plots all files given info on the gui
    def gui_to_vals(self):
        self.errormsgvar.set('')
        out = []
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
            signal = 'Vn001'
            changed += 1
        if self.dmlockvar.get():
            signal = 'Vn014'
            changed += 1
        if self.dflockvar.get():
            signal = 'Vn026'
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
        atype = ''
        if self.var.get() == 'Bode':
            atype = 'ac'
        if self.var.get() == 'Transient':
            atype = 'trans'
        out.append(atype)
        self.plot = False
        todo = []
        for r in out[0]:
            for l in out[1]:
                for c in out[2]:
                    todo.append(((r,l,c),out[3],signal,out[5]))
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
        self.new_figure('Test', instructions)
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
            #     return str(round((num*(10**m)),2)) + conversion[m]
            # else:
            #     return str(num*(10**m)) + conversion[m]
                if m<10:
                    if num*(10**m)>=20.0:
                        return str(int(num*(10**m))) + conversion[m]
                    else:
                        return str(round((num*(10**m)),3)) + conversion[m]
                else:
                    if num*(10**m)>=25:
                        return str(int(num*(10**m))) + conversion[m]
                    else:
                        return str(round((num*(10**m)),3)) + conversion[m]
            #return str(round((num*(10**m)),2)) + conversion[m]
    # takes values and returns a key for self.folder_dict
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
    # returns the element in list closest to val
    def findClosest(self,list,val):
        '''
        list:      the list to be searched
        val:       the value to be compared

        returns the element in list closest to val
        '''
        return min(list, key=(lambda x:abs(x-val)))
    # returns a list of keys for every file in the folder, and a list of all the Rs, Ls and Cs
    def createFileDict(self, folder_name):
        '''
        folder_name:the path of the folder to be scanned

        returns a list of keys for every file in the folder, and a list of all the Rs, Ls and Cs
        '''
        #each key will be formatted as such: (r, l, c), Cdrp, signal, ac/trans
        #Ex. folder_dict[[(1.02, 1.659e-9, 2.42e-12), 1, "Vout", "trans"]] =
        #r"MultidropTransient\Cdrp_1p_Vout\StepInformationRline=1.02Lline=1.659nCline=2.42pRun1491331.csv"
        self.folder_dict = {}
        #self.<X>vals lists will be in floats
        self.Rvals = []
        self.Lvals = []
        self.Cvals = []
        dict_key = [0, 0, 0, 0]
        dict_rlc_tuple = (0,0,0)
        #determine the Rvals, Lvals, and Cvals from a folder in the home directory
        for r in arange(self.R_low, self.R_high, self.R_step):
            self.Rvals.append(r)
        for l in arange(self.L_low, self.L_high, self.L_step):
            self.Lvals.append(l)
        for c in arange(self.C_low, self.C_high, self.C_step):
            self.Cvals.append(c)
        ###this changes the current directory to folder_name, so there wouldn't be directory to list named folder_name###
        ###os.chdir(folder_name)###
        for lib in ["MultidropBode", "MultidropTransient"]:
            if lib == "MultidropBode":
                dict_key[3] = "ac"
            elif lib == "MultidropTransient":
                dict_key[3] = "trans"
            else:
                dict_key[3] = "error in lib"
            for signal in ["Vout", "Vn001", "Vn014", "Vn026"]:
                dict_key[2] = signal
                for cdrp in ["0p", "1p", "2p", "3p", "4p", "5p"]:
                    dict_key[1] = int(cdrp[0])
                    for r in self.Rvals:
                        for l in self.Lvals:
                            for c in self.Cvals:
                                dict_key[0] = (r, l, c)
                                perm_dict_key = tuple(dict_key)
                                self.folder_dict[perm_dict_key] =r"{var_actrans}\Cdrp_{var_cdrp}_{var_signal}\StepInformationRline={var_r}Lline={var_l}Cline={var_c}.csv".format(var_actrans=lib, var_cdrp=cdrp, var_signal=signal, var_r=self.floatToSci(r), var_l=self.floatToSci(l), var_c=self.floatToSci(c))

        return self.folder_dict
    # ==========================================================================


    # ==========================================================================
    # --------------------------------------------------------------------------
    # Plotting Functionality
    # --------------------------------------------------------------------------
    # generates new figure and shows it
    def new_figure(self, title,plot_info):
        '''
        title:      plot
        plot_info:  list of lists: [[r'file_path1', name1, plot_color1], [r'file_path2', name2, plot_color2], ... ]

        generates new figure and shows it
        '''
        fig = self.create_new_figure(title)
        for i in plot_info:
            self.plot_file(fig, i[0], i[1], i[2])
        plt.show()
    # adds a file to the figure as a plot
    def plot_file(self, fig,file_path,name,plot_color):
        '''
        fig:        figure which plot will be on
        file_path:  r'(file directory)'
        name:       '(name of line)'
        plot_color: '(a valid color)' (blue, green, red, cyan, magenta, yellow, black, white)
                    RGB tuple (0,1,0)
                    Hexstring ('#008000')

        plots a file as a line on a given figure
        '''
        ax = fig.add_subplot(111)
        ax.set_xlabel('Time(s)')
        ax.set_ylabel('Voltage(V)')
        data = np.genfromtxt(file_path, delimiter=',', names=['t', name])
        ax.plot(data['t'], data[name], color=plot_color, linestyle='-') #, label=name
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
    root.geometry("445x150+300+300")
    mainmenu = MainMenu(root)
    root.mainloop()

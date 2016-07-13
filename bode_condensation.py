import os

def condensate(file_path):

    with open(file_path, 'r') as file:
        data = file.readlines()

    data[0] = "Freq.,V"

    for line in data[1:]:
        new_line = ''
        line.replace("\n", "")
        entries = line.split(",")
        try:
            v = float(entries[1])-float(entries[2])
            v = ',' + str(v) + "\n"
        except IndexError:
            v = ''
        new_line = entries[0] + v
        print new_line

    print file_path

    #with open(file_path, 'w') as file:
        #file.writelines(data)

def cut_run_num(folder_path):

    pwd = os.getcwd()
    file_names = os.listdir(folder_path)

    os.chdir(folder_path)
    for file_name in file_names:
        edited = file_name.split("Run")
        if len(edited) > 1:
            edited = edited[0] + '.csv'
        else:
            edited = edited[0]
        os.rename(file_name, edited)
        print "renamed", file_name, "to", edited
    os.chdir(pwd)

# for i in range(0,6):
#     cut_run_num((r"MultidropBode\Cdrp_{n}p_Vout").format(n = i))

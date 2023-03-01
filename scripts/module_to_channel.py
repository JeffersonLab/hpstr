import argparse
import sys


def string_to_hw_dict():

    string_to_hw = {}
    string_to_hw["L0T_axial"] = "F0H0"
    string_to_hw["L0T_stereo"] = "F0H1"
    string_to_hw["L1T_axial"] = "F0H2"
    string_to_hw["L1T_stereo"] = "F0H3"
    string_to_hw["L1B_axial"] = "F1H0"
    string_to_hw["L1B_stereo"] = "F1H1"
    string_to_hw["L0B_axial"] = "F1H2"
    string_to_hw["L0B_stereo"] = "F1H3"
    string_to_hw["L2T_axial"] = "F2H0"
    string_to_hw["L2T_stereo"] = "F2H1"
    string_to_hw["L3T_stereo"] = "F2H2"
    string_to_hw["L3T_axial"] = "F2H3"
    string_to_hw["L2B_stereo"] = "F3H2"
    string_to_hw["L2B_axial"] = "F3H3"
    string_to_hw["L3B_stereo"] = "F3H0"
    string_to_hw["L3B_axial"] = "F3H1"

    string_to_hw["L4T_axial_ele"] = "F4H0"
    string_to_hw["L4T_axial_pos"] = "F4H1"
    string_to_hw["L4T_stereo_ele"] = "F4H2"
    string_to_hw["L4T_stereo_pos"] = "F4H3"
    string_to_hw["L4B_stereo_ele"] = "F5H0"
    string_to_hw["L4B_stereo_pos"] = "F5H1"
    string_to_hw["L4B_axial_ele"] = "F5H2"
    string_to_hw["L4B_axial_pos"] = "F5H3"
    string_to_hw["L5T_axial_ele"] = "F6H0"
    string_to_hw["L5T_axial_pos"] = "F6H1"
    string_to_hw["L5T_stereo_ele"] = "F6H2"
    string_to_hw["L5T_stereo_pos"] = "F6H3"
    string_to_hw["L5B_stereo_ele"] = "F7H0"
    string_to_hw["L5B_stereo_pos"] = "F7H1"
    string_to_hw["L5B_axial_ele"] = "F7H2"
    string_to_hw["L5B_axial_pos"] = "F7H3"
    string_to_hw["L6T_axial_ele"] = "F8H0"
    string_to_hw["L6T_axial_pos"] = "F8H1"
    string_to_hw["L6T_stereo_ele"] = "F8H2"
    string_to_hw["L6T_stereo_pos"] = "F8H3"
    string_to_hw["L6B_stereo_ele"] = "F9H0"
    string_to_hw["L6B_stereo_pos"] = "F9H1"
    string_to_hw["L6B_axial_ele"] = "F9H2"
    string_to_hw["L6B_axial_pos"] = "F9H3"
    return string_to_hw


def apv_map(feb):
    apv = {}
    if feb < 2:
        apv["0"] = [i for i in range(128, 256)]
        apv["1"] = [i for i in range(0, 128)]
        apv["2"] = [i for i in range(256, 384)]
        apv["3"] = [i for i in range(384, 512)]

    else:
        apv["0"] = [i for i in range(512, 640)]
        apv["1"] = [i for i in range(384, 512)]
        apv["2"] = [i for i in range(256, 384)]
        apv["3"] = [i for i in range(128, 256)]
        apv["4"] = [i for i in range(0, 128)]
    return apv


##########################################################################
parser = argparse.ArgumentParser(description="Module and Channel number inputs for translation")
parser.add_argument('--module', '-m', type=str, dest="module", help="Input module name found in dictionary string_to_hw", default="")
parser.add_argument('--channel', '-c', type=int, dest="channel", help="Input channel number 0-640 from monitoring plot", default=-9999)
options = parser.parse_args()

#Get dictionary for translation
mod_name = options.module
chin = options.channel
hw_name = None

print("Translating %s Ch %i to APV Physical Channel" % (mod_name, chin))

string_to_hw = string_to_hw_dict()
try:
    if "_hole" in mod_name:
        mod_name = mod_name.replace("_hole", "_ele")
    if "_slot" in mod_name:
        mod_name = mod_name.replace("_slot", "_pos")
    hw_name = string_to_hw["%s" % (mod_name)]
except BaseException:
    print("ERROR! Invalid module name. Please check 'module_to_channel.py' dictionary of possible names")
    sys.exit()
feb = hw_name[1:2]
hyb = hw_name[3:4]
apv = None
chout = None

try:
    apvs = apv_map(int(feb))
except BaseException:
    sys.exit()

for a in apvs:
    if chin in apvs[a]:
        apv = a
        chout = apvs[a].index(chin)
        break
if chout is None:
    print("ERROR! Invalid channel number entered. Please enter a channel number 0-511 for L0-1, or 0-639 for L2-6")
    sys.exit()

print("RESULT")
print("Feb%s Hybrid%s APV%s Ch%s" % (feb, hyb, apv, chout))

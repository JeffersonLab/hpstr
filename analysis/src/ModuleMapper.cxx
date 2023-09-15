#include "ModuleMapper.h"
#include <iostream>
#include "TString.h"
ModuleMapper::ModuleMapper(const int year) {

    year_ = year;
    std::cout << "MODULE MAPPER YEAR " << year_ << std::endl;

    if (year_ == 2019 || year_ == 2021) {

        hw_to_sw["F0H0"] = "ly1_m0" ;
        hw_to_sw["F0H1"] = "ly2_m0" ;
        hw_to_sw["F0H2"] = "ly3_m0" ;
        hw_to_sw["F0H3"] = "ly4_m0" ;
        hw_to_sw["F1H0"] = "ly4_m1" ;
        hw_to_sw["F1H1"] = "ly3_m1" ;
        hw_to_sw["F1H2"] = "ly2_m1" ;
        hw_to_sw["F1H3"] = "ly1_m1" ;
        hw_to_sw["F2H0"] = "ly5_m0" ;
        hw_to_sw["F2H1"] = "ly6_m0" ;
        hw_to_sw["F2H2"] = "ly8_m0" ;
        hw_to_sw["F2H3"] = "ly7_m0" ;
        hw_to_sw["F3H0"] = "ly7_m1";
        hw_to_sw["F3H1"] = "ly8_m1";
        hw_to_sw["F3H2"] = "ly5_m1";
        hw_to_sw["F3H3"] = "ly6_m1";
        hw_to_sw["F4H0"] = "ly9_m0" ;
        hw_to_sw["F4H1"] = "ly9_m2" ;
        hw_to_sw["F4H2"] = "ly10_m0";
        hw_to_sw["F4H3"] = "ly10_m2";
        hw_to_sw["F5H0"] = "ly9_m1" ;
        hw_to_sw["F5H1"] = "ly9_m3" ;
        hw_to_sw["F5H2"] = "ly10_m1";
        hw_to_sw["F5H3"] = "ly10_m3";
        hw_to_sw["F6H0"] = "ly11_m0";
        hw_to_sw["F6H1"] = "ly11_m2";
        hw_to_sw["F6H2"] = "ly12_m0";
        hw_to_sw["F6H3"] = "ly12_m2";
        hw_to_sw["F7H0"] = "ly11_m1";
        hw_to_sw["F7H1"] = "ly11_m3";
        hw_to_sw["F7H2"] = "ly12_m1";
        hw_to_sw["F7H3"] = "ly12_m3";
        hw_to_sw["F8H0"] = "ly13_m0";
        hw_to_sw["F8H1"] = "ly13_m2";
        hw_to_sw["F8H2"] = "ly14_m0";
        hw_to_sw["F8H3"] = "ly14_m2";
        hw_to_sw["F9H0"] = "ly13_m1";
        hw_to_sw["F9H1"] = "ly13_m3";
        hw_to_sw["F9H2"] = "ly14_m1";
        hw_to_sw["F9H3"] = "ly14_m3";

        sw_to_hw["ly1_m0" ] = "F0H0" ;
        sw_to_hw["ly2_m0" ] = "F0H1" ;
        sw_to_hw["ly3_m0" ] = "F0H2" ;
        sw_to_hw["ly4_m0" ] = "F0H3" ;
        sw_to_hw["ly4_m1" ] = "F1H0" ;
        sw_to_hw["ly3_m1" ] = "F1H1" ;
        sw_to_hw["ly2_m1" ] = "F1H2" ;
        sw_to_hw["ly1_m1" ] = "F1H3" ;
        sw_to_hw["ly5_m0" ] = "F2H0" ;
        sw_to_hw["ly6_m0" ] = "F2H1" ;
        sw_to_hw["ly8_m0" ] = "F2H2" ;
        sw_to_hw["ly7_m0" ] = "F2H3" ;
        sw_to_hw["ly5_m1" ] = "F3H2" ;
        sw_to_hw["ly6_m1" ] = "F3H3" ;
        sw_to_hw["ly7_m1" ] = "F3H0" ;
        sw_to_hw["ly8_m1" ] = "F3H1" ;
        sw_to_hw["ly9_m0" ] = "F4H0" ;
        sw_to_hw["ly9_m2" ] = "F4H1" ;
        sw_to_hw["ly10_m0"] = "F4H2" ;
        sw_to_hw["ly10_m2"] = "F4H3" ;
        sw_to_hw["ly9_m1" ] = "F5H0" ;
        sw_to_hw["ly9_m3" ] = "F5H1" ;
        sw_to_hw["ly10_m1"] = "F5H2" ;
        sw_to_hw["ly10_m3"] = "F5H3" ;
        sw_to_hw["ly11_m0"] = "F6H0" ;
        sw_to_hw["ly11_m2"] = "F6H1" ;
        sw_to_hw["ly12_m0"] = "F6H2" ;
        sw_to_hw["ly12_m2"] = "F6H3" ;
        sw_to_hw["ly11_m1"] = "F7H0" ;
        sw_to_hw["ly11_m3"] = "F7H1" ;
        sw_to_hw["ly12_m1"] = "F7H2" ;
        sw_to_hw["ly12_m3"] = "F7H3" ;
        sw_to_hw["ly13_m0"] = "F8H0" ;
        sw_to_hw["ly13_m2"] = "F8H1" ;
        sw_to_hw["ly14_m0"] = "F8H2" ;
        sw_to_hw["ly14_m2"] = "F8H3" ;
        sw_to_hw["ly13_m1"] = "F9H0" ;
        sw_to_hw["ly13_m3"] = "F9H1" ;
        sw_to_hw["ly14_m1"] = "F9H2" ;
        sw_to_hw["ly14_m3"] = "F9H3" ;

        // HW to string and viceversa

        hw_to_string["F0H0"] = "L0T_axial"     ;
        hw_to_string["F0H1"] = "L0T_stereo"    ;
        hw_to_string["F0H2"] = "L1T_axial"     ;
        hw_to_string["F0H3"] = "L1T_stereo"    ;
        hw_to_string["F1H0"] = "L1B_axial"     ;
        hw_to_string["F1H1"] = "L1B_stereo"    ;
        hw_to_string["F1H2"] = "L0B_axial"     ;
        hw_to_string["F1H3"] = "L0B_stereo"    ;
        hw_to_string["F2H0"] = "L2T_axial"     ;
        hw_to_string["F2H1"] = "L2T_stereo"    ;
        hw_to_string["F2H2"] = "L3T_stereo"    ;
        hw_to_string["F2H3"] = "L3T_axial"     ;
        hw_to_string["F3H2"] = "L2B_stereo"    ;
        hw_to_string["F3H3"] = "L2B_axial"     ;
        hw_to_string["F3H0"] = "L3B_stereo"    ;
        hw_to_string["F3H1"] = "L3B_axial"     ;
        hw_to_string["F4H0"] = "L4T_axial_ele" ;
        hw_to_string["F4H1"] = "L4T_axial_pos" ;
        hw_to_string["F4H2"] = "L4T_stereo_ele";
        hw_to_string["F4H3"] = "L4T_stereo_pos";
        hw_to_string["F5H0"] = "L4B_stereo_ele";
        hw_to_string["F5H1"] = "L4B_stereo_pos";
        hw_to_string["F5H2"] = "L4B_axial_ele" ;
        hw_to_string["F5H3"] = "L4B_axial_pos" ;
        hw_to_string["F6H0"] = "L5T_axial_ele" ;
        hw_to_string["F6H1"] = "L5T_axial_pos" ;
        hw_to_string["F6H2"] = "L5T_stereo_ele";
        hw_to_string["F6H3"] = "L5T_stereo_pos";
        hw_to_string["F7H0"] = "L5B_stereo_ele";
        hw_to_string["F7H1"] = "L5B_stereo_pos";
        hw_to_string["F7H2"] = "L5B_axial_ele" ;
        hw_to_string["F7H3"] = "L5B_axial_pos" ;
        hw_to_string["F8H0"] = "L6T_axial_ele" ;
        hw_to_string["F8H1"] = "L6T_axial_pos" ;
        hw_to_string["F8H2"] = "L6T_stereo_ele";
        hw_to_string["F8H3"] = "L6T_stereo_pos";
        hw_to_string["F9H0"] = "L6B_stereo_ele";
        hw_to_string["F9H1"] = "L6B_stereo_pos";
        hw_to_string["F9H2"] = "L6B_axial_ele" ;
        hw_to_string["F9H3"] = "L6B_axial_pos" ; 

        string_to_hw["L0T_axial"     ] = "F0H0";
        string_to_hw["L0T_stereo"    ] = "F0H1";
        string_to_hw["L1T_axial"     ] = "F0H2";
        string_to_hw["L1T_stereo"    ] = "F0H3";
        string_to_hw["L1B_axial"     ] = "F1H0";
        string_to_hw["L1B_stereo"    ] = "F1H1";
        string_to_hw["L0B_axial"     ] = "F1H2";
        string_to_hw["L0B_stereo"    ] = "F1H3";
        string_to_hw["L2T_axial"     ] = "F2H0";
        string_to_hw["L2T_stereo"    ] = "F2H1";
        string_to_hw["L3T_stereo"    ] = "F2H2";
        string_to_hw["L3T_axial"     ] = "F2H3";
        string_to_hw["L2B_stereo"    ] = "F3H2";
        string_to_hw["L2B_axial"     ] = "F3H3";
        string_to_hw["L3B_stereo"    ] = "F3H0";
        string_to_hw["L3B_axial"     ] = "F3H1";
        string_to_hw["L4T_axial_ele" ] = "F4H0";
        string_to_hw["L4T_axial_pos" ] = "F4H1";
        string_to_hw["L4T_stereo_ele"] = "F4H2";
        string_to_hw["L4T_stereo_pos"] = "F4H3";
        string_to_hw["L4B_stereo_ele"] = "F5H0";
        string_to_hw["L4B_stereo_pos"] = "F5H1";
        string_to_hw["L4B_axial_ele" ] = "F5H2";
        string_to_hw["L4B_axial_pos" ] = "F5H3";
        string_to_hw["L5T_axial_ele" ] = "F6H0";
        string_to_hw["L5T_axial_pos" ] = "F6H1";
        string_to_hw["L5T_stereo_ele"] = "F6H2";
        string_to_hw["L5T_stereo_pos"] = "F6H3";
        string_to_hw["L5B_stereo_ele"] = "F7H0";
        string_to_hw["L5B_stereo_pos"] = "F7H1";
        string_to_hw["L5B_axial_ele" ] = "F7H2";
        string_to_hw["L5B_axial_pos" ] = "F7H3";
        string_to_hw["L6T_axial_ele" ] = "F8H0";
        string_to_hw["L6T_axial_pos" ] = "F8H1";
        string_to_hw["L6T_stereo_ele"] = "F8H2";
        string_to_hw["L6T_stereo_pos"] = "F8H3";
        string_to_hw["L6B_stereo_ele"] = "F9H0";
        string_to_hw["L6B_stereo_pos"] = "F9H1";
        string_to_hw["L6B_axial_ele" ] = "F9H2";
        string_to_hw["L6B_axial_pos" ] = "F9H3";


        //sw to string


        sw_to_string["ly1_m0" ] = "L0T_axial"     ;
        sw_to_string["ly2_m0" ] = "L0T_stereo"    ;
        sw_to_string["ly3_m0" ] = "L1T_axial"     ;
        sw_to_string["ly4_m0" ] = "L1T_stereo"    ;
        sw_to_string["ly4_m1" ] = "L1B_axial"     ;
        sw_to_string["ly3_m1" ] = "L1B_stereo"    ;
        sw_to_string["ly2_m1" ] = "L0B_axial"     ;
        sw_to_string["ly1_m1" ] = "L0B_stereo"    ;
        sw_to_string["ly5_m0" ] = "L2T_axial"     ;
        sw_to_string["ly6_m0" ] = "L2T_stereo"    ;
        sw_to_string["ly8_m0" ] = "L3T_stereo"    ;
        sw_to_string["ly7_m0" ] = "L3T_axial"     ;
        sw_to_string["ly5_m1" ] = "L2B_stereo"    ;
        sw_to_string["ly6_m1" ] = "L2B_axial"     ;
        sw_to_string["ly7_m1" ] = "L3B_stereo"    ;
        sw_to_string["ly8_m1" ] = "L3B_axial"     ;
        sw_to_string["ly9_m0" ] = "L4T_axial_ele" ;
        sw_to_string["ly9_m2" ] = "L4T_axial_pos" ;
        sw_to_string["ly10_m0"] = "L4T_stereo_ele";
        sw_to_string["ly10_m2"] = "L4T_stereo_pos";
        sw_to_string["ly9_m1" ] = "L4B_stereo_ele";
        sw_to_string["ly9_m3" ] = "L4B_stereo_pos";
        sw_to_string["ly10_m1"] = "L4B_axial_ele" ;
        sw_to_string["ly10_m3"] = "L4B_axial_pos" ;
        sw_to_string["ly11_m0"] = "L5T_axial_ele" ;
        sw_to_string["ly11_m2"] = "L5T_axial_pos" ;
        sw_to_string["ly12_m0"] = "L5T_stereo_ele";
        sw_to_string["ly12_m2"] = "L5T_stereo_pos";
        sw_to_string["ly11_m1"] = "L5B_stereo_ele";
        sw_to_string["ly11_m3"] = "L5B_stereo_pos";
        sw_to_string["ly12_m1"] = "L5B_axial_ele" ;
        sw_to_string["ly12_m3"] = "L5B_axial_pos" ;
        sw_to_string["ly13_m0"] = "L6T_axial_ele" ;
        sw_to_string["ly13_m2"] = "L6T_axial_pos" ;
        sw_to_string["ly14_m0"] = "L6T_stereo_ele";
        sw_to_string["ly14_m2"] = "L6T_stereo_pos";
        sw_to_string["ly13_m1"] = "L6B_stereo_ele";
        sw_to_string["ly13_m3"] = "L6B_stereo_pos";
        sw_to_string["ly14_m1"] = "L6B_axial_ele" ;
        sw_to_string["ly14_m3"] = "L6B_axial_pos" ;


        string_to_sw["L0T_axial"     ] = "ly1_m0" ;
        string_to_sw["L0T_stereo"    ] = "ly2_m0" ;
        string_to_sw["L1T_axial"     ] = "ly3_m0" ;
        string_to_sw["L1T_stereo"    ] = "ly4_m0" ;
        string_to_sw["L1B_axial"     ] = "ly4_m1" ;
        string_to_sw["L1B_stereo"    ] = "ly3_m1" ;
        string_to_sw["L0B_axial"     ] = "ly2_m1" ;
        string_to_sw["L0B_stereo"    ] = "ly1_m1" ;
        string_to_sw["L2T_axial"     ] = "ly5_m0" ;
        string_to_sw["L2T_stereo"    ] = "ly6_m0" ;
        string_to_sw["L3T_stereo"    ] = "ly8_m0" ;
        string_to_sw["L3T_axial"     ] = "ly7_m0" ;
        string_to_sw["L2B_stereo"    ] = "ly5_m1" ;
        string_to_sw["L2B_axial"     ] = "ly6_m1" ;
        string_to_sw["L3B_stereo"    ] = "ly7_m1" ;
        string_to_sw["L3B_axial"     ] = "ly8_m1" ;
        string_to_sw["L4T_axial_ele" ] = "ly9_m0" ;
        string_to_sw["L4T_axial_pos" ] = "ly9_m2" ;
        string_to_sw["L4T_stereo_ele"] = "ly10_m0";
        string_to_sw["L4T_stereo_pos"] = "ly10_m2";
        string_to_sw["L4B_stereo_ele"] = "ly9_m1" ;
        string_to_sw["L4B_stereo_pos"] = "ly9_m3" ;
        string_to_sw["L4B_axial_ele" ] = "ly10_m1";
        string_to_sw["L4B_axial_pos" ] = "ly10_m3";
        string_to_sw["L5T_axial_ele" ] = "ly11_m0";
        string_to_sw["L5T_axial_pos" ] = "ly11_m2";
        string_to_sw["L5T_stereo_ele"] = "ly12_m0";
        string_to_sw["L5T_stereo_pos"] = "ly12_m2";
        string_to_sw["L5B_stereo_ele"] = "ly11_m1";
        string_to_sw["L5B_stereo_pos"] = "ly11_m3";
        string_to_sw["L5B_axial_ele" ] = "ly12_m1";
        string_to_sw["L5B_axial_pos" ] = "ly12_m3";
        string_to_sw["L6T_axial_ele" ] = "ly13_m0";
        string_to_sw["L6T_axial_pos" ] = "ly13_m2";
        string_to_sw["L6T_stereo_ele"] = "ly14_m0";
        string_to_sw["L6T_stereo_pos"] = "ly14_m2";
        string_to_sw["L6B_stereo_ele"] = "ly13_m1";
        string_to_sw["L6B_stereo_pos"] = "ly13_m3";
        string_to_sw["L6B_axial_ele" ] = "ly14_m1";
        string_to_sw["L6B_axial_pos" ] = "ly14_m3";
    }
    else if (year_ == 2016) 
    {

        std::cout<<"WARNING: ModuleMapper: using a mapping that might be wrong"<<std::endl;


        hw_to_sw["F9H0"] = "ly1_m0" ;
        hw_to_sw["F2H0"] = "ly1_m1" ;
        hw_to_sw["F9H1"] = "ly2_m0" ;
        hw_to_sw["F2H1"] = "ly2_m1" ;
        hw_to_sw["F0H0"] = "ly3_m0" ;
        hw_to_sw["F6H0"] = "ly3_m1" ;
        hw_to_sw["F0H1"] = "ly4_m0" ;
        hw_to_sw["F6H1"] = "ly4_m1" ;
        hw_to_sw["F0H2"] = "ly5_m0" ;
        hw_to_sw["F6H2"] = "ly5_m1" ;
        hw_to_sw["F0H3"] = "ly6_m0" ;
        hw_to_sw["F6H3"] = "ly6_m1" ;
        hw_to_sw["F5H0"] = "ly7_m0" ;
        hw_to_sw["F1H0"] = "ly7_m1" ;
        hw_to_sw["F5H1"] = "ly7_m2" ;
        hw_to_sw["F1H1"] = "ly7_m3" ;
        hw_to_sw["F5H2"] = "ly8_m0" ;
        hw_to_sw["F1H2"] = "ly8_m1" ;
        hw_to_sw["F5H3"] = "ly8_m2" ;
        hw_to_sw["F1H3"] = "ly8_m3" ;
        hw_to_sw["F8H0"] = "ly9_m0" ;
        hw_to_sw["F4H0"] = "ly9_m1" ;
        hw_to_sw["F8H1"] = "ly9_m2" ;
        hw_to_sw["F4H1"] = "ly9_m3" ;
        hw_to_sw["F8H2"] = "ly10_m0";
        hw_to_sw["F4H2"] = "ly10_m1";
        hw_to_sw["F8H3"] = "ly10_m2";
        hw_to_sw["F4H3"] = "ly10_m3";
        hw_to_sw["F7H0"] = "ly11_m0";
        hw_to_sw["F3H0"] = "ly11_m1";
        hw_to_sw["F7H1"] = "ly11_m2";
        hw_to_sw["F3H1"] = "ly11_m3";
        hw_to_sw["F7H2"] = "ly12_m0";
        hw_to_sw["F3H2"] = "ly12_m1";
        hw_to_sw["F7H3"] = "ly12_m2";
        hw_to_sw["F3H3"] = "ly12_m3";

        sw_to_hw["ly1_m0" ] = "F9H0" ;
        sw_to_hw["ly1_m1" ] = "F2H0" ;
        sw_to_hw["ly2_m0" ] = "F9H1" ;
        sw_to_hw["ly2_m1" ] = "F2H1" ;
        sw_to_hw["ly3_m0" ] = "F0H0" ;
        sw_to_hw["ly3_m1" ] = "F6H0" ;
        sw_to_hw["ly4_m0" ] = "F0H1" ;
        sw_to_hw["ly4_m1" ] = "F6H1" ;
        sw_to_hw["ly5_m0" ] = "F0H2" ;
        sw_to_hw["ly5_m1" ] = "F6H2" ;
        sw_to_hw["ly6_m0" ] = "F0H3" ;
        sw_to_hw["ly6_m1" ] = "F6H3" ;
        sw_to_hw["ly7_m0" ] = "F5H0" ;
        sw_to_hw["ly7_m1" ] = "F1H0" ;
        sw_to_hw["ly7_m2" ] = "F5H1" ;
        sw_to_hw["ly7_m3" ] = "F1H1" ;
        sw_to_hw["ly8_m0" ] = "F5H2" ;
        sw_to_hw["ly8_m1" ] = "F1H2" ;
        sw_to_hw["ly8_m2" ] = "F5H3" ;
        sw_to_hw["ly8_m3" ] = "F1H3" ;
        sw_to_hw["ly9_m0" ] = "F8H0" ;
        sw_to_hw["ly9_m1" ] = "F4H0" ;
        sw_to_hw["ly9_m2" ] = "F8H1" ;
        sw_to_hw["ly9_m3" ] = "F4H1" ;
        sw_to_hw["ly10_m0"] = "F8H2" ;
        sw_to_hw["ly10_m1"] = "F4H2" ;
        sw_to_hw["ly10_m2"] = "F8H3" ;
        sw_to_hw["ly10_m3"] = "F4H3" ;
        sw_to_hw["ly11_m0"] = "F7H0" ;
        sw_to_hw["ly11_m1"] = "F3H0" ;
        sw_to_hw["ly11_m2"] = "F7H1" ;
        sw_to_hw["ly11_m3"] = "F3H1" ;
        sw_to_hw["ly12_m0"] = "F7H2" ;
        sw_to_hw["ly12_m1"] = "F3H2" ;
        sw_to_hw["ly12_m2"] = "F7H3" ;
        sw_to_hw["ly12_m3"] = "F3H3" ;

        // HW to string and viceversa

        hw_to_string["F9H0"] = "L1T_axial"     ;
        hw_to_string["F2H0"] = "L1B_stereo"    ;
        hw_to_string["F9H1"] = "L1T_stereo"    ;
        hw_to_string["F2H1"] = "L1B_axial"     ;
        hw_to_string["F0H0"] = "L2T_axial"     ;
        hw_to_string["F6H0"] = "L2B_stereo"    ;
        hw_to_string["F0H1"] = "L2T_stereo"    ;
        hw_to_string["F6H1"] = "L2B_axial"     ;
        hw_to_string["F0H2"] = "L3T_axial"     ;
        hw_to_string["F6H2"] = "L3B_stereo"    ;
        hw_to_string["F0H3"] = "L3T_stereo"    ;
        hw_to_string["F6H3"] = "L3B_axial"     ;
        hw_to_string["F5H0"] = "L4T_axial_ele" ;
        hw_to_string["F1H0"] = "L4B_stereo_ele";
        hw_to_string["F5H1"] = "L4T_axial_pos" ;
        hw_to_string["F1H1"] = "L4B_stereo_pos";
        hw_to_string["F5H2"] = "L4T_stereo_ele";
        hw_to_string["F1H2"] = "L4B_axial_ele" ;
        hw_to_string["F5H3"] = "L4T_stereo_pos";
        hw_to_string["F1H3"] = "L4B_axial_pos" ;
        hw_to_string["F8H0"] = "L5T_axial_ele" ;
        hw_to_string["F4H0"] = "L5B_stereo_ele";
        hw_to_string["F8H1"] = "L5T_axial_pos" ;
        hw_to_string["F4H1"] = "L5B_stereo_pos";
        hw_to_string["F8H2"] = "L5T_stereo_ele";
        hw_to_string["F4H2"] = "L5B_axial_ele" ;
        hw_to_string["F8H3"] = "L5T_stereo_pos";
        hw_to_string["F4H3"] = "L5B_axial_pos" ;
        hw_to_string["F7H0"] = "L6T_axial_ele" ;
        hw_to_string["F3H0"] = "L6B_stereo_ele";
        hw_to_string["F7H1"] = "L6T_axial_pos" ;
        hw_to_string["F3H1"] = "L6B_stereo_pos";
        hw_to_string["F7H2"] = "L6T_stereo_ele";
        hw_to_string["F3H2"] = "L6B_axial_ele" ;
        hw_to_string["F7H3"] = "L6T_stereo_pos";
        hw_to_string["F3H3"] = "L6B_axial_pos" ;


        string_to_hw["L1T_axial"     ] = "F9H0";
        string_to_hw["L1B_stereo"    ] = "F2H0";
        string_to_hw["L1T_stereo"    ] = "F9H1";
        string_to_hw["L1B_axial"     ] = "F2H1";
        string_to_hw["L2T_axial"     ] = "F0H0";
        string_to_hw["L2B_stereo"    ] = "F6H0";
        string_to_hw["L2T_stereo"    ] = "F0H1";
        string_to_hw["L2B_axial"     ] = "F6H1";
        string_to_hw["L3T_axial"     ] = "F0H2";
        string_to_hw["L3B_stereo"    ] = "F6H2";
        string_to_hw["L3T_stereo"    ] = "F0H3";
        string_to_hw["L3B_axial"     ] = "F6H3";
        string_to_hw["L4T_axial_ele" ] = "F5H0";
        string_to_hw["L4B_stereo_ele"] = "F1H0";
        string_to_hw["L4T_axial_pos" ] = "F5H1";
        string_to_hw["L4B_stereo_pos"] = "F1H1";
        string_to_hw["L4T_stereo_ele"] = "F5H2";
        string_to_hw["L4B_axial_ele" ] = "F1H2";
        string_to_hw["L4T_stereo_pos"] = "F5H3";
        string_to_hw["L4B_axial_pos" ] = "F1H3";
        string_to_hw["L5T_axial_ele" ] = "F8H0";
        string_to_hw["L5B_stereo_ele"] = "F4H0";
        string_to_hw["L5T_axial_pos" ] = "F8H1";
        string_to_hw["L5B_stereo_pos"] = "F4H1";
        string_to_hw["L5T_stereo_ele"] = "F8H2";
        string_to_hw["L5B_axial_ele" ] = "F4H2";
        string_to_hw["L5T_stereo_pos"] = "F8H3";
        string_to_hw["L5B_axial_pos" ] = "F4H3";
        string_to_hw["L6T_axial_ele" ] = "F7H0";
        string_to_hw["L6B_stereo_ele"] = "F3H0";
        string_to_hw["L6T_axial_pos" ] = "F7H1";
        string_to_hw["L6B_stereo_pos"] = "F3H1";
        string_to_hw["L6T_stereo_ele"] = "F7H2";
        string_to_hw["L6B_axial_ele" ] = "F3H2";
        string_to_hw["L6T_stereo_pos"] = "F7H3";
        string_to_hw["L6B_axial_pos" ] = "F3H3";


        //sw to string

        sw_to_string["ly1_m0" ] = "L1T_axial"     ;
        sw_to_string["ly1_m1" ] = "L1B_stereo"    ;
        sw_to_string["ly2_m0" ] = "L1T_stereo"    ;
        sw_to_string["ly2_m1" ] = "L1B_axial"     ;
        sw_to_string["ly3_m0" ] = "L2T_axial"     ;
        sw_to_string["ly3_m1" ] = "L2B_stereo"    ;
        sw_to_string["ly4_m0" ] = "L2T_stereo"    ;
        sw_to_string["ly4_m1" ] = "L2B_axial"     ;
        sw_to_string["ly5_m0" ] = "L3T_axial"     ;
        sw_to_string["ly5_m1" ] = "L3B_stereo"    ;
        sw_to_string["ly6_m0" ] = "L3T_stereo"    ;
        sw_to_string["ly6_m1" ] = "L3B_axial"     ;
        sw_to_string["ly7_m0" ] = "L4T_axial_ele" ;
        sw_to_string["ly7_m1" ] = "L4B_stereo_ele";
        sw_to_string["ly7_m2" ] = "L4T_axial_pos" ;
        sw_to_string["ly7_m3" ] = "L4B_stereo_pos";
        sw_to_string["ly8_m0" ] = "L4T_stereo_ele";
        sw_to_string["ly8_m1" ] = "L4B_axial_ele" ;
        sw_to_string["ly8_m2" ] = "L4T_stereo_pos";
        sw_to_string["ly8_m3" ] = "L4B_axial_pos" ;
        sw_to_string["ly9_m0" ] = "L5T_axial_ele" ;
        sw_to_string["ly9_m1" ] = "L5B_stereo_ele";
        sw_to_string["ly9_m2" ] = "L5T_axial_pos" ;
        sw_to_string["ly9_m3" ] = "L5B_stereo_pos";
        sw_to_string["ly10_m0"] = "L5T_stereo_ele";
        sw_to_string["ly10_m1"] = "L5B_axial_ele" ;
        sw_to_string["ly10_m2"] = "L5T_stereo_pos";
        sw_to_string["ly10_m3"] = "L5B_axial_pos" ;
        sw_to_string["ly11_m0"] = "L6T_axial_ele" ;
        sw_to_string["ly11_m1"] = "L6B_stereo_ele";
        sw_to_string["ly11_m2"] = "L6T_axial_pos" ;
        sw_to_string["ly11_m3"] = "L6B_stereo_pos";
        sw_to_string["ly12_m0"] = "L6T_stereo_ele";
        sw_to_string["ly12_m1"] = "L6B_axial_ele" ;
        sw_to_string["ly12_m2"] = "L6T_stereo_pos";
        sw_to_string["ly12_m3"] = "L6B_axial_pos" ;


        string_to_sw["L1T_axial"     ] = "ly1_m0" ;
        string_to_sw["L1B_stereo"    ] = "ly1_m1" ;
        string_to_sw["L1T_stereo"    ] = "ly2_m0" ;
        string_to_sw["L1B_axial"     ] = "ly2_m1" ;
        string_to_sw["L2T_axial"     ] = "ly3_m0" ;
        string_to_sw["L2B_stereo"    ] = "ly3_m1" ;
        string_to_sw["L2T_stereo"    ] = "ly4_m0" ;
        string_to_sw["L2B_axial"     ] = "ly4_m1" ;
        string_to_sw["L3T_axial"     ] = "ly5_m0" ;
        string_to_sw["L3B_stereo"    ] = "ly5_m1" ;
        string_to_sw["L3T_stereo"    ] = "ly6_m0" ;
        string_to_sw["L3B_axial"     ] = "ly6_m1" ;
        string_to_sw["L4T_axial_ele" ] = "ly7_m0" ;
        string_to_sw["L4B_stereo_ele"] = "ly7_m1" ;
        string_to_sw["L4T_axial_pos" ] = "ly7_m2" ;
        string_to_sw["L4B_stereo_pos"] = "ly7_m3" ;
        string_to_sw["L4T_stereo_ele"] = "ly8_m0" ;
        string_to_sw["L4B_axial_ele" ] = "ly8_m1" ;
        string_to_sw["L4T_stereo_pos"] = "ly8_m2" ;
        string_to_sw["L4B_axial_pos" ] = "ly8_m3" ;
        string_to_sw["L5T_axial_ele" ] = "ly9_m0" ;
        string_to_sw["L5B_stereo_ele"] = "ly9_m1" ;
        string_to_sw["L5T_axial_pos" ] = "ly9_m2" ;
        string_to_sw["L5B_stereo_pos"] = "ly9_m3" ;
        string_to_sw["L5T_stereo_ele"] = "ly10_m0";
        string_to_sw["L5B_axial_ele" ] = "ly10_m1";
        string_to_sw["L5T_stereo_pos"] = "ly10_m2";
        string_to_sw["L5B_axial_pos" ] = "ly10_m3";
        string_to_sw["L6T_axial_ele" ] = "ly11_m0";
        string_to_sw["L6B_stereo_ele"] = "ly11_m1";
        string_to_sw["L6T_axial_pos" ] = "ly11_m2";
        string_to_sw["L6B_stereo_pos"] = "ly11_m3";
        string_to_sw["L6T_stereo_ele"] = "ly12_m0";
        string_to_sw["L6B_axial_ele" ] = "ly12_m1";
        string_to_sw["L6T_stereo_pos"] = "ly12_m2";
        string_to_sw["L6B_axial_pos" ] = "ly12_m3";
    }
    else 
    {
        std::cout << "ERROR: Module Mapper cannot be setup for this year " << year_ << std::endl;
    }
} 

std::map<std::string, std::map<int,int>> ModuleMapper::buildChannelSvtIDMap(){

    std::map<std::string, std::map<int,int>> channel_map;
    std::map<int,int> local_to_svtid_map;
    int channel_index = 0;
    for(int feb=0; feb < 10; feb++){
        std::string str_feb = "F" + std::to_string(feb);
        int max_channel = 640;
        if(year_ != 2016)
            if (feb == 0 || feb == 1) max_channel = 512;
        for(int hybrid=0; hybrid < 4; hybrid++){
            if(year_ == 2016){
            if( (feb == 2 || feb == 9) and hybrid > 1)
                continue;
            }
            std::string str_hybrid = "H" + std::to_string(hybrid);
            for(int channel=0; channel < max_channel; channel++){
                int svtid = channel_index + channel;
                local_to_svtid_map[channel] = svtid;
            }
            channel_map[str_feb + str_hybrid] = local_to_svtid_map ;
            channel_index += max_channel;
        }
    }
    return channel_map;
}

int ModuleMapper::getSvtIDFromHWChannel(int channel, std::string hwTag, std::map<std::string,std::map<int,int>> svtid_map) {
      std::map<int,int> channelMap = svtid_map[hwTag];
      std::map<int,int>::iterator it = channelMap.find(channel);
      if(it != channelMap.end()){
        return it->second;
      }
      else
          return 99999; //Indicates channel not found in map. Feb 0-1 have max_channel = 512. If function receives channel > 512 in these    cases, returns 99999 for svt_id
  }




ModuleMapper::~ModuleMapper() {
}

std::vector<std::string> ModuleMapper::getHybridStrings() {
    std::vector<std::string> hybrids;
    for(int i=0; i < 7; ++i){
        if( i < 4) {
            hybrids.push_back(Form("L%iT_axial",i));
            hybrids.push_back(Form("L%iT_stereo",i));
            hybrids.push_back(Form("L%iB_stereo",i));
            hybrids.push_back(Form("L%iB_axial",i));
        }
        else {
            hybrids.push_back(Form("L%iT_axial_ele",i));
            hybrids.push_back(Form("L%iT_stereo_ele",i));
            hybrids.push_back(Form("L%iT_axial_pos",i));
            hybrids.push_back(Form("L%iT_stereo_pos",i));
            hybrids.push_back(Form("L%iB_axial_ele",i));
            hybrids.push_back(Form("L%iB_stereo_ele",i));
            hybrids.push_back(Form("L%iB_axial_pos",i));
            hybrids.push_back(Form("L%iB_stereo_pos",i));
        }
    }

    return hybrids;
}

void ModuleMapper::buildApvChannelMap() {

    //Database file format is <FebN> <HybN> 
    //For FebN < 2 
        //<apv 0: 128->255> <apv 1: 0->127> <apv 2: 256->383> <apv 3: 384->512>
    //For FebN >= 2
        //<apv 0: 512->639> <apv 1: 384->511> <apv 2: 256->383> <apv 3: 128->255> <pv 4: 0->127>

    //std::pair<int std::make_pair

    //For each row, grab first three value, Feb, Hybrid, and APV
    //Read the row, and store values into vector at appropriate index.
    std::map<std::string,std::map<std::string,std::vector<int>>> map; 
    for(int feb = 0; feb < 10; feb++){
        int napvs = 5;
        if(feb < 2)
            napvs = 4;
        for(int hybrid = 0; hybrid < 5; hybrid++){
            std::string FH_code = std::to_string(feb) + std::to_string(hybrid);
            std::map<std::string, std::vector<int>> apv_channels;
            for(int apv = 0; apv < napvs; apv++){
                //std::pair<int,std::vector<int>> apv_channels;
                std::vector<int> channels;
                int a;
                int b;
                if(year_ != 2016){
                    if(feb < 2){
                        if(apv == 0){
                            a=128;
                            b=255;
                        }
                        if(apv == 1){
                            a=0;
                            b=127;
                        }
                        if(apv == 2){
                            a=256;
                            b=383;
                        }
                        if(apv == 3){
                            a=384;
                            b=511;
                        }
                    }
                    else {
                        if(apv == 0){
                            a=512;
                            b=639;
                        }
                        if(apv == 1){
                            a=384;
                            b=511;
                        }
                        if(apv == 2){
                            a=256;
                            b=383;
                        }
                        if(apv == 3){
                            a=128;
                            b=255;
                        }
                        if(apv == 4){
                            a=0;
                            b=127;
                        }
                    }
                }
                else {
                    if(apv == 0){
                        a=512;
                        b=639;
                    }
                    if(apv == 1){
                        a=384;
                        b=511;
                    }
                    if(apv == 2){
                        a=256;
                        b=383;
                    }
                    if(apv == 3){
                        a=128;
                        b=255;
                    }
                    if(apv == 4){
                        a=0;
                        b=127;
                    }
                }

                for (int i = a; i < b+1; i++){
                   channels.push_back(i); 
                }
                //apv_channels.first = apv;
                //apv_channels.second = channels;
                apv_channels[std::to_string(apv)] = channels;
            }
            map[FH_code] = apv_channels;
        }
    }

    apvChannelMap_ = map;;
}

void ModuleMapper::ReadThresholdsFile(std::string filename){

    std::map<std::string, std::vector<int>> readThresholds;
    std::ifstream threshfile;
    threshfile.open(filename);
    if(!threshfile){
        std::cout << "ERROR! THRESHOLD FILE " << filename << " DOES NOT EXIST!" << std::endl;
        std::cout << "FITTING WILL FAIL. PLEASE SPECIFY THE DATABASE THRESHOLD FILE ASSOCIATED WITH RUN" << std::endl;
    }

    std::string row;
    while (getline (threshfile,row)) {
        std::vector<int> thresholds;
        std::string feb = row.substr(0,1);
        std::string hyb = row.substr(2,1);
        std::string apv = row.substr(4,1);
        std::string data_string = row.substr(5);
        std::stringstream lineStream(data_string);

        unsigned int value;
        while(lineStream >> std::hex >> value)
        {
            thresholds.push_back(value);
        }

        readThresholds[feb+hyb+apv] = thresholds;
    }

    threshfile.close();
    thresholdsIn_ = readThresholds;
}

std::pair<std::string,int> ModuleMapper::findApvChannel(std::string feb, std::string hybrid, int channel) {

    //std::cout << "Looking for Feb " << feb << " Hybrid " << hybrid << "channel " << channel << std::endl;
    std::pair<std::string, int> apv_chindex;
    std::map<std::string, std::map<std::string,std::vector<int>>>::iterator it;
    for (it = apvChannelMap_.begin(); it != apvChannelMap_.end(); it++){
        std::string FH = it->first;
        std::string ifeb = (FH.substr(0,1));
        std::string ihyb = (FH.substr(1,1));
        if(ifeb == feb && ihyb == hybrid)
            bool found = true;
        else
            continue;
        //std::cout << "Found Feb: " << ifeb << " Hybrid: " << ihyb << std::endl;
        std::map<std::string, std::vector<int>> apv_map = it->second;
        std::map<std::string, std::vector<int>>::iterator itb;
        bool foundapv = false;
        for (itb = apv_map.begin(); itb != apv_map.end(); itb++){
            std::string apv = itb->first;
            std::vector<int> channels = itb->second;
            //std::cout << "apv " << apv << std::endl;
            //std::cout << "channels size : " << channels.size() << std::endl;
            for(int i = 0; i < channels.size(); i++){
                if(channels[i] == channel){
                    apv_chindex.first = apv;
                    apv_chindex.second = i;
                    //std::cout << "For channel " << channel << ": APV " << apv << " index " << i << std::endl;
                    foundapv = true;
                    break;
                }
            }
            if(foundapv)
                break;
        }
        if(foundapv)
            break;
    }

    return apv_chindex;
}

int ModuleMapper::getThresholdValue(std::string feb, std::string hybrid, int channel){
    std::pair<std::string,int> apv_channel = findApvChannel(feb, hybrid, channel);
    std::string apv = apv_channel.first;
    int apvchannel = apv_channel.second;
    int threshold = thresholdsIn_[feb+hybrid+apv].at(apvchannel);
    return threshold;
}






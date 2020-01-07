/**
 *  @file   hpstr.cxx
 *  @brief  App used to create and analyze HPS DST files.
 *  @author Omar Moreno, SLAC National Accelerator Laboratory
 *  @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

//----------------//
//   C++ StdLib   //
//----------------//
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept> 

//-----------//
//   hpstr   //
//-----------//
#include "ConfigurePython.h"

using namespace std; 

void displayUsage(); 

int main(int argc, char **argv) { 

    if (argc < 2) {
        displayUsage(); 
        return EXIT_FAILURE;
    }

    int ptrpy = 1;
    for (ptrpy = 1; ptrpy < argc; ptrpy++) {
        std::cout << argv[ptrpy] << std::endl;
        if (strstr(argv[ptrpy], ".py"))
            break;
    }

    if (ptrpy == argc) {
        displayUsage(); 
        printf("  ** No python script provided. **\n");
        return EXIT_FAILURE;
    }

    try {

        std::cout << "---- [ hpstr ]: Loading configuration --------" << std::endl;
        
        ConfigurePython cfg(argv[ptrpy], argv + ptrpy + 1, argc - ptrpy -1);

        std::cout << "---- [ hpstr ]: Configuration load complete  --------" << std::endl;

        Process* p = cfg.makeProcess();
        int run_mode = p->getRunMode();

        std::cout << "---- [ hpstr ]: Process mode " << run_mode << " initialized.  --------" << std::endl;

        // If Ctrl-c is used, immediately exit the application.
        struct sigaction act;
        memset (&act, '\0', sizeof(act));
        if (sigaction(SIGINT, &act, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        std::cout << "---- [ hpstr ]: Start of processing --------" << std::endl;

        //TODO Make this better
        if (run_mode == 0) 
        {
            std::cout<<"---- [ hpstr ]: Running LCIO -> ROOT Process --------" << std::endl;
            p->run();
        }
        else if (run_mode == 1) 
        {
            std::cout<<"---- [ hpstr ]: Running ROOT -> Histo Process --------" << std::endl;
            p->runOnRoot();
        }
        else if (run_mode == 2)
        {
            std::cout<<"---- [ hpstr ]: Running Histo Analysis Process --------" << std::endl;
            p->runOnHisto();
        }
        else 
        {
            std::cout<<"---- [ hpstr ]: Run Mode " << run_mode << " does not exist! --------" << std::endl;
        }

        std::cout << "---- [ hpstr ]: Event processing complete  --------" << std::endl;

    } catch (exception& e) { 
        //std::cerr << "Error! [" << e.name() << "] : " << e.message() << std::endl;
        //std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;

    } 

    return EXIT_SUCCESS;

}

void displayUsage() {
    printf("Usage: hpstr [application arguments] {configuration_script.py}"
            " [arguments to configuration script]\n");
}

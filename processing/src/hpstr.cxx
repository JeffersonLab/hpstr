/**
 *  @file   hpstr.cxx
 *  @brief  App used to create and analyze HPS DST files.
 *  @author Omar Moreno, SLAC National Accelerator Laboratory
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

        std::cout << "---- [ hpstr ]: Process initialized.  --------" << std::endl;

        // If Ctrl-c is used, immediately exit the application.
        struct sigaction act;
        memset (&act, '\0', sizeof(act));
        if (sigaction(SIGINT, &act, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        std::cout << "---- [ hpstr ]: Starting event processing --------" << std::endl;

        //TODO Make this better
        if (p->processRootFiles()) {
            std::cout<<"---- [ hpstr ]: Running on ROOT Files --------" << std::endl;
            p->runOnRoot();
        }
        else {
            std::cout<<"---- [ hpstr ]: Running on LCIO Files --------" << std::endl;
            p->run();
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

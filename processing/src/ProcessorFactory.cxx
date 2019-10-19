/**
 * @file ProcessorFactory.cxx
 * @brief Class which provides a singleton module factory that creates Processor
 *        objects
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "ProcessorFactory.h"

#include <dlfcn.h>

ClassImp(ProcessorFactory)

void ProcessorFactory::registerProcessor(const std::string& classname, ProcessorMaker* maker) {
    auto ptr = module_info_.find(classname);
    if (ptr != module_info_.end()) {
        std::cout << "Error: " << std::endl;
    }
    ProcessorInfo mi;
    mi.classname = classname;
    mi.maker = maker;
    module_info_[classname] = mi;
}

/*
std::vector<std::string> ProcessorFactory::getProcessorClasses() const {
    std::vector<std::string> classes;
    for (auto ptr : module_info_) {
        classes.push_back(ptr.first);
    }
    return classes;
}
*/

Processor* ProcessorFactory::createProcessor(const std::string& classname, const std::string& module_instance_name, Process& process) {
    auto ptr = module_info_.find(classname);
    if (ptr == module_info_.end()) {
        return 0;
    }
    return ptr->second.maker(module_instance_name, process);
}

void ProcessorFactory::loadLibrary(const std::string& libname) {
    
    std::cout << "[ ProcessorFactory ]: Loading library " << libname << std::endl;

    if (libs_loaded_.find(libname) != libs_loaded_.end()) {
        return; // already loaded
    }

    void* handle = dlopen(libname.c_str(), RTLD_NOW);
    if (handle == NULL) {
        std::cout << dlerror() << std::endl;
        throw std::runtime_error("[ ProcessorFactory ]: Error loading library " + libname + ": " + dlerror());         
    }
    
    libs_loaded_.insert(libname);
}


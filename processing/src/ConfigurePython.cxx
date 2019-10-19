/**
 * @file ConfigurePython.cxx
 * @brief Utility class that reads/executes a python script and creates a 
 *        Process object based on the input.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "ConfigurePython.h"

static std::string stringMember(PyObject* owner, const std::string& name) {

    std::string retval;
    PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
    if (temp != 0) {
#if PY_MAJOR_VERSION >= 3
      PyObject* pyStr = PyUnicode_AsEncodedString(temp, "utf-8","Error ~");
      retval = PyBytes_AS_STRING(pyStr);
      Py_XDECREF(pyStr);
#else
        retval = PyString_AsString(temp);
#endif
        Py_DECREF(temp);
    }
    return retval;
}


static long intMember(PyObject* owner, const std::string& name) {
    
    long retval;
    PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
    if (temp != 0) {
        retval = PyLong_AsLong(temp);
        Py_DECREF(temp);
    }
    return retval;
}


ConfigurePython::ConfigurePython(const std::string& python_script, char* args[], int nargs) {

    std::string path(".");
    std::string cmd = python_script;

    // if a path was specified, extract it and the command
    if (python_script.rfind("/") != std::string::npos) {
        path = python_script.substr(0, python_script.rfind("/"));
        cmd = python_script.substr(python_script.rfind("/") + 1);
    }
    cmd = cmd.substr(0, cmd.find(".py"));

    // Initialize the python interpreter. 
    Py_Initialize();

    // Set the command line arguments passed to the python script to be 
    // executed. Note that the first parameter in the list or arguments 
    // should refer to the script to be executed.
    if (nargs > 0) {
#if PY_MAJOR_VERSION >= 3
      wchar_t** targs = new wchar_t*[nargs + 1];
      targs[0] = Py_DecodeLocale(python_script.c_str(),NULL);
      for (int i = 0; i < nargs; i++)
          targs[i + 1] = Py_DecodeLocale(args[i],NULL);
      
      PySys_SetArgv(nargs+1, targs);
      delete[] targs;
#else
      char** targs = new char*[nargs + 1];
      targs[0] = (char*) python_script.c_str();
      for (int i = 0; i < nargs; i++)
          targs[i + 1] = args[i];

      PySys_SetArgvEx(nargs+1, targs, 1);
      delete[] targs;
#endif
    }

    PyObject* script = nullptr; 
    PyObject* process = nullptr; 
    PyObject* p_main = nullptr; 
    PyObject* py_list = nullptr; 
    PyObject* p_process = nullptr; 
  
    try { 
      // Load the python script.
      script = PyImport_ImportModule(cmd.c_str());
      
    // If a reference to the python script couldn't be created, raise 
    // an exception.  
     if (script == 0) {
        PyErr_Print();
        throw std::runtime_error("[ ConfigurePython ]: Problem loading python script."); 
    }

    Py_DECREF(script);

    // Load the script that is used create a processor
    PyObject* pCMod = PyObject_GetAttrString(script, "HpstrConf");
    if (pCMod == 0) {
        PyErr_Print();
        throw std::runtime_error("[ ConfigurePython ]: Problem loading configuration"); 
    }

    PyObject* p_process_class = PyObject_GetAttrString(pCMod, "Process");
    Py_DECREF(pCMod);
    if (p_process_class == 0) {
        PyErr_Print();
        throw std::runtime_error("[ ConfigurePython ]: Problem loading Process class"); 
    }
    
    p_process = PyObject_GetAttrString(p_process_class, "lastProcess");
    Py_DECREF(p_process_class);
    if (p_process == 0) {
        PyErr_Print();
        throw std::runtime_error("[ ConfigurePython ]: Problem loading Process class"); 
    }

    event_limit_ = intMember(p_process, "max_events");

    PyObject* p_sequence = PyObject_GetAttrString(p_process, "sequence");
    if (!PyList_Check(p_sequence)) {
        throw std::runtime_error("[ ConfigurePython ]: Sequence is not a python list as expected."); 
    }
    
    for (Py_ssize_t i = 0; i < PyList_Size(p_sequence); i++) {
        PyObject* processor = PyList_GetItem(p_sequence, i);
        ProcessorInfo pi;
        pi.classname_ = stringMember(processor, "class_name");
        pi.instancename_ = stringMember(processor, "instance_name");
        std::cout << pi.classname_ << std::endl;
        
        PyObject* params = PyObject_GetAttrString(processor, "parameters");
        if (params != 0 && PyDict_Check(params)) {
            PyObject *key(0), *value(0);
            Py_ssize_t pos = 0;

            while (PyDict_Next(params, &pos, &key, &value)) {
#if PY_MAJOR_VERSION >= 3
              PyObject* pyStr = PyUnicode_AsEncodedString(key, "utf-8","Error ~");
              std::string skey = PyBytes_AS_STRING(pyStr);

               if (PyLong_Check(value)) {
                   pi.params_.insert(skey, int(PyLong_AsLong(value)));
                   //printf("Int Key: %s\n",skey.c_str());
               } else if (PyFloat_Check(value)) {
                   pi.params_.insert(skey, PyFloat_AsDouble(value));
                   //printf("Double Key: %s\n",skey.c_str());
               } else if (PyUnicode_Check(value)) {
                   PyObject* pyStr = PyUnicode_AsEncodedString(value, "utf-8","Error ~");
                   pi.params_.insert(skey, PyBytes_AS_STRING(pyStr));
                   Py_XDECREF(pyStr);
                   //printf("String Key: %s\n",skey.c_str());
               } else if (PyList_Check(value)) { // assume everything is same value as first value
                   if (PyList_Size(value) > 0) {
                       PyObject* vec0 = PyList_GetItem(value, 0);
                       if (PyLong_Check(vec0)) {
                           std::vector<int> vals;
                           for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                               vals.push_back(PyLong_AsLong(PyList_GetItem(value, j)));
                           pi.params_.insert(skey, vals);
                           //printf("VInt Key: %s\n",skey.c_str());
                       } else if (PyFloat_Check(vec0)) {
                           std::vector<double> vals;
                           for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                               vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));
                           pi.params_.insert(skey, vals);
                           //printf("VDouble Key: %s\n",skey.c_str());
                       } else if (PyUnicode_Check(vec0)) {
                           std::vector<std::string> vals;
                           for (Py_ssize_t j = 0; j < PyList_Size(value); j++){
                               PyObject* pyStr = PyUnicode_AsEncodedString(PyList_GetItem(value, j), "utf-8","Error ~");
                               vals.push_back( PyBytes_AS_STRING(pyStr));
                                Py_XDECREF(pyStr);
                           }
                           pi.params_.insert(skey, vals);
                           //printf("VString Key: %s\n",skey.c_str());
                       }
                   }
               }

#else
                std::string skey = PyString_AsString(key);
                if (PyInt_Check(value)) {
                    pi.params_.insert(skey, int(PyInt_AsLong(value)));
                    //printf("Int Key: %s\n",skey.c_str());
                } else if (PyFloat_Check(value)) {
                    pi.params_.insert(skey, PyFloat_AsDouble(value));
                    //printf("Double Key: %s\n",skey.c_str());
                } else if (PyString_Check(value)) {
                    pi.params_.insert(skey, PyString_AsString(value));
                    //printf("String Key: %s\n",skey.c_str());
                } else if (PyList_Check(value)) { // assume everything is same value as first value
                    if (PyList_Size(value) > 0) {
                        PyObject* vec0 = PyList_GetItem(value, 0);
                        if (PyInt_Check(vec0)) {
                            std::vector<int> vals;
                            for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                vals.push_back(PyInt_AsLong(PyList_GetItem(value, j)));
                            pi.params_.insert(skey, vals);
                            //printf("VInt Key: %s\n",skey.c_str());
                        } else if (PyFloat_Check(vec0)) {
                            std::vector<double> vals;
                            for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));
                            pi.params_.insert(skey, vals);
                            //printf("VDouble Key: %s\n",skey.c_str());
                        } else if (PyString_Check(vec0)) {
                            std::vector<std::string> vals;
                            for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                vals.push_back(PyString_AsString(PyList_GetItem(value, j)));
                            pi.params_.insert(skey, vals);
                            //printf("VString Key: %s\n",skey.c_str());
                        }
                    }
                }
#endif
            }
        }

        sequence_.push_back(pi);
    }
    Py_DECREF(p_sequence);
    
    py_list = PyObject_GetAttrString(p_process, "input_files");
    if (!PyList_Check(py_list)) {
        throw std::runtime_error("[ ConfigurePython ]: Input files is not a python list as expected."); 
        return;
    }
    for (Py_ssize_t i = 0; i < PyList_Size(py_list); i++) {
        PyObject* elem = PyList_GetItem(py_list, i);
#if PY_MAJOR_VERSION >= 3
        PyObject* pyStr = PyUnicode_AsEncodedString(elem, "utf-8","Error ~");
        input_files_.push_back(PyBytes_AS_STRING(pyStr));
        Py_XDECREF(pyStr);
#else
        input_files_.push_back(PyString_AsString(elem));
#endif
    }
    Py_DECREF(py_list);

    py_list = PyObject_GetAttrString(p_process, "output_files");
    if (!PyList_Check(py_list)) {
        throw std::runtime_error("[ ConfigurePython ]: Output files is not a python list as expected."); 
        return;
    }
    for (Py_ssize_t i = 0; i < PyList_Size(py_list); i++) {
        PyObject* elem = PyList_GetItem(py_list, i);
#if PY_MAJOR_VERSION >= 3
        PyObject* pyStr = PyUnicode_AsEncodedString(elem, "utf-8","Error ~");
        output_files_.push_back(PyBytes_AS_STRING(pyStr));
        Py_XDECREF(pyStr);
#else
        output_files_.push_back(PyString_AsString(elem));
#endif
    }
    Py_DECREF(py_list);

    py_list = PyObject_GetAttrString(p_process, "libraries");
    if (!PyList_Check(py_list)) {
        throw std::runtime_error("[ ConfigurePython ]: libraries is not a python list as expected."); 
        return;
    }
    for (Py_ssize_t i = 0; i < PyList_Size(py_list); i++) {
        PyObject* elem = PyList_GetItem(py_list, i);
#if PY_MAJOR_VERSION >= 3
        PyObject* pyStr = PyUnicode_AsEncodedString(elem, "utf-8","Error ~");
        libraries_.push_back(PyBytes_AS_STRING(pyStr));
        Py_XDECREF(pyStr);
#else
      libraries_.push_back(PyString_AsString(elem));
#endif
    }
    Py_DECREF(py_list);

    } catch (std::exception& e) { 
        std::cout << e.what() << std::endl;
    }

}

ConfigurePython::~ConfigurePython() {
    Py_Finalize();
}

Process* ConfigurePython::makeProcess() { 
    Process* p = new Process();

    for (auto lib : libraries_) {
        ProcessorFactory::instance().loadLibrary(lib);
    }

    for (auto proc : sequence_) {
        Processor* ep = ProcessorFactory::instance().createProcessor(proc.classname_, proc.instancename_, *p);
        if (ep == 0) {
            throw std::runtime_error("[ ConfigurePython ]: Unable to create instance of " + proc.instancename_); 
        }
        ep->configure(proc.params_);
        p->addToSequence(ep);    
    }
        
    for (auto file : input_files_) {
        p->addFileToProcess(file);
    }

    for (auto file : output_files_) { 
        p->addOutputFileName(file); 
    }

    p->setEventLimit(event_limit_);

    return p; 
}

/**
 * @file ParameterSet.h
 * @brief Class which contains parameters passed to a Processor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _PARAMETER_SET_H_
#define _PARAMETER_SET_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <map>
#include <stdexcept>
#include <vector>
#include <string>

/**
 * @brief description
 * 
 */
class ParameterSet { 

    public:
        /**
         * @brief Get an integer by name or throw an exception if not available,
         *        or not the right type.
         * @param name Name of the integer.
         */
        int getInteger(const std::string& name) const;

        /**
         * @brief Get an integer by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the integer.
         * @param defaultValue Default value to use if the parameter was not provided.
         */
        int getInteger(const std::string& name, int defaultValue) const;

        /**
         * @brief Get a double by name or throw an exception if not available,
         *        or not the right type.
         * 
         * @param name Name of the double parameter.
         */
        double getDouble(const std::string& name) const;

        /**
         * @brief Get a double by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the double parameter.
         * @param defaultValue Default value to ue if the parameter was not provided.
         */
        double getDouble(const std::string& name, double defaultValue) const;

        /**
         * Get a string by name or throw an exception if not available, or not the right type.
         * @param name Name of the string parameter.
         */
        const std::string& getString(const std::string& name) const;

        /**
         * @brief Get a string by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the string parameter.
         * @param defaultValue Default value to ue if the parameter was not provided.
         */
        const std::string& getString(const std::string& name, const std::string& defaultValue) const;

        /**
         * @brief Get a vector of integers by name or throw an exception if not available,
         *        or not the right type.
         * @param name Name of the vector of integers.
         */
        const std::vector<int>& getVInteger(const std::string& name) const;

        /**
         * @brief Get a vector of integers by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the vector of integers.
         * @param defaultValue Default value to ue if the parameter was not provided.
         */
        const std::vector<int>& getVInteger(const std::string& name, const std::vector<int>& defaultValue) const;

        /**
         * @brief Get a vector of doubles by name or throw an exception if not available,
         *        or not the right type.
         * @param name Name of the vector of doubles.
         */
        const std::vector<double>& getVDouble(const std::string& name) const;

        /**
         * @brief Get a vector of doubles by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the vector of doubles.
         * @param defaultValue Default value to ue if the parameter was not provided.
         */
        const std::vector<double>& getVDouble(const std::string& name, const std::vector<double>& defaultValue) const;

        /**
         * @brief Get a vector of strings by name or throw an exception if not available,
         *        or not the right type.
         * @param name Name of the vector of strings.
         */
        const std::vector<std::string>& getVString(const std::string& name) const;

        /**
         * @brief Get a vector of strings by name or use the given default if not available.
         * 
         * If the wrong type, throw an exception.
         * 
         * @param name Name of the vector of strings.
         * @param defaultValue Default value to ue if the parameter was not provided.
         */
        const std::vector<std::string>& getVString(const std::string& name, const std::vector<std::string>& defaultValue) const;

        /**
         * @brief Add an integer to the ParameterSet.
         * 
         * @param name Name of the integer parameter.
         * @param value Value of the integer parameter.
         */
        void insert(const std::string& name, int value);

        /**
         * @brief Add a double to the ParameterSet.
         * 
         * @param name Name of the double parameter.
         * @param value Value of the double parameter.
         */
        void insert(const std::string& name, double value);

        /**
         * @brief Add a string to the ParameterSet.
         * 
         * @param name Name of the string parameter.
         * @param value Value of the string parameter.
         */
        void insert(const std::string& name, const std::string& value);

        /**
         * @brief Add a vector of integers to the ParameterSet.
         * 
         * @param name Name of the integer vector parameter.
         * @param values Values of the integer vector parameter.
         */
        void insert(const std::string& name, const std::vector<int>& values);

        /**
         * @brief Add a vector of doubles to the ParameterSet.
         * 
         * @param name Name of the double vector parameter.
         * @param values Values of the double vector parameter.
         */
        void insert(const std::string& name, const std::vector<double>& values);

        /**
         * @brief Add a vector of strings to the ParameterSet.
         * 
         * @param name Name of the string vector parameter.
         * @param values Values of the string vector parameter.
         */
        void insert(const std::string& name, const std::vector<std::string>& values);

        /**
         * @enum Specifies the type of a parameter in a ParameterSet.
         */
        typedef enum {
            et_NoType, 
            et_Bool, 
            et_Integer, 
            et_Double, 
            et_String,
            et_VInteger,
            et_VDouble,
            et_VString, 
            et_ParameterSet
        } ElementType;

        /**
         * @struct Element
         * @brief Backing data structure containing parameter values
         *
         * @todo Fully document me!
         */
        struct Element {

            Element() : et_{et_NoType} {;}

            Element(bool inval) : et_{et_Bool}, boolval_{inval} {;}

            Element(int inval) : et_{et_Integer}, intval_{inval} {;}

            Element(double inval) : et_ {et_Double}, doubleval_{inval} {;}

            Element(const std::string& inval) : et_ { et_String }, strval_ { inval } {;}

            Element(const std::vector<int>& inval);

            Element(const std::vector<double>& inval);

            Element(const std::vector<std::string>& inval);

            ElementType et_;
            bool boolval_{false};
            int intval_{0};
            double doubleval_{0};
            std::string strval_;
            std::vector<int> ivecVal_;
            std::vector<double> dvecVal_;
            std::vector<std::string> svecVal_;
            ParameterSet* subsetVal_ { 0 };
        };

        std::map<std::string, Element> elements_;
}; 

#endif // _PARAMETER_SET_H_

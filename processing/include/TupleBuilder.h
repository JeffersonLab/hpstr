/**
 * @file TupleBuilder.h
 * @brief Builder class to build ROOT ntuples.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __TUPLE_BUILDER_H__
#define __TUPLE_BUILDER_H__

#include <iostream>

//----------//
//   ROOT   //
//----------//
#include "TTree.h"

//-----------//
//   boost   //
//-----------//
#include "boost/assign/list_of.hpp"
#include "boost/variant.hpp"
#include "boost/unordered_map.hpp"

typedef boost::variant< short, int, float, double, long > v; 

// Forward declerations
class TTree; 

class TupleBuilder {

    public: 

        enum Type { 
            Short, 
            Int, 
            Float, 
            Double, 
            Long, 
            Bool 
        };

        /** Constructor */
        TupleBuilder(TTree* tree); 

        /**
         * Add a leaf to the root tree. 
         *
         * @param name The leaf name
         * @param type The leaf type
         */
        template <typename T>
        TTree* add(std::string name, Type type) {
            variables_[name] = T(-9999); 
            tree_->Branch(name.c_str(), &boost::get<T>(variables_[name]), 
                (name + "/" + type_string_[type]).c_str()); 
            return tree_;
        }

        /**
         * Access a leaf in the tree.
         * 
         * @param name The leaf name.
         */
        template <typename T>
        void set(std::string name, T value) { 
            variables_[name] = value;
        }


    private: 

        /** ROOT TTree used to build the tuple. */
        TTree* tree_{nullptr}; 

        /** Container for variables. */
        std::map<std::string, v> variables_; 
        
        /** Map from Type to string representation used by ROOT. */
        boost::unordered_map<Type, const char*> type_string_  = 
            { 
                {Type::Short,  "S"}, 
                {Type::Int,    "I"},
                {Type::Float,  "F"},
                {Type::Double, "D"},
                {Type::Long,   "L"},
                {Type::Bool,   "O"}
            };

}; // TupleBuilder 

#endif // __TUPLE_BUILDER_H__

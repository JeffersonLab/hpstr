/**
 * @file HodoHit.h
 * @brief Class that encapsulates hodoscope hit information
 * @author Maurik Holtrop, University of New Hampshire
 * *
 * * TODO: Entire thing can be replaced by a simple struct.
 */

#ifndef _HODO_HIT_H_
#define _HODO_HIT_H_

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

class HodoHit : public TObject {
    
    public:    

        /** Constructor */
        HodoHit(){};

        /** Destructor */
        ~HodoHit(){ Clear(); };

        /**
         * Set the energy of the hit in GeV.
         *
         * @param energy The energy of the hit in GeV. 
         */
        void setEnergy(const double energy){ energy_ = energy; }; 
      
        /** @return The energy of the hit in GeV. */
        double getEnergy() const { return energy_; };
        /**
         * Set the hole number of the hit.
         *
         * @param hole The hole number of the hit.
         */
        void setHole(const int hole) { hole_ = hole; };

        /** @return The layer number of the hit. */
        double getHole() const { return hole_; };

        /**
         * Set the layer of the hit.
         *
         * @param layer The layer of the hit.
         */
        void setLayer(const int layer) { layer_ = layer; };

        /** @return The layer number of the hit. */
        double getLayer() const { return layer_; };

        /**
         * Set the time of the hit in ns. 
         *
         * @param time The time of the hit in ns. 
         */
        void setTime(const double time) { time_ = time; };

        /** @return The time of the hit in ns. */
        double getTime() const { return time_; };

        /**
         * Set the indices of the crystal.
         *
         * @param index_x The index along x
         * @param index_y The index along y
         */
        void setIndices(int index_x, int index_y){
            index_x_ = index_x;
            index_y_ = index_y;
        };

        /** @return The crystal indices. */ 
        std::vector<int> getIndices() const { return { index_x_, index_y_ }; }

    private: 
        
        /** The crystal index along x. */
        int index_x_{-9999};

        /** The crystal index along y. */
        int index_y_{-9999};
  
        int layer_{-9999};
    
        int hole_{-9999};

        /** The energy of the hit in GeV. */ 
        double energy_{-9999};

        /** The time of the hit in ns. */
        double time_{0};
        
        ClassDef(HodoHit, 1);
};

#endif

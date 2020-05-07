/**
 * @file HodoCluster.cxx
 * @brief Class used to encapsulate hodoscope cluster information.
 * @author Maurik Holtrop, University of New Hampshire.
 */

#ifndef _HODO_CLUSTER_H__
#define _HODO_CLUSTER_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <vector>

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>
#include <TObject.h>
#include <TRefArray.h>
#include <TRef.h>

class HodoCluster : public TObject {
    
public:
    
    /** Constructor */
    HodoCluster(){};
    HodoCluster(int ix,int iy, int layer, double energy, double time):
    index_x_{ix},index_y_{iy},layer_{layer},energy_{energy},time_{time} {};
    
    /** Destructor */
    ~HodoCluster();
    
    /** Reset the Cluster object */
    void Clear(Option_t *option="");
    
    /**
     * Add a reference to a calorimeter hit composing this cluster.
     *
     * @param hit : Cal hit composing with this cluster
     */
    void addHit(TObject* hit);
    
    /**
     * @return An array of references to the calorimeter hits composing
     * this cluster.
     */
    TRefArray* getHits() const { return hits_; }
    
    /**
     * Set the energy of the calorimeter cluster.
     *
     * @param energy : The energy of the calorimeter cluster.
     */
    void setEnergy(const double energy) { energy_ = energy; };
    
    /** @return The energy of the calorimeter cluster. */
    double getEnergy() const { return energy_; };
    
    /**
     * Set the layer of the hit.
     *
     * @param layer The layer of the hit.
     */
    void setLayer(const int layer) { layer_ = layer; };
    
    /** @return The layer number of the hit. */
    double getLayer() const { return layer_; };
    
    
    /**
     * Set the time of the calorimeter clusters.
     *
     * @param time The cluster time
     */
    void setTime(const double time) { time_ = time; };
    
    /** @return The time of the cluster. */
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
    
    
    ClassDef(HodoCluster, 1);
    
    //  private:
    
    /** An array of references to the hits associated withi this cluster. */
    TRefArray* hits_{new TRefArray{}};
    
    /** The number of hits composing this cluster. */
    int n_hits_{0};
    
    /** The x index of the cluster */
    int index_x_{-9999};
    
    /** The y index of the cluster */
    int index_y_{-9999};
    
    /** The layer of the cluster */
    int layer_{-9999};
    
    /** The energy of the cluster in GeV. */
    double energy_{-9999};
    
    /** The cluster time. */
    double time_{-9999};
    
};

#endif // _HODO_CLUSTER_H_

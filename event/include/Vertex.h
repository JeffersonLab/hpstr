/**
 * @file Vertex.h
 * @brief Class used to encapsulate Vertex information.
 * @author PF, SLAC
 * @author Cameron Bravo, SLAC
 */

#ifndef _VERTEX_H_
#define _VERTEX_H_


#include <vector>

#include <TObject.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TVector3.h>

//TODO make float/doubles accordingly.

class Vertex : public TObject {

    public:

        /** Constructor */
        Vertex();

        /** Destructor */
        ~Vertex();

        /** Reset The Vertex object */
        void Clear(Option_t *option="");

        /** 
         * Add a reference to a Track used for this vertex
         * 
         * @param: A Track object
         */

        void addParticle(TObject* part);

        //TODO unify
        /** Set the chi2 */
        void setChi2(const double chi2) {chi2_ = chi2;}

        /** Set the NDF */
        void setNdf(const double ndf) {ndf_ = ndf;}

        /** Set the X position */
        void setX(const double x) {pos_.SetX(x);}

        /** Set the Y position */
        void setY(const double y) {pos_.SetY(y);}

        /** Set the Z position */
        void setZ(const double z) {pos_.SetZ(z);}

        /** Set the position of the vertex. If rotate is set to true, then position is defined in svt coordinates */
        void setPos(const float* pos, bool rotate = false);

        /** Set the position from a TVector */
        void setPos(const TVector3& pos);

        /** Vertex parameters depend on LCIO files. 
         *  The available parameters are invMass, p1X, p2Y, p2X, p1Z, p2Z, p1Y, invMassError
         *  For unconstrained:
         * V0PzErr, invMass, V0Pz, vXErr, V0Py, V0Px, V0PErr, V0TargProjY, vZErr, V0TargProjXErr, vYErr, V0TargProjYErr, invMassError, p1X, p2Y, p2X, V0P, p1Z, p1Y, p2Z, V0TargProjX, layerCode, V0PxErr, V0PyErr,
         */
        void setVtxParameters(const std::vector<float>& parameters);

        /** Set the type */
        void setType(const std::string& type) {type_ = type;}

        /** Set the ID */
        void setID (const int id) {id_=id;}

        /** Set the invariant mass */
        float setInvMass() const {return invM_;}

        /** Set the invariant mass error */
        float setInvMassErr() const {return invMerr_;}

        /** Sets the covariance matrix as a simple vector of values
         *  Covariance matrix of the position (stored as lower triangle matrix, i.e.
         *  cov(xx),cov(y,x),cov(y,y) ).
         */
        void setCovariance( const std::vector<float>& vec);

        /** Set the probability */
        void setProbability(const float probability) {probability_ = probability;}

        TRefArray* getParticles(); 

        /** Returns the covariance matrix as a simple vector of values */
        const std::vector<float>& getCovariance() const {return covariance_;}

        /** Get the probability */
        float getProbability() const {return probability_;}

        /** Get the chi2 */
        double getChi2() const {return chi2_;}

        /** Get the NDF */
        double getNdf() const {return ndf_;}

        /** Get the X position */
        double getX() const {return pos_.X();}

        /** Get the Y position */
        double getY() const {return pos_.Y();}

        /** Get the Z position */
        double getZ() const {return pos_.Z();}

        /** Get the position vector */
        TVector3 getPos() const {return pos_;}

        /** Get the type */
        std::string getType() const {return type_;}

        /** Get the ID */
        int getID () {return id_;}

        /** Get the invariant mass */
        float getInvMass     () const {return invM_;}

        /** Get the invariant mass error */
        float getInvMassErr  () const {return invMerr_;}

        //TODO - Change these to use the track references after filling the
        //track refs
        TVector3 getP1       () const {return p1_;}
        double   getP1X      () const {return p1_.X();}
        double   getP1Y      () const {return p1_.Y();}
        double   getP1Z      () const {return p1_.Z();}

        TVector3 getP2       () const {return p2_;}
        double   getP2X      () const {return p2_.X();}
        double   getP2Y      () const {return p2_.Y();}
        double   getP2Z      () const {return p2_.Z();}

        ClassDef(Vertex,1);

    private:

        double chi2_{-999};
        int ndf_{-999};  
        TVector3 pos_,p1_,p2_,p1e_,p2e_;

        float invM_{-999};
        float invMerr_{-999};

        std::vector<float> covariance_{};
        float probability_{-999};
        int id_;
        std::string type_{""};
        TRefArray* parts_{new TRefArray()};
        int n_parts_{0};
        std::vector<float> parameters_;

}; // Vertex

#endif // __VERTEX_H__

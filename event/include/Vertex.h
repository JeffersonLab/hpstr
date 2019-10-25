/**
 * @file Vertex.h
 * @brief Class used to encapsulate Vertex information.
 * @author PF, SLAC
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

  void addTrack(TObject* track);

  /**
   * @return A reference to the tracks associated with this vertex
   */
  TRefArray* getTracks() const {return tracks_;}; 
  
  //TODO unify
  void   setChi2     (const double chi2) {chi2_ = chi2;};
  double getChi2     () const {return chi2_;};

  void   setX        (const double x) {x_ = x;};
  double getX        () const {return x_;};

  void   setY        (const double y) {y_ = y;};
  double getY        () const {return y_;};

  void   setZ        (const double z) {z_ = z;};
  double getZ        () const {return z_;};
  
  /** Set the position of the vertex. If rotate is set to true, then position is defined in svt coordinates */
  void     setPos    (const float* pos, bool rotate = false);
  
  /** Set the position from a TVector */
  void     setPos    (const TVector3& pos);
  TVector3 getPos    () const {return pos_;};

  /** Vertex parameters depend on LCIO files. 
   *  The available parameters are invMass, p1X, p2Y, p2X, p1Z, p2Z, p1Y, invMassError
   *  For unconstrained:
   * V0PzErr, invMass, V0Pz, vXErr, V0Py, V0Px, V0PErr, V0TargProjY, vZErr, V0TargProjXErr, vYErr, V0TargProjYErr, invMassError, p1X, p2Y, p2X, V0P, p1Z, p1Y, p2Z, V0TargProjX, layerCode, V0PxErr, V0PyErr,
   */
  
  void setVtxParameters(const std::vector<float>& parameters);
   
  void        setType (const std::string& type) {type_ = type;};
  std::string getType() const {return type_;};

  bool vxTracksAvailable() const ;

  int nTracks() const; 

  /** Returns the covariance matrix as a simple vector of values */
  const std::vector<float>& getCovariance () const {return covariance_;};
  
  /** Sets the covariance matrix as a simple vector of values
   *  Covariance matrix of the position (stored as lower triangle matrix, i.e.
   *  cov(xx),cov(y,x),cov(y,y) ).
   */
  
  void setCovariance( const std::vector<float>& vec);
  
  void   setNdf     (const double ndf) {ndf_ = ndf;};
  double getNdf     () const {return ndf_;};

  void setProbability(const float probability) {probability_ = probability;};
  float getProbability () const {return probability_;};
  
  //sets the id
  void setID (const int id) {id_=id;};

  //gets the ID
  int getID () {return id_;}

  //TODO - 2 tracks only?
  double   getP1X      () const {return p1x_;};
  double   getP1Y      () const {return p1y_;};
  double   getP1Z      () const {return p1z_;};
  TVector3 getP1       () const {return p1_;};

  double   getP2X      () const {return p2x_;};
  double   getP2Y      () const {return p2y_;};
  double   getP2Z      () const {return p2z_;};
  TVector3 getP2       () const {return p2_;};

  float getInvMass     () const {return invM_;};
  float getInvMassErr  () const {return invMerr_;};
    
 private:

  double chi2_{-999};
  double x_{-999};
  double y_{-999};
  double z_{-999};
  int ndf_{-999};  
  TVector3 pos_,p1_,p2_;
  int ntracks_;

  double p1x_{-999};
  double p2x_{-999};

  double p1y_{-999};
  double p2y_{-999};

  double p1z_{-999};
  double p2z_{-999};

  float invM_{-999};
  float invMerr_{-999};
  
  
  std::vector<float> covariance_{};
  float probability_{-999};
  int id_;
  std::string type_{""};
  TRefArray* tracks_{nullptr};
  std::vector<float> parameters_;
  
  ClassDef(Vertex,1);

}; // Vertex

#endif // __VERTEX_H__

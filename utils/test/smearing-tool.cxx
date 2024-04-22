#include "TrackSmearingTool.h"
#include "TrackerHit.h"
#include "Track.h"

#include "TTree.h"
#include "TFile.h"

void usage() {
  std::cout << R"(
 Run the smearing tool over known momentum values for testing.

  smearing-tool {smearing_file.root} {output.root}

 The output.root file will contain a TTree of the input data
 and the resulting smeared momentum.
)" << std::flush;
}

void smearing_test(
    TDirectoryFile* output_d,
    const std::string& smearing_file,
    const std::string& name,
    std::function<double()> unsmeared_sampler,
    int trials = 100,
    int min_nhits = 8,
    int max_nhits = 12
) {
  TrackSmearingTool tst{smearing_file};
  TrackerHit empty_hit;
  Track track;
  
  track.setTrackParameters(
      0. /* d0 */,
      0. /* phi0 */,
      1. /* omega */,
      1. /* tan_lambda - choosing positive (top) */,
      0. /* z0 */
  );
  for (int i{0}; i < min_nhits-1; i++) track.addHit(&empty_hit);

  bool top = true;
  int nhits = min_nhits;
  double unsmeared_momentum = 1.;
  double smeared_momentum = 1.;

  output_d->cd();
  TTree data{name.c_str(), name.c_str()};
  data.Branch("top", &top);
  data.Branch("nhits", &nhits);
  data.Branch("unsmeared_momentum", &unsmeared_momentum);
  data.Branch("smeared_momentum", &smeared_momentum);

  for (nhits = min_nhits; nhits < max_nhits+1; nhits++) {
    track.addHit(&empty_hit);
    for (int _i{0}; _i < trials; _i++) {
      unsmeared_momentum = unsmeared_sampler();
      track.setMomentum(0. /*px*/, 0. /*py*/, unsmeared_momentum /*pz*/);
      tst.updateWithSmearP(track);
      smeared_momentum = track.getP();
      data.Fill();
    }
  }
  data.Write();
}

double fixed_momentum_sampler() {
  return 1.0;
}

double normal_momentum() {
  static std::normal_distribution dist(1.0, 0.05);
  static std::mt19937 gen;
  return dist(gen);
}

int main(int nargs, char* argv[]) try {
  /*
  std::cout << nargs << " : ";
  for (int i{0}; i < nargs; i++) std::cout << argv[i] << " ";
  std::cout << std::endl;
  */
  if (nargs < 2) {
    usage();
    return 1;
  } else if (nargs < 3) {
    usage();
    std::cout << "\n  ERROR: Need two files for testing." << std::endl;
  }
  std::string smearing_file{argv[1]}, output_file{argv[2]};
  TFile output{argv[2], "RECREATE"};

  smearing_test(
      &output,
      smearing_file,
      "fixed",
      fixed_momentum_sampler
  );

  smearing_test(
      &output,
      smearing_file,
      "gaussian",
      normal_momentum,
      10000 /* num trials */
  );

  output.Write();
  output.Close();
} catch (const std::runtime_error& rte) {
  std::cerr << rte.what() << std::endl;
  return 127;
}

R__LOAD_LIBRARY(libevent);
R__LOAD_LIBRARY(libprocessing);
R__LOAD_LIBRARY(libprocessors);

int Hpstr(void){
  
  Process* p = new Process();
  Processor *ep;
  auto fact = ProcessorFactory::instance();
  ep  = fact.createProcessor("EventProcessor","header", *p);
  p->addToSequence(ep);
  ep = fact.createProcessor("SvtDataProcessor","svt", *p);
  p->addToSequence(ep);
  ep = fact.createProcessor("ECalDataProcessor","ecal", *p);
  p->addToSequence(ep);
  ep = fact.createProcessor("ParticleProcessor","particle", *p);
  p->addToSequence(ep);
  std::vector<string> colls;
  colls.push_back("FinalStateParticles");
  colls.push_back("BeamspotConstrainedMollerCandidates");
  colls.push_back("BeamspotConstrainedV0Candidates");
  colls.push_back("TargetConstrainedMollerCandidates");
  colls.push_back("TargetConstrainedV0Candidates");
  colls.push_back("UnconstrainedMollerCandidates");
  colls.push_back("UnconstrainedV0Candidates");
  ParameterSet params;
  params.insert("Collections",colls);
  ep->configure(params);
  p->addToSequence(ep);

  p->addFileToProcess("/data/HPS/data/physrun2016/slcio/hps_007983.evio.108.slcio");
  p->addOutputFileName("dst_r.root");
  p->run();
  return(1);
}
  

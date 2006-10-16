// -*- C++ -*-
//
// Package:    TauTagVal
// Class:      TauTagVal
// 
/**\class TauTagVal TauTagVal.cc RecoTauTag/ConeIsolation/test/TauTagVal.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Simone Gennai
//         Created:  Wed Apr 12 11:12:49 CEST 2006
// $Id: TauTagVal.cc,v 1.3 2006/10/10 13:28:08 gennai Exp $
//
//


// user include files
#include "Validation/RecoTau/interface/TauTagVal.h"
#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "Math/GenVector/VectorUtil.h"
#include "Math/GenVector/PxPyPzE4D.h"
using namespace edm;
using namespace std;
using namespace reco;


TauTagVal::TauTagVal(const edm::ParameterSet& iConfig)
{
  nEvent = 0;
  jetTagSrc = iConfig.getParameter<InputTag>("JetTagProd");
  outPutFile = iConfig.getParameter<string>("OutPutFile");
  rSig = iConfig.getParameter<double>("SignalCone");
  rMatch = iConfig.getParameter<double>("MatchingCone");
  rIso = iConfig.getParameter<double>("IsolationCone");
  ptLeadTk = iConfig.getParameter<double>("MinimumTransverseMomentumLeadingTrack");

  nEventsRiso.reserve(6);
  nEventsUsed.reserve(6);
  nEventsEnergy.reserve(6);
  nEventsEnergyUsed.reserve(6);
  for(int i=0;i<6;i++)
    {    
      nEventsRiso[i]=0;
      nEventsUsed[i]=0;
      nEventsEnergy[i]=0;
      nEventsEnergyUsed[i]=0;
    }

DaqMonitorBEInterface* dbe = &*edm::Service<DaqMonitorBEInterface>();
  if(dbe) {
    dbe->setCurrentFolder("TauJetTask_" + jetTagSrc.label());    
    ptLeadingTrack = dbe->book1D("PtLeadTk", "Pt LeadTk", 30, 0., 300.);
    ptJet  = dbe->book1D("PtJet", "Pt Jet", 30, 0., 300.);
    nSignalTracks = dbe->book1D("NSigTks", "NSigTks", 10, 0., 10.);
    nSignalTracksAfterIsolation = dbe->book1D("NSigTksAI", "NSigTksAI", 10, 0., 10.);
    nAssociatedTracks = dbe->book1D("NAssTks", "NAssTks", 10, 0., 10.);
    nSelectedTracks = dbe->book1D("NSelTks", "NSelTks", 10, 0., 10.);

    effVsRiso = dbe->book1D("EffIsol","EffIsol",6,0.2,0.5);
    EventseffVsRiso = dbe->book1D("EventsIsol","EvEffIsol",6,0.2,0.5);
    EventsToteffVsRiso = dbe->book1D("EventsTotIsol","EvTotEffIsol",6,0.2,0.5);
    effVsEt = dbe->book1D("EffVsEtJet","EffVsEtJet",6,0.,300.);
    EventseffVsEt = dbe->book1D("EventsEffVsEtJet","EvEffVsEtJet",6,0.,300.);
    EventsToteffVsEt = dbe->book1D("EventsTotEffVsEtJet","EvTotEffVsEtJet",6,0.,300.);
    effFindLeadTk =dbe->book1D("EffLeadTk","EffLeadTk",2,0.,2.);
   
    deltaRLeadTk_Jet = dbe->book1D("DeltaR_LT_Jet","DeltaR",20,0.,0.2);
    
  }

    
  if (outPutFile.empty ()) {
    LogInfo("OutputInfo") << " TauJet histograms will NOT be saved";
  } else {
    LogInfo("OutputInfo") << " TauJethistograms will be saved to file:" << outPutFile;
  }
  
}

void TauTagVal::beginJob(){ 
}

  


void TauTagVal::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace reco;
  
  Handle<IsolatedTauTagInfoCollection> tauTagInfoHandle;
  iEvent.getByLabel(jetTagSrc, tauTagInfoHandle);
  
  const IsolatedTauTagInfoCollection & tauTagInfo = *(tauTagInfoHandle.product());
  
  IsolatedTauTagInfoCollection::const_iterator i = tauTagInfo.begin();
  int it=0;
  for (; i != tauTagInfo.end(); ++i) {
    //Take only the first jet waiting for a jet selector
    if(it == 0) {
      for(int ii=0;ii<6;ii++)
	{
	  nEventsUsed[ii]++;
	  float Riso = ii*0.05 + 0.2;
	  float Etmin = ii*50.;
	  float Etmax  = Etmin+50;
	  float Rmatch = rMatch;
	  float Rsig = rSig;
	  float pT_LT = ptLeadTk;
	  float pT_min =1.;
	  if( i->discriminator(Rmatch,Rsig,Riso,pT_LT,pT_min) > 0) {
	    nEventsRiso[ii]++;
	  }
	  if(i->jet().pt() > Etmin && i->jet().pt()<Etmax)
	    {
	      nEventsEnergyUsed[ii]++;
	      if( i->discriminator(Rmatch,Rsig,rIso,pT_LT,pT_min) > 0) nEventsEnergy[ii]++;
	    }
	  if(!(i->leadingSignalTrack(rMatch, ptLeadTk)))
	    {
	      effFindLeadTk->Fill(0.);
	    }else{
	      effFindLeadTk->Fill(1.);
	    }
	  const TrackRef leadTkTmp= (i->leadingSignalTrack(0.5, 1.));
	  if(!leadTkTmp){
	  }else{
	    math::XYZVector momentum = (*leadTkTmp).momentum();
	    math::XYZVector jetMomentum(i->jet().px(), i->jet().py(), i->jet().pz());
	    float deltaR = ROOT::Math::VectorUtil::DeltaR(jetMomentum, momentum);
	    deltaRLeadTk_Jet->Fill(deltaR);
	  }
	  
	  const TrackRef leadTk= (i->leadingSignalTrack(rMatch, 1.));
	  if(!leadTk){
	    LogInfo("LeadingTrack") << " No LeadingTrack";
	  }else{
	    ptLeadingTrack->Fill((*leadTk).pt());
	    ptJet->Fill((i->jet()).pt());
	    math::XYZVector momentum = (*leadTk).momentum();
	    float nsigtks = (i->tracksInCone(momentum, rSig,  1.)).size();
	    nSignalTracks->Fill(nsigtks);
	    if(i->discriminator(rMatch,rSig,rIso,ptLeadTk,1.) == 1)
	      nSignalTracksAfterIsolation->Fill(nsigtks);
	  }
	  float allTracks = i->allTracks().size();
	  nAssociatedTracks->Fill(allTracks);
	  float selectedTracks = i->selectedTracks().size();
	  nSelectedTracks->Fill(selectedTracks);
	}	      
    }
  }
}

void TauTagVal::endJob(){
  int ibin;
  for(int ii=0; ii<6; ii++){
    if(nEventsUsed[ii] > 0.)
      {
	ibin= ii+1;
	float eff= nEventsRiso[ii]/nEventsUsed[ii];
	effVsRiso->setBinContent(ibin,eff);
	float nEvents = 1.*nEventsRiso[ii];
	float nEventsTot = 1.*nEventsUsed[ii];
	EventseffVsRiso->setBinContent(ibin, nEvents);
	EventsToteffVsRiso->setBinContent(ibin, nEventsTot);
      }
    if(nEventsEnergyUsed[ii] > 0.)
      {
	ibin= ii+1;
	float eff= nEventsEnergy[ii]/nEventsEnergyUsed[ii];
	effVsEt->setBinContent(ibin,eff);
	float nEvents = 1.*nEventsEnergy[ii];
	float nEventsTot = 1.*nEventsEnergyUsed[ii];
	EventseffVsEt->setBinContent(ibin,nEvents);
	EventsToteffVsEt->setBinContent(ibin,nEventsTot);
      }
  }
  
  if (!outPutFile.empty() && &*edm::Service<DaqMonitorBEInterface>()) edm::Service<DaqMonitorBEInterface>()->save (outPutFile);
  
}

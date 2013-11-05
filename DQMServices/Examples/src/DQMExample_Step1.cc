#include "DQMServices/Examples/interface/DQMExample_Step1.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

// Geometry
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "TLorentzVector.h"

// CORAL
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/IProperty.h"
#include "CoralKernel/IPropertyManager.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <sstream>
#include <math.h>
#include <vector>

//
// -------------------------------------- Constructor --------------------------------------------
//
DQMExample_Step1::DQMExample_Step1(const edm::ParameterSet& ps): m_connectionService(), m_session(), m_connectionString( "" )
{
  edm::LogInfo("DQMExample_Step1") <<  "Constructor  DQMExample_Step1::DQMExample_Step1 " << std::endl;
  
  // Get parameters from configuration file
  theElectronCollection_   = consumes<reco::GsfElectronCollection>(ps.getParameter<edm::InputTag>("electronCollection"));
  theCaloJetCollection_    = consumes<reco::CaloJetCollection>(ps.getParameter<edm::InputTag>("caloJetCollection"));
  thePfMETCollection_      = consumes<reco::PFMETCollection>(ps.getParameter<edm::InputTag>("pfMETCollection"));
  theConversionCollection_ = consumes<reco::ConversionCollection>(ps.getParameter<edm::InputTag>("conversionsCollection"));
  thePVCollection_         = consumes<reco::VertexCollection>(ps.getParameter<edm::InputTag>("PVCollection"));
  theBSCollection_         = consumes<reco::BeamSpot>(ps.getParameter<edm::InputTag>("beamSpotCollection"));
  triggerEvent_            = consumes<trigger::TriggerEvent>(ps.getParameter<edm::InputTag>("TriggerEvent"));
  triggerResults_          = consumes<edm::TriggerResults>(ps.getParameter<edm::InputTag>("TriggerResults"));
  triggerFilter_           = ps.getParameter<edm::InputTag>("TriggerFilter");
  triggerPath_             = ps.getParameter<std::string>("TriggerPath");


  // cuts:
  ptThrL1_ = ps.getUntrackedParameter<double>("PtThrL1");
  ptThrL2_ = ps.getUntrackedParameter<double>("PtThrL2");
  ptThrJet_ = ps.getUntrackedParameter<double>("PtThrJet");
  ptThrMet_ = ps.getUntrackedParameter<double>("PtThrMet");
 
  //DQMStore
  dbe_ = edm::Service<DQMStore>().operator->();
  
  //Database connection configuration parameters
  edm::ParameterSet connectionParameters = ps.getParameter<edm::ParameterSet>("DBParameters");
  std::string authPath = connectionParameters.getUntrackedParameter<std::string>("authPath", "");
  int messageLevel = connectionParameters.getUntrackedParameter<int>("messageLevel",0);
  coral::MsgLevel level = coral::Error;
  switch (messageLevel) {
  case 0 :
    level = coral::Error;
    break;    
  case 1 :
      level = coral::Warning;
    break;
  case 2 :
    level = coral::Info;
    break;
  case 3 :
    level = coral::Debug;
    break;
  default:
    level = coral::Error;
  }
  bool enableConnectionSharing = connectionParameters.getUntrackedParameter<bool>("enableConnectionSharing",true);
  int connectionTimeOut = connectionParameters.getUntrackedParameter<int>("connectionTimeOut",600);
  bool enableReadOnlySessionOnUpdateConnection = connectionParameters.getUntrackedParameter<bool>("enableReadOnlySessionOnUpdateConnection",true);
  int connectionRetrialPeriod = connectionParameters.getUntrackedParameter<int>("connectionRetrialPeriod",30);
  int connectionRetrialTimeOut = connectionParameters.getUntrackedParameter<int>("connectionRetrialTimeOut",180);
  bool enablePoolAutomaticCleanUp = connectionParameters.getUntrackedParameter<bool>("enablePoolAutomaticCleanUp",false);
  //connection string
  m_connectionString = ps.getParameter<std::string>("connect");
  //now configure the DB connection
  coral::IConnectionServiceConfiguration& coralConfig = m_connectionService.configuration();
  //TODO: set up the authentication mechanism
  
  // message streaming
  coral::MessageStream::setMsgVerbosity( level );
  //connection sharing
  if(enableConnectionSharing) coralConfig.enableConnectionSharing();
  else coralConfig.disableConnectionSharing();
  //connection timeout
  coralConfig.setConnectionTimeOut(connectionTimeOut);
  //read-only session on update connection
  if(enableReadOnlySessionOnUpdateConnection) coralConfig.enableReadOnlySessionOnUpdateConnections();
  else coralConfig.disableReadOnlySessionOnUpdateConnections();
  //connection retrial period
  coralConfig.setConnectionRetrialPeriod( connectionRetrialPeriod );
  //connection retrial timeout
  coralConfig.setConnectionRetrialTimeOut( connectionRetrialTimeOut );
  //pool automatic cleanup
  if(enablePoolAutomaticCleanUp) coralConfig.enablePoolAutomaticCleanUp();
  else coralConfig.disablePoolAutomaticCleanUp();
}

//
// -- Destructor
//
DQMExample_Step1::~DQMExample_Step1()
{
  edm::LogInfo("DQMExample_Step1") <<  "Destructor DQMExample_Step1::~DQMExample_Step1 " << std::endl;
}

//
// -------------------------------------- beginJob --------------------------------------------
//
void DQMExample_Step1::beginJob()
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::beginJob " << std::endl;
}
//
// -------------------------------------- beginRun --------------------------------------------
//
void DQMExample_Step1::beginRun(edm::Run const& run, edm::EventSetup const& eSetup) 
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::beginRun" << std::endl;
  
  //book at beginRun
  bookHistos(dbe_);
  
  //open the CORAL session at beginRun:
  //connect to DB only if you have events to process!
  m_session.reset( m_connectionService.connect( m_connectionString, coral::Update ) );
  //do not run in production!
  //create the relevant tables
  coral::ISchema& schema = m_session->nominalSchema();
  m_session->transaction().start( false );
  bool dqmTablesExist = schema.existsTable( "DQM_HISTOS" );
  if( ! dqmTablesExist ) {
    int columnSize = 200;
    coral::TableDescription descr;
    descr.setName( "DQM_HISTOS" );
    descr.insertColumn( "HISTO_NAME", coral::AttributeSpecification::typeNameForType<std::string>(), columnSize, false );
    descr.insertColumn( "RUN_NUMBER", coral::AttributeSpecification::typeNameForType<unsigned int>() );
    descr.insertColumn( "LUMISECTION", coral::AttributeSpecification::typeNameForType<unsigned int>() );
    descr.insertColumn( "X_BINS", coral::AttributeSpecification::typeNameForType<int>() );
    descr.insertColumn( "X_LOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_UP", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_BINS", coral::AttributeSpecification::typeNameForType<int>() );
    descr.insertColumn( "Y_LOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_UP", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_BINS", coral::AttributeSpecification::typeNameForType<int>() );
    descr.insertColumn( "Z_LOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_UP", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "ENTRIES", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_MEAN", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_MEAN_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_RMS", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_RMS_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_UNDERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "X_OVERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_MEAN", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_MEAN_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_RMS", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_RMS_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_UNDERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Y_OVERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_MEAN", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_MEAN_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_RMS", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_RMS_ERROR", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_UNDERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.insertColumn( "Z_OVERFLOW", coral::AttributeSpecification::typeNameForType<double>() );
    descr.setNotNullConstraint( "HISTO_NAME" );
    descr.setNotNullConstraint( "RUN_NUMBER" );
    descr.setNotNullConstraint( "LUMISECTION" );
    descr.setNotNullConstraint( "X_BINS" );
    descr.setNotNullConstraint( "X_LOW" );
    descr.setNotNullConstraint( "X_UP" );
    descr.setNotNullConstraint( "Y_BINS" );
    descr.setNotNullConstraint( "Y_LOW" );
    descr.setNotNullConstraint( "Y_UP" );
    descr.setNotNullConstraint( "Z_BINS" );
    descr.setNotNullConstraint( "Z_LOW" );
    descr.setNotNullConstraint( "Z_UP" );
    descr.setNotNullConstraint( "ENTRIES" );
    descr.setNotNullConstraint( "X_MEAN" );
    descr.setNotNullConstraint( "X_MEAN_ERROR" );
    descr.setNotNullConstraint( "X_RMS" );
    descr.setNotNullConstraint( "X_RMS_ERROR" );
    descr.setNotNullConstraint( "X_UNDERFLOW" );
    descr.setNotNullConstraint( "X_OVERFLOW" );
    descr.setNotNullConstraint( "Y_MEAN" );
    descr.setNotNullConstraint( "Y_MEAN_ERROR" );
    descr.setNotNullConstraint( "Y_RMS" );
    descr.setNotNullConstraint( "Y_RMS_ERROR" );
    descr.setNotNullConstraint( "Y_UNDERFLOW" );
    descr.setNotNullConstraint( "Y_OVERFLOW" );
    descr.setNotNullConstraint( "Z_MEAN" );
    descr.setNotNullConstraint( "Z_MEAN_ERROR" );
    descr.setNotNullConstraint( "Z_RMS" );
    descr.setNotNullConstraint( "Z_RMS_ERROR" );
    descr.setNotNullConstraint( "Z_UNDERFLOW" );
    descr.setNotNullConstraint( "Z_OVERFLOW" );
    std::vector<std::string> columnsForIndex;
    columnsForIndex.push_back( "HISTO_NAME" );
    columnsForIndex.push_back( "RUN_NUMBER" );
    columnsForIndex.push_back( "LUMISECTION" );
    descr.setPrimaryKey( columnsForIndex );
    schema.createTable( descr );
  }
  m_session->transaction().commit();
}
//
// -------------------------------------- beginLuminosityBlock --------------------------------------------
//
void DQMExample_Step1::beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, 
                                            edm::EventSetup const& context) 
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::beginLuminosityBlock" << std::endl;
}


//
// -------------------------------------- Analyze --------------------------------------------
//
void DQMExample_Step1::analyze(edm::Event const& e, edm::EventSetup const& eSetup)
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::analyze" << std::endl;


  //-------------------------------
  //--- Vertex Info
  //-------------------------------
  edm::Handle<reco::VertexCollection> vertexHandle;
  e.getByToken(thePVCollection_, vertexHandle);
  if ( !vertexHandle.isValid() ) 
    {
      edm::LogError ("DQMClientExample") << "invalid collection: vertex" << "\n";
      return;
    }
  
  int vertex_number = vertexHandle->size();
  reco::VertexCollection::const_iterator v = vertexHandle->begin();

  math::XYZPoint PVPoint(-999, -999, -999);
  if(vertex_number != 0)
    PVPoint = math::XYZPoint(v->position().x(), v->position().y(), v->position().z());
  
  PVPoint_=PVPoint;

  //-------------------------------
  //--- MET
  //-------------------------------
  edm::Handle<reco::PFMETCollection> pfMETCollection;
  e.getByToken(thePfMETCollection_, pfMETCollection);
  if ( !pfMETCollection.isValid() )    
    {
      edm::LogError ("DQMClientExample") << "invalid collection: MET" << "\n";
      return;
    }
  //-------------------------------
  //--- Electrons
  //-------------------------------
  edm::Handle<reco::GsfElectronCollection> electronCollection;
  e.getByToken(theElectronCollection_, electronCollection);
  if ( !electronCollection.isValid() )
    {
      edm::LogError ("DQMClientExample") << "invalid collection: electrons" << "\n";
      return;
    }

  float nEle=0;
  int posEle=0, negEle=0;
  const reco::GsfElectron* ele1 = NULL;
  const reco::GsfElectron* ele2 = NULL;
  for (reco::GsfElectronCollection::const_iterator recoElectron=electronCollection->begin(); recoElectron!=electronCollection->end(); ++recoElectron)
    {
      //decreasing pT
      if( MediumEle(e,eSetup,*recoElectron) )
	{
	  if(!ele1 && recoElectron->pt() > ptThrL1_)
	    ele1 = &(*recoElectron);
	  
	  else if(!ele2 && recoElectron->pt() > ptThrL2_)
	    ele2 = &(*recoElectron);

	}
      
      if(recoElectron->charge()==1)
	posEle++;
      else if(recoElectron->charge()==-1)
	negEle++;

    } // end of loop over electrons
  
  nEle = posEle+negEle;
  
  //-------------------------------
  //--- Jets
  //-------------------------------
  edm::Handle<reco::CaloJetCollection> caloJetCollection;
  e.getByToken (theCaloJetCollection_,caloJetCollection);
  if ( !caloJetCollection.isValid() ) 
    {
      edm::LogError ("DQMClientExample") << "invalid collection: jets" << "\n";
      return;
    }

  int   nJet = 0;
  const reco::CaloJet* jet1 = NULL;
  const reco::CaloJet* jet2 = NULL;
  
  for (reco::CaloJetCollection::const_iterator i_calojet = caloJetCollection->begin(); i_calojet != caloJetCollection->end(); ++i_calojet) 
    {
      //remove jet-ele matching
      if(ele1)
	if (Distance(*i_calojet,*ele1) < 0.3) continue;
      
      if(ele2)
	if (Distance(*i_calojet,*ele2) < 0.3) continue;
      
      if (i_calojet->pt() < ptThrJet_) continue;

      nJet++;
      
      if (!jet1) 
	jet1 = &(*i_calojet);
      
      else if (!jet2)
	jet2 = &(*i_calojet);
    }
  
  // ---------------------------
  // ---- Analyze Trigger Event
  // ---------------------------

  //check what is in the menu
  edm::Handle<edm::TriggerResults> hltresults;
  e.getByToken(triggerResults_,hltresults);
  
  if(!hltresults.isValid())
    {
      edm::LogError ("DQMClientExample") << "invalid collection: TriggerResults" << "\n";
      return;
    }
  
  bool hasFired = false;
  const edm::TriggerNames& trigNames = e.triggerNames(*hltresults);
  unsigned int numTriggers = trigNames.size();
  
  for( unsigned int hltIndex=0; hltIndex<numTriggers; ++hltIndex )
    {
      if (trigNames.triggerName(hltIndex)==triggerPath_ &&  hltresults->wasrun(hltIndex) &&  hltresults->accept(hltIndex))
	hasFired = true;
    }
  


  //access the trigger event
  edm::Handle<trigger::TriggerEvent> triggerEvent;
  e.getByToken(triggerEvent_, triggerEvent);
  if( triggerEvent.failedToGet() )
    {
      edm::LogError ("DQMClientExample") << "invalid collection: TriggerEvent" << "\n";
      return;
    }


  reco::Particle* ele1_HLT = NULL;
  int nEle_HLT = 0;

  size_t filterIndex = triggerEvent->filterIndex( triggerFilter_ );
  trigger::TriggerObjectCollection triggerObjects = triggerEvent->getObjects();
  if( !(filterIndex >= triggerEvent->sizeFilters()) )
    {
      const trigger::Keys& keys = triggerEvent->filterKeys( filterIndex );
      std::vector<reco::Particle> triggeredEle;
      
      for( size_t j = 0; j < keys.size(); ++j ) 
	{
	  trigger::TriggerObject foundObject = triggerObjects[keys[j]];
	  if( abs( foundObject.particle().pdgId() ) != 11 )  continue; //make sure that it is an electron
	  
	  triggeredEle.push_back( foundObject.particle() );
	  ++nEle_HLT;
	}
      
      if( triggeredEle.size() >= 1 ) 
	ele1_HLT = &(triggeredEle.at(0));
    }

  //-------------------------------
  //--- Fill the histos
  //-------------------------------

  //vertex
  h_vertex_number -> Fill( vertex_number );

  //met
  h_pfMet -> Fill( pfMETCollection->begin()->et() );

  //multiplicities
  h_eMultiplicity->Fill(nEle);       
  h_jMultiplicity->Fill(nJet);
  h_eMultiplicity_HLT->Fill(nEle_HLT);

  //leading not matched
  if(ele1)
    {
      h_ePt_leading->Fill(ele1->pt());
      h_eEta_leading->Fill(ele1->eta());
      h_ePhi_leading->Fill(ele1->phi());
    }
  if(ele1_HLT)
    {
      h_ePt_leading_HLT->Fill(ele1_HLT->pt());
      h_eEta_leading_HLT->Fill(ele1_HLT->eta());
      h_ePhi_leading_HLT->Fill(ele1_HLT->phi());
    }
  //leading Jet
  if(jet1)
    {
      h_jPt_leading->Fill(jet1->pt());
      h_jEta_leading->Fill(jet1->eta());
      h_jPhi_leading->Fill(jet1->phi());
    }


  //fill only when the trigger candidate mathes with the reco one
  if( ele1 && ele1_HLT && deltaR(*ele1_HLT,*ele1) < 0.3 && hasFired==true )
    {
      h_ePt_leading_matched->Fill(ele1->pt());
      h_eEta_leading_matched->Fill(ele1->eta());
      h_ePhi_leading_matched->Fill(ele1->phi());
      
      h_ePt_leading_HLT_matched->Fill(ele1_HLT->pt());
      h_eEta_leading_HLT_matched->Fill(ele1_HLT->eta());
      h_ePhi_leading_HLT_matched->Fill(ele1_HLT->phi());

      h_ePt_diff->Fill(ele1->pt()-ele1_HLT->pt());
    }
}
//
// -------------------------------------- endLuminosityBlock --------------------------------------------
//
void DQMExample_Step1::endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, edm::EventSetup const& eSetup) 
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::endLuminosityBlock" << std::endl;
  //get the data from the histograms and fill the DB table
  m_session->transaction().start(false);
  coral::ITableDataEditor& editor = m_session->nominalSchema().tableHandle( "DQM_HISTOS" ).dataEditor();
  coral::AttributeList insertData;
  insertData.extend< std::string >( "HISTO_NAME" );
  insertData.extend< unsigned int >( "RUN_NUMBER" );
  insertData.extend< unsigned int >( "LUMISECTION" );
  insertData.extend< int >( "X_BINS" );
  insertData.extend< double >( "X_LOW" );
  insertData.extend< double >( "X_UP" );
  insertData.extend< int >( "Y_BINS" );
  insertData.extend< double >( "Y_LOW" );
  insertData.extend< double >( "Y_UP" );
  insertData.extend< int >( "Z_BINS" );
  insertData.extend< double >( "Z_LOW" );
  insertData.extend< double >( "Z_UP" );
  insertData.extend< double >( "ENTRIES" );
  insertData.extend< double >( "X_MEAN" );
  insertData.extend< double >( "X_MEAN_ERROR" );
  insertData.extend< double >( "X_RMS" );
  insertData.extend< double >( "X_RMS_ERROR" );
  insertData.extend< double >( "X_UNDERFLOW");
  insertData.extend< double >( "X_OVERFLOW" );
  insertData.extend< double >( "Y_MEAN" );
  insertData.extend< double >( "Y_MEAN_ERROR" );
  insertData.extend< double >( "Y_RMS" );
  insertData.extend< double >( "Y_RMS_ERROR" );
  insertData.extend< double >( "Y_UNDERFLOW");
  insertData.extend< double >( "Y_OVERFLOW" );
  insertData.extend< double >( "Z_MEAN" );
  insertData.extend< double >( "Z_MEAN_ERROR" );
  insertData.extend< double >( "Z_RMS" );
  insertData.extend< double >( "Z_RMS_ERROR" );
  insertData.extend< double >( "Z_UNDERFLOW");
  insertData.extend< double >( "Z_OVERFLOW" );
  insertData[ "HISTO_NAME" ].data< std::string >() = h_vertex_number->getFullname();
  insertData[ "RUN_NUMBER" ].data< unsigned int >() = lumiSeg.run();
  insertData[ "LUMISECTION" ].data< unsigned int >() = lumiSeg.luminosityBlock();
  insertData[ "X_BINS" ].data< int >() = h_vertex_number->getNbinsX(); //or h_vertex_number->getTH1()->GetNbinsX() ?
  insertData[ "X_LOW" ].data< double >() = h_vertex_number->getTH1()->GetXaxis()->GetXmin();
  insertData[ "X_UP" ].data< double >() = h_vertex_number->getTH1()->GetXaxis()->GetXmax();
  //FIXME: should determine from the ME itself whether or not
  // the definitions of the 2nd and 3rd dimensions of the histograms are to be inserted!
  insertData[ "Y_BINS" ].data< int >() = 0; //h_vertex_number->getNbinsY();
  insertData[ "Y_LOW" ].data< double >() = 0.; //h_vertex_number->getTH1()->GetYaxis()->GetXMin();
  insertData[ "Y_UP" ].data< double >() = 0.; //h_vertex_number->getTH1()->GetYaxis()->GetXMax();
  insertData[ "Z_BINS" ].data< int >() = 0; //h_vertex_number->getNbinsZ();
  insertData[ "Z_LOW" ].data< double >() = 0.; //h_vertex_number->getTH1()->GetZaxis()->GetXMin();
  insertData[ "Z_UP" ].data< double >() = 0.; //h_vertex_number->getTH1()->GetZaxis()->GetXMax();
  insertData[ "ENTRIES" ].data< double >() = h_vertex_number->getEntries(); //or h_vertex_number->getTH1()->GetEntries() ?
  //FIXME: if we use MonitorElement::getMean{Error} or MonitorElement::getRMS{Error}
  // there is a check on the dimension of the TH1, which must be larger than the axis number - 1
  // i.e. 0 for x axis, 1 for y axis, 2 for z axis.
  // If we get the pointer to the TH1 object, we can avoid this check, and we are guaranteed that
  // TH1 will give 0 for non-existing dimensions
  // (indeed, in TH1::GetStats, the stats array is inizialized to 0 for elements between 4 and 10).
  insertData[ "X_MEAN" ].data< double >() = h_vertex_number->getTH1()->GetMean();
  insertData[ "X_MEAN_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetMeanError();
  insertData[ "X_RMS" ].data< double >() = h_vertex_number->getTH1()->GetRMS();
  insertData[ "X_RMS_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetRMSError();
  //FIXME: should determine from the ME itself whether or not the underflow and overflow bins are to be inserted.
  // Also, we should define what underflow and overflow mean in 2-D and 3-D histos.
  insertData[ "X_UNDERFLOW" ].data< double >() = h_vertex_number->getTH1()->GetBinContent( 0 );
  insertData[ "X_OVERFLOW" ].data< double >() = h_vertex_number->getTH1()->GetBinContent( h_vertex_number->getTH1()->GetNbinsX() + 1 );
  insertData[ "Y_MEAN" ].data< double >() = h_vertex_number->getTH1()->GetMean( 2 );
  insertData[ "Y_MEAN_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetMeanError( 2 );
  insertData[ "Y_RMS" ].data< double >() = h_vertex_number->getTH1()->GetRMS( 2 );
  insertData[ "Y_RMS_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetRMSError( 2 );
  insertData[ "Y_UNDERFLOW" ].data< double >() = 0.;
  insertData[ "Y_OVERFLOW" ].data< double >() = 0.;
  insertData[ "Z_MEAN" ].data< double >() = h_vertex_number->getTH1()->GetMean( 3 );
  insertData[ "Z_MEAN_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetMeanError( 3 );
  insertData[ "Z_RMS" ].data< double >() = h_vertex_number->getTH1()->GetRMS( 3 );
  insertData[ "Z_RMS_ERROR" ].data< double >() = h_vertex_number->getTH1()->GetRMSError( 3 );
  insertData[ "Z_UNDERFLOW" ].data< double >() = 0.;
  insertData[ "Z_OVERFLOW" ].data< double >() = 0.;
  editor.insertRow( insertData );
  m_session->transaction().commit();
}


//
// -------------------------------------- endRun --------------------------------------------
//
void DQMExample_Step1::endRun(edm::Run const& run, edm::EventSetup const& eSetup)
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::endRun" << std::endl;
  //no more data to process:
  //close DB session
  m_session.reset();
}

//
// -------------------------------------- endJob --------------------------------------------
//
void DQMExample_Step1::endJob()
{
  edm::LogInfo("DQMExample_Step1") <<  "DQMExample_Step1::endJob" << std::endl;
}


//
// -------------------------------------- book histograms --------------------------------------------
//
void DQMExample_Step1::bookHistos(DQMStore* dbe)
{
  dbe->cd();
  dbe->setCurrentFolder("Physics/TopTest");

  h_vertex_number = dbe->book1D("Vertex_number", "Number of event vertices in collection", 40,-0.5,   39.5 );

  h_pfMet        = dbe->book1D("pfMet",        "Pf Missing E_{T}; GeV"          , 20,  0.0 , 100);

  h_eMultiplicity = dbe_->book1D("NElectrons","# of electrons per event",10,0.,10.);
  h_ePt_leading_matched = dbe_->book1D("ElePt_leading_matched","Pt of leading electron",50,0.,100.);
  h_eEta_leading_matched = dbe_->book1D("EleEta_leading_matched","Eta of leading electron",50,-5.,5.);
  h_ePhi_leading_matched = dbe_->book1D("ElePhi_leading_matched","Phi of leading electron",50,-3.5,3.5);

  h_ePt_leading = dbe_->book1D("ElePt_leading","Pt of leading electron",50,0.,100.);
  h_eEta_leading = dbe_->book1D("EleEta_leading","Eta of leading electron",50,-5.,5.);
  h_ePhi_leading = dbe_->book1D("ElePhi_leading","Phi of leading electron",50,-3.5,3.5);

  h_jMultiplicity = dbe_->book1D("NJets","# of electrons per event",10,0.,10.);
  h_jPt_leading = dbe_->book1D("JetPt_leading","Pt of leading Jet",150,0.,300.);
  h_jEta_leading = dbe_->book1D("JetEta_leading","Eta of leading Jet",50,-5.,5.);
  h_jPhi_leading = dbe_->book1D("JetPhi_leading","Phi of leading Jet",50,-3.5,3.5);

  h_eMultiplicity_HLT = dbe_->book1D("NElectrons_HLT","# of electrons per event @HLT",10,0.,10.);
  h_ePt_leading_HLT = dbe_->book1D("ElePt_leading_HLT","Pt of leading electron @HLT",50,0.,100.);
  h_eEta_leading_HLT = dbe_->book1D("EleEta_leading_HLT","Eta of leading electron @HLT",50,-5.,5.);
  h_ePhi_leading_HLT = dbe_->book1D("ElePhi_leading_HLT","Phi of leading electron @HLT",50,-3.5,3.5);

  h_ePt_leading_HLT_matched = dbe_->book1D("ElePt_leading_HLT_matched","Pt of leading electron @HLT",50,0.,100.);
  h_eEta_leading_HLT_matched = dbe_->book1D("EleEta_leading_HLT_matched","Eta of leading electron @HLT",50,-5.,5.);
  h_ePhi_leading_HLT_matched = dbe_->book1D("ElePhi_leading_HLT_matched","Phi of leading electron @HLT",50,-3.5,3.5);

  h_ePt_diff = dbe_->book1D("ElePt_diff_matched","pT(RECO) - pT(HLT) for mathed candidates",100,-10,10.);

  dbe->cd();  

}


//
// -------------------------------------- functions --------------------------------------------
//
double DQMExample_Step1::Distance( const reco::Candidate & c1, const reco::Candidate & c2 ) {
        return  deltaR(c1,c2);
}

double DQMExample_Step1::DistancePhi( const reco::Candidate & c1, const reco::Candidate & c2 ) {
        return  deltaPhi(c1.p4().phi(),c2.p4().phi());
}

// This always returns only a positive deltaPhi
double DQMExample_Step1::calcDeltaPhi(double phi1, double phi2) {
  double deltaPhi = phi1 - phi2;
  if (deltaPhi < 0) deltaPhi = -deltaPhi;
  if (deltaPhi > 3.1415926) {
    deltaPhi = 2 * 3.1415926 - deltaPhi;
  }
  return deltaPhi;
}

//
// -------------------------------------- electronID --------------------------------------------
//
bool DQMExample_Step1::MediumEle (const edm::Event & iEvent, const edm::EventSetup & iESetup, const reco::GsfElectron & electron)
{
    
  //********* CONVERSION TOOLS
  edm::Handle<reco::ConversionCollection> conversions_h;
  iEvent.getByToken(theConversionCollection_, conversions_h);
  
  bool isMediumEle = false; 
  
  float pt = electron.pt();
  float eta = electron.eta();
    
  int isEB            = electron.isEB();
  float sigmaIetaIeta = electron.sigmaIetaIeta();
  float DetaIn        = electron.deltaEtaSuperClusterTrackAtVtx();
  float DphiIn        = electron.deltaPhiSuperClusterTrackAtVtx();
  float HOverE        = electron.hadronicOverEm();
  float ooemoop       = (1.0/electron.ecalEnergy() - electron.eSuperClusterOverP()/electron.ecalEnergy());
  
  int mishits             = electron.gsfTrack()->trackerExpectedHitsInner().numberOfHits();
  int nAmbiguousGsfTracks = electron.ambiguousGsfTracksSize();
  
  reco::GsfTrackRef eleTrack  = electron.gsfTrack() ;
  float dxy           = eleTrack->dxy(PVPoint_);  
  float dz            = eleTrack->dz (PVPoint_);
  
  edm::Handle<reco::BeamSpot> BSHandle;
  iEvent.getByToken(theBSCollection_, BSHandle);
  const reco::BeamSpot BS = *BSHandle;
  
  bool isConverted = ConversionTools::hasMatchedConversion(electron, conversions_h, BS.position());
  
  // default
  if(  (pt > 12.) && (fabs(eta) < 2.5) &&
       ( ( (isEB == 1) && (fabs(DetaIn)  < 0.004) ) || ( (isEB == 0) && (fabs(DetaIn)  < 0.007) ) ) &&
       ( ( (isEB == 1) && (fabs(DphiIn)  < 0.060) ) || ( (isEB == 0) && (fabs(DphiIn)  < 0.030) ) ) &&
       ( ( (isEB == 1) && (sigmaIetaIeta < 0.010) ) || ( (isEB == 0) && (sigmaIetaIeta < 0.030) ) ) &&
       ( ( (isEB == 1) && (HOverE        < 0.120) ) || ( (isEB == 0) && (HOverE        < 0.100) ) ) &&
       ( ( (isEB == 1) && (fabs(ooemoop) < 0.050) ) || ( (isEB == 0) && (fabs(ooemoop) < 0.050) ) ) &&
       ( ( (isEB == 1) && (fabs(dxy)     < 0.020) ) || ( (isEB == 0) && (fabs(dxy)     < 0.020) ) ) &&
       ( ( (isEB == 1) && (fabs(dz)      < 0.100) ) || ( (isEB == 0) && (fabs(dz)      < 0.100) ) ) &&
       ( ( (isEB == 1) && (!isConverted) ) || ( (isEB == 0) && (!isConverted) ) ) &&
       ( mishits == 0 ) &&
       ( nAmbiguousGsfTracks == 0 )      
       )
    isMediumEle=true;
  
  return isMediumEle;
}

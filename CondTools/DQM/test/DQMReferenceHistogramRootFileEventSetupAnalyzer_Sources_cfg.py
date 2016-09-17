import FWCore.ParameterSet.Config as cms

process = cms.Process( 'ROOTFILERETRIEVER' )

process.MessageLogger = cms.Service( "MessageLogger",
                                     destinations = cms.untracked.vstring( 'cout' ),
                                     cout = cms.untracked.PSet( threshold = cms.untracked.string( 'INFO' ) ),
                                     )

# import of standard configurations

# Put reference histograms into the EventSetup
process.load( 'CondTools.DQM.DQMReferenceHistogramRootFileEventSetupAnalyzer_Sources_cff' )

# Load DQM store
# and retrieve reference histograms from EventSetup
process.load( 'CondTools.DQM.DQMReferenceHistogramRootFileEventSetupAnalyzer_cfi' )
process.DQMStore.verbose   = cms.untracked.int32( 1 )
process.DQMStore.verboseQT = cms.untracked.int32( 0 )

# Analyzing the content of the Event Setup
process.load( 'CondTools.DQM.DQMReferenceHistogramRootFileEventSetupAnalyzer_DataGetter_cff' )

# Path definition
process.dqmReferenceHistoPath = cms.Path( process.dqmRefHistoRootFileGetter + process.recordDataGetter )

# Schedule definition
process.schedule = cms.Schedule( process.dqmReferenceHistoPath
                               , process.esout
                                 )

for name, module in process.es_sources_().iteritems():
    print "ESModules> provider:%s '%s'" % ( name, module.type_() )
for name, module in process.es_producers_().iteritems():
    print "ESModules> provider:%s '%s'" % ( name, module.type_() )

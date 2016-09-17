import time
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.AlCa.autoCond import autoCond

options = VarParsing.VarParsing()
options.register('globalTag',
                 autoCond['run2_data'], #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "GlobalTag")
options.register('runNumber',
                 4294967292, #default value, int limit -3
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Run number; default gives latest IOV")
options.register('eventsPerLumi',
                 3, #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "number of events per lumi")
options.register('numberOfLumis',
                 3, #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "number of lumisections per run")
options.register('numberOfRuns',
                 3, #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "number of runs in the job")
options.register('messageLevel',
                 0, #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Message level; default to 0")
options.parseArguments()

process = cms.Process( 'ROOTFILERETRIEVER' )

process.MessageLogger = cms.Service( "MessageLogger",
                                     destinations = cms.untracked.vstring( 'cout' ),
                                     cout = cms.untracked.PSet( threshold = cms.untracked.string( 'INFO' ) ),
                                     )

# setting the Global Tag
process.load( "Configuration.StandardSequences.FrontierConditions_GlobalTag_cff" )
process.GlobalTag.globaltag = cms.string( options.globalTag ) 

# Load standard modules
process.load( "Configuration.StandardSequences.GeometryRecoDB_cff" )
process.load( "Configuration.StandardSequences.MagneticField_cff" )

# Load DQM store
# and retrieve reference histograms from EventSetup
process.load( 'CondTools.DQM.DQMReferenceHistogramRootFileEventSetupAnalyzer_cfi' )
process.DQMStore.verbose   = cms.untracked.int32( options.messageLevel )
process.DQMStore.verboseQT = cms.untracked.int32( 0 )

# Input source
process.source = cms.Source( "EmptySource",
                             firstRun = cms.untracked.uint32( options.runNumber ),
                             firstTime = cms.untracked.uint64( ( long( time.time() ) - 24 * 3600 ) << 32 ), #24 hours ago in nanoseconds
                             numberEventsInRun = cms.untracked.uint32( options.eventsPerLumi *  options.numberOfLumis ), # options.numberOfLumis lumi sections per run
                             numberEventsInLuminosityBlock = cms.untracked.uint32( options.eventsPerLumi )
                             )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( options.eventsPerLumi *  options.numberOfLumis * options.numberOfRuns ) ) #options.numberOfRuns runs per job

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

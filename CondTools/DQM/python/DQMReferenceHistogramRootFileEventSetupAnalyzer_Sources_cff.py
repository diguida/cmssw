import time
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing()
options.register('connectionString',
                 'sqlite_file:DQMReferenceHistogram_PopCon_test.db', #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Connection string")
options.register('tag',
                 'DQMReferenceHistogram_PopCon_test', #default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Tag")
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

from CondCore.CondDB.CondDB_cfi import *
CondDBReference = CondDB.clone( connect = cms.string( options.connectionString ) )
CondDBReference.DBParameters.messageLevel = cms.untracked.int32( options.messageLevel )

#EventSetup source
ReferenceRetrieval = cms.ESSource( "PoolDBESSource"
                                 , CondDBReference
                                 , toGet = cms.VPSet( cms.PSet( record = cms.string( 'DQMReferenceHistogramRootFileRcd' )
                                                              , tag = cms.string( options.tag )
                                                                )
                                                      )
                                   )

# Input source
source = cms.Source( "EmptySource",
                     firstRun = cms.untracked.uint32( options.runNumber ),
                     firstTime = cms.untracked.uint64( ( long( time.time() ) - 24 * 3600 ) << 32 ), #24 hours ago in nanoseconds
                     numberEventsInRun = cms.untracked.uint32( options.eventsPerLumi *  options.numberOfLumis ), # options.numberOfLumis lumi sections per run
                     numberEventsInLuminosityBlock = cms.untracked.uint32( options.eventsPerLumi )
                     )
maxEvents = cms.untracked.PSet( input = cms.untracked.int32( options.eventsPerLumi *  options.numberOfLumis * options.numberOfRuns ) ) #options.numberOfRuns runs per job

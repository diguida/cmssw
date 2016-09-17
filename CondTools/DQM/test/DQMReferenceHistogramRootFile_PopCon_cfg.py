import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from CondCore.CondDB.CondDB_cfi import *

options = VarParsing.VarParsing()
options.register( 'runNumber'
                , 1 #default value
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.int
                , "Run number to be uploaded."
                  )
options.register( 'destinationConnection'
                , 'sqlite_file:DQMReferenceHistogram_PopCon_test.db' #default value
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.string
                , "Connection string to the DB where payloads will be possibly written."
                  )
options.register( 'targetConnection'
                , '' #default value
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.string
                , """Connection string to the target DB:
                     if not empty (default), this provides the latest IOV and payloads to compare;
                     it is the DB where payloads should be finally uploaded."""
                  )
options.register( 'tag'
                , 'DQMReferenceHistogram_PopCon_test'
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.string
                , "Tag written in destinationConnection and finally appended in targetConnection."
                  )
options.register( 'messageLevel'
                , 0 #default value
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.int
                , "Message level; default to 0"
                  )
options.register( 'dqmFile'
                , 'DQMReference.root' #default value
                , VarParsing.VarParsing.multiplicity.singleton
                , VarParsing.VarParsing.varType.string
                , "DQM reference histogram ROOT file to be stored."
                 )
options.parseArguments()

CondDBConnection = CondDB.clone( connect = cms.string( options.destinationConnection ) )
CondDBConnection.DBParameters.messageLevel = cms.untracked.int32( options.messageLevel )

process = cms.Process( "DQMReferenceHistogramRootFileDBWriter" )

process.MessageLogger = cms.Service( "MessageLogger"
                                   , destinations = cms.untracked.vstring( "cout" )
                                   , cout = cms.untracked.PSet( threshold = cms.untracked.string( 'INFO' ) )
                                     )

if options.messageLevel == 3:
    #enable LogDebug output: remember the USER_CXXFLAGS="-DEDM_ML_DEBUG" compilation flag!
    process.MessageLogger.cout = cms.untracked.PSet( threshold = cms.untracked.string( 'DEBUG' ) )
    process.MessageLogger.debugModules = cms.untracked.vstring( '*' )

process.source = cms.Source( "EmptyIOVSource"
                           , timetype = cms.string( 'runnumber' )
                           , firstValue = cms.uint64( options.runNumber )
                           , lastValue = cms.uint64( options.runNumber )
                           , interval = cms.uint64( 1 )
                             )

process.PoolDBOutputService = cms.Service( "PoolDBOutputService"
                                         , CondDBConnection
                                         , timetype = cms.untracked.string( 'runnumber' )
                                         , toPut = cms.VPSet( cms.PSet( record = cms.string( 'FileBlob' )
                                                                      , tag = cms.string( options.tag ) 
                                                                        )
                                                              )
                                           )

process.popConDQMReferenceHistogramRootFile = cms.EDAnalyzer( "DQMReferenceHistogramRootFilePopConAnalyzer"
                                                            , SinceAppendMode = cms.bool( True )
                                                            , record = cms.string( 'FileBlob' )
                                                            , Source = cms.PSet( ROOTFile = cms.untracked.string( options.dqmFile )
                                                                               , firstSince = cms.untracked.uint64( options.runNumber )
                                                                               , debug = cms.untracked.bool( bool( options.messageLevel ) )
                                                                                 )
                                                            , loggingOn = cms.untracked.bool( True )
                                                            , targetDBConnectionString = cms.untracked.string( options.targetConnection )
                                                              )

process.p = cms.Path( process.popConDQMReferenceHistogramRootFile )

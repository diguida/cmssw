import FWCore.ParameterSet.Config as cms

recordDataGetter = cms.EDAnalyzer( "EventSetupRecordDataGetter",
                                   toGet =  cms.VPSet(),
                                   verbose = cms.untracked.bool( True )
                                   )
escontent = cms.EDAnalyzer( "PrintEventSetupContent",
                            compact = cms.untracked.bool( True ),
                            printProviders = cms.untracked.bool( True )
                            )
esretrieval = cms.EDAnalyzer( "PrintEventSetupDataRetrieval",
                              printProviders = cms.untracked.bool( True )
                              )

esout = cms.EndPath( escontent + esretrieval )

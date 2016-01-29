#This is the default configuration for the connection to the frontier servlets
#in order to fetch the condition payloads in CMSSW.
import socket
import FWCore.ParameterSet.Config as cms
from CondCore.CondDB.CondDB_cfi import *

CondDBConnection = CondDB.clone( connect = cms.string( 'frontier://FrontierProd/CMS_CONDITIONS' ) )
if socket.getfqdn().find('.cms') != -1:
    CondDBConnection.connect = cms.string('frontier://(proxyurl=http://localhost:3128)(serverurl=http://localhost:8000/FrontierOnProd)(serverurl=http://localhost:8000/FrontierOnProd)(retrieve-ziplevel=0)(failovertoserver=no)/CMS_CONDITIONS')
    print '# Conditions read from  CMS_CONDITIONS  via squid running in localhost and pointig to FrontierOnProd '
else:
    print '# Conditions read from  CMS_CONDITIONS  via FrontierProd '
GlobalTag = cms.ESSource( "PoolDBESSource",
                          CondDBConnection,
                          globaltag        = cms.string( '' ),
                          snapshotTime     = cms.string( '' ),
                          toGet            = cms.VPSet(),   # hook to override or add single payloads
                          DumpStat         = cms.untracked.bool( False ),
                          ReconnectEachRun = cms.untracked.bool( False ),
                          RefreshAlways    = cms.untracked.bool( False ),
                          RefreshEachRun   = cms.untracked.bool( False ),
                          RefreshOpenIOVs  = cms.untracked.bool( False ),
                          pfnPostfix       = cms.untracked.string( '' ),
                          pfnPrefix        = cms.untracked.string( '' ),
                          )

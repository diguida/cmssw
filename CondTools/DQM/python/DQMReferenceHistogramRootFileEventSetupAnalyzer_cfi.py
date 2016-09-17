import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMStore_cfg import *

dqmRefHistoRootFileGetter = cms.EDAnalyzer( "DQMReferenceHistogramRootFileEventSetupAnalyzer" ) 

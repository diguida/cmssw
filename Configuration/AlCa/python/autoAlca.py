AlCaRecoMatrix = { 'cosmics'     : { 'AlCaLumiPixels'      : 'LumiPixels',
                                     'Cosmics'             : 'TkAlCosmics0T+HcalCalHOCosmics+DtCalibCosmics+MuAlGlobalCosmics',
                                     'ExpressCosmics'      : 'SiStripPCLHistos+SiStripCalZeroBias+TkAlCosmics0T+DtCalibCosmics',
                                     'HcalHPDNoise'        : '',
                                     'HcalNZS'             : 'HcalCalMinBias+HcalCalIterativePhiSym',
                                     'MinimumBias'         : 'SiStripCalZeroBias+TkAlMinBias',
                                     'NoBPTX'              : '',
                                     },
                   'pp'          : { 'AlCaLumiPixels'      : 'LumiPixels',
                                     'Commissioning'       : 'HcalCalIsoTrk',
                                     'DoubleElectron'      : 'EcalCalZElectron+EcalUncalZElectron',
                                     'DoubleMu'            : 'TkAlZMuMu+DtCalib+MuAlCalIsolatedMu+MuAlOverlaps',
                                     'DoubleMuParked'      : 'TkAlZMuMu+DtCalib+MuAlCalIsolatedMu+MuAlOverlaps',
                                     'HcalNZS'             : 'HcalCalMinBias',
                                     'MinimumBias'         : 'SiStripCalMinBias+TkAlMinBias',
                                     'MuOnia'              : 'TkAlJpsiMuMu+TkAlUpsilonMuMu',
                                     'MuOniaParked'        : 'TkAlJpsiMuMu+TkAlUpsilonMuMu',
                                     'SingleElectron'      : 'EcalCalWElectron+EcalUncalWElectron',
                                     'SingleMu'            : 'TkAlMuonIsolated+DtCalib+MuAlCalIsolatedMu+MuAlOverlaps',
                                     'StreamExpress'       : 'SiStripPCLHistos+SiStripCalMinBias+SiStripCalZeroBias+TkAlMinBias+DtCalib+MuAlCalIsolatedMu',
                                     'TestEnablesTracker'  : 'TkAlLAS'
                                     },
                   'cosmicsRun2' : { 'AlCaLumiPixels'      : 'LumiPixels',
                                     'Cosmics'             : 'TkAlCosmics0T+HcalCalHOCosmics+DtCalibCosmics+MuAlGlobalCosmics',
                                     'ExpressCosmics'      : 'SiStripPCLHistos+SiStripCalZeroBias+TkAlCosmics0T+DtCalibCosmics',
                                     'HcalHPDNoise'        : '',
                                     'HcalNZS'             : 'HcalCalMinBias+HcalCalIterativePhiSym',
                                     'MinimumBias'         : 'SiStripCalZeroBias+TkAlMinBias',
                                     'NoBPTX'              : '',
                                     },
                   'ppRun2'      : { 'AlCaLumiPixels'      : 'LumiPixels',
                                     'Commissioning'       : 'HcalCalIsoTrk',
                                     'Charmonium'          : 'TkAlJpsiMuMu',
                                     'DoubleEG'            : 'EcalCalZElectron+EcalUncalZElectron+HcalCalIterativePhiSym',
                                     'DoubleMu'            : 'TkAlZMuMu+MuAlCalIsolatedMu+MuAlOverlaps+MuAlZMuMu',
                                     'HcalNZS'             : 'HcalCalMinBias',
                                     'HLTPhysics'          : 'SiStripCalMinBias+TkAlMinBias',
                                     'JetHT'               : 'HcalCalDijets',
                                     'MET'                 : 'HcalCalNoise',
                                     'MuOnia'              : 'TkAlUpsilonMuMu',
                                     'SingleElectron'      : 'EcalCalWElectron+EcalUncalWElectron+EcalCalZElectron+EcalUncalZElectron+HcalCalIterativePhiSym',
                                     'SingleMu'            : 'TkAlMuonIsolated+HcalCalIterativePhiSym+DtCalib+MuAlCalIsolatedMu+MuAlOverlaps+MuAlZMuMu',
                                     'SinglePhoton'        : 'HcalCalGammaJet',
                                     'StreamExpress'       : 'SiStripPCLHistos+SiStripCalZeroBias+SiStripCalMinBias+TkAlMinBias+DtCalib',
                                     'TestEnablesEcalHcal' : 'HcalCalPedestals',
                                     'ZeroBias'            : 'SiStripCalZeroBias+TkAlMinBias+LumiPixelsMinBias',
                                     },
                 }

def buildList(pdList, matrix):
    """Takes a list of primary datasets (PDs) and the AlCaRecoMatrix (a dictionary) and returns a string with all the AlCaRecos for the selected PDs separated by the '+' character without duplicates."""
    alCaRecoList = []
    for pd in pdList:
        alCaRecoList.extend(matrix[pd].split("+"))
    # remove duplicates converting to a set
    alCaRecoList = set(alCaRecoList)
    stringList = ''
    for alCaReco in alCaRecoList:
        if stringList == '':
            stringList += alCaReco
        else:
            stringList += '+'+alCaReco
    return stringList

# Update the lists anytime a new PD is added to the matrix
autoAlca = { 'allForExpress'            : buildList( [ 'StreamExpress' ], AlCaRecoMatrix[ 'pp' ] ),
             'allForExpressCosmics'     : buildList( [ 'ExpressCosmics' ], AlCaRecoMatrix[ 'cosmics' ] ),
             'allForPrompt'             : buildList( [ 'MinimumBias', 'Commissioning', 'SingleMu', 'DoubleMu', 'MuOnia', 'DoubleMuParked', 'MuOniaParked', 'SingleElectron', 'DoubleElectron', 'HcalNZS' ], AlCaRecoMatrix[ 'pp' ] ),
             'allForPromptCosmics'      : buildList( [ 'Cosmics' ], AlCaRecoMatrix[ 'cosmics' ] ),
             'allForExpressRun2'        : buildList( [ 'StreamExpress' ], AlCaRecoMatrix[ 'ppRun2' ] ),
             'allForExpressCosmicsRun2' : buildList( [ 'ExpressCosmics' ], AlCaRecoMatrix[ 'cosmicsRun2' ] ),
             'allForPromptRun2'         : buildList( [ 'Commissioning', 'SingleMu', 'DoubleMu', 'MuOnia', 'Charmonium', 'MET', 'JetHT', 'SingleElectron', 'SinglePhoton', 'DoubleEG', 'ZeroBias' ,'HLTPhysics', 'HcalNZS' ], AlCaRecoMatrix[ 'ppRun2' ] ),
             'allForPromptCosmicsRun2'  : buildList( [ 'Cosmics' ], AlCaRecoMatrix[ 'cosmicsRun2' ] ),
             }
autoAlca.update(AlCaRecoMatrix)

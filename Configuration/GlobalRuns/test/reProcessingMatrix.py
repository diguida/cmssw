import optparse
import os
from Configuration.AlCa.autoAlca import alCaRecoMatrix

usage= "--list"
parser = optparse.OptionParser( usage )
parser.add_option( "--GT" )
parser.add_option( "--TLR", default = "--customise Configuration/DataProcessing/RecoTLR" )
parser.add_option( "--options", default = "" )
parser.add_option( "--output", default = "RECO,AOD,DQM" )
parser.add_option("--rel", default = "74X" )

( options, args ) = parser.parse_args()

com = 'cmsDriver.py reco -s RAW2DIGI,L1Reco,RECO%s,DQM%s  --data --magField AutoFromDBCurrent --scenario %s --datatier %s --eventcontent %s %s%s --no_exec --python_filename=rereco_%s%s.py --conditions %s '+options.options

#collision configuration without Alca
pp_scenario = 'pp'
pp_recoSpec = ''
pp_customise = '.customisePPData'

#os.system(
print com %( pp_recoSpec,
                  '',
                  pp_scenario,
                  options.output,
                  options.output,
                  options.TLR,
                  pp_customise,
                  '',
                  pp_scenario,
                  options.GT
                  )
#           )

#cosmic configuration without Alca
cosmic_scenario = 'cosmics'
cosmic_recoSpec = ''
cosmic_customise = '.customiseCosmicData'

#os.system(
print com %( cosmic_recoSpec,
                  '',
                  cosmic_scenario,
                  options.output,
                  options.output,
                  options.TLR,
                  cosmic_customise,
                  '',
                  cosmic_scenario,
                  options.GT
                  )
#           )

#collision and cosmic configurations with AlCa
for scenario, matrix in alCaRecoMatrix.items():
    if scenario == 'pp':
        recoSpec = pp_recoSpec
        customise = pp_customise
        output = options.output
    elif scenario == 'cosmics':
        recoSpec = cosmic_recoSpec
        customise = cosmic_customise
        output = options.output
    elif scenario == 'hcalnzs':
        recoSpec = ':reconstruction_HcalNZS'
        customise = pp_customise
        output = "RECO,DQM"
    else:
        print "Scenario \"%s\" not supported." %( scenario, )
        continue
    for PD in matrix:
        #os.system(
        print com %( recoSpec,
                          ',ALCA:'+'+'.join( matrix[ PD ] ) if matrix[ PD ] else '',
                          scenario,
                          output,
                          output,
                          options.TLR,
                          customise,
                          PD+'_',
                          scenario,
                          options.GT
                          )
#                   )

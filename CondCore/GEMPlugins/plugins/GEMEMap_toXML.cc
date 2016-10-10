

#include "CondCore/Utilities/interface/PayloadToXML.h"
#include "CondCore/Utilities/src/CondFormats.h"

namespace { // Avoid cluttering the global namespace.

  // converter methods
  std::string GEMEMap2xml( std::string const &payloadData, std::string const &payloadType ) {
    return cond::convertToXML<GEMEMap> (payloadData, payloadType);
  }

}


BOOST_PYTHON_MODULE( pluginGEMEMap_toXML )
{
    using namespace boost::python;
    def ("GEMEMap2xml"   , &GEMEMap2xml);

}

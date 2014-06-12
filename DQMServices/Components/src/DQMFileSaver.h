#ifndef DQMSERVICES_COMPONENTS_DQMFILESAVER_H
#define DQMSERVICES_COMPONENTS_DQMFILESAVER_H

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include <sys/time.h>
#include <string>

class DQMStore;
class DQMFileSaver : public edm::EDAnalyzer
{
public:
  DQMFileSaver(const edm::ParameterSet &ps);

protected:
  virtual void beginJob(void);
  virtual void beginRun(const edm::Run &, const edm::EventSetup &);
  virtual void beginLuminosityBlock(const edm::LuminosityBlock &, const edm::EventSetup &);
  virtual void analyze(const edm::Event &e, const edm::EventSetup &);
  virtual void endLuminosityBlock(const edm::LuminosityBlock &, const edm::EventSetup &);
  virtual void endRun(const edm::Run &, const edm::EventSetup &);
  virtual void endJob(void);
  virtual void postForkReacquireResources(unsigned int childIndex, unsigned int numberOfChildren);

public:
  enum Convention
  {
    Online,
    Offline,
    FilterUnit
  };

  enum FileFormat
  {
    ROOT,
    PB
  };

private:
  void saveForOfflinePB(const std::string &workflow, int run);
  void saveForOffline(const std::string &workflow, int run, int lumi);
  void saveForOnlinePB(const std::string &suffix);
  void saveForOnline(const std::string &suffix, const std::string &rewrite);
  void saveForFilterUnitPB(int run, int lumi);
  void saveForFilterUnit(const std::string& rewrite, int run, int lumi);
  void saveJobReport(const std::string &filename);
  void saveJson(const std::string& jsonFileName, const std::string& dataFileName);

  Convention	convention_;
  FileFormat    fileFormat_;
  std::string	workflow_;
  std::string	producer_;
  std::string	dirName_;
  std::string   child_;
  std::string	filterName_;
  int        	version_;
  bool		runIsComplete_;
  bool          enableMultiThread_;

  int		saveByLumiSection_;
  int		saveByEvent_;
  int		saveByMinute_;
  int		saveByTime_;
  int		saveByRun_;
  bool		saveAtJobEnd_;
  int		saveReference_;
  int		saveReferenceQMin_;
  int		forceRunNumber_;

  std::string	fileBaseName_;
  std::string	fileUpdate_;

  DQMStore	*dbe_;

  int		irun_;
  int		ilumi_;
  int		ilumiprev_;
  int		ievent_;
  int		nrun_;
  int		nlumi_;
  int		nevent_;
  timeval	start_;
  timeval	saved_;

  int			 numKeepSavedFiles_;
  std::list<std::string> pastSavedFiles_;

  
};

#endif // DQMSERVICES_COMPONEntS_DQMFILESAVER_H

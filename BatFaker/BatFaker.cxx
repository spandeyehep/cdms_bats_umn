#include "FakePulseBuilder.h"
#include "CommandLineHelper.h"

#include <string>

int main(int argc, char** argv)
{
  //evaluate command line arguments
  CommandLineHelper cmd("BatFaker [<options>] <rawdatatopdir> <outdir> <controlfile>");
  cmd.AddCommandSwitch(' ',"skim","Only copy events explicitly mentioned in control file");
  cmd.AddCommandSwitch(' ',"nosubdirs", "Don't create subdirectories for output series");
  cmd.AddCommandSwitch(' ',"mapdir", "Directory containing .eventmap files","dir");
  cmd.AddCommandSwitch(' ',"templatedir", "Directory containing BatRoot templates (PulseTemplates)","dir");
  cmd.AddCommandSwitch(' ',"detstatusfile","File containing detector status entries","file");
  if(cmd.ProcessCommandLine(argc, argv) != 3)
    cmd.PrintSwitches();
  
  bool copyallevents = cmd.GetNCallsToOption("skim") == 0;
  bool createsubdirs = cmd.GetNCallsToOption("nosubdirs") == 0;
  std::string mapdir = "";
  if(cmd.GetNCallsToOption("mapdir") > 0)
    mapdir = cmd.GetArgumentCall("mapdir");
  std::string templatedir = "";
  if(cmd.GetNCallsToOption("templatedir") > 0)
    templatedir = cmd.GetArgumentCall("templatedir");
  std::string detstatusfile = "";
  if(cmd.GetNCallsToOption("detstatusfile") > 0)
    detstatusfile = cmd.GetArgumentCall("detstatusfile");
  
  
  FakePulseBuilder faker(cmd.GetCommandArg(0), mapdir,
			  templatedir, detstatusfile);
  faker.SetCreateSeriesDirs(createsubdirs);
  faker.SetCopyAllEvents(copyallevents);
  
  int status = faker.GenerateFakeDataFiles(cmd.GetCommandArg(2),
					   cmd.GetCommandArg(1));
  if(status < 0)
    return status;
  return 0;

}

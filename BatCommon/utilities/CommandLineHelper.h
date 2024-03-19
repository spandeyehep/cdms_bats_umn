/**     
    @file CommandLineHelper.h
    @brief Defines the CommandLineHelper configuration class
    @author bloer
*/

#ifndef COMMANDLINEHELPER_h
#define COMMANDLINEHELPER_h 1
#include <string>
#include <vector>
#include <set>

/** @class CommandLineHelper
    @brief Define and parse list of available command line switches
    
*/


class CommandLineHelper{
 public:
  CommandLineHelper(const std::string& usagetext="");
  ~CommandLineHelper();
  
  /** @brief Add a command line switch.  
      
      Only one of shortname,longname are required
      use ' ' (space) and "" (empty string) to skip either parameter
      returns 0 if no error; 1 if overlapping name already exists
      
      @param shortname allow user to call the switch with a single dash (-<c>)
      @param longname allow user to call switch with double dash name
      @param helptext a short description of what this switch does
      @param parname Name of argument, if any; all arguments are required
  */
  
  int AddCommandSwitch(char shortname,
		       const std::string& longname,
		       const std::string& helptext,
		       const std::string& argname="");
  /// Remove a previously registered command line switch
  int RemoveCommandSwitch(char shortname, const std::string& longname);
  
  ///Prints info for the registered switches
  void PrintSwitches(bool quit=true);
  ///Set the usage string given by the '-h' or '--help' options
  void SetProgramUsageString(const std::string& usage){ _program_usage=usage;}
  /// Get the program usage string
  std::string GetProgramUsageString(){ return _program_usage; }
  
  ///Process the command line for the registered switches.
  ///Returns the number of non-switch arguments remaining, neg value if error
  int ProcessCommandLine(int& argc, char** argv);
  
  ///Check the number of times a switch was called
  int GetNCallsToOption(char shortopt);
  int GetNCallsToOption(const std::string& longopt);
  
  ///Get argument for the nth time an option was called
  char* GetArgumentCall(char shortopt, int call=0);
  char* GetArgumentCall(const std::string& longopt, int call=0);
  
  ///Get a vector of arguments of the switch (one for each call)
  std::vector<char*> GetArgumentsList(char shortopt);
  std::vector<char*> GetArgumentsList(const std::string& longopt);
  
  ///Get the number of non-switch arguments to the command line.
  ///Returns neg if command line has not been processed yet
  int GetNCommandArgs() { return _cmd_args.size(); }
  
  ///Get the nth non-switch command line argument
  const char* GetCommandArg(size_t n) { return _cmd_args.at(n); }
  



 private:
  std::string _program_usage;      ///< String detailing how to use program

  std::vector<char*> _cmd_args;  ///< command-line arguments read
  
  
  class Switch{
  public:
    Switch(char shortopt, const std::string& longopt, const std::string& help, 
	   const std::string& arg="") : 
      shortname(shortopt), longname(longopt),helptext(help), argname(arg),
      ncalls(0)
    {}
    char shortname;
    std::string longname;
    std::string helptext;
    std::string argname;
    mutable int ncalls;
    mutable std::vector<char*> args;
    //need mutable to be able to contain in set; ok because not sort part
    
    //comparison operators for set
    bool operator==(const Switch& right) const
    { return shortname==right.shortname && longname == right.longname; }
    bool operator==(char shortopt) const
    { return shortname == shortopt; }
    bool operator==(const std::string& longopt) const 
    { return longname == longopt; }
    bool operator<(const Switch& right) const;
    
  };
  
  std::set<Switch> _switches;
  const Switch* GetSwitch(char shortopt) const;
  const Switch* GetSwitch(const std::string& longopt) const;
};





#endif /*COMMANDLINEHELPER_h*/

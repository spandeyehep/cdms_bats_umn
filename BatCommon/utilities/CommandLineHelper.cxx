#include "CommandLineHelper.h"

#include <iostream>
#include <algorithm>


#include <stdio.h>


CommandLineHelper::CommandLineHelper(const std::string& usagetext) : 
  _program_usage(usagetext)
{
  AddCommandSwitch('h',"help","Print program usage info");
}

CommandLineHelper::~CommandLineHelper() 
{}
  


/// Needed to order commands within the set
bool CommandLineHelper::Switch::
operator<(const CommandLineHelper::Switch& right) const
{
  //true is a < b; i.e. a comes before b
  //if shortname is blank, use the first letter of longname
  char shorta = (shortname == ' ' ? longname.at(0) : shortname);
  char shortb = (right.shortname == ' ' ? right.longname.at(0) : right.shortname);
  
  //first order by shortname
  if( shorta < shortb )
    return true;
  else if( shorta > shortb )
    return false;
  //if we get here, shortnames must be equal
  return longname < right.longname;
}

/// First make sure an existing switch doesn't exist, then add this one
int CommandLineHelper::AddCommandSwitch(char shortname,
					const std::string& longname,
					const std::string& helptext,
					const std::string& argname)
{
  //Make sure both shortname and longname aren't empty
  if(shortname == ' ' && longname == ""){
    std::cerr<<"Error: At least one of shortname and longname must be "
	     <<"non-empty for switches!"<<std::endl;
    return -1;
  }
  //Make sure there isn't already a registered switch  with the same keys
  if(shortname != ' ' && 
     std::find(_switches.begin(),_switches.end(),shortname) !=_switches.end()){
    std::cerr<<"Switch already registered with shortname '"
	       <<shortname<<"'"<<std::endl;
    return -2;
  }
  if(longname != "" &&
     std::find(_switches.begin(),_switches.end(),longname) != _switches.end()){
    std::cerr<<"Switch already registered with longname \""
	     <<longname<<"\""<<std::endl;
    return -3;
  }
    
  _switches.insert(Switch(shortname,longname,helptext,argname) );  
  return 0;
}


int CommandLineHelper::RemoveCommandSwitch(char shortname, 
				       const std::string& longname){
  
  Switch temp(shortname,longname,"");
  std::set<Switch>::iterator it = _switches.find(temp);
  if(it != _switches.end())
    _switches.erase(it);
  else
    std::cerr<<"Unable to find switch "<<shortname<<","<<longname<<std::endl;
  return 1;
}

void CommandLineHelper::PrintSwitches(bool quit)
{
  std::cout<<"Usage: "<<_program_usage<<std::endl;
  size_t maxlong = 0;
  size_t maxpar = 0;
  //struct winsize w;
  //ioctl(0,TIOCGWINSZ, &w);
  //size_t termsize=w.ws_col;
  size_t termsize = 80;
    
  
  std::set<Switch>::iterator cmd = _switches.begin();
  for( ; cmd != _switches.end(); ++cmd){
    maxlong = (maxlong > cmd->longname.size() ? 
		   maxlong : cmd->longname.size() );
    maxpar = (maxpar > cmd->argname.size() ? 
		   maxpar : cmd->argname.size() );
  }
  std::cout<<"Available Options:"<<std::endl;
  for(cmd = _switches.begin(); cmd != _switches.end(); ++cmd){
    std::cout<<" ";
    if( cmd->shortname != ' ')
      std::cout<<'-'<<cmd->shortname;
    else
      std::cout<<"  ";
    
    if( cmd->shortname != ' ' && cmd->longname != "")
      std::cout<<',';
    else 
      std::cout<<' ';
    if( cmd->longname != "" ) 
      std::cout<<"--"<<cmd->longname;
    else
      std::cout<<"  ";
    for(size_t i=0; i < maxlong - cmd->longname.size(); i++)
      std::cout<<' ';
    if( cmd->argname != "" )
      std::cout<<" <"<<cmd->argname<<'>';
    else 
      std::cout<<"   ";
    for(size_t i=0; i < maxpar - cmd->argname.size(); i++)
      std::cout<<' ';
    std::cout<<"  ";
    //insert line-breaks into helptext
    int offset=1+2+1+2+maxlong+3+maxpar+2+2; //final 2 for indent
    std::string spaces;
    spaces.assign(offset,' ');
    size_t length=termsize-offset;
    size_t mypos = 0;
    while(cmd->helptext.size()-mypos > length ){
      //look for a space and replace it with a line-break
      std::string chunk = cmd->helptext.substr(mypos, length);
      size_t spacepos = chunk.rfind(' ');
      if(spacepos == std::string::npos)
	break;
      std::cout<<chunk.substr(0,spacepos)<<"\n"<<spaces;
      mypos += spacepos+1;
    }
    std::cout<<cmd->helptext.substr(mypos)<<std::endl;
  }
  if(quit)
    exit(0);
}

int CommandLineHelper::ProcessCommandLine(int& argc, char** argv)
{
  int status = 0;
  for(int arg = 1; arg<argc; arg++){
    if(status != 0){
      std::cerr<<"Error: Problem encountered while processing command line "
	       <<std::endl;
      return status;
    }
    if( argv[arg][0] != '-' ){
      //we're done with switches
      for( int i=arg; i<argc; i++){
	_cmd_args.push_back(argv[i]);
	argv[1 + i - arg] = argv[i];
      }
      //reset argc, argv to remove switches
      argc = 1 + argc - arg;
      break;
      //return _cmd_args.size();
    }
    
    std::set<Switch>::iterator it;
    bool islong = (argv[arg][1] == '-');
    std::string key;
    int nkeys=1;
    if(islong){
      key = argv[arg]+2;
      it = std::find(_switches.begin(), _switches.end(), key);
    }
    else{
      key = argv[arg]+1;
      nkeys = key.size();
    }
    for(int i=0; i<nkeys; ++i){
      char subkey = key[i];
      if(!islong)
	it = std::find(_switches.begin(),_switches.end(),subkey);
      if(it != _switches.end()){
	//check to see if we need an argument
	bool need_arg = (it->argname != "");
	bool next_is_parameter = ( arg != argc-1 && argv[arg+1][0] != '-' 
				   && !(!islong && i<nkeys-1) );
	it->ncalls++;
	if(need_arg){
	  if(!next_is_parameter){
	    std::cerr<<"Error processing command line: Switch ";
	    if(islong)
	      std::cerr<<"--"<<key;
	    else
	      std::cerr<<"-"<<subkey;
	    std::cerr<<"  is missing required argument "<<it->argname<<"\n";
	    PrintSwitches();
	  }
	  it->args.push_back(argv[++arg]);
	}
      }
      else{
	std::cerr<<"Error processing command line: Unknown switch ";
	if(islong)
	  std::cerr<<"--"<<key;
	else
	  std::cerr<<"-"<<subkey;
	std::cerr<<std::endl;
	PrintSwitches();
      }  
    } 
    
  } //end for loop over args
  if(GetNCallsToOption("help")>0)
     PrintSwitches();
  if(status == 0 ) 
    argc = _cmd_args.size() + 1;
  else if(status < 0) return status;
  
  return _cmd_args.size();
}

const CommandLineHelper::Switch* 
CommandLineHelper::GetSwitch(char shortopt) const
{
  std::set<Switch>::const_iterator it = 
    std::find(_switches.begin(), _switches.end(), shortopt);
  if(it != _switches.end())
    return &(*it);
  //if we get here, wasn't found
  std::cerr<<"No command switch with short name "<<shortopt<<" known.";
  return 0;
}

const CommandLineHelper::Switch* 
CommandLineHelper::GetSwitch(const std::string& longopt) const
{
  std::set<Switch>::const_iterator it = 
    std::find(_switches.begin(), _switches.end(), longopt);
  if(it != _switches.end())
    return &(*it);
  //if we get here, wasn't found
  std::cerr<<"No command switch with long name "<<longopt<<" known.";
  return 0;
}

int CommandLineHelper::GetNCallsToOption(char name)
{
  const Switch* s = GetSwitch(name);
  return s ? s->ncalls : 0;
}

int CommandLineHelper::GetNCallsToOption(const std::string& name)
{
  const Switch* s = GetSwitch(name);
  return s ? s->ncalls : 0;
}

///Get argument for the nth time an option was called
char* CommandLineHelper::GetArgumentCall(char name, int call)
{
  const Switch* s = GetSwitch(name);
  if(s){
    if( (size_t)call >= s->args.size()){
      std::cerr<<"Switch "<<name<<"was only called "<<s->args.size()
	       <<" times; requested arg "<<call<<"\n";
      return 0;
    }
    return s->args[call];
  }
  return 0;
}

///Get argument for the nth time an option was called
char* CommandLineHelper::GetArgumentCall(const std::string& name, int call)
{
  const Switch* s = GetSwitch(name);
  if(s){
    if( (size_t)call >= s->args.size()){
      std::cerr<<"Switch "<<name<<"was only called "<<s->args.size()
	       <<" times; requested arg "<<call<<"\n";
      return 0;
    }
    return s->args[call];
  }
  return 0;
}

///Get a vector of arguments of the switch (one for each call)
std::vector<char*> CommandLineHelper::GetArgumentsList(char name)
{
  const Switch* s = GetSwitch(name);
  return s ? s->args : std::vector<char*>(0); 
}

///Get a vector of arguments of the switch (one for each call)
std::vector<char*> CommandLineHelper::GetArgumentsList(const std::string& name)
{
  const Switch* s = GetSwitch(name);
  return s ? s->args : std::vector<char*>(0); 
}


  

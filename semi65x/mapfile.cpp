#include <stdio.h>
#include <stdint.h>

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/regex.hpp>

#include "mapfile.h"

using namespace std;

vector<string> modules;
vector<string> segments;
map<uint16_t, string> exports;
vector<string> imports;

enum filepart
{
  UNKNOWN,
  MODULES_LIST,
  SEGMENT_LIST,
  EXPORTS_LIST,
  IMPORTS_LIST
};

void
add_symbol (uint16_t addr, string symname)
{
  //cerr << "Adding symbol '" << symname << "' with value " << addr << endl;
  exports[addr] = symname;
}

string
find_symbol (int addr)
{
  map<uint16_t,string>::iterator mi;
  mi = exports.upper_bound(addr);

  if (exports.empty() || mi == exports.begin())
    {
      std::stringstream ss;
      ss << '$' << std::hex << addr;
      return ss.str();
    }
  else
    {
      mi--;
      if (mi->first == addr)
	return mi->second;
      else
        {
	  std::stringstream ss;
	  ss << mi->second << "+" << addr - mi->first;
	  return ss.str();
	}
    }
}

void
parse_map (const char *filename)
{
  filepart part = UNKNOWN;
  bool seen_separator = false;

  modules.clear();
  segments.clear();
  exports.clear();
  imports.clear();
  
  ifstream ifs;
  
  boost::regex one_export_re("([A-Za-z_]+[A-Za-z0-9_$]*)\\s+([0-9A-F]+)\\s+"
			     "([ALR]+)\\s+");
  boost::regex two_exports_re("([A-Za-z_]+[A-Za-z0-9_$]*)\\s+([0-9A-F]+)\\s+"
			      "([ALR]+)\\s+"
			      "([A-Za-z_]+[A-Za-z0-9_$]*)\\s+([0-9A-F]+)\\s+"
			      "([ALR]+)\\s+");
  
  ifs.open (filename, ifstream::in);
  
  while (ifs.good())
    {
      string line;
      
      getline (ifs, line);
      
      switch (part)
        {
	case UNKNOWN:
	  if (line.compare("Modules list:") == 0)
	    {
              part = MODULES_LIST;
	      seen_separator = false;
	      continue;
	    }
	  break;
	
	case MODULES_LIST:
	  if (line.compare("Segment list:") == 0)
	    {
	      part = SEGMENT_LIST;
	      seen_separator = false;
	      continue;
	    }
	  
	  if (line.compare("-------------") == 0)
	    seen_separator = true;
	  else
	    {
	    
	    }
	  break;

	case SEGMENT_LIST:
          if (line.compare("Exports list by name:") == 0)
	    {
	      part = EXPORTS_LIST;
	      seen_separator = false;
	      continue;
	    }
	  
	  if (line.compare("-------------") == 0)
	    seen_separator = true;
	  else
	    {
	    
	    }
	  break;
	
	case EXPORTS_LIST:
	  if (line.compare("Imports list:") == 0)
	    {
	      part = IMPORTS_LIST;
	      seen_separator = false;
	      continue;
	    }
	  
	  if (line.compare("-------------") == 0)
	    seen_separator = true;
	  else
	    {
	      boost::smatch sm;
	      if (boost::regex_match(line, sm, two_exports_re))
	        {
		  std::stringstream ss;
		  int addr1, addr2;

		  ss << std::hex << sm[2] << ' ' << sm[5];
		  ss >> addr1 >> addr2;

		  add_symbol (addr1, sm[1]);
		  add_symbol (addr2, sm[4]);
		}
	      else if (boost::regex_match(line, sm, one_export_re))
	        {
		  std::stringstream ss;
		  int addr;
		  
		  ss << std::hex << sm[2];
		  ss >> addr;
		  add_symbol (addr, sm[1]);
		}
	    }
	  break;
	
	case IMPORTS_LIST:
	  break;
	}
    }
  
  /*cerr << "Symbol at 529 is " << find_symbol(529) << endl;
  cerr << "Symbol at 530 is " << find_symbol(530) << endl;
  cerr << "Symbol at 531 is " << find_symbol(531) << endl;*/
  
  ifs.close ();
}

#include <cstring>

#include "RTTL/common/MapOptions.hxx"

namespace RTTL {

  /// Instantiates MapOptions entry which is visible in any file which includes MapOptions.hxx.
/// \note The preferred way to parse command line parameters is to add them to options as
/// <b>options.parse(argc-1, argv+1);</b>
/// In this case, this variable must be defined (for example, by including this file in the project).
MapOptions options;

  /// Remove blank chars from the end of token.
  static _INLINE char* trimEnd(char* token) {
    char* pe = token + strlen(token) - 1;
    while ((*pe == ' ' || *pe == '\t' || *pe == 10 || *pe == 13) && pe >= token) *pe-- = 0;
    return token;
  }

  /// Return size of directory name of filename fn.
  static _INLINE int pathLength(const string& fn)
  {
#ifdef _WIN32
    const char* del = "\\/:";
#else
    const char* del = "/";
#endif
    return string(fn).find_last_of(del) + 1;
  }

  /// Dtor.
  MapOptions::~MapOptions() {
    // First, report unused entries.
    bool start = true;
    for (iterator it = begin(); it != end(); it++) {
      vector_of_strings* entry = it->second;
      map<void*, bool>::iterator itused = m_used_entries.find(it->second);
      if (itused->second == false) {
        if (start) {
          start = false;
          puts("\n");
        }
        //printf("MapOptions: unused parameter: -%s\n", it->first.c_str());
      }
    }
    // Second, clear m_used_entries
    m_used_entries.clear();
    // Third, free everything.
    clear();
  }

  void MapOptions::remove_all_entries() { /// iterate, clear and delete all entries.
    for (iterator it = begin(); it != end(); it++) {
      vector_of_strings* entry = it->second;
      entry->clear();
      delete entry;
    }
  }

  /// Clean everything (it is not required if dtor is called implicitly).
  void MapOptions::clear() {
    remove_all_entries();
    map_strings_to_vector_of_strings::clear();
  }

  //   /// Add converted value to the named parameter.
  //   template<typename DataType>
  //    void add(const string& name, const DataType value) {
  //     add_string(name, Convert::to_string(value));
  //   }

  /// Add value string to the named parameter.
  void MapOptions::add_string(const string& name, const string& value) {
    (*this)[name]->push_back(value);
  }

  /// Return 1st entry (with index 0) of the named parameter as a string
  /// or return name as a string if it does not exist (even in the environment).
  string MapOptions::get(string name) const {
    return get(name, name, 0);
  }
  /// Return the named vector using defvalue for missing components.
  //   template<int N, typename DataType>
  //    RTVec_t<N, DataType> getVector(string name, DataType defvalue = 0) const {
  //     RTVec_t<N, DataType> v;
  //     getArray<N, DataType>(name, v, defvalue);
  //     return v;
  //   }
  /// Specialization <3, float>.
  RTVec3f MapOptions::getVec3f(string name, float defvalue) const {
    return RTVec3f(getVector<3, float>(name, defvalue));
  }
  /// Specialization <3, int>.
  RTVec3f MapOptions::getVec3i(string name, int defvalue) const {
    return RTVec3f(getVector<3, int>(name, defvalue));
  }
  /// Specialization <2, float>.
  RTVec2f MapOptions::getVec2f(string name, float defvalue) const {
    return RTVec2f(getVector<2, float>(name, defvalue));
  }
  /// Specialization <2, int>.
  /// X windows-like settings of n1Xn2 are allowed.
  RTVec2i MapOptions::getVec2i(string name, int defvalue) const {
    if (defined(name)) {
      string rs = get(name);
      const char* rc = rs.c_str();
      int lx = strcspn(rc, "xX");
      if (rc[lx]) {
        return RTVec2i(atoi(rc), atoi(rc + lx + 1));
      } else {
        return getVector<2, int>(name, defvalue);
      }
    }
    return RTVec2i(defvalue);
  }

  /// Write the named vector to vtgt (no more than N entries) or use defvalue
  /// if it is smaller than N.
  //   template<int N, typename DataType>
  //    void getArray(string name, DataType* vtgt, DataType defvalue = 0) const {
  //     int n = vector_size(name);
  //     int verboselevel = verbose();
  //     if (n == 0 && verboselevel >= 1) {
  //       std::cout << "MapOptions: parameter " << name << " is undefined; using default value of " << defvalue << std::endl;
  //     }
  //     int i = 0;
  //     if (n) {
  //       if (verboselevel >= 2) {
  //         std::cout << "MapOptions: using " << name << " = [ ";
  //       }
  //       const vector_of_strings& vs = *(*this)[name];
  //       for (vector_of_strings::const_iterator it = vs.begin(); it != vs.end(); it++) {
  //         vtgt[i++] = Convert::to<DataType>(*it);
  //         if (verboselevel >= 2) {
  //           std::cout << vtgt[i-1] << " ";
  //         }
  //       }
  //       if (verboselevel >= 2) {
  //         std::cout << "]" << std::endl;
  //       }
  //     }
  //     for (; i < N; i++) {
  //       vtgt[i] = defvalue;
  //     }
  //   }

  /// Indicate what MapOptions should report during calls to get() functions.
  /// It is based on (user supplied) integer value of 'verbose' keyword and
  /// is choosen as follows:
  /// <ul>
  /// <li> 0 - do nothing except reporting unused parameters at the exit (default value)
  /// <li> 1 - report undefined parameters (ones for which default values were used)
  /// <li> 2 - report all requested parameters.
  /// </ul>
  int MapOptions::verbose() const {
    const_iterator it = find("verbose");
    if (it == end()) return 0;
    const vector_of_strings& entry = *it->second;
    return Convert::to<int>(entry[0]);
  }

  /// return index entry in the named parameter if it exists or
  /// defvalue otherwise.
  //   template<typename DataType>
  //    DataType get(const string& name, DataType defvalue, unsigned int index = 0) const {
  //     const_iterator it = find(name);
  //     int verboselevel = verbose();
  //     if (it == end()) {
  //       // See if it is defined in the environment...
  //       const char* parenv = getenv(name.c_str());
  //       if (parenv) {
  //         return Convert::to<DataType>(parenv);
  //       }
  //       // Nope, return caller-supplied default value.
  //       if (verboselevel >= 1) {
  //         std::cout << "MapOptions: parameter " << name << "[" << index << "]"
  //         << " is undefined; using default value of " << defvalue << std::endl;
  //       }
  //       return defvalue;
  //     } else {
  //       const vector_of_strings& entry = *it->second;
  //       if (index >= entry.size())    return defvalue;
  //       if (entry[index].size() == 0) return defvalue;
  //       DataType value = Convert::to<DataType>(entry[index]);
  //       if (verboselevel >= 2) {
  //         std::cout << "MapOptions: using " << name << "[" << index << "] = " << value << std::endl;
  //       }
  //       return value;
  //     }
  //   }

  /// Return # of components in the named vector or 0.
  unsigned int MapOptions::vector_size(const string& name) const {
    const_iterator it = find(name);
    if (it == end()) {
      return 0;
    } else {
      vector_of_strings& entry = *it->second;
      return entry.size();
    }
  }

  /// Return true iff the parameter defined by len characters of name exists either in this or in the environment.
  bool MapOptions::defined(const char* name, int len) const {
    return defined(string(name, len));
  }
  /// Return true iff the named parameter exists either in this or in the environment.
  bool MapOptions::defined(const string& name) const {
    return find(name) != end();
  }

  /// Remove the named parameter.
  void MapOptions::remove(const string& name) {
    // Remove named entry.
    iterator it = find(name);
    if (it != end())
      erase(it);
  }

  /// Parse named tokens defined by argv (see the detailed class description).
  /// If keep_all_filenames is false (default value), non-existing files will be deleted from 'files' group.
  bool MapOptions::parse(int argc, char* argv[], bool keep_all_filenames) {
    // Just to allow using main() parameters directly without
    // any stupid warnings.
    return parse(argc, (const char**)argv, keep_all_filenames);
  }
  /// Parse named tokens defined by a.
  bool MapOptions::parse(const char* a) {
    const char* argv[] = {a};
    return parse(1, argv);
  }

  /// Return true iff all characters of s before separator (" \t,;])" or 0) represents a number
  /// (using integer, float or exponential notation).
  bool MapOptions::isNumber(const char* s) {
    // I'll be damned -- parsing a number is not so easy.
    int nss = 0, ndd = 0, nee = 0;
    int n = 0;
    do {
      int ns = s[n] == '-' || s[n] == '+';
      // Signs are either first chars or after exponent.
      if (ns && n && s[n-1] != 'e' && s[n-1] != 'E') return false;
      // Need a digit after the sign.
      if (ns && !(s[n+1] >= '0' && s[n+1] <= '9')) return false;
      nss += ns;
      int nd = s[n] == '.';
      // Need either digit or EOS or exponent after the dot.
      if (nd && !(s[n+1] == 0 || s[n+1] == 'e' || s[n+1] == 'E' || (s[n+1] >= '0' && s[n+1] <= '9'))) return false;
      // No dots after exponent.
      if (nd && nee) return false;
      ndd += nd;
      // No more than 2 signs and one dot.
      if (nss > 2 || ndd > 1) return false;
      int ne = s[n] == 'e' || s[n] == 'E';
      nee += ne;
      // Only one exponent; could not be the first char.
      if (nee > 2 || (ne && !n)) return false;
      if (ne) {
        n++;
        if (s[n] == 0 || strchr(" \t,;])", s[n]))  return false;
        continue;
      }
      if (!((s[n] >= '0' && s[n] <= '9') || ns || nd))
        return false;
      n++;
    } while (s[n] && !strchr(" \t,;])", s[n]));
    return true;
  }

  /// Parse named tokens defined by argv (see the detailed class description).
  /// If keep_all_filenames is false (default value), non-existing files will be deleted from 'files' group.
  bool MapOptions::parse(int argc, const char* argv[], bool keep_all_filenames)
  {
    const char* name = 0;
    const char* ptr;
    int added = 1;
    int namelen;
    for (int count = 0; count < argc; count++) {

      const char* arg = argv[count];
      if (isNumber(arg)) {
        if (!name) {
          printf("Unnamed number: %s\n", arg);
          return false; // report failure
        }
        goto parse_parameters;
      }

      if (arg[0] == '-' || arg[0] == '+') {
        // It is named parameter.
        if (name && !added) {
          // If the previous parameter does not have any value, just add it with the default value of "".
          add(name, "");
        }

        bool accumulate = arg[0] == '+';

        // Get name of the current parameter.
        name = arg + 1;
        while (*name == '-') name++; // take care of --parname
        added = 0;
        namelen = strcspn(name, "=");

        if (!accumulate) {
          // Delete all previous name entries to allow overloading parameters with latter values.
          remove(string(name, namelen));
        }

        if (namelen == strlen(name))
          continue;

        // fused name=value
        arg  = name + namelen + 1;
        arg += strspn(arg, " \t");
        goto parse_parameters;

      } else if (added && (ptr = strrchr(arg, '.'))) {
        // This is not a parameter or number or parameter's name; assume that
        // it is a file and check if file exists.
        // This block is executed only if previous parameter was finalized
        // (i.e. have non-trivial value).
#ifdef _WIN32
        // Replace Linux delimiters.
        char* parg = (char*)arg; while (parg = strchr(parg, '/')) *parg = '\\';
#endif
        if (keep_all_filenames == false && access(arg, 0) == -1) {
          printf("File %s does not exist.\n", arg);
          return false; // file do not exist; report failure
        }
        // It is a filename. Check if it contains parameters (*.ini).
        if (strcasecmp(ptr + 1, "ini") == 0) {
          if (parse_file(arg) == false)
            return false;
        } else {
          // It is a model file; include it into "files".
          add("files", arg);
        }
        name = 0; // need a new name

      } else if (name) {
      parse_parameters:
        // Add named parameter. Vectors are allowed, like
        // -camera.pos 1.0 2.0 3.0

        char term[80];
        // Remove prefixed separators if they exist (commas, =, or brackets).
        bool remove_separator = *arg == '=' || *arg == '(' || *arg == '[' || *arg == ',' || *arg == ';';
        strncpy(term, arg + (remove_separator ? 1 : 0), sizeof(term));
        int remain = strlen(term);
        char* pterm = term;

        // Take apart fused expressions like 1,2,3 and convert them to a vector.
      unfuse:
        int term_end = strcspn(pterm, ",;])");
        pterm[term_end] = 0;

        if (*pterm == 0)
          continue;

        ++added;
        add(string(name, namelen), pterm);
        pterm  += term_end + 1;
        remain -= term_end + 1;
        if (remain > 0) goto unfuse;

      } else {
        return false; // wrong (unnamed) parameter; report failure
      }
    }
    if (name && !added) {
      // If the last parameter does not have any value, just add it with the default value of "".
      add(name, "");
    }

    return true;
  }

  /// Parse all named tokens in file filename (see the detailed class description).
  bool MapOptions::parse_file(const char* filename) {
    FILE* fs = fopen(filename, "rt");
    if (fs == 0) {
      printf("File %s does not exist.\n", filename);
      return false;
    }

    char buf[300];
    char prefix[80];
    prefix[0] = 0;

    // ==============================================================================
    // Parse lines like
    // pname = pvalue
    // or
    // pname = [v1, v2, v3] or similar.
    // ==============================================================================
    while (fgets(buf, sizeof(buf), fs)) {
	  trimEnd(buf);

      char* pname = buf + strspn(buf, " \t\n");   // skip blank chars
      if (*pname == 0 || (pname[0] == '/' && pname[1] == '/')) continue; // commented or empty line

      // Skip comments.
      if (*pname == '#')
        continue;

      if (!strncmp(pname, "exit", 4))
        break;

      if (*pname == '[') {
        // [section] will be prepended to all following parameters.
        strncpy(prefix, pname + 1, sizeof(prefix));
        int prefixend = strcspn(prefix, "] \t");
        prefix[prefixend] = 0;
        continue;
      }

      if (pname[0] == '-' || pname[0] == '+') {
        // command-line style
        trimEnd(pname);

      split_multiple_tokens_into_groups:
        int nameend = strcspn(pname, " \t");
        int namelen = strlen(pname);
        if (nameend < namelen)
          pname[nameend] = '=';

        char* nextpname = pname;
      next_token:
        unsigned int postoken = strcspn(++nextpname, "-+");
        if (postoken < strlen(nextpname)) {
          nextpname += postoken;
          if (nextpname[-1] == ' ' || nextpname[-1] == '\t') {
            if (nextpname[1] >= '0' && nextpname[1] <= '9') {
              // It is a number, continue looking for next token.
              goto next_token;
            } else {
              // It is bona fide parameter name (or so I think), split the line before it.
              nextpname[-1] = 0;
            }
          } else {
            goto next_token;
          }
        } else {
          nextpname = NULL;
        }
        parse(pname);
        if (!nextpname)
          continue;
        pname = nextpname;
        goto split_multiple_tokens_into_groups;

      } else if (strrchr(pname, '.') && !strrchr(pname, '=')) {
        // filename (not a float number)
        if (access(pname, 0) == -1) {
          // File do not exist -- try to append the directory of the current ini file.
          const string fnini(filename);
          const string fullfn = fnini.substr(0, pathLength(fnini)) + string(pname);
          const char* pname = fullfn.c_str();
          if (access(pname, 0) != -1) {
            // load/parse the existing file
            parse(pname);
          } else {
            // Better kill it now than be sorry latter...
            FATAL(string("Cannot open file ") + string(pname));
          }
        } else {
          // load/parse the existing file
          parse(pname);
        }
        continue;
      }

      // The line in the form of
      // name = value

      bool accumulate = *pname == '+';
      if (accumulate) pname++;

      int name_end = strcspn(pname, "= \t");
      pname[name_end] = 0;

      char* pvalue = pname + name_end + 1;

      bool first_token = true;
      string token_name = *prefix ? string(prefix) + string(".") + string(pname) : string(pname);
      if (!accumulate) {
        // Delete all previous name entries to allow overloading parameters with latter values.
        remove(token_name);
      }

      // Loop over multiple values (in vector).
    next_value:
      pvalue += strspn(pvalue, "=,;[( \t\n");
      if (*pvalue == 0) continue;

      int value_end;
      if (*pvalue == '\"') {
        // "value inside"
        value_end = strcspn(++pvalue, "\"\n");
      } else {
        // look for separators
        value_end = strcspn(pvalue, ",;]) \t\n");
      }
      pvalue[value_end] = 0;

      if (first_token) {
        if (strcasecmp(pname, "include") == 0 ||
            strcasecmp(pname, "#include") == 0) {
          // Parse nested file.
          if (!parse_file(pvalue)) {
            return false;
          }
          continue;
        }
        if (*pname == '#') continue; // skip comments other than #include
        first_token = false;
      }

      add(token_name, pvalue);

      // Look for the next value (for vectors) if it exists.
      pvalue += value_end + 1;
      goto next_value;
    }

    fclose(fs);
    return true;
  }

  /// const version.
  const vector_of_strings* MapOptions::operator[](const string& name) const {
    const_iterator it = find(name);
    if (it == end())
      return 0;
    return it->second;
  }
  /// Return either pointer to the vector_of_strings defined by the name or pointer to the newly created empty vector.
  vector_of_strings* MapOptions::operator[](const string& name) {
    vector_of_strings* entry;
    iterator it = find(name);
    if (it == end()) {
      entry = new vector_of_strings;
      insert(value_type(name, entry));
      m_used_entries.insert(map<void*, bool>::value_type(entry, false));
    } else {
      entry = it->second;
    }
    return entry;
  }

  /// Overload find operation to allow component names (like "nt; nthread; nthreads").
  /// This function will also check m_used_entries.
  /// \note Order of items in component name is important (first one is checked first, etc).
  MapOptions::iterator MapOptions::find(const string& name) {
    iterator it;
    const char* ns = name.c_str();
    const char* del = ",; \t|";
    if (strcspn(ns, del) == strlen(ns)) {
      it = map_strings_to_vector_of_strings::find(name);
    } else {
      // Name includes multiple terms: check for each one individually.
      const char* pname = ns;
      do {
        int sep = strcspn(pname, del);
        it = find(pname, sep);
        if (it != end())
          break;
        pname += sep;
        pname += strspn(pname, del);
      } while (*pname && *pname != '\n');
    }

    if (it != end()) {
      map<void*, bool>::iterator itused = m_used_entries.find(it->second);
      assert(itused != m_used_entries.end());
      itused->second = true;
    }
    return it;
  }
  MapOptions::const_iterator MapOptions::find(const string& name) const {
    // Find through double cast to un-const member.
    return (const_iterator)((MapOptions*)this)->find(name);
  }
  MapOptions::iterator MapOptions::find(const char* name, int len) {
    return find(string(name, len));
  }
  MapOptions::const_iterator MapOptions::find(const char* name, int len) const {
    return (const_iterator)find(string(name, len));
  }
};

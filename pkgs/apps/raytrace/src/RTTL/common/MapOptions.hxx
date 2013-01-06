#ifndef MAPOPTIONS_HXX
#define MAPOPTIONS_HXX

/// \file MapOptions.hxx
/// Defines namespace Convert (to convert between some atomic types and strings)
/// and class MapOptions (to define and access a set of named parameters).
///
/// <b>namespace Convert</b> defines conversion functions between atomic <b>int/float/double</b> types and <b>strings</b>.
/// <h2>Example:</h2>
/// \code
/// string pis = "pi is " + Convert::to_string(4 * atan2(1.0, 1.0f));
/// float  pif = Convert::to<float>(pis.c_str() + 6);
/// int    pii = Convert::to<int>  (pis.c_str() + 6);
/// std::cout << "pi is " << pif << "; rounded to " << pii << endl;
/// \endcode

#include "RTTL/common/RTVec.hxx"

#include <cstring>
#include <vector>
#include <map>


using namespace std;

namespace RTTL {

  /// Defines <b>string <=> atomic types</b> conversions.
  namespace Convert
  {
    _INLINE static string to_string(int    v)
    {
      char s[12]; sprintf(s, "%-10i "   , v); *strchr(s, ' ') = 0; return string(s);
    }
    _INLINE static string to_string(float  v)
    {
      char s[16]; sprintf(s, "%-14.7g " , v); *strchr(s, ' ') = 0; return string(s);
    }
    _INLINE static string to_string(double v)
    {
      char s[24]; sprintf(s, "%-22.14g ", v); *strchr(s, ' ') = 0; return string(s);
    }
    _INLINE static string to_string(const string& s)
    {
      return s;
    }
    template<typename DataType> _INLINE static DataType to(const string& s)
    {
      cerr << "No default converter...\n"
           << "This is most likely happen when char* values\n"
           << "are assigned to (should be const char*).\n\n";
      exit(1);
      return DataType(0);
    }
    template<> _INLINE int         to<int   >(const string& s)
    {
      return atoi(s.c_str());
    }
    template<> _INLINE float       to<float >(const string& s)
    {
      return float(atof(s.c_str()));
    }
    template<> _INLINE double      to<double>(const string& s)
    {
      return atof(s.c_str());
    }
    template<> _INLINE const char* to<const char*>(const string& s)
    {
      return s.c_str();
    }
    template<> _INLINE string      to<string>(const string& s)
    {
      return s;
    }
  };



  /// \class MapOptions
  /// Defines a set of the named vectors of strings (parameters) and
  /// methods to access them.
  ///
  /// Implementation details (see <em><b>example below</b></em>):
  /// <ul>
  /// <li> In order to access parameters, they must be parsed (with parse()) or added (with add()) first.
  /// <li> parse() processes the second argument argv as a list of the named tokens.
  /// <li> "name" of the parameter is defined by the first character(s) which is either
  /// <ol>
  /// <li> '-' or '--'. In this case, the new parameter replaces the old one with the same name (if it exists).
  /// <li> '+'. In this case, the new parameter is appended to the vector of parameters with the same name.
  /// </ol>
  /// <li> Any token, which does not represent a name (with prefix), is considered to be an entry
  /// and appended to the previously named parameter, thus creating potentially infinite vector of
  /// parameters with the same name (different entries are accessed using index argument in get() function).
  /// <li> Different tokens are either specified as separate entries in argv array or separated with ',' or ';' inside
  /// one argv[i] entry.
  /// <li> Extra brackets ('[' and ']') could be specified for readability, which are ignored by the parser.
  /// <li> <b>There is one exception to this scheme. All <i>independent</i> tokens with '.' are considered to be filenames
  /// and added to the "files" group. If the specified file does not exist, parse() returns false immediately.
  /// (unless keep_all_filenames parameter is true) </b>
  /// <li> <b>Additionally, all files with *.ini extension are assumed to include parameters and loaded recursively.</b>
  /// <li> <b>Only those parameters are considered to define independent filenames,
  /// which do not have the preceding parameter name
  /// (that is, "-save debug.txt" defines token "save" with value "debug.txt" and not a filename).</b>
  /// </ul>
  ///
  /// <b>*.ini</b> file format allows to specify different parameters on different lines.
  /// Additionally, nested parameter files could be loaded with the 'include' directive.
  ///
  /// It is allowed to specify component name using [prefixed] notation.
  /// Accordingly, the following fragment
  /// <pre>
  ///[camera]
  /// pos = -2.9 1.5 -2.2
  /// dir = 2.9,-0.5,2.2
  /// up = 0; 1; 0
  /// </pre>
  /// defines "camera.pos", "camera.dir", and "camera.up" parameters
  /// (note different ways of defining the vectors).
  ///
  /// The parsed parameters could be retrieved with the templated get() function,
  /// which defines the return data type by the default parameter.
  /// \note If the named parameter does not exist, attempt is made to
  /// retrieve this parameter from the environment.
  ///
  /// \note MT support exists insofar STL supports it. Accessing/updating vectors simultaneously
  /// may crush the system (read-only multiple accesses are safe, I think :).
  ///
  /// <h2>Example for the following command line parameters:</h2>
  /// <pre>
  /// echo > head.obj; echo > body.obj
  /// bin/TestOptions -pbo=1 -pos [1, 2, 3] -up 0,0,1 +dir 5 +dir 6 +dir 7 -nthreads 3 -verb head.obj body.obj
  /// </pre>
  /// \code
  /// #include "RTTL/common/MapOptions.hxx"
  /// int main(int argc, const char* argv[]) {
  /// // Read parameters from the command line and print them out.
  /// options.parse(argc-1, argv+1);
  /// cout << options << endl << endl;
  ///
  /// // Get environment value (defined in Windows and Linux as well).
  /// // Note: Under IDE with icc, it will not be defined (as a matter of fact),
  /// // but it will be defined if compiled using mvsc.
  /// cout << "USERNAME = " << options.get("USERNAME") << endl;
  /// // Try to get the value from the environment. get<> type is defined by defvalue.
  /// cout << "PROCESSOR_LEVEL = " << options.get("PROCESSOR_LEVEL", -1) << endl;
  ///
  /// cout << "PBO = " << options.get("pbo", 0) << endl;
  /// cout << "output is " << (options.defined("statistics, stat, all")? "report stat":"compact") << endl;
  /// cout << "pos is ["
  ///   << options.get("pos", 0.0f, 0) << ","
  ///   << options.get("pos", 0.0f, 1) << ","
  ///   << options.get("pos", 0.0f, 2) << "]" << endl;
  /// (*options["up"])[1] = Convert::to_string(-1); // change 2nd entry
  /// cout << "up  is " << options.getVec3f("up") << endl;
  /// cout << "dir is " << options.getVec3f("dir") << endl;
  /// cout << options.get("nthreads") << endl; // returns string("3")
  /// options.remove("nthreads");
  /// cout << options.get("nthreads") << endl; // returns the name
  /// int nfiles = options.vector_size("files");
  /// for (int fi = 0; fi < nfiles; fi++) {
  ///  const string& fn = (*options["files"])[fi];
  ///  cout << "Adding obj file: " << fn << endl;
  /// }
  /// }
  /// \endcode
  /// <h2>Output (on Linux machine):</h2>
  /// <pre>
  ///       dir = [5, 6, 7];
  ///     files = [head.obj, body.obj];
  ///  nthreads = 3;
  ///       pbo = 1;
  ///       pos = [1, 2, 3];
  ///        up = [0, 0, 1];
  ///
  /// USERNAME = aresheto
  /// PROCESSOR_LEVEL = -1
  /// PBO = 1
  /// output is report stat
  /// pos is [1,2,3]
  /// up  is [0,-1,1]
  /// dir is [5,6,7]
  /// 3
  /// nthreads
  /// Adding obj file: head.obj
  /// Adding obj file: body.obj
  /// </pre>
  ///
  /// <h2>Q&A</h2>
  /// <ul>
  /// <li> Is -pbo the same as --pbo ?
  /// - Yes.
  /// <li> what's the syntax of specifying an options to a parameter:
  /// --pbo=on", or "--pbo=true", or "--pbo=1", or no "=" sign at all, or
  /// "--pbo means true, --pbo=0 means off" or "--pbo/--no-pbo" (icc-style)
  /// <ol>
  /// - Sign ('=') is optional.
  /// - Any "pbo" definition will result in defined("pbo") returning true, but its value could be different, depending on comand line parameter.
  /// - --pbo=0 will just mean that "pbo" is defined and its value is "0".
  /// Then query options.get("pbo", 1) will return integer 0, that's all.</ol>
  /// <li>what's the preference of arguments: if two instances of same
  /// parameter are specified on cmd line, I assume it's the one the appears
  /// last that counts ... is that so ? what about env-vars -- do they
  /// _override_ cmd line vars, or is it the other way around ?
  /// - Last parameter (either on command line or in ini file overrides previous one with the same name,
  /// unless it is prefixed with '+'. In that case the new value will be added to the named vector.
  /// Environment has the lowest priority; it is querried only if the value was not specified on command line
  /// or in included file(s).
  ///
  /// <li> Can we add a specific file name extension that's being parsed as if all it's lines are specified in the command line ? i.e., if we create a file default.rtopt (or whatever extension we pick...) with content
  /// <pre>
  /// res X Y
  /// pbo off
  /// </pre>
  /// then that's the same as having specified "--res X Y --pbo=off" on the command line ?
  /// - For whom do you think documentation is written for? Just go and read it :)
  /// </ul>

  typedef vector<string> vector_of_strings;
  typedef map<string, vector_of_strings*> map_strings_to_vector_of_strings;


  /// Declares mapping of strings to vector of strings, what did you expect?
  class MapOptions: public map_strings_to_vector_of_strings
  {
  public:

    typedef map_strings_to_vector_of_strings::iterator iterator;
    typedef map_strings_to_vector_of_strings::const_iterator const_iterator;

    // Default ctor.

    /// Dtor.
    ~MapOptions();

    /// Clean everything (it is not required if dtor is called implicitly).
    void clear();

    /// Add converted value to the named parameter.
    template<typename DataType>
    void add(const string& name, const DataType value)
    {
      add_string(name, Convert::to_string(value));
    }

    /// Add value string to the named parameter.
    void add_string(const string& name, const string& value);

    /// Return 1st entry (with index 0) of the named parameter as a string
    /// or return name as a string if it does not exist (even in the environment).
    string get(string name) const;
    /// Return the named vector using defvalue for missing components.
    template<int N, typename DataType>
    RTVec_t<N, DataType> getVector(string name, DataType defvalue = 0) const {
      RTVec_t<N, DataType> v;
      getArray<N, DataType>(name, v, defvalue);
      return v;
    }
    /// Specialization <3, float>.
    RTVec3f getVec3f(string name, float defvalue = 0) const;
    /// Specialization <3, int>.
    RTVec3f getVec3i(string name, int defvalue = 0) const;
    /// Specialization <2, float>.
    RTVec2f getVec2f(string name, float defvalue = 0) const;
    /// Specialization <2, int>.
    /// X windows-like settings of n1Xn2 are allowed.
    RTVec2i getVec2i(string name, int defvalue = 0) const;
    /// Write the named vector to vtgt (no more than N entries) or use defvalue
    /// if it is smaller than N.
    template<int N, typename DataType>
    void getArray(string name, DataType* vtgt, DataType defvalue = 0) const {
      int n = vector_size(name);
      if (N > 0) n = min(n, N);
      int verboselevel = verbose();
      if (n == 0 && verboselevel >= 1) {
        std::cout << "MapOptions: parameter " << name << " is undefined; using default value of " << defvalue << std::endl;
      }
      int i = 0;
      if (n) {
        if (verboselevel >= 2) {
          std::cout << "MapOptions: using " << name << " = [ ";
        }
        const vector_of_strings& vs = *(*this)[name];
        for (vector_of_strings::const_iterator it = vs.begin(); i < n; it++) {
          vtgt[i++] = Convert::to<DataType>(*it);
          if (verboselevel >= 2) {
            std::cout << vtgt[i-1] << " ";
          }
        }
        if (verboselevel >= 2) {
          std::cout << "]" << std::endl;
        }
      }
      for (; i < N; i++) {
        vtgt[i] = defvalue;
      }
    }

    /// Indicate what MapOptions should report during calls to get() functions.
    /// It is based on (user supplied) integer value of 'verbose' keyword and
    /// is choosen as follows:
    /// <ul>
    /// <li> 0 - do nothing except reporting unused parameters at the exit (default value)
    /// <li> 1 - report undefined parameters (ones for which default values were used)
    /// <li> 2 - report all requested parameters.
    /// </ul>
    int verbose() const;
    /// return index entry in the named parameter if it exists or
    /// defvalue otherwise.
    template<typename DataType>
    DataType get(const string& name, DataType defvalue, unsigned int index = 0) const {
      const_iterator it = find(name);
      int verboselevel = verbose();
      if (it == end()) {
        // See if it is defined in the environment...
        const char* parenv = getenv(name.c_str());
        if (parenv) {
          return Convert::to<DataType>(parenv);
        }
        // Nope, return caller-supplied default value.
        if (verboselevel >= 1) {
          std::cout << "MapOptions: parameter " << name << "[" << index << "]"
                    << " is undefined; using default value of " << defvalue << std::endl;
        }
        return defvalue;
      } else {
        const vector_of_strings& entry = *it->second;
        if (index >= entry.size())    return defvalue;
        if (entry[index].size() == 0) return defvalue;
        DataType value = Convert::to<DataType>(entry[index]);
        if (verboselevel >= 2) {
          std::cout << "MapOptions: using " << name << "[" << index << "] = " << value << std::endl;
        }
        return value;
      }
    }

    /// Return # of components in the named vector or 0.
    unsigned int vector_size(const string& name) const;
    /// Return true iff the parameter defined by len characters of name exists either in this or in the environment.
    bool defined(const char* name, int len) const;
    /// Return true iff the named parameter exists either in this or in the environment.
    bool defined(const string& name) const;

    /// Remove the named parameter.
    void remove(const string& name);

    /// Parse named tokens defined by argv (see the detailed class description).
    /// If keep_all_filenames is false (default value), non-existing files will be deleted from 'files' group.
    bool parse(int argc, char* argv[], bool keep_all_filenames = false);
    /// Parse named tokens defined by a.
    bool parse(const char* a);

    /// Return true iff all characters of s before separator (" \t,;])" or 0) represents a number
    /// (using integer, float or exponential notation).
    bool isNumber(const char* s);
    /// Parse named tokens defined by argv (see the detailed class description).
    /// If keep_all_filenames is false (default value), non-existing files will be deleted from 'files' group.
    bool parse(int argc, const char* argv[], bool keep_all_filenames = false);
    /// Parse all named tokens in file filename (see the detailed class description).
    bool parse_file(const char* filename);
    /// const version.
    const vector_of_strings* operator[](const string& name) const;
    /// Return either pointer to the vector_of_strings defined by the name or pointer to the newly created empty vector.
    vector_of_strings* operator[](const string& name);
  protected:
    void remove_all_entries();
    /// Overload find operation to allow component names (like "nt; nthread; nthreads").
    /// This function will also check m_used_entries.
    /// \note Order of items in component name is important (first one is checked first, etc).
    iterator find(const string& name);
    const_iterator find(const string& name) const;
    iterator find(const char* name, int len);
    const_iterator find(const char* name, int len) const;

  protected:
    std::map<void*, bool> m_used_entries;

  };

  //ostream& operator<<(ostream& out, const MapOptions& mo);
  //ostream& operator<<(ostream& out, MapOptions::const_iterator it);
  //ostream& operator<<(ostream& out, MapOptions::iterator it);


  _INLINE ostream& operator<<(ostream& out, MapOptions::const_iterator it)
  {
    const vector_of_strings& vec = *it->second;
    unsigned int sz = vec.size();
    out << it->first << " = ";
    if (sz > 1) out << "[";
    unsigned int i = 0;
    while (true) {
      out << vec[i];
      if (++i == sz) break;
      out << ", ";
    }
    if (sz > 1) out << "]";
    out << ";" << endl;
    return out;
  }

  _INLINE ostream& operator<<(ostream& out, MapOptions::iterator it)
  {
    out << (MapOptions::const_iterator&)it;
    return out;
  }

  _INLINE ostream& operator<<(ostream& out, const MapOptions& mo)
  {
    MapOptions::const_iterator it;
    unsigned int maxwidth = 0;
    for (it = mo.begin(); it != mo.end(); it++) {
      maxwidth = max(maxwidth, (unsigned int)it->first.length());
    }
    maxwidth++;
    for (it = mo.begin(); it != mo.end(); it++) {
      cout.width(maxwidth);
      cout << it;
    }

    return out;
  }

  /// Allows access to this variable defined in Global.cxx.
  extern MapOptions options;
}

#endif

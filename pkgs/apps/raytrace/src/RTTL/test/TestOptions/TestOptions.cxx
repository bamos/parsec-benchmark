#include "RTTL/common/MapOptions.hxx"
#include <math.h>

using namespace RTTL;

MapOptions so;
/// MapOptions testbed example.
int main(int argc, const char* argv[]) {
    //goto command_line;

    // Three "first" entries.
    so.add("first", "1.0");
    so.add("first", 1.1f);
    so.add("first", 1.2);
    // Three "second" entries.
    so.add("second", 2);
    so.add("second", 2.1);
    so.add("second", 1.2e+30f);

    cout << "\"first\" size is " << so.vector_size("first") << endl;

    // let's get the first thing first.
    cout << so.get("first") << endl;                    // const char*, defvalue = arg
    cout << so.get("first", 0)+0 << endl;               // int
    cout << so.get("first", 0.0f)+0.0f << endl;         // float
    cout << so.get("first", "0") << endl;               // const char*, defvalue = "0"
    cout << so.get<const char*>("first", 0, 0) << endl; // will crush if first is not defined
    cout << so.get("1st") << endl;                      // OK, returns "1st"
    cout << so.get<const char*>("1st", "X", 0) << endl; // OK, returns "X"
    cout << so.get("first", -1.0f, 1)+0.0f << endl;
    cout << so.get("first", -1.0f, 2)+0.0f << endl;
    cout << so.get("1st", -1.0f, 2)+0.0f << endl;       // returns default value
    cout << so.get<string>("first", "empty", 2) << endl;
    cout << so.get<string>("first", "empty", 3) << endl;

    // And now "second" entry.
    cout << so.get<float>("second", -1, 0) << endl;
    cout << so.get("second", -1, 1) << endl;
    cout << 1.0e30 + so.get<float>("second", -1, 2) << endl;
    cout << so.get<int>("second", -1, 10) << endl;

    // Other possibilities.
    cout << (*so["second"])[1] << endl;
    (*so["second"])[1] = Convert::to_string(3.14f);
    cout << (*so["second"])[1] << endl;

    so["third"]->resize(3);
    (*so["third"])[0] = "one";
    (*so["third"])[1] = "two";
    (*so["third"])[2] = "three";

    // Get entry[0] as string (const char* arg converted to string name).
    cout << so.get("third") << endl;

    // Another way to add option.
    so["completed"]->push_back("setup");
    so["completed"]->push_back("computations");
    so["completed"]->push_back("exit");

    // Print out all entries...
    cout << so << endl << endl;

    // ... and clear them out.
    so.clear();
	//command_line:
    // Read parameters from the command line and print them out.
    so.parse(argc-1, argv+1);
    cout << so << endl << endl;

	// Sample command line:
	// echo > head.obj; echo > body.obj
	// bin/TestOptions -pbo=1 -pos [1, 2, 3] -up 0,0,1 +dir 5 +dir 6 +dir 7 -nthreads 3 -verb head.obj body.obj

    // Get environment value (defined in Windows and Linux as well).
    // Note: Under IDE with icc, it will not be defined (as a matter of fact),
    // but it will be defined if compiled using mvsc.
    cout << "USERNAME = " << so.get("USERNAME") << endl;
    // Try to get the value from the environment. get<> type is defined by defvalue.
    cout << "PROCESSOR_LEVEL = " << so.get("PROCESSOR_LEVEL", -1) << endl;

	cout << "PBO = " << so.get("pbo", 0) << endl;
	cout << "output is " << (so.defined("verbose, verb, all")? "verbose":"compact") << endl;
	cout << "pos is [" 
		 << so.get("pos", 0.0f, 0) << "," 
		 << so.get("pos", 0.0f, 1) << "," 
		 << so.get("pos", 0.0f, 2) << "]" << endl;

    if (so.defined("up")) {
        // Will result in abnormal termination if "up" is not defined.
        (*so["up"])[1] = Convert::to_string(-1); // change 2nd entry
    }

	cout << "up  is " << so.getVec3f("up") << endl;
	cout << "dir is " << so.getVec3f("dir") << endl;
	cout << so.get("nthreads") << endl; // returns string("3")
	so.remove("nthreads");
	cout << so.get("nthreads") << endl; // returns the name1

    int nfiles = so.vector_size("files");
    for (int fi = 0; fi < nfiles; fi++) {
        const string& fn = (*so["files"])[fi];
        cout << "Adding obj file: " << fn << endl;
    }


	// Test namespace Convert functions.
	string pis = "pi is " + Convert::to_string(4 * atan2(1.0f, 1.0f));
	float  pif = Convert::to<float>(pis.c_str() + 6);
	int    pii = Convert::to<int>  (pis.c_str() + 6);
	cout << "pi is " << pif << "; rounded to " << pii << endl;

    return 0;
}

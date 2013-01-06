#ifdef AUTOLIB
AUTOLIB(datalog)
#endif

typedef struct DataLog DataLog;
typedef struct DataLogIter DataLogIter;
typedef struct _cass_datum_t cass_datum_t;

enum {
	DataLogVersion	= 1,
	DataLogItemMax	= 1<<22,
};

/*
 * All log records must be less than DataLogItemMax in size.
 *
 * All functions returning an int return 0 on success and -1 on failure.
 *
 * DataLogclose closes at log handle.
 * DataLogtruncate truncates the log to the specified LSN.
 * DataLogappend appends a cass_datum_t to the log and sets the new LSN.
 * DataLogmaxlsn returns the current maximum LSN.
 *
 * Mkdatalogiter creates a new log iterator; datalogiterclose closes it.
 * Datalogitersetlsn positions a log iterator at the specified LSN.
 * Datalogiternext returns the next log record and associated LSN.
 */

DataLog         *datalogopen(int omode, char *fname, int cr, 
                  void (*panic)(char*, ...), uint64_t lsn, uint64_t offset);
int             datalogclose(DataLog*);
int             datalogsync(DataLog*);  // Sync datalog to disk.
int             datalogtruncate(DataLog*, uint64_t offset);
int             datalogappend(DataLog*, cass_datum_t* data);
int             datalogmaxlsn(DataLog*, uint64_t* lsn, uint64_t* offset);
                  // Return current LSN and offset.

DataLogIter     *mkdatalogiter(DataLog*);
void            datalogiterclose(DataLogIter*);
//int           datalogitersetlsn(DataLogIter*, uint64_t); // Not supported.
int             datalogiternext(DataLogIter*, uint64_t*, cass_datum_t*);


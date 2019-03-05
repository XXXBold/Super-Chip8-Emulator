#ifndef APPCONFIG_H_INCLUDED
  #define APPCONFIG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  DATA_TYPE_INT,
  DATA_TYPE_UINT,
  DATA_TYPE_DOUBLE,
  DATA_TYPE_BOOLEAN,
  DATA_TYPE_STRING
}EDataType;

/**
 * Structure to define diffrent entries for the config. <br>
 * define an array of this and initialise with default values <br>
 * before calling appConfig_Load. Make sure this is valid the whole time <br>
 * while this library is in use. <br>
 * An Example to show the (global) definition of an array containg some config: <br>
 *
 * char gCharArray[100]="Test initial String";
 *
 * AppConfigEntry taEntrys_m[]={{NULL, "ValInt",    DATA_TYPE_INT,     sizeof(int),          .data={.iVal=10}},
 *                              {NULL, "ValUint",   DATA_TYPE_UINT,    sizeof(unsigned int), .data={.uiVal=1010}},
 *                              {NULL, "ValBool",   DATA_TYPE_BOOLEAN, sizeof(int),          .data={.bVal=1}},
 *                              {NULL, "ValDouble", DATA_TYPE_DOUBLE,  sizeof(double),       .data={.dVal=1234.567}},
 *                              {NULL, "ValString", DATA_TYPE_STRING,  sizeof(gCharArray),   .data={.pcVal=gCharArray}},
 *                             };
 */
typedef struct
{
  /**
   * Used to group the config. Can be NULL if not needed. <br>
   */
  const char *groupName;
  /**
   * The keyName to identify the entry. Must be uniqe per group.
   */
  const char *keyName;
  /**
   * @see enum EDataType, defines what datatype union data contains.
   */
  EDataType dataType;
  /**
   * Size of the current data, for now this is only important for Strings. <br>
   * If dataType is set to String, this is the usable size of the buffer pointed by data.pcVal. <br>
   * For other types use sizeof(type) to initialize.
   */
  size_t dataSize;
  /**
   * Union to store the diffrent datatypes. <br>
   * If String (pcVal) is used, make sure it points to a valid location, <br>
   * that will stay valid during this library is used.
   */
  union DataType_T
  {
    int iVal;
    unsigned int uiVal;
    double dVal;
    int bVal;
    char *pcVal;
  }data;
}AppConfigEntry;

typedef struct TagAppConfig_T* AppConfig;


/**
 * Loads a config, if exist, or creates a new one.
 *
 * @param appName  _IN_ The name of the app, can't be NULL.
 * @param entries  _IN_ Pointer to AppConfigEntry struct defined by the user.
 *                 Only a reference to this will be stored internal, <br>
 *                 make sure it's contents are valid until appConfig_Close(), <br>
 *                 so best would be define this on heap. Should be initialized before Loading the config.
 * @param entriesCount
 *                 _IN_ Count of param entries.
 * @param fileName _IN_OPT_ Filename for the config file, NULL for default name.
 * @param location _IN_OPT_ Overwrite the default Location for configfiles by specify this parameter. <br>
 *                 Pass NULL to use the default location of your Platform. <br>
 *                 Example: Default on Windows is: "C:User\<user>\AppData".
 *
 * @return New config on success, NULL on failure.
 */
extern AppConfig appConfig_Load(const char *appName,
                                AppConfigEntry *entries,
                                size_t entriesCount,
                                const char *fileName,
                                const char *location);

/**
 * Saves the configuration to disk.
 *
 * @param config _IN_ The config, created by calling appConfig_Load.
 *
 * @return 0 on success, nonzero on failure.
 */
extern int appConfig_Save(AppConfig config);

/**
 * Returns the current configuration path
 *
 * @param config The config
 *
 * @return The Configuration path which is currently set.
 */
extern const char *appConfig_GetPath(AppConfig config);

/**
 * Clears up the internal data and frees all memory used for the config.
 * Don't use the config afther that anymore.
 *
 * @param config _IN_ The config object to clean up
 */
extern void appConfig_Close(AppConfig config);


#ifdef __cplusplus
}
#endif

#endif /* APPCONFIG_H_INCLUDED */

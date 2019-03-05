#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "appconfig.h"
#include "inifile.h"

#ifdef _WIN32

  #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
  #undef WIN32_LEAN_AND_MEAN
  #define PATH_SEPARATOR '\\'
  #define ENV_VARNAME_1 "APPDATA"
#elif __linux__
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #define PATH_SEPARATOR '/'
  #define ENV_VARNAME_1 "XDG_CONFIG_HOME"
  #define ENV_VARNAME_2 "HOME"
  #define LINUX_CFG_DIR ".config"
#endif

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT static
  #define INLINE_PROT static
#endif /* __STDC_VERSION__ >= C99 */

/* TODO: Test this on linux/other os than Windows! */

#define DEFAULT_FILENAME "config.ini"

enum
{
  MAX_PATHLEN=260,
};

struct TagAppConfig_T
{
  char caPath[MAX_PATHLEN+1];
  AppConfigEntry *pEntries;
  size_t szEntriesCount;
  union StoreMethod_T /* Here add probably more methods for storage, e.g. registry for windws... */
  {
    Inifile iniFile;
  }store;
};

INLINE_PROT int iAppConfig_AssembleDestFolderPath_m(AppConfig pCfg,
                                                    unsigned int *puiBufSize,
                                                    const char *pcAppName,
                                                    const char *pcFileName,
                                                    const char *pcLocation);
INLINE_PROT int iAppConfig_CreateNew_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_GetDefaultConfigPath_m(char *pcPath,
                                                  unsigned int *uiBufsize);
INLINE_PROT int iAppConfig_FileExists_m(const char *pcPath);
INLINE_PROT int iAppConfig_CreateDirIfNotExist_m(const char *pcDir);
INLINE_PROT int iAppConfig_Inifile_Load_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_Inifile_Save_m(AppConfig pCfg);

AppConfig appConfig_Load(const char *appName,
                         AppConfigEntry *entries,
                         size_t entriesCount,
                         const char *fileName,
                         const char *location)
{
  unsigned int uiTmp;
  AppConfig pNewAppCfg;

  if((!entries) || (!entriesCount))
    return(NULL);

  if(!(pNewAppCfg=malloc(sizeof(struct TagAppConfig_T))))
  {
    fputs("malloc() failed\n",stderr);
    return(NULL);
  }

  if(!fileName)
    fileName=DEFAULT_FILENAME;

  uiTmp=MAX_PATHLEN;
  if(iAppConfig_AssembleDestFolderPath_m(pNewAppCfg,
                                         &uiTmp,
                                         appName,
                                         fileName,
                                         location))
  {
    fputs("iAppConfig_AssembleDestFolderPath_m() failed\n",stderr);
    free(pNewAppCfg);
    return(NULL);
  }
  if(iAppConfig_CreateDirIfNotExist_m(pNewAppCfg->caPath)<0)
  {
    fputs("iAppConfig_CreateDirIfNotExist_m() failed\n",stderr);
    free(pNewAppCfg);
    return(NULL);
  }
  /* bufferlength is checked in AssembleDestFolderPath, so strcpy is fine */
  strcpy(&pNewAppCfg->caPath[uiTmp],fileName);
  pNewAppCfg->pEntries=entries;
  pNewAppCfg->szEntriesCount=entriesCount;
  if(!iAppConfig_FileExists_m(pNewAppCfg->caPath))
  {
    if(iAppConfig_CreateNew_m(pNewAppCfg))
    {
      free(pNewAppCfg);
      return(NULL);
    }
    return(pNewAppCfg);
  }
  else if(iAppConfig_Inifile_Load_m(pNewAppCfg))
  {
    free(pNewAppCfg);
    return(NULL);
  }
  return(pNewAppCfg);
}

int appConfig_Save(AppConfig config)
{
  return(iAppConfig_Inifile_Save_m(config));
}

const char *appConfig_GetPath(AppConfig config)
{
  return(config->caPath);
}

void appConfig_Close(AppConfig config)
{
  IniFile_Delete(config->store.iniFile);
  free(config);
}

INLINE_FCT int iAppConfig_Inifile_Load_m(AppConfig pCfg)
{
  void *pParam;
  AppConfigEntry *pCurrEntry;
  unsigned int uiIndex;
  EIniDataType eIniType;
  if(!(pCfg->store.iniFile=IniFile_Read(pCfg->caPath)))
  {
    fputs("IniFile_Read() failed",stderr);
    return(-1);
  }
  for(uiIndex=0;uiIndex<pCfg->szEntriesCount;++uiIndex)
  {
    pCurrEntry=&pCfg->pEntries[uiIndex];
    pParam=&pCurrEntry->data;
    switch(pCurrEntry->dataType)
    {
      case DATA_TYPE_INT:
        eIniType=eIniDataType_Int;
        break;
      case DATA_TYPE_UINT:
        eIniType=eIniDataType_Uint;
        break;
      case DATA_TYPE_DOUBLE:
        eIniType=eIniDataType_Double;
        break;
      case DATA_TYPE_BOOLEAN:
        eIniType=eIniDataType_Boolean;
        break;
      case DATA_TYPE_STRING:
        pParam=pCurrEntry->data.pcVal;
        eIniType=eIniDataType_String;
        break;
      default:
        fprintf(stderr,
                "iAppConfig_Inifile_Load_m() Unknown datatype for key %s: %d\n",
                pCurrEntry->keyName,
                pCurrEntry->dataType);
        return(-1);
    }
    if(IniFile_FindEntry_GetValue(pCfg->store.iniFile,
                                  pCurrEntry->groupName,
                                  pCurrEntry->keyName,
                                  eIniType,
                                  pParam,
                                  pCurrEntry->dataSize))
    {
      fprintf(stderr,
              "Key \"%s\" in group \"%s\" not found\n",
              pCurrEntry->keyName,
              pCurrEntry->groupName);
    }
  }
  return(0);
}

INLINE_FCT int iAppConfig_Inifile_Save_m(AppConfig pCfg)
{
  void *pParam;
  AppConfigEntry *pCurrEntry;
  unsigned int uiIndex;
  EIniDataType eIniType;
  for(uiIndex=0;uiIndex<pCfg->szEntriesCount;++uiIndex)
  {
    pCurrEntry=&pCfg->pEntries[uiIndex];
    pParam=&pCurrEntry->data;
    switch(pCurrEntry->dataType)
    {
      case DATA_TYPE_INT:
        eIniType=eIniDataType_Int;
        break;
      case DATA_TYPE_UINT:
        eIniType=eIniDataType_Uint;
        break;
      case DATA_TYPE_DOUBLE:
        eIniType=eIniDataType_Double;
        break;
      case DATA_TYPE_BOOLEAN:
        eIniType=eIniDataType_Boolean;
        break;
      case DATA_TYPE_STRING:
        pParam=pCurrEntry->data.pcVal;
        eIniType=eIniDataType_String;
        break;
      default:
        fprintf(stderr,"iAppConfig_Inifile_Save_m() Unknown datatype: %d\n",pCurrEntry->dataType);
        return(-1);

    }
    if(IniFile_CreateEntry_SetValue(pCfg->store.iniFile,
                                    pCfg->pEntries[uiIndex].groupName,
                                    pCfg->pEntries[uiIndex].keyName,
                                    eIniType,
                                    pParam))
    {
      fprintf(stderr,
              "failed to set val for Key %s in group %s\n",
              pCfg->pEntries[uiIndex].keyName,
              pCfg->pEntries[uiIndex].groupName);
    }
  }
//IniFile_DumpContent(pCfg->store.iniFile);
  if(IniFile_Write(pCfg->store.iniFile,pCfg->caPath))
  {
    fprintf(stderr,"Failed to store inifile \"%s\"\n",pCfg->caPath);
    return(-1);
  }
  return(0);
}

INLINE_FCT int iAppConfig_AssembleDestFolderPath_m(AppConfig pCfg,
                                                   unsigned int *puiBufSize,
                                                   const char *pcAppName,
                                                   const char *pcFileName,
                                                   const char *pcLocation)
{
  unsigned int uiTmp;
  if(!pcAppName) /* Appname must be specified */
    return(-1);

  if(!pcLocation)
  {
    uiTmp=*puiBufSize;
    if(iAppConfig_GetDefaultConfigPath_m(pCfg->caPath,
                                         &uiTmp))
    {
      fputs("iAppConfig_GetDefaultPath_m() failed\n",stderr);
      return(-1);
    }
  }
  else
  {
    if((uiTmp=strlen(pcLocation)+1)> *puiBufSize-2)
      return(-1);
    memcpy(pCfg->caPath,pcLocation,uiTmp);
  }
  /* Check if buffer is big enough to hold the whole path */
  if(uiTmp + strlen(pcAppName) + strlen(pcFileName) > *puiBufSize-2)
  {
    fputs("Error: Buffer for path too small, would exceed\n",stderr);
    return(-1);
  }
  if(pCfg->caPath[uiTmp-1]!=PATH_SEPARATOR) /* Add path separator if needed */
  {
    pCfg->caPath[uiTmp++]=PATH_SEPARATOR;
  }
  strcpy(&pCfg->caPath[uiTmp],pcAppName);
  uiTmp=strlen(pCfg->caPath);
  if(pCfg->caPath[uiTmp-1]!=PATH_SEPARATOR) /* Add path separator if needed */
  {
    pCfg->caPath[uiTmp++]=PATH_SEPARATOR;
    pCfg->caPath[uiTmp]='\0';
  }
  *puiBufSize=uiTmp;
  return(0);
}

INLINE_FCT int iAppConfig_CreateNew_m(AppConfig pCfg)
{
  if(!(pCfg->store.iniFile=IniFile_New()))
    return(-1);

  return(0);
}

INLINE_FCT int iAppConfig_GetDefaultConfigPath_m(char *pcPath,
                                                 unsigned int *puiBufSize)
{
  char *pcTmp;
  unsigned int uiTmp=0;
#ifdef _WIN32
  if((!(pcTmp=getenv(ENV_VARNAME_1))) || (pcTmp[0]=='\0'))
    return(-1);

  uiTmp=strlen(pcTmp);
  if(uiTmp+2>*puiBufSize)
    return(-1);
  memcpy(pcPath,pcTmp,uiTmp+1);
  *puiBufSize=uiTmp;
  #elif __linux__

  #ifdef _GNU_SOURCE
    #define GETENV(env) secure_getenv(env)
  #else /* Fallback to default getenv function... */
    #define GETENV(env) getenv(env)
  #endif

  /* Look for XDG_CONFIG_HOME Environment var first */
  if((pcTmp=GETENV(ENV_VARNAME_1)) && (pcTmp[0]!='\0'))
  {
    uiTmp=strlen(pcTmp);
    if(uiTmp+2>*puiBufSize)
      return(-1);

    memcpy(pcPath,pcTmp,uiTmp+1);
    *puiBufSize=uiTmp;
  }/* As 2nd option, look for home, and add .config folder manually */
  else if((pcTmp=GETENV(ENV_VARNAME_2)) && (pcTmp[0]!='\0'))
  {
    uiTmp=strlen(pcTmp);
    if(uiTmp+sizeof(LINUX_CFG_DIR)+2>*puiBufSize)
      return(-1);
    memcpy(pcPath,pcTmp,uiTmp);
    pcPath[uiTmp++]=PATH_SEPARATOR;
    memcpy(&pcPath[uiTmp],LINUX_CFG_DIR,sizeof(LINUX_CFG_DIR));
    *puiBufSize=uiTmp+sizeof(LINUX_CFG_DIR);
  }
  else
    return(-1); /* Env not found or empty */
#else
  #error "Unknown platform, can't determine config path"
#endif
  return(0);
}

INLINE_FCT int iAppConfig_FileExists_m(const char *pcPath)
{
  FILE *fp;
  if(!(fp=fopen(pcPath,"r")))
    return(0);
  fclose(fp);
  return(1);
}

INLINE_FCT int iAppConfig_CreateDirIfNotExist_m(const char *pcDir)
{
#ifdef _WIN32
  if(CreateDirectory(pcDir,NULL))
    return(1);
  if(GetLastError()==ERROR_ALREADY_EXISTS)
    return(0);
  return(-1);
#elif __linux__
  errno=0;
  if(mkdir(pcDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0)
    return(1);
  if(errno==EEXIST)
    return(0);
  return(-1);
#endif
}

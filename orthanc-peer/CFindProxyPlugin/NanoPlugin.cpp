#include <Plugins/Samples/Common/OrthancPluginCppWrapper.h>      
#include <Core/SystemToolbox.h>   
#include <Core/Toolbox.h>   
#include <Core/Logging.h>  
#include <json/value.h>
#include <json/reader.h>  
#include <string>  
#include <Plugins/Include/orthanc/OrthancCPlugin.h>

ORTHANC_PLUGINS_API OrthancPluginErrorCode OnFindCallback (OrthancPluginFindAnswers *answers,
     const OrthancPluginFindQuery *query, const char *issuerAet, const char *calledAet)
{
  OrthancPluginContext* context = OrthancPlugins::GetGlobalContext(); 
  OrthancPluginMemoryBuffer temp2;

  //std::string s = std::to_string(taggroup);
  //char const *pchar = s.c_str();
  /*
  OrthancPlugins::MemoryBuffer dicom;
  Json::Value json;
  dicom.GetDicomQuery((OrthancPluginWorklistQuery*) query);
  dicom.DicomToJson(json, OrthancPluginDicomToJsonFormat_Short, static_cast<OrthancPluginDicomToJsonFlags>(0), 0);
  char* post_body = OrthancPluginDicomBufferToJson(context,
                                                    query,
                                                    sizeof(query),
                                                    OrthancPluginDicomToJsonFormat_Full, 
                                                    OrthancPluginDicomToJsonFlags_IncludePrivateTags, 
                                                    0);
  
  OrthancPlugins::LogInfo("Received query from remote modality " +
                          std::string(issuerAet) + ":\n" + json.toStyledString());
  */
  
  std::string json_body = "{";

  for (uint16_t i = 0; i < OrthancPluginGetFindQuerySize(context, query); i++)
  {  
    uint16_t  taggroup, tagelement;
    uint16_t* taggroup_ptr = &taggroup;
    uint16_t* tagelement_ptr = &tagelement;
    OrthancPluginGetFindQueryTag(context, taggroup_ptr, tagelement_ptr, query, i);
    char* tagvalue = OrthancPluginGetFindQueryValue(context, query, i);

    std::stringstream taggroup_str, tagelement_str;
    taggroup_str << std::setfill('0') << std::setw(4) << std::hex << taggroup;
    tagelement_str << std::setfill('0') << std::setw(4) << std::hex << tagelement;

    std::string json_tuple = "\"" + taggroup_str.str() + "," + tagelement_str.str() + "\"" + ":" 
                              + "\"" + tagvalue + "\"";
    if(i < OrthancPluginGetFindQuerySize(context, query)-1) {
      json_tuple = json_tuple + ", ";
    }
    json_body = json_body + json_tuple;
  }
  json_body = json_body + "}";
  OrthancPluginLogWarning(context, json_body.c_str()); 

  OrthancPluginErrorCode enhanceError = OrthancPluginRestApiPostAfterPlugins(context, 
                                                                             &temp2,
                                                                             "/enhancequery",
                                                                             json_body.c_str(),
                                                                             strlen(json_body.c_str()));
  if (enhanceError) {
    return enhanceError;
  }
  OrthancPluginFreeMemoryBuffer(context, &temp2);
  return OrthancPluginErrorCode_Success;
}

extern "C"
{    
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* context)     
  {    
    Orthanc::Logging::Initialize(context);    
    OrthancPlugins::SetGlobalContext(context);   
    OrthancPluginSetDescription(context, "Orthanc Plugin Enhance Incoming C-Find queries");
    OrthancPluginRegisterFindCallback(context, (OrthancPluginFindCallback) OnFindCallback);
    return 0;   
  }    
  ORTHANC_PLUGINS_API void OrthancPluginFinalize()    
  {   
    LOG(INFO) << "NanoPlugin is finalizing";   
  }    
  ORTHANC_PLUGINS_API const char* OrthancPluginGetName()    
  {    
    return "NanoPlugin";    
  }     
  ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()    
  {    
    return "1.0.0";    
  }    
} 
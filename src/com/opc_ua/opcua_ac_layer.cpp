

#include "opcua_ac_layer.h"
#include "../../core/utils/parameterParser.h"
#include "../../core/cominfra/basecommfb.h"
#include "opcua_local_handler.h"
#include "opcua_objectstruct_helper.h"

using namespace forte::com_infra;

const std::string COPC_UA_AC_Layer::scmModeINITMSG = "INITMSG";
const std::string COPC_UA_AC_Layer::scmModeINITUSERTEXT = "INITUSERTEXT";
const std::string COPC_UA_AC_Layer::scmModeTRIGGER = "TRIGGER";
const std::string COPC_UA_AC_Layer::scmTypeALARM = "Alarm";
const std::string COPC_UA_AC_Layer::scmTypeEVENT = "Event";

const std::string COPC_UA_AC_Layer::scmAlarmTypeBrowsePath = "/Types/0:EventTypes/0:BaseEventType/0:ConditionType/0:AcknowledgeableConditionType/0:AlarmConditionType/%d:";
const std::string COPC_UA_AC_Layer::scmEventTypeBrowsePath = "/Types/0:EventTypes/0:BaseEventType/%d:";
const std::string COPC_UA_AC_Layer::scmUserTextAttribute = "TEXT0%d";

char COPC_UA_AC_Layer::smEmptyString[] = "";

COPC_UA_AC_Layer::COPC_UA_AC_Layer(CComLayer *paUpperLayer, CBaseCommFB *paComFB) :
  CComLayer(paUpperLayer, paComFB), mHandler(nullptr) {
  
}

COPC_UA_AC_Layer::~COPC_UA_AC_Layer() {
  UA_NodeId_clear(&mTypeNodeId);
  for(char *typeName : mTypeNames) {
    delete[] typeName;
  }
  for(UA_NodeId nodeId : mTypePropertyNodes) {
    UA_NodeId_clear(&nodeId);
  }
  UA_NodeId_clear(&mTypeNodeId);
  UA_NodeId_clear(&mConditionSourceId);
  UA_NodeId_clear(&mConditionInstanceId);
}

EComResponse COPC_UA_AC_Layer::openConnection(char *paLayerParameter) {
  EComResponse eRetVal = e_InitTerminated;
  CParameterParser parser(paLayerParameter, ';', scmNumberOfParameters);
  size_t nrOfParams = parser.parseParameters();
  if(nrOfParams == scmNumberOfParameters) {
    std::string mode = parser[Mode];
    mHandler = static_cast<COPC_UA_HandlerAbstract*>(&getExtEvHandler<COPC_UA_Local_Handler>());
    if(mode == scmModeINITMSG || mode == scmModeINITUSERTEXT) {
      eRetVal = initOPCUAType(mode, parser[InitType], parser[TypeName]);
    } else if (mode == scmModeTRIGGER) {
      eRetVal = createOPCUAObject(parser[InitType], parser[TypeName]);
    } else {
      DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong usage of layer parameters! Expected first param: %s | %s | %s, actual: %s\n", scmModeINITMSG.c_str(), scmModeINITUSERTEXT.c_str(), scmModeTRIGGER.c_str(), mode.c_str());
      return eRetVal;   
    }
  }
  return eRetVal;
}

void COPC_UA_AC_Layer::closeConnection() {
  // TODO
}

EComResponse COPC_UA_AC_Layer::recvData(const void*, unsigned int) {
  return e_ProcessDataOk;
}

EComResponse COPC_UA_AC_Layer::sendData(void*, unsigned int) {
  return e_ProcessDataOk;
}

EComResponse COPC_UA_AC_Layer::processInterrupt() {
  // TODO
  return e_ProcessDataOk;
}

EComResponse COPC_UA_AC_Layer::initOPCUAType(const std::string &paMode, const std::string &paType, const std::string &paTypeName) {
  EComResponse eRetVal = e_InitTerminated;
  COPC_UA_Local_Handler* localHandler = static_cast<COPC_UA_Local_Handler*>(mHandler);
  localHandler->enableHandler();
  UA_Server *server = localHandler->getUAServer();
  if(paType == scmTypeALARM) {
    eRetVal = createAlarmType(server, paTypeName);
  } else if(paType == scmTypeEVENT) {
    eRetVal = createEventType(server, paTypeName);
  } else {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong usage of layer parameters! Expected second param: %s/%s, actual: %s\n", scmTypeALARM.c_str(), scmTypeEVENT.c_str(), paType.c_str());
    return e_InitTerminated;
  }
    return eRetVal == e_InitOk ? addOPCUATypeProperties(server, paMode, paTypeName) : eRetVal;
}

EComResponse COPC_UA_AC_Layer::createOPCUAObject(const std::string &paType, const std::string &paTypeName) {
  std::string browsePathPrefix;
  if(paType == scmTypeALARM) {
    browsePathPrefix = scmAlarmTypeBrowsePath;
  } else if(paType == scmTypeEVENT) {
    browsePathPrefix = scmAlarmTypeBrowsePath;
  } else {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong usage of layer parameters! Expected second param: %s/%s, actual: %s\n", scmTypeALARM.c_str(), scmTypeEVENT.c_str(), paType.c_str());
    return e_InitTerminated;
  }
  std::string browsePath(COPC_UA_ObjectStruct_Helper::getBrowsePath(browsePathPrefix, paTypeName, 1));  // TODO Change 1 to namespaceIndex
  if(!isOPCUAObjectPresent(browsePath)) {
    return e_InitTerminated;
  }
  COPC_UA_Local_Handler* localHandler = static_cast<COPC_UA_Local_Handler*>(mHandler);
  localHandler->enableHandler();
  UA_Server *server = localHandler->getUAServer();
  if(createOPCUAObjectNode(server) != UA_STATUSCODE_GOOD) {
    return e_InitTerminated;
  }
  if(addOPCUACondition(server) != UA_STATUSCODE_GOOD) {
    return e_InitTerminated;
  }
  return e_InitOk;
}

UA_StatusCode COPC_UA_AC_Layer::createOPCUAObjectNode(UA_Server *paServer) {
  std::string instanceNameStr(getFBNameFromConnection());
  if(instanceNameStr.empty()) {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Retrieving FB Instance Name failed!");
    return UA_STATUSCODE_BAD;
  }
  char *instanceName = getNameFromString(instanceNameStr);
  UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    attr.eventNotifier = UA_EVENTNOTIFIER_SUBSCRIBE_TO_EVENT;
    attr.displayName = UA_LOCALIZEDTEXT(smEmptyString, instanceName);
    UA_StatusCode status =  UA_Server_addObjectNode(paServer, UA_NODEID_NULL,
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                      UA_QUALIFIEDNAME(1, instanceName),            // TODO Change 1 to namespaceIndex
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                      attr, NULL, &mConditionSourceId);

    if(status != UA_STATUSCODE_GOOD) {
      DEVLOG_ERROR("[OPC UA A&C LAYER]: Creating Object Node failed. StatusCode %s\n", UA_StatusCode_name(status));
      return status;
    }
    status = UA_Server_addReference(paServer, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER),
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_HASNOTIFIER),
                                     UA_EXPANDEDNODEID_NUMERIC(mConditionSourceId.namespaceIndex,
                                                               mConditionSourceId.identifier.numeric),
                                     UA_TRUE);
    if(status != UA_STATUSCODE_GOOD) {
      DEVLOG_ERROR("[OPC UA A&C LAYER]: Adding reference to Object Node failed. StatusCode %s\n", UA_StatusCode_name(status));
    }
    return status;
}

UA_StatusCode COPC_UA_AC_Layer::addOPCUACondition(UA_Server *paServer) {
  char *conditionName = getNameFromString(std::string("Condition"));
  UA_StatusCode status = UA_Server_createCondition(paServer, UA_NODEID_NULL, mTypeNodeId,
                                  UA_QUALIFIEDNAME(1, conditionName), mConditionSourceId,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                  &mConditionInstanceId);
  if(status != UA_STATUSCODE_GOOD) {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Adding Condition failed. StatusCode %s\n", UA_StatusCode_name(status));  
  }
  return status;
}

EComResponse COPC_UA_AC_Layer::createAlarmType(UA_Server *paServer, const std::string &paTypeName) {
  std::string browsePath(COPC_UA_ObjectStruct_Helper::getBrowsePath(scmAlarmTypeBrowsePath, paTypeName, 1));  // TODO Change 1 to namespaceIndex
  if(isOPCUAObjectPresent(browsePath)) {
    return e_InitOk;
  }
  char* typeName = getNameFromString(paTypeName);
  mTypeNodeId = UA_NODEID_STRING(1, typeName);          // TODO Change 1 to namespaceIndex
  UA_ObjectTypeAttributes oAttr = UA_ObjectTypeAttributes_default;
  oAttr.displayName = UA_LOCALIZEDTEXT(smEmptyString, typeName);
  UA_StatusCode status = UA_Server_addObjectTypeNode(paServer, mTypeNodeId,
    UA_NODEID_NUMERIC(0, UA_NS0ID_ALARMCONDITIONTYPE),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
    UA_QUALIFIEDNAME(1, typeName), oAttr,             // TODO Change 1 to namespaceIndex
    nullptr, &mTypeNodeId);

  if (status != UA_STATUSCODE_GOOD) {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Failed to create OPC UA Alarm Type Node for Type %s, Status Code: %s\n", paTypeName.c_str(), UA_StatusCode_name(status));
    return e_InitTerminated;
  }
  return e_InitOk;
}

EComResponse COPC_UA_AC_Layer::createEventType(UA_Server *paServer, const std::string &paTypeName) {
  std::string browsePath(COPC_UA_ObjectStruct_Helper::getBrowsePath(scmEventTypeBrowsePath, paTypeName, 1));  // TODO Change 1 to namespaceIndex
  if(isOPCUAObjectPresent(browsePath)) {
    return e_InitOk;
  }
  char* typeName = getNameFromString(paTypeName);
  mTypeNodeId = UA_NODEID_STRING(1, typeName);          // TODO Change 1 to namespaceIndex
  UA_ObjectTypeAttributes oAttr = UA_ObjectTypeAttributes_default;
  oAttr.displayName = UA_LOCALIZEDTEXT(smEmptyString, typeName);
  UA_StatusCode status = UA_Server_addObjectTypeNode(paServer, mTypeNodeId,
    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
    UA_QUALIFIEDNAME(1, typeName), oAttr,             // TODO Change 1 to namespaceIndex
    nullptr, &mTypeNodeId);

  if (status != UA_STATUSCODE_GOOD) {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Failed to create OPC UA Event Type Node for Type %s, Status Code: %s\n", paTypeName.c_str(), UA_StatusCode_name(status));
    return e_InitTerminated;
  }
  return e_InitOk;
}

EComResponse COPC_UA_AC_Layer::addOPCUATypeProperties(UA_Server *paServer, const std::string &paMode, const std::string &paTypeName) {
  EComResponse eRetVal = e_InitTerminated;
  bool isPublisher;
  switch (mFb->getComServiceType()) {
  case e_Publisher:
    isPublisher = true;
    break;
  case e_Subscriber: 
    isPublisher = false;
    break;
  default:
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong CommFB used for FB %s! Expected: Publish/Subscribe\n", getCommFB()->getInstanceName());
    return e_InitTerminated;
  }

  if(paMode == scmModeINITMSG) {
    eRetVal = addOPCUATypeMSGProperties(paServer, paTypeName, isPublisher);
  } else if (paMode == scmModeINITUSERTEXT) {
    eRetVal = addOPCUATypeUSERProperties(paServer, paTypeName, isPublisher);
  } else {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong mode specified for FB %s, Mode: %s\n", getCommFB()->getInstanceName(), paMode.c_str());
  }
  return eRetVal;
}

forte::com_infra::EComResponse COPC_UA_AC_Layer::addOPCUATypeMSGProperties(UA_Server *paServer, const std::string &paParentTypeName, bool paIsPublisher) {
  CIEC_ANY **apoDataPorts = paIsPublisher ? getCommFB()->getSDs() : getCommFB()->getRDs();
  size_t numDataPorts = paIsPublisher ? getCommFB()->getNumSD() : getCommFB()->getNumRD();
  const SFBInterfaceSpec *interfaceSpec = getCommFB()->getFBInterfaceSpec();
  const CStringDictionary::TStringId *dataPortNameIds = paIsPublisher ? interfaceSpec->mDINames : interfaceSpec->mDONames;
  for(size_t i = 0; i < numDataPorts; i++) {
    std::string dataPortName = getPortNameFromConnection(dataPortNameIds[i+2], paIsPublisher);
    char* propertyName = getNameFromString(dataPortName);
    UA_StatusCode status = addVariableNode(paServer, paParentTypeName, propertyName, apoDataPorts[i]->unwrap());
    if(status != UA_STATUSCODE_GOOD) {
      DEVLOG_ERROR("[OPC UA A&C LAYER]: Failed to add OPCUA MSG Property for FB %s, Port: %s, Status: %s\n", getCommFB()->getInstanceName(), dataPortName, UA_StatusCode_name(status));
      return e_InitTerminated;
    }
  }
  return e_InitOk;
}

forte::com_infra::EComResponse COPC_UA_AC_Layer::addOPCUATypeUSERProperties(UA_Server *paServer, const std::string &paParentTypeName, bool paIsPublisher) {
  CIEC_ANY **apoDataPorts = paIsPublisher ? getCommFB()->getSDs() : getCommFB()->getRDs();
  size_t numDataPorts = paIsPublisher ? getCommFB()->getNumSD() : getCommFB()->getNumRD();
  for(size_t i = 0; i < numDataPorts; i++) {
    char* propertyName = new char[scmUserTextAttribute.length() + 2];
    snprintf(propertyName, scmUserTextAttribute.length() + 2, scmUserTextAttribute.c_str(), i+1);
    propertyName[scmUserTextAttribute.length() + 1] = '\0';
    mTypeNames.emplace_back(propertyName);

    UA_StatusCode status = addVariableNode(paServer, paParentTypeName, propertyName, apoDataPorts[i]->unwrap());
    if(status != UA_STATUSCODE_GOOD) {
      DEVLOG_ERROR("[OPC UA A&C LAYER]: Failed to add OPCUA MSG Property for FB %s, Port: %s, Status: %s\n", getCommFB()->getInstanceName(), propertyName, UA_StatusCode_name(status));
      return e_InitTerminated;
    }
  }
  return e_InitOk;
}

UA_StatusCode COPC_UA_AC_Layer::addVariableNode(UA_Server *paServer, const std::string &paParentTypeName, char *paVariableName, CIEC_ANY &paVariableType) {
  std::string propertyBrowsePathStr = COPC_UA_ObjectStruct_Helper::getMemberBrowsePath(paParentTypeName, paVariableName);
  char* propertyBrowsePath = getNameFromString(propertyBrowsePathStr);

  UA_VariableAttributes vAttr = UA_VariableAttributes_default;
    vAttr.displayName = UA_LOCALIZEDTEXT(smEmptyString, paVariableName);
    vAttr.valueRank = UA_VALUERANK_SCALAR;
    vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    vAttr.dataType = COPC_UA_Helper::getOPCUATypeFromAny(paVariableType)->typeId;

  UA_NodeId memberNodeId;
  memberNodeId = UA_NODEID_STRING(1, propertyBrowsePath);     // TODO Change 1 to namespaceIndex
  UA_StatusCode status = UA_Server_addVariableNode(paServer, memberNodeId, mTypeNodeId,
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
    UA_QUALIFIEDNAME(1, propertyBrowsePath),    // TODO Change 1 to namespaceIndex
    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, nullptr, &memberNodeId);
  mTypePropertyNodes.emplace_back(memberNodeId);
  return status;
}

bool COPC_UA_AC_Layer::isOPCUAObjectPresent(std::string &paBrowsePath) {
  COPC_UA_Local_Handler* localHandler = static_cast<COPC_UA_Local_Handler*>(mHandler);
  if(localHandler) {
    CActionInfo::CNodePairInfo nodePair(nullptr, paBrowsePath);
    if(localHandler->isOPCUAObjectPresent(nodePair)) {
      UA_NodeId_copy(nodePair.getNodeId(), &mTypeNodeId);  
      return true;
    }
  } else {
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Failed to get LocalHandler because LocalHandler is null!\n");
  }
  return false;
}

std::string COPC_UA_AC_Layer::getPortNameFromConnection(CStringDictionary::TStringId paPortNameId, bool paIsPublisher) {
  const CDataConnection *portConnection = paIsPublisher ? getCommFB()->getDIConnection(paPortNameId) : getCommFB()->getDOConnection(paPortNameId);
  const CConnectionPoint connectionPoint = portConnection->getSourceId();
  TPortId portId = connectionPoint.mPortId;
  return std::string(CStringDictionary::getInstance().get(connectionPoint.mFB->getFBInterfaceSpec()->mDINames[portId]));
}

std::string COPC_UA_AC_Layer::getFBNameFromConnection() {
  bool isPublisher;
  switch (getCommFB()->getComServiceType()) {
  case e_Publisher:
    isPublisher = true;
    break;
  case e_Subscriber: 
    isPublisher = false;
    break;
  default:
    DEVLOG_ERROR("[OPC UA A&C LAYER]: Wrong CommFB used for FB %s! Expected: Publish/Subscribe\n", getCommFB()->getInstanceName());
    return std::string();
  }
  const CStringDictionary::TStringId *dataPortNameIds = isPublisher ? getCommFB()->getFBInterfaceSpec()->mDINames : getCommFB()->getFBInterfaceSpec()->mDONames;
  const CDataConnection *portConnection = isPublisher ? getCommFB()->getDIConnection(dataPortNameIds[2]) : getCommFB()->getDOConnection(dataPortNameIds[2]);
  if(!portConnection) {
     DEVLOG_ERROR("[OPC UA A&C LAYER]: Error at connection of FB %s!\n", getCommFB()->getInstanceName());
     return std::string();
  }
  const CConnectionPoint connectionPoint = portConnection->getSourceId();
  return std::string(connectionPoint.mFB->getInstanceName());
}

char *COPC_UA_AC_Layer::getNameFromString(const std::string &paName) {
  char* name = new char[paName.length() + 1];
  strncpy(name, paName.c_str(), paName.length());
  name[paName.length()] = '\0';
  mTypeNames.emplace_back(name);
  return name;
}
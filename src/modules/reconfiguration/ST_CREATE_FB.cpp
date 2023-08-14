/*******************************************************************************
 * Copyright (c) 2014, 2015 Profactor GmbH, fortiss GmbH
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Matthias Plasch, Gerhard Ebenhofer, Alois Zoitl - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "ST_CREATE_FB.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "ST_CREATE_FB_gen.cpp"
#endif

#include "criticalregion.h"
#include "resource.h"

#include "device.h"
#include "mgmcmdstruct.h"
#include "../../stdfblib/ita/DEV_MGR.h"

DEFINE_FIRMWARE_FB(FORTE_ST_CREATE_FB, g_nStringIdST_CREATE_FB)

const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anDataInputNames[] = {g_nStringIdQI, g_nStringIdFB_NAME, g_nStringIdFB_TYPE, g_nStringIdDST};
const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anDataInputTypeIds[] = {g_nStringIdBOOL, g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING};
const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anDataOutputNames[] = {g_nStringIdQO, g_nStringIdSTATUS};
const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anDataOutputTypeIds[] = {g_nStringIdBOOL, g_nStringIdWSTRING};
const TDataIOID FORTE_ST_CREATE_FB::scm_anEIWith[] = {1, 2, 3, 0, scmWithListDelimiter};
const TForteInt16 FORTE_ST_CREATE_FB::scm_anEIWithIndexes[] = {0};
const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anEventInputNames[] = {g_nStringIdREQ};
const TDataIOID FORTE_ST_CREATE_FB::scm_anEOWith[] = {1, 0, scmWithListDelimiter};
const TForteInt16 FORTE_ST_CREATE_FB::scm_anEOWithIndexes[] = {0};
const CStringDictionary::TStringId FORTE_ST_CREATE_FB::scm_anEventOutputNames[] = {g_nStringIdCNF};
const SFBInterfaceSpec FORTE_ST_CREATE_FB::scm_stFBInterfaceSpec = {
  1, scm_anEventInputNames, scm_anEIWith, scm_anEIWithIndexes,
  1, scm_anEventOutputNames, scm_anEOWith, scm_anEOWithIndexes,
  4, scm_anDataInputNames, scm_anDataInputTypeIds,
  2, scm_anDataOutputNames, scm_anDataOutputTypeIds,
  0, nullptr,
  0, nullptr
};

FORTE_ST_CREATE_FB::FORTE_ST_CREATE_FB(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes) :
    CFunctionBlock( pa_poSrcRes, &scm_stFBInterfaceSpec, pa_nInstanceNameId),
    var_conn_QO(var_QO),
    var_conn_STATUS(var_STATUS),
    conn_CNF(this, 0),
    conn_QI(nullptr),
    conn_FB_NAME(nullptr),
    conn_FB_TYPE(nullptr),
    conn_DST(nullptr),
    conn_QO(this, 0, &var_conn_QO),
    conn_STATUS(this, 1, &var_conn_STATUS) {
};

void FORTE_ST_CREATE_FB::setInitialValues() {
  var_QI = 0_BOOL;
  var_FB_NAME = u""_WSTRING;
  var_FB_TYPE = u""_WSTRING;
  var_DST = u""_WSTRING;
  var_QO = 0_BOOL;
  var_STATUS = u""_WSTRING;
}

void FORTE_ST_CREATE_FB::executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) {
  switch(paEIID) {
    case scm_nEventREQID:
      var_QO = var_QI;
      if(var_QI) {
        executeRQST();
      } else {
        var_STATUS = u"Not Ready"_WSTRING;
      }
      sendOutputEvent(scm_nEventCNFID, paECET);
      break;
  }
}

void FORTE_ST_CREATE_FB::executeRQST(){
  forte::core::SManagementCMD theCommand;

  theCommand.mDestination = CStringDictionary::getInstance().getId(var_DST.getValue());
  theCommand.mFirstParam.pushBack(CStringDictionary::getInstance().insert(var_FB_NAME.getValue()));
  theCommand.mSecondParam.pushBack(CStringDictionary::getInstance().getId(var_FB_TYPE.getValue()));
  theCommand.mCMD = EMGMCommandType::CreateFBInstance;

  EMGMResponse resp = getResource().getDevice().executeMGMCommand(theCommand);

  //calculate return value
  var_QO = CIEC_BOOL(resp == EMGMResponse::Ready);
  const std::string retVal(DEV_MGR::getResponseText(resp));
  DEVLOG_DEBUG("%s\n", retVal.c_str());
  var_STATUS = CIEC_WSTRING(retVal.c_str());
}

void FORTE_ST_CREATE_FB::readInputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventREQID: {
      RES_DATA_CON_CRITICAL_REGION();
      readData(1, var_FB_NAME, conn_FB_NAME);
      readData(2, var_FB_TYPE, conn_FB_TYPE);
      readData(3, var_DST, conn_DST);
      readData(0, var_QI, conn_QI);
      break;
    }
    default:
      break;
  }
}

void FORTE_ST_CREATE_FB::writeOutputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventCNFID: {
      RES_DATA_CON_CRITICAL_REGION();
      writeData(1, var_STATUS, conn_STATUS);
      writeData(0, var_QO, conn_QO);
      break;
    }
    default:
      break;
  }
}

CIEC_ANY *FORTE_ST_CREATE_FB::getDI(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_QI;
    case 1: return &var_FB_NAME;
    case 2: return &var_FB_TYPE;
    case 3: return &var_DST;
  }
  return nullptr;
}

CIEC_ANY *FORTE_ST_CREATE_FB::getDO(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_QO;
    case 1: return &var_STATUS;
  }
  return nullptr;
}

CIEC_ANY *FORTE_ST_CREATE_FB::getDIO(size_t) {
  return nullptr;
}

CEventConnection *FORTE_ST_CREATE_FB::getEOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_CNF;
  }
  return nullptr;
}

CDataConnection **FORTE_ST_CREATE_FB::getDIConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_QI;
    case 1: return &conn_FB_NAME;
    case 2: return &conn_FB_TYPE;
    case 3: return &conn_DST;
  }
  return nullptr;
}

CDataConnection *FORTE_ST_CREATE_FB::getDOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_QO;
    case 1: return &conn_STATUS;
  }
  return nullptr;
}

CInOutDataConnection **FORTE_ST_CREATE_FB::getDIOInConUnchecked(TPortId) {
  return nullptr;
}

CInOutDataConnection *FORTE_ST_CREATE_FB::getDIOOutConUnchecked(TPortId) {
  return nullptr;
}



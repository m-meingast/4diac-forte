/*************************************************************************
 * Copyright (c) 2018 TU Wien/ACIN
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Martin Melik-Merkumians - adds intial implememtation
 *************************************************************************/

#include "IB.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "IB_gen.cpp"
#endif

#include "criticalregion.h"
#include "resource.h"

DEFINE_FIRMWARE_FB(FORTE_IB, g_nStringIdIB)

const CStringDictionary::TStringId FORTE_IB::scm_anDataInputNames[] = {g_nStringIdQI, g_nStringIdPARAMS};
const CStringDictionary::TStringId FORTE_IB::scm_anDataInputTypeIds[] = {g_nStringIdBOOL, g_nStringIdSTRING};
const CStringDictionary::TStringId FORTE_IB::scm_anDataOutputNames[] = {g_nStringIdQO, g_nStringIdSTATUS, g_nStringIdIN};
const CStringDictionary::TStringId FORTE_IB::scm_anDataOutputTypeIds[] = {g_nStringIdBOOL, g_nStringIdSTRING, g_nStringIdBYTE};
const TDataIOID FORTE_IB::scm_anEIWith[] = {0, 1, scmWithListDelimiter, 0, scmWithListDelimiter};
const TForteInt16 FORTE_IB::scm_anEIWithIndexes[] = {0, 3};
const CStringDictionary::TStringId FORTE_IB::scm_anEventInputNames[] = {g_nStringIdINIT, g_nStringIdREQ};
const TDataIOID FORTE_IB::scm_anEOWith[] = {0, 1, scmWithListDelimiter, 0, 1, 2, scmWithListDelimiter, 0, 1, 2, scmWithListDelimiter};
const TForteInt16 FORTE_IB::scm_anEOWithIndexes[] = {0, 3, 7};
const CStringDictionary::TStringId FORTE_IB::scm_anEventOutputNames[] = {g_nStringIdINITO, g_nStringIdCNF, g_nStringIdIND};
const SFBInterfaceSpec FORTE_IB::scm_stFBInterfaceSpec = {
  2, scm_anEventInputNames, scm_anEIWith, scm_anEIWithIndexes,
  3, scm_anEventOutputNames, scm_anEOWith, scm_anEOWithIndexes,
  2, scm_anDataInputNames, scm_anDataInputTypeIds,
  3, scm_anDataOutputNames, scm_anDataOutputTypeIds,
  0, nullptr,
  0, nullptr
};

FORTE_IB::FORTE_IB(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes) :
        CProcessInterface(pa_poSrcRes, &scm_stFBInterfaceSpec, pa_nInstanceNameId),
    var_conn_QO(var_QO),
    var_conn_STATUS(var_STATUS),
    var_conn_IN(var_IN),
    conn_INITO(this, 0),
    conn_CNF(this, 1),
    conn_IND(this, 2),
    conn_QI(nullptr),
    conn_PARAMS(nullptr),
    conn_QO(this, 0, &var_conn_QO),
    conn_STATUS(this, 1, &var_conn_STATUS),
    conn_IN(this, 2, &var_conn_IN) {
};

void FORTE_IB::setInitialValues() {
  var_QI = 0_BOOL;
  var_PARAMS = ""_STRING;
  var_QO = 0_BOOL;
  var_STATUS = ""_STRING;
  var_IN = 0_BYTE;
}

void FORTE_IB::executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) {
  switch(paEIID) {
    case cg_nExternalEventID:
      sendOutputEvent(scm_nEventINDID, paECET);
      break;
    case scm_nEventINITID:
      if (var_QI) {
        var_QO = CIEC_BOOL(CProcessInterface::initialise(true)); //initialise as input
      } else {
        var_QO = CIEC_BOOL(CProcessInterface::deinitialise());
      }
      sendOutputEvent(scm_nEventINITOID, paECET);
      break;
    case scm_nEventREQID:
      if (var_QI) {
        var_QO = CIEC_BOOL(CProcessInterface::read(var_IN));
      } else {
        var_QO = false_BOOL;
      }
      sendOutputEvent(scm_nEventCNFID, paECET);
      break;
  }
}

void FORTE_IB::readInputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventINITID: {
      RES_DATA_CON_CRITICAL_REGION();
      readData(0, var_QI, conn_QI);
      readData(1, var_PARAMS, conn_PARAMS);
      break;
    }
    case scm_nEventREQID: {
      RES_DATA_CON_CRITICAL_REGION();
      readData(0, var_QI, conn_QI);
      break;
    }
    default:
      break;
  }
}

void FORTE_IB::writeOutputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventINITOID: {
      RES_DATA_CON_CRITICAL_REGION();
      writeData(0, var_QO, conn_QO);
      writeData(1, var_STATUS, conn_STATUS);
      break;
    }
    case scm_nEventCNFID: {
      RES_DATA_CON_CRITICAL_REGION();
      writeData(0, var_QO, conn_QO);
      writeData(1, var_STATUS, conn_STATUS);
      writeData(2, var_IN, conn_IN);
      break;
    }
    case scm_nEventINDID: {
      RES_DATA_CON_CRITICAL_REGION();
      writeData(0, var_QO, conn_QO);
      writeData(1, var_STATUS, conn_STATUS);
      writeData(2, var_IN, conn_IN);
      break;
    }
    default:
      break;
  }
}

CIEC_ANY *FORTE_IB::getDI(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_QI;
    case 1: return &var_PARAMS;
  }
  return nullptr;
}

CIEC_ANY *FORTE_IB::getDO(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_QO;
    case 1: return &var_STATUS;
    case 2: return &var_IN;
  }
  return nullptr;
}

CIEC_ANY *FORTE_IB::getDIO(size_t) {
  return nullptr;
}

CEventConnection *FORTE_IB::getEOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_INITO;
    case 1: return &conn_CNF;
    case 2: return &conn_IND;
  }
  return nullptr;
}

CDataConnection **FORTE_IB::getDIConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_QI;
    case 1: return &conn_PARAMS;
  }
  return nullptr;
}

CDataConnection *FORTE_IB::getDOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_QO;
    case 1: return &conn_STATUS;
    case 2: return &conn_IN;
  }
  return nullptr;
}

CInOutDataConnection **FORTE_IB::getDIOInConUnchecked(TPortId) {
  return nullptr;
}

CInOutDataConnection *FORTE_IB::getDIOOutConUnchecked(TPortId) {
  return nullptr;
}



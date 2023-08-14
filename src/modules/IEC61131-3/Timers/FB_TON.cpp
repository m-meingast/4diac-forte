/*******************************************************************************
 * Copyright (c) 2009, 2023 ACIN, fortiss GmbH
 *                          Martin Erich Jobst
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Monika Wenger, Alois Zoitl,  Ingo Hengy
 *   - initial API and implementation and/or initial documentation
 *   Martin Jobst - add readInputData and writeOutputData
 *******************************************************************************/
#include "FB_TON.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "FB_TON_gen.cpp"
#endif

#include "criticalregion.h"
#include "resource.h"

DEFINE_FIRMWARE_FB(FORTE_FB_TON, g_nStringIdFB_TON)

const CStringDictionary::TStringId FORTE_FB_TON::scm_anDataInputNames[] = {g_nStringIdIN, g_nStringIdPT};
const CStringDictionary::TStringId FORTE_FB_TON::scm_anDataInputTypeIds[] = {g_nStringIdBOOL, g_nStringIdTIME};
const CStringDictionary::TStringId FORTE_FB_TON::scm_anDataOutputNames[] = {g_nStringIdQ, g_nStringIdET};
const CStringDictionary::TStringId FORTE_FB_TON::scm_anDataOutputTypeIds[] = {g_nStringIdBOOL, g_nStringIdTIME};
const TDataIOID FORTE_FB_TON::scm_anEIWith[] = {0, 1, scmWithListDelimiter};
const TForteInt16 FORTE_FB_TON::scm_anEIWithIndexes[] = {0};
const CStringDictionary::TStringId FORTE_FB_TON::scm_anEventInputNames[] = {g_nStringIdREQ};
const TDataIOID FORTE_FB_TON::scm_anEOWith[] = {0, 1, scmWithListDelimiter};
const TForteInt16 FORTE_FB_TON::scm_anEOWithIndexes[] = {0};
const CStringDictionary::TStringId FORTE_FB_TON::scm_anEventOutputNames[] = {g_nStringIdCNF};
const SFBInterfaceSpec FORTE_FB_TON::scm_stFBInterfaceSpec = {
  1, scm_anEventInputNames, scm_anEIWith, scm_anEIWithIndexes,
  1, scm_anEventOutputNames, scm_anEOWith, scm_anEOWithIndexes,
  2, scm_anDataInputNames, scm_anDataInputTypeIds,
  2, scm_anDataOutputNames, scm_anDataOutputTypeIds,
  0, nullptr,
  0, nullptr
};

FORTE_FB_TON::FORTE_FB_TON(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes) :
    CFunctionBlock( pa_poSrcRes, &scm_stFBInterfaceSpec, pa_nInstanceNameId),
    var_conn_Q(var_Q),
    var_conn_ET(var_ET),
    conn_CNF(this, 0),
    conn_IN(nullptr),
    conn_PT(nullptr),
    conn_Q(this, 0, &var_conn_Q),
    conn_ET(this, 1, &var_conn_ET) {
};

void FORTE_FB_TON::setInitialValues() {
  var_IN = 0_BOOL;
  var_PT = 0_TIME;
  var_Q = 0_BOOL;
  var_ET = 0_TIME;
}

void FORTE_FB_TON::executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) {
  switch(paEIID) {
    case scm_nEventREQID:
      if(!var_IN) {
        var_Q = false_BOOL;
        var_ET = 0_TIME;
        risingEdge = false;
        start = 0_TIME;
      } else {
        if(!risingEdge) {
          risingEdge = true;
          start = func_NOW_MONOTONIC();
        } else {
          count = func_SUB(func_NOW_MONOTONIC(), start);
          if(func_LE(var_PT, count)) {
            var_Q = true_BOOL;
            var_ET = var_PT;
          } else {
            var_ET = count;
          }
        }
      }
      sendOutputEvent(scm_nEventCNFID, paECET);
      break;
  }
}

void FORTE_FB_TON::readInputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventREQID: {
      RES_DATA_CON_CRITICAL_REGION();
      readData(0, var_IN, conn_IN);
      readData(1, var_PT, conn_PT);
      break;
    }
    default:
      break;
  }
}

void FORTE_FB_TON::writeOutputData(TEventID paEIID) {
  switch(paEIID) {
    case scm_nEventCNFID: {
      RES_DATA_CON_CRITICAL_REGION();
      writeData(0, var_Q, conn_Q);
      writeData(1, var_ET, conn_ET);
      break;
    }
    default:
      break;
  }
}

CIEC_ANY *FORTE_FB_TON::getDI(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_IN;
    case 1: return &var_PT;
  }
  return nullptr;
}

CIEC_ANY *FORTE_FB_TON::getDO(size_t paIndex) {
  switch(paIndex) {
    case 0: return &var_Q;
    case 1: return &var_ET;
  }
  return nullptr;
}

CIEC_ANY *FORTE_FB_TON::getDIO(size_t) {
  return nullptr;
}

CEventConnection *FORTE_FB_TON::getEOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_CNF;
  }
  return nullptr;
}

CDataConnection **FORTE_FB_TON::getDIConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_IN;
    case 1: return &conn_PT;
  }
  return nullptr;
}

CDataConnection *FORTE_FB_TON::getDOConUnchecked(TPortId paIndex) {
  switch(paIndex) {
    case 0: return &conn_Q;
    case 1: return &conn_ET;
  }
  return nullptr;
}

CInOutDataConnection **FORTE_FB_TON::getDIOInConUnchecked(TPortId) {
  return nullptr;
}

CInOutDataConnection *FORTE_FB_TON::getDIOOutConUnchecked(TPortId) {
  return nullptr;
}



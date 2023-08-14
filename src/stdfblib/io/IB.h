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

#pragma once

#include "funcbloc.h"
#include "forte_bool.h"
#include "forte_string.h"
#include "forte_byte.h"
#include "iec61131_functions.h"
#include "forte_array_common.h"
#include "forte_array.h"
#include "forte_array_fixed.h"
#include "forte_array_variable.h"

#include "processinterface.h"

class FORTE_IB final : public CProcessInterface {
  DECLARE_FIRMWARE_FB(FORTE_IB)

private:
  static const CStringDictionary::TStringId scm_anDataInputNames[];
  static const CStringDictionary::TStringId scm_anDataInputTypeIds[];
  static const CStringDictionary::TStringId scm_anDataOutputNames[];
  static const CStringDictionary::TStringId scm_anDataOutputTypeIds[];
  static const TEventID scm_nEventINITID = 0;
  static const TEventID scm_nEventREQID = 1;
  static const TDataIOID scm_anEIWith[];
  static const TForteInt16 scm_anEIWithIndexes[];
  static const CStringDictionary::TStringId scm_anEventInputNames[];
  static const TEventID scm_nEventINITOID = 0;
  static const TEventID scm_nEventCNFID = 1;
  static const TEventID scm_nEventINDID = 2;
  static const TDataIOID scm_anEOWith[]; 
  static const TForteInt16 scm_anEOWithIndexes[];
  static const CStringDictionary::TStringId scm_anEventOutputNames[];

  static const SFBInterfaceSpec scm_stFBInterfaceSpec;

  void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;

  void readInputData(TEventID paEIID) override;
  void writeOutputData(TEventID paEIID) override;
  void setInitialValues() override;

public:
  FORTE_IB(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes);

  CIEC_BOOL var_QI;
  CIEC_STRING var_PARAMS;
  CIEC_BOOL var_QO;
  CIEC_STRING var_STATUS;
  CIEC_BYTE var_IN;
  CIEC_BOOL var_conn_QO;
  CIEC_STRING var_conn_STATUS;
  CIEC_BYTE var_conn_IN;
  CEventConnection conn_INITO;
  CEventConnection conn_CNF;
  CEventConnection conn_IND;
  CDataConnection *conn_QI;
  CDataConnection *conn_PARAMS;
  CDataConnection conn_QO;
  CDataConnection conn_STATUS;
  CDataConnection conn_IN;
  CIEC_ANY *getDI(size_t) override;
  CIEC_ANY *getDO(size_t) override;
  CIEC_ANY *getDIO(size_t) override;
  CEventConnection *getEOConUnchecked(TPortId) override;
  CDataConnection **getDIConUnchecked(TPortId) override;
  CDataConnection *getDOConUnchecked(TPortId) override;
  CInOutDataConnection **getDIOInConUnchecked(TPortId) override;
  CInOutDataConnection *getDIOOutConUnchecked(TPortId) override;
  void evt_INIT(const CIEC_BOOL &pa_QI, const CIEC_STRING &pa_PARAMS, CIEC_BOOL &pa_QO, CIEC_STRING &pa_STATUS, CIEC_BYTE &pa_IN) {
    var_QI = pa_QI;
    var_PARAMS = pa_PARAMS;
    receiveInputEvent(scm_nEventINITID, nullptr);
    pa_QO = var_QO;
    pa_STATUS = var_STATUS;
    pa_IN = var_IN;
  }
  void evt_REQ(const CIEC_BOOL &pa_QI, const CIEC_STRING &pa_PARAMS, CIEC_BOOL &pa_QO, CIEC_STRING &pa_STATUS, CIEC_BYTE &pa_IN) {
    var_QI = pa_QI;
    var_PARAMS = pa_PARAMS;
    receiveInputEvent(scm_nEventREQID, nullptr);
    pa_QO = var_QO;
    pa_STATUS = var_STATUS;
    pa_IN = var_IN;
  }
  void operator()(const CIEC_BOOL &pa_QI, const CIEC_STRING &pa_PARAMS, CIEC_BOOL &pa_QO, CIEC_STRING &pa_STATUS, CIEC_BYTE &pa_IN) {
    evt_INIT(pa_QI, pa_PARAMS, pa_QO, pa_STATUS, pa_IN);
  }
};




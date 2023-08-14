/*******************************************************************************
 * Copyright (c) 2014 Profactor GmbH
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Matthias Plasch, Gerhard Ebenhofer - initial API and implementation and/or initial documentation
 *******************************************************************************/

#pragma once

#include "funcbloc.h"
#include "forte_bool.h"
#include "forte_wstring.h"
#include "iec61131_functions.h"
#include "forte_array_common.h"
#include "forte_array.h"
#include "forte_array_fixed.h"
#include "forte_array_variable.h"

#include "resource.h"

class FORTE_EC_KILL_ELEM final : public CFunctionBlock {
  DECLARE_FIRMWARE_FB(FORTE_EC_KILL_ELEM)

private:
  static const CStringDictionary::TStringId scm_anDataInputNames[];
  static const CStringDictionary::TStringId scm_anDataInputTypeIds[];
  static const CStringDictionary::TStringId scm_anDataOutputNames[];
  static const CStringDictionary::TStringId scm_anDataOutputTypeIds[];
  static const TEventID scm_nEventREQID = 0;
  static const TDataIOID scm_anEIWith[];
  static const TForteInt16 scm_anEIWithIndexes[];
  static const CStringDictionary::TStringId scm_anEventInputNames[];
  static const TEventID scm_nEventCNFID = 0;
  static const TDataIOID scm_anEOWith[]; 
  static const TForteInt16 scm_anEOWithIndexes[];
  static const CStringDictionary::TStringId scm_anEventOutputNames[];

  static const SFBInterfaceSpec scm_stFBInterfaceSpec;

  void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
  void executeRQST();

  void readInputData(TEventID paEIID) override;
  void writeOutputData(TEventID paEIID) override;
  void setInitialValues() override;

public:
  FORTE_EC_KILL_ELEM(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes);

  CIEC_BOOL var_QI;
  CIEC_WSTRING var_ELEM_NAME;
  CIEC_WSTRING var_DST;
  CIEC_BOOL var_QO;
  CIEC_WSTRING var_STATUS;
  CIEC_BOOL var_conn_QO;
  CIEC_WSTRING var_conn_STATUS;
  CEventConnection conn_CNF;
  CDataConnection *conn_QI;
  CDataConnection *conn_ELEM_NAME;
  CDataConnection *conn_DST;
  CDataConnection conn_QO;
  CDataConnection conn_STATUS;
  CIEC_ANY *getDI(size_t) override;
  CIEC_ANY *getDO(size_t) override;
  CIEC_ANY *getDIO(size_t) override;
  CEventConnection *getEOConUnchecked(TPortId) override;
  CDataConnection **getDIConUnchecked(TPortId) override;
  CDataConnection *getDOConUnchecked(TPortId) override;
  CInOutDataConnection **getDIOInConUnchecked(TPortId) override;
  CInOutDataConnection *getDIOOutConUnchecked(TPortId) override;
  void evt_REQ(const CIEC_BOOL &pa_QI, const CIEC_WSTRING &pa_ELEM_NAME, const CIEC_WSTRING &pa_DST, CIEC_BOOL &pa_QO, CIEC_WSTRING &pa_STATUS) {
    var_QI = pa_QI;
    var_ELEM_NAME = pa_ELEM_NAME;
    var_DST = pa_DST;
    receiveInputEvent(scm_nEventREQID, nullptr);
    pa_QO = var_QO;
    pa_STATUS = var_STATUS;
  }
  void operator()(const CIEC_BOOL &pa_QI, const CIEC_WSTRING &pa_ELEM_NAME, const CIEC_WSTRING &pa_DST, CIEC_BOOL &pa_QO, CIEC_WSTRING &pa_STATUS) {
    evt_REQ(pa_QI, pa_ELEM_NAME, pa_DST, pa_QO, pa_STATUS);
  }
};




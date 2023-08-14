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


class FORTE_ST_REC_CONN final : public CFunctionBlock {
  DECLARE_FIRMWARE_FB(FORTE_ST_REC_CONN)

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
  FORTE_ST_REC_CONN(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes);

  CIEC_BOOL var_QI;
  CIEC_WSTRING var_OLD_SRC_FB;
  CIEC_WSTRING var_OLD_SRC_FB_OUT;
  CIEC_WSTRING var_OLD_DST_FB;
  CIEC_WSTRING var_OLD_DST_FB_IN;
  CIEC_WSTRING var_NEW_SRC_FB;
  CIEC_WSTRING var_NEW_SRC_FB_OUT;
  CIEC_WSTRING var_NEW_DST_FB;
  CIEC_WSTRING var_NEW_DST_FB_IN;
  CIEC_WSTRING var_DST;
  CIEC_BOOL var_QO;
  CIEC_WSTRING var_STATUS;
  CIEC_BOOL var_conn_QO;
  CIEC_WSTRING var_conn_STATUS;
  CEventConnection conn_CNF;
  CDataConnection *conn_QI;
  CDataConnection *conn_OLD_SRC_FB;
  CDataConnection *conn_OLD_SRC_FB_OUT;
  CDataConnection *conn_OLD_DST_FB;
  CDataConnection *conn_OLD_DST_FB_IN;
  CDataConnection *conn_NEW_SRC_FB;
  CDataConnection *conn_NEW_SRC_FB_OUT;
  CDataConnection *conn_NEW_DST_FB;
  CDataConnection *conn_NEW_DST_FB_IN;
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
  void evt_REQ(const CIEC_BOOL &pa_QI, const CIEC_WSTRING &pa_OLD_SRC_FB, const CIEC_WSTRING &pa_OLD_SRC_FB_OUT, const CIEC_WSTRING &pa_OLD_DST_FB, const CIEC_WSTRING &pa_OLD_DST_FB_IN, const CIEC_WSTRING &pa_NEW_SRC_FB, const CIEC_WSTRING &pa_NEW_SRC_FB_OUT, const CIEC_WSTRING &pa_NEW_DST_FB, const CIEC_WSTRING &pa_NEW_DST_FB_IN, const CIEC_WSTRING &pa_DST, CIEC_BOOL &pa_QO, CIEC_WSTRING &pa_STATUS) {
    var_QI = pa_QI;
    var_OLD_SRC_FB = pa_OLD_SRC_FB;
    var_OLD_SRC_FB_OUT = pa_OLD_SRC_FB_OUT;
    var_OLD_DST_FB = pa_OLD_DST_FB;
    var_OLD_DST_FB_IN = pa_OLD_DST_FB_IN;
    var_NEW_SRC_FB = pa_NEW_SRC_FB;
    var_NEW_SRC_FB_OUT = pa_NEW_SRC_FB_OUT;
    var_NEW_DST_FB = pa_NEW_DST_FB;
    var_NEW_DST_FB_IN = pa_NEW_DST_FB_IN;
    var_DST = pa_DST;
    receiveInputEvent(scm_nEventREQID, nullptr);
    pa_QO = var_QO;
    pa_STATUS = var_STATUS;
  }
  void operator()(const CIEC_BOOL &pa_QI, const CIEC_WSTRING &pa_OLD_SRC_FB, const CIEC_WSTRING &pa_OLD_SRC_FB_OUT, const CIEC_WSTRING &pa_OLD_DST_FB, const CIEC_WSTRING &pa_OLD_DST_FB_IN, const CIEC_WSTRING &pa_NEW_SRC_FB, const CIEC_WSTRING &pa_NEW_SRC_FB_OUT, const CIEC_WSTRING &pa_NEW_DST_FB, const CIEC_WSTRING &pa_NEW_DST_FB_IN, const CIEC_WSTRING &pa_DST, CIEC_BOOL &pa_QO, CIEC_WSTRING &pa_STATUS) {
    evt_REQ(pa_QI, pa_OLD_SRC_FB, pa_OLD_SRC_FB_OUT, pa_OLD_DST_FB, pa_OLD_DST_FB_IN, pa_NEW_SRC_FB, pa_NEW_SRC_FB_OUT, pa_NEW_DST_FB, pa_NEW_DST_FB_IN, pa_DST, pa_QO, pa_STATUS);
  }
};




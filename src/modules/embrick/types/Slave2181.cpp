/*******************************************************************************
 * Copyright (c) 2016 - 2018 Johannes Messmer (admin@jomess.com), fortiss GmbH
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Johannes Messmer - initial API and implementation and/or initial documentation
 *   Jose Cabral - Cleaning of namespaces
 *******************************************************************************/

#include "Slave2181.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "Slave2181_gen.cpp"
#endif

#include "../handler/bus.h"

DEFINE_FIRMWARE_FB(EmbrickSlave2181, g_nStringIdEBSlave2181)

const CStringDictionary::TStringId EmbrickSlave2181::scmDataInputNames[] = {
    g_nStringIdQI, g_nStringIdDigitalInput_1, g_nStringIdDigitalInput_2,
    g_nStringIdDigitalInput_3, g_nStringIdDigitalInput_4,
    g_nStringIdDigitalInput_5, g_nStringIdDigitalInput_6,
    g_nStringIdDigitalInput_7, g_nStringIdDigitalInput_8,
    g_nStringIdDigitalOutput_1, g_nStringIdDigitalOutput_2,
    g_nStringIdDigitalOutput_3, g_nStringIdDigitalOutput_4,
    g_nStringIdDigitalOutput_5, g_nStringIdDigitalOutput_6,
    g_nStringIdDigitalOutput_7, g_nStringIdDigitalOutput_8,
    g_nStringIdUpdateInterval };

const CStringDictionary::TStringId EmbrickSlave2181::scmDataInputTypeIds[] = {
    g_nStringIdBOOL, g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING,
    g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING,
    g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING,
    g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING,
    g_nStringIdWSTRING, g_nStringIdWSTRING, g_nStringIdWSTRING,
    g_nStringIdWSTRING, g_nStringIdUINT };

const CStringDictionary::TStringId EmbrickSlave2181::scmDataOutputNames[] = {
    g_nStringIdQO, g_nStringIdSTATUS };

const CStringDictionary::TStringId EmbrickSlave2181::scmDataOutputTypeIds[] = {
    g_nStringIdBOOL, g_nStringIdWSTRING };

const TForteInt16 EmbrickSlave2181::scmEIWithIndexes[] = { 0 };
const TDataIOID EmbrickSlave2181::scmEIWith[] = { 1, 2, 5, 3, 4, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 0, scmWithListDelimiter };
const CStringDictionary::TStringId EmbrickSlave2181::scmEventInputNames[] = {
    g_nStringIdMAP };

const TDataIOID EmbrickSlave2181::scmEOWith[] = { 0, scmWithListDelimiter, 0, 1, scmWithListDelimiter };
const TForteInt16 EmbrickSlave2181::scmEOWithIndexes[] = { 0, 2, -1 };
const CStringDictionary::TStringId EmbrickSlave2181::scmEventOutputNames[] = {
    g_nStringIdMAPO, g_nStringIdIND };

const SAdapterInstanceDef EmbrickSlave2181::scmAdapterInstances[] = { {
    g_nStringIdEBBusAdapter, g_nStringIdBusAdapterOut, true }, {
    g_nStringIdEBBusAdapter, g_nStringIdBusAdapterIn, false } };

const SFBInterfaceSpec EmbrickSlave2181::scmFBInterfaceSpec = { 1,
    scmEventInputNames, scmEIWith, scmEIWithIndexes, 2,
    scmEventOutputNames, scmEOWith, scmEOWithIndexes, 18,
    scmDataInputNames, scmDataInputTypeIds, 2, scmDataOutputNames,
    scmDataOutputTypeIds, 2, scmAdapterInstances };

const TForteUInt8 EmbrickSlave2181::scmSlaveConfigurationIO[] = { 17 };
const TForteUInt8 EmbrickSlave2181::scmSlaveConfigurationIONum = 1;

void EmbrickSlave2181::initHandles() {
  // Initialize handles
  int iCount = 8;
  int oCount = 8;
  int iOffset = 1;
  int oOffset = iOffset + iCount;

  for (int i = 0; i < iCount; i++) {
    EmbrickBusHandler::HandleDescriptor desc = EmbrickBusHandler::HandleDescriptor(
        *static_cast<CIEC_WSTRING*>(getDI(iOffset + i)), forte::core::io::IOMapper::In, mIndex,
        EmbrickBusHandler::Bit, (uint8_t) (i / 8), (uint8_t) (i % 8));
    initHandle(&desc);
  }

  for (int i = 0; i < oCount; i++) {
    EmbrickBusHandler::HandleDescriptor desc = EmbrickBusHandler::HandleDescriptor(
        *static_cast<CIEC_WSTRING*>(getDI(oOffset + i)), forte::core::io::IOMapper::Out, mIndex,
        EmbrickBusHandler::Bit, (uint8_t) (i / 8), (uint8_t) (i % 8));
    initHandle(&desc);
  }
}




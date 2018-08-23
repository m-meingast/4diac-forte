/*******************************************************************************
 * Copyright (c) 2015, 2018 fortiss GmbH, Johannes Kepler University
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Alois Zoitl
 *      - initial implementation and rework communication infrastructure
 *******************************************************************************/
#include "monitoring.h"
#include "resource.h"
#include "device.h"
#include "ecet.h"
#include "utils/criticalregion.h"

using namespace forte::core;

CMonitoringHandler::CMonitoringHandler(CResource &paResource) :
    mTriggerEvent(0, 0),
        mResource(paResource){

}

EMGMResponse CMonitoringHandler::executeMonitoringCommand(SManagementCMD &paCommand){
  EMGMResponse retVal = e_UNSUPPORTED_CMD;

  switch (paCommand.mCMD){
    case cg_nMGM_CMD_Monitoring_Add_Watch:
      retVal = addWatch(paCommand.mFirstParam);
      break;
    case cg_nMGM_CMD_Monitoring_Remove_Watch:
      retVal = removeWatch(paCommand.mFirstParam);
      break;
    case cg_nMGM_CMD_Monitoring_Read_Watches:
      retVal = readWatches(paCommand.mMonitorResponse);
      break;
    case cg_nMGM_CMD_Monitoring_Get_Watches:
      retVal =
          getWatches(paCommand.mMonitorResponse, paCommand.mAdditionalParams.getValue()[0]);
      break;
    case cg_nMGM_CMD_Monitoring_Force:
      retVal = mResource.writeValue(paCommand.mFirstParam, paCommand.mAdditionalParams, true);
      break;
    case cg_nMGM_CMD_Monitoring_ClearForce:
      retVal = clearForce(paCommand.mFirstParam);
      break;
    case cg_nMGM_CMD_Monitoring_Trigger_Event:
      retVal = triggerEvent(paCommand.mFirstParam);
      break;
    case cg_nMGM_CMD_Monitoring_Reset_Event_Count:
      retVal = resetEventCount(paCommand.mFirstParam);
      break;
    default:
      break;
  }
  return retVal;
}

CFunctionBlock* CMonitoringHandler::getFB(forte::core::TNameIdentifier &paNameList){
  forte::core::TNameIdentifier::CIterator runner(paNameList.begin());

  CFunctionBlock *fb = mResource.getContainedFB(runner);
  if((0 != fb) && (!runner.isLastEntry())){
    ++runner;
    fb = fb->getFB(runner);
  }
  return fb;
}

EMGMResponse CMonitoringHandler::addWatch(forte::core::TNameIdentifier &paNameList){
  EMGMResponse eRetVal = e_NO_SUCH_OBJECT;

  CStringDictionary::TStringId portName = paNameList.back();
  paNameList.popBack();
  CFunctionBlock *fB = getFB(paNameList);

  if(0 != fB){
    SFBMonitoringEntry &fbMonitoringEntry(findOrCreateFBMonitoringEntry(fB, paNameList));

    CIEC_ANY *dataVal = fB->getVar(&portName, 1);
    if(0 != dataVal){
      addDataWatch(fbMonitoringEntry, portName, *dataVal);
      eRetVal = e_RDY;
    }
    else{
      TEventID eventId = fB->getEIID(portName);
      if(cg_nInvalidEventID != eventId){
        addEventWatch(fbMonitoringEntry, portName, fB->getEIMonitorData(eventId));
        eRetVal = e_RDY;
      }
      else{
        eventId = fB->getEOID(portName);
        if(cg_nInvalidEventID != eventId){
          addEventWatch(fbMonitoringEntry, portName, fB->getEOMonitorData(eventId));
          eRetVal = e_RDY;
        }
      }
    }
  }

  return eRetVal;
}

EMGMResponse CMonitoringHandler::removeWatch(forte::core::TNameIdentifier &paNameList){
  EMGMResponse eRetVal = e_NO_SUCH_OBJECT;

  CStringDictionary::TStringId portName = paNameList.back();
  paNameList.popBack();
  CFunctionBlock *fB = getFB(paNameList);
  if(0 != fB){
    TFBMonitoringList::Iterator itRefNode = mFBMonitoringList.end(); //needed for deleting the entry from the list
    TFBMonitoringList::Iterator itRunner = mFBMonitoringList.begin();

    while(itRunner != mFBMonitoringList.end()){
      if(itRunner->m_poFB == fB){
        SFBMonitoringEntry &poFBMonitoringEntry(*itRunner);

        if(!removeDataWatch(poFBMonitoringEntry, portName)){
          if(!removeEventWatch(poFBMonitoringEntry, portName)){
            //element is not watched end search and return error
            break;
          }
        }

        if((poFBMonitoringEntry.m_lstWatchedDataPoints.isEmpty())
            && (poFBMonitoringEntry.m_lstWatchedEventPoints.isEmpty())){
          //no further values are monitored so remove the entry
          if(itRefNode == mFBMonitoringList.end()){
            //we have the first entry in the list
            mFBMonitoringList.popFront();
          }
          else{
            mFBMonitoringList.eraseAfter(itRefNode);
          }
        }
        eRetVal = e_RDY;
        break;
      }

      itRefNode = itRunner;
      ++itRunner;
    }
  }
  return eRetVal;
}

EMGMResponse CMonitoringHandler::readWatches(CIEC_STRING &paResponse){
  paResponse.clear();
  if(0 == mResource.getResourcePtr()){
    //we are in the device
    for(CFBContainer::TFunctionBlockList::Iterator itRunner = mResource.getFBList().begin();
        itRunner != mResource.getFBList().end();
        ++itRunner){
      ((CResource*) (*itRunner))->getMonitoringHandler().readResourceWatches(paResponse);
    }
  }
  else{
    //we are within a resource
    readResourceWatches(paResponse);
  }

  return e_RDY;
}

EMGMResponse CMonitoringHandler::getWatches(CIEC_STRING &paResponse, char paQualifier){
  paResponse.clear();
  if(0 == mResource.getResourcePtr()){
    //we are in the device
    for(CFBContainer::TFunctionBlockList::Iterator itRunner = mResource.getFBList().begin();
        itRunner != mResource.getFBList().end();
        ++itRunner){
      ((CResource*) (*itRunner))->getMonitoringHandler().getResourceWatches(paResponse, paQualifier);
    }
  }
  else{
    //we are within a resource
    getResourceWatches(paResponse, paQualifier);
  }

  return e_RDY;
}

EMGMResponse CMonitoringHandler::clearForce(forte::core::TNameIdentifier &paNameList){
  EMGMResponse eRetVal = e_NO_SUCH_OBJECT;
  CStringDictionary::TStringId portName = paNameList.back();
  paNameList.popBack();
  CFunctionBlock *fB = getFB(paNameList);

  if(0 != fB){
    CIEC_ANY *poDataVal = fB->getVar(&portName, 1);
    if(0 != poDataVal){
      poDataVal->setForced(false);
      eRetVal = e_RDY;
    }
  }
  return eRetVal;
}

EMGMResponse CMonitoringHandler::triggerEvent(forte::core::TNameIdentifier &paNameList){
  EMGMResponse eRetVal = e_NO_SUCH_OBJECT;
  CStringDictionary::TStringId portName = paNameList.back();
  paNameList.popBack();
  CFunctionBlock *fB = getFB(paNameList);
  if(0 != fB){
    TEventID eventId = fB->getEIID(portName);
    if(cg_nInvalidEventID != eventId){
      //only a single event can be triggered simultaneously (until it is executed by ecet!)
      //otherwise the triggerEvent structure needs to be moved to another place!
      mTriggerEvent.mFB = fB;
      mTriggerEvent.mPortId = eventId;
      mResource.getResourceEventExecution()->startEventChain(&mTriggerEvent);
      eRetVal = e_RDY;
    }
    else{
      eventId = fB->getEOID(portName);
      if(cg_nInvalidEventID != eventId){
        fB->m_poInvokingExecEnv = mResource.getResourceEventExecution();
        fB->sendOutputEvent(eventId);
        mResource.getResourceEventExecution()->resumeSelfSuspend();
        eRetVal = e_RDY;
      }
    }

  }
  return eRetVal;
}

EMGMResponse CMonitoringHandler::resetEventCount(forte::core::TNameIdentifier &paNameList){
  EMGMResponse eRetVal = e_NO_SUCH_OBJECT;
  CStringDictionary::TStringId portName = paNameList.back();
  paNameList.popBack();
  CFunctionBlock *fB = getFB(paNameList);
  if(0 != fB){
    TEventID eventId = fB->getEIID(portName);
    SMonitorEvent *pstEventMonitorData = 0;

    if(cg_nInvalidEventID != eventId){
      pstEventMonitorData = &fB->getEIMonitorData(eventId);
    }
    else{
      eventId = fB->getEOID(portName);
      if(cg_nInvalidEventID != eventId){
        pstEventMonitorData = &fB->getEOMonitorData(eventId);
      }
    }
    if(0 != pstEventMonitorData){
      CCriticalRegion criticalRegion(fB->getResource().m_oResDataConSync);
      pstEventMonitorData->mEventCount = 0;
      eRetVal = e_RDY;
    }
  }
  return eRetVal;
}

CMonitoringHandler::SFBMonitoringEntry &CMonitoringHandler::findOrCreateFBMonitoringEntry(
    CFunctionBlock *paFB, forte::core::TNameIdentifier &paNameList){
  for(TFBMonitoringList::Iterator itRunner = mFBMonitoringList.begin();
      itRunner != mFBMonitoringList.end(); ++itRunner){
    if(itRunner->m_poFB == paFB){
      return *itRunner;
    }
  }

  mFBMonitoringList.pushBack(SFBMonitoringEntry());
  TFBMonitoringList::Iterator itLastEntry(mFBMonitoringList.back());
  itLastEntry->m_poFB = paFB;
  createFullFBName(itLastEntry->mFullFBName, paNameList);
  return *itLastEntry;
}

void CMonitoringHandler::addDataWatch(SFBMonitoringEntry &paFBMonitoringEntry,
    CStringDictionary::TStringId paPortId, CIEC_ANY &paDataVal){
  for(TDataWatchList::Iterator itRunner = paFBMonitoringEntry.m_lstWatchedDataPoints.begin();
      itRunner != paFBMonitoringEntry.m_lstWatchedDataPoints.end(); ++itRunner){
    if(itRunner->mPortId == paPortId){
      //the data point is already in the watch list
      return;
    }
  }
  paFBMonitoringEntry.m_lstWatchedDataPoints.pushBack(SDataWatchEntry(paPortId, paDataVal));
}

bool CMonitoringHandler::removeDataWatch(SFBMonitoringEntry &paFBMonitoringEntry,
    CStringDictionary::TStringId paPortId){
  bool bRetVal = false;

  TDataWatchList::Iterator itRunner = paFBMonitoringEntry.m_lstWatchedDataPoints.begin();
  TDataWatchList::Iterator itRefNode = paFBMonitoringEntry.m_lstWatchedDataPoints.end();

  while(itRunner != paFBMonitoringEntry.m_lstWatchedDataPoints.end()){
    if(itRunner->mPortId == paPortId){
      if(itRefNode == paFBMonitoringEntry.m_lstWatchedDataPoints.end()){
        //we have the first entry in the list
        paFBMonitoringEntry.m_lstWatchedDataPoints.popFront();
      }
      else{
        paFBMonitoringEntry.m_lstWatchedDataPoints.eraseAfter(itRefNode);
      }
      bRetVal = true;
      break;
    }

    itRefNode = itRunner;
    ++itRunner;
  }

  return bRetVal;
}

void CMonitoringHandler::addEventWatch(SFBMonitoringEntry &paFBMonitoringEntry,
    CStringDictionary::TStringId paPortId, SMonitorEvent &paEventData){
  for(TEventWatchList::Iterator itRunner = paFBMonitoringEntry.m_lstWatchedEventPoints.begin();
      itRunner != paFBMonitoringEntry.m_lstWatchedEventPoints.end(); ++itRunner){
    if(itRunner->m_unPortId == paPortId){
      //the event point is already in the watch list
      return;
    }
  }
  paFBMonitoringEntry.m_lstWatchedEventPoints.pushBack(SEventWatchEntry(paPortId, paEventData));
}

bool CMonitoringHandler::removeEventWatch(SFBMonitoringEntry &paFBMonitoringEntry,
    CStringDictionary::TStringId paPortId){
  bool bRetVal = false;

  TEventWatchList::Iterator itRunner = paFBMonitoringEntry.m_lstWatchedEventPoints.begin();
  TEventWatchList::Iterator itRefNode = paFBMonitoringEntry.m_lstWatchedEventPoints.end();

  while(itRunner != paFBMonitoringEntry.m_lstWatchedEventPoints.end()){
    if(itRunner->m_unPortId == paPortId){
      if(itRefNode == paFBMonitoringEntry.m_lstWatchedEventPoints.end()){
        //we have the first entry in the list
        paFBMonitoringEntry.m_lstWatchedEventPoints.popFront();
      }
      else{
        paFBMonitoringEntry.m_lstWatchedEventPoints.eraseAfter(itRefNode);
      }
      bRetVal = true;
      break;
    }

    itRefNode = itRunner;
    ++itRunner;
  }

  return bRetVal;
}

void CMonitoringHandler::readResourceWatches(CIEC_STRING &paResponse){
  if(!mFBMonitoringList.isEmpty()){
    paResponse.append("<Resource name=\"");
    paResponse.append(mResource.getInstanceName());
    paResponse.append("\">");

    { //begin critical region
      CCriticalRegion criticalRegion(mResource.m_oResDataConSync);

      for(TFBMonitoringList::Iterator itRunner = mFBMonitoringList.begin();
          itRunner != mFBMonitoringList.end(); ++itRunner){
        paResponse.append("<FB name=\"");
        paResponse.append(itRunner->mFullFBName.getValue());
        paResponse.append("\">");

        //add the data watches
        for(TDataWatchList::Iterator itDataRunner = itRunner->m_lstWatchedDataPoints.begin();
            itDataRunner != itRunner->m_lstWatchedDataPoints.end(); ++itDataRunner){
          appendDataWatch(paResponse, *itDataRunner);
        }

        //add the event watches
        for(TEventWatchList::Iterator itEventRunner = itRunner->m_lstWatchedEventPoints.begin();
            itEventRunner != itRunner->m_lstWatchedEventPoints.end();
            ++itEventRunner){
          appendEventWatch(paResponse, *itEventRunner);
        }

        paResponse.append("</FB>");
      }
    } //end critical region
    paResponse.append("</Resource>");
  }
}

void CMonitoringHandler::getResourceWatches(CIEC_STRING &paResponse, char){
   //FIXME: char pa_cQualifier is avoided in order to compile without warnings. Work in the breakpoints is needed.
  if(!mFBMonitoringList.isEmpty()){
    paResponse.append("<Resource name=\"");
    paResponse.append(mResource.getInstanceName());
    paResponse.append("\">");

    for(TFBMonitoringList::Iterator itRunner = mFBMonitoringList.begin();
        itRunner != mFBMonitoringList.end(); ++itRunner){
      paResponse.append("<FB name=\"");
      paResponse.append(itRunner->mFullFBName.getValue());
      paResponse.append("\">");

      //FIXME implement the watches

      paResponse.append("</FB>");
    }

    paResponse.append("</Resource>");
  }
}

void CMonitoringHandler::appendDataWatch(CIEC_STRING &paResponse,
    SDataWatchEntry &paDataWatchEntry){
  int bufferSize = paDataWatchEntry.mDataValue.getToStringBufferSize();
  appendPortTag(paResponse, paDataWatchEntry.mPortId);
  paResponse.append("<Data value=\"");
  char* acDataValue = new char [bufferSize]; //TODO try to directly use the response string instead
  int nConsumedBytes;
  switch (paDataWatchEntry.mDataValue.getDataTypeID()){
    case CIEC_ANY::e_WSTRING:
      case CIEC_ANY::e_STRING:
      nConsumedBytes =
          static_cast<CIEC_WSTRING&>(paDataWatchEntry.mDataValue).toUTF8(acDataValue, bufferSize, false);
      break;
    default:
      nConsumedBytes = paDataWatchEntry.mDataValue.toString(acDataValue, bufferSize);
      break;
  }
  if(-1 != nConsumedBytes){
    acDataValue[nConsumedBytes] = '\0';
  }
  paResponse.append(acDataValue);
  paResponse.append("\" forced=\"");
  paResponse.append(((paDataWatchEntry.mDataValue.isForced()) ? "true" : "false"));
  paResponse.append("\"/></Port>");
  delete [] acDataValue;
}

void CMonitoringHandler::appendPortTag(CIEC_STRING &paResponse,
    CStringDictionary::TStringId paPortId){
  paResponse.append("<Port name=\"");
  paResponse.append(CStringDictionary::getInstance().get(paPortId));
  paResponse.append("\">");
}

void CMonitoringHandler::appendEventWatch(CIEC_STRING &paResponse,
    SEventWatchEntry &paEventWatchEntry){
  appendPortTag(paResponse, paEventWatchEntry.m_unPortId);


  CIEC_UDINT udint = paEventWatchEntry.m_roEventData.mEventCount;
  SForteTime timeStamp = mResource.getDevice().getTimer().getForteTime();  // for backwards compatibility send the the current time as timestamp
  CIEC_ULINT ulint = ((((TForteUInt64) timeStamp.m_nUpperValue) << 32) & 0xFFFFFFFF00000000ULL) +
      (((TForteUInt64) timeStamp.m_nLowerValue) & 0xFFFFFFFFULL);

  paResponse.append("<Data value=\"");
  char buf[21]; // the bigest number in an ulint is 18446744073709551616, TODO directly use pa_roResponse
  udint.toString(buf, sizeof(buf));
  paResponse.append(buf);
  paResponse.append("\" time=\"");
  ulint.toString(buf, sizeof(buf));
  paResponse.append(buf);
  paResponse.append("\"/>\n</Port>");
}

void CMonitoringHandler::createFullFBName(CIEC_STRING &paFullName, forte::core::TNameIdentifier &paNameList){
  for(forte::core::TNameIdentifier::CIterator runner(paNameList.begin()); runner != paNameList.end(); ++runner){
    paFullName.append(CStringDictionary::getInstance().get(*runner));
    if(!runner.isLastEntry()){
      paFullName.append(".");
    }
  }
}


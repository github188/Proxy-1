//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : TaskDispatcher.cpp
//  @ Date : 2011-1-21
//  @ Author : chenjinyi
//
//


#include "TaskDispatcher.h"

CTaskDispatcher::CTaskDispatcher():m_packetInfoCond(m_packetInfoMutex)
{
    m_bQuit = false;
}

CTaskDispatcher::~CTaskDispatcher()
{
    //todo退出处理。。
//    m_bQuit = true;
//    Join();
}

//不能在这里发送，会阻塞
int CTaskDispatcher::Dispatch(CSession* pSession, const char* pData, int dataSize)
{
    printf("CTaskDispatcher::Dispatch, client ip = %s, listen port = %d, dataSize = %d\n", pSession->GetIP(), pSession->GetListenPort(), dataSize);
    PutPacketInfo(pSession, pData, dataSize);
    return 0;
}

void CTaskDispatcher::EventCallBack(CSession* pSession, short event)
{
    printf("taskdispatcher event callback, event = %d\n", event);
}

void CTaskDispatcher::PutPacketInfo(CSession* pSession, const char* pData, int dataSize)
{
    if (pSession == NULL || pData == NULL || dataSize <= 0)
    {
        return;
    }

    PacketInfo packet;
    packet.pSession = pSession;
    packet.pData = new char[dataSize];
    memcpy(packet.pData, pData, dataSize);
    packet.dataSize = dataSize;

    CMutexLock lock(m_packetInfoMutex);
    if (m_lPacketList.empty())
    {
        m_lPacketList.push_back(packet);
        m_packetInfoCond.Wake();
    }
    else
    {
        m_lPacketList.push_back(packet);
    }
}

PacketInfo CTaskDispatcher::GetPacketInfo()
{
    CMutexLock lock(m_packetInfoMutex);
    if (m_lPacketList.empty())
    {
        m_packetInfoCond.Wait();
    }

    PacketInfo packet;

    if (!m_lPacketList.empty())
    {
        packet = m_lPacketList.front();
        m_lPacketList.pop_front();
    }

    if (m_lPacketList.size() >= 100)
    {
        printf("task dispatcher thread has reach unreachable num , clear all data\n");
        ClearAllData();
    }

    return packet;
}


void CTaskDispatcher::ClearAllData()
{
    while (!m_lPacketList.empty())
    {
        PacketInfo packet;

        packet = m_lPacketList.front();
        m_lPacketList.pop_front();

        delete[] packet.pData;
    }
}


void CTaskDispatcher::Run()
{
    m_bQuit = false;

    while (!m_bQuit)
    {

        PacketInfo packet = GetPacketInfo();

        ProcessPacketInfo(packet);

        delete[] packet.pData;
    }
}

void CTaskDispatcher::ProcessPacketInfo(const PacketInfo& packet)
{
    printf("void CTaskDispatcher::ProcessPacketInfo(const PacketInfo& packet), do nothing, fuck!!!\n");
    return;
}


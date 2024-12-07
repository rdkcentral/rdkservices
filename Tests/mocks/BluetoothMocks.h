//#pragma once
#ifndef BLUETOOTH_MOCKS_H
#define BLUETOOTH_MOCKS_H

#include <gmock/gmock.h>
#include "BluetoothMgr.h"
#include "Bluetooth.h"

// Mock class for Bluetooth Manager functions
class MockBluetoothManager {
public:
    BTRMGR_EventCallback evBluetoothHandler;

    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetNumberOfAdapters, (unsigned char* pNumOfAdapters));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StartDeviceDiscovery, (unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StopDeviceDiscovery, (unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetAdapterPowerStatus, (unsigned char aui8AdapterIdx, unsigned char power_status));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_IsAdapterDiscoverable, (unsigned char aui8AdapterIdx, unsigned char* pDiscoverable));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetAdapterName, (unsigned char aui8AdapterIdx, char* pNameOfAdapter));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetDeviceVolumeMute, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char ui8Volume, unsigned char ui8Mute));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetEventResponse, (unsigned char aui8AdapterIdx, BTRMGR_EventResponse_t* apstBTRMgrEvtRsp));
    MOCK_METHOD(const char*, BTRMGR_GetDeviceTypeAsString, (BTRMGR_DeviceType_t type));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StopAudioStreamingIn, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StopAudioStreamingOut, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_UnRegisterFromCallbacks, (const char* apcProcessName));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StartAudioStreamingOut, (unsigned char adapterIndex, BTRMgrDeviceHandle deviceHandle, BTRMGR_DeviceOperationType_t connectAs));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StartAudioStreamingOut_StartUp, (unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevConT));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetPairedDevices, (unsigned char aui8AdapterIdx, BTRMGR_PairedDevicesList_t* pPairedDevices));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetDiscoveredDevices, (unsigned char aui8AdapterIdx, BTRMGR_DiscoveredDevicesList_t* pDiscoveredDevices));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_ConnectToDevice, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetAudioStreamingOutType, (unsigned char aui8AdapterIdx, BTRMGR_StreamOut_Type_t type));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetAdapterPowerStatus, (unsigned char aui8AdapterIdx, unsigned char* pPowerStatus));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_MediaControl, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaControlCommand_t mediaCtrlCmd));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetAdapterDiscoverable, (unsigned char aui8AdapterIdx, unsigned char discoverable, int timeout));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_StartAudioStreamingIn, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetMediaTrackInfo, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaTrackInfo_t* mediaTrackInfo));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_UnpairDevice, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_RegisterEventCallback, (BTRMGR_EventCallback afpcBBTRMgrEventOut));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_RegisterForCallbacks, (const char* apcProcessName));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetConnectedDevices, (unsigned char aui8AdapterIdx, BTRMGR_ConnectedDevicesList_t* pConnectedDevices));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_DisconnectFromDevice, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetDeviceVolumeMute, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char* pui8Volume, unsigned char* pui8Mute));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_GetDeviceProperties, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DevicesProperty_t* pDeviceProperty));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_PairDevice, (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl));
    MOCK_METHOD(BTRMGR_Result_t, BTRMGR_SetAdapterName, (unsigned char adapterIndex, const char* adapterName));
};

// Global pointer to access mock instance in C functions
extern MockBluetoothManager* mockBluetoothManagerInstance;

#endif // BLUETOOTH_MOCKS_H

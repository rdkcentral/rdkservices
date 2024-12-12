#include "BluetoothMocks.h"

// Define the mock functions for Bluetooth Manager C API
extern "C" {

BTRMGR_Result_t BTRMGR_GetNumberOfAdapters(unsigned char* pNumOfAdapters) {
    return mockBluetoothManagerInstance->BTRMGR_GetNumberOfAdapters(pNumOfAdapters);
}

BTRMGR_Result_t BTRMGR_StartDeviceDiscovery(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT) {
    return mockBluetoothManagerInstance->BTRMGR_StartDeviceDiscovery(aui8AdapterIdx, aenBTRMgrDevOpT);
}

BTRMGR_Result_t BTRMGR_StopDeviceDiscovery(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT) {
    return mockBluetoothManagerInstance->BTRMGR_StopDeviceDiscovery(aui8AdapterIdx, aenBTRMgrDevOpT);
}

BTRMGR_Result_t BTRMGR_SetAdapterPowerStatus(unsigned char aui8AdapterIdx, unsigned char power_status) {
    return mockBluetoothManagerInstance->BTRMGR_SetAdapterPowerStatus(aui8AdapterIdx, power_status);
}

BTRMGR_Result_t BTRMGR_IsAdapterDiscoverable(unsigned char aui8AdapterIdx, unsigned char* pDiscoverable) {
    return mockBluetoothManagerInstance->BTRMGR_IsAdapterDiscoverable(aui8AdapterIdx, pDiscoverable);
}

BTRMGR_Result_t BTRMGR_GetAdapterName(unsigned char aui8AdapterIdx, char* pNameOfAdapter) {
    return mockBluetoothManagerInstance->BTRMGR_GetAdapterName(aui8AdapterIdx, pNameOfAdapter);
}

BTRMGR_Result_t BTRMGR_SetDeviceVolumeMute(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char ui8Volume, unsigned char ui8Mute) {
    return mockBluetoothManagerInstance->BTRMGR_SetDeviceVolumeMute(aui8AdapterIdx, ahBTRMgrDevHdl, deviceOpType, ui8Volume, ui8Mute);
}

BTRMGR_Result_t BTRMGR_SetEventResponse(unsigned char aui8AdapterIdx, BTRMGR_EventResponse_t* apstBTRMgrEvtRsp) {
    return mockBluetoothManagerInstance->BTRMGR_SetEventResponse(aui8AdapterIdx, apstBTRMgrEvtRsp);
}

const char* BTRMGR_GetDeviceTypeAsString(BTRMGR_DeviceType_t type) {
    return mockBluetoothManagerInstance->BTRMGR_GetDeviceTypeAsString(type);
}

BTRMGR_Result_t BTRMGR_StopAudioStreamingIn(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl) {
    return mockBluetoothManagerInstance->BTRMGR_StopAudioStreamingIn(aui8AdapterIdx, ahBTRMgrDevHdl);
}

BTRMGR_Result_t BTRMGR_StopAudioStreamingOut(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl) {
    return mockBluetoothManagerInstance->BTRMGR_StopAudioStreamingOut(aui8AdapterIdx, ahBTRMgrDevHdl);
}

// BTRMGR_UnRegisterFromCallbacks method not required to be mocked
BTRMGR_Result_t BTRMGR_UnRegisterFromCallbacks(const char* apcProcessName) {
    return BTRMGR_RESULT_SUCCESS;
}

BTRMGR_Result_t BTRMGR_StartAudioStreamingOut(unsigned char adapterIndex, BTRMgrDeviceHandle deviceHandle, BTRMGR_DeviceOperationType_t connectAs) {
    return mockBluetoothManagerInstance->BTRMGR_StartAudioStreamingOut(adapterIndex, deviceHandle, connectAs);
}

BTRMGR_Result_t BTRMGR_StartAudioStreamingOut_StartUp(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevConT) {
    return mockBluetoothManagerInstance->BTRMGR_StartAudioStreamingOut_StartUp(aui8AdapterIdx, aenBTRMgrDevConT);
}

BTRMGR_Result_t BTRMGR_GetPairedDevices(unsigned char aui8AdapterIdx, BTRMGR_PairedDevicesList_t* pPairedDevices) {
    return mockBluetoothManagerInstance->BTRMGR_GetPairedDevices(aui8AdapterIdx, pPairedDevices);
}

BTRMGR_Result_t BTRMGR_GetDiscoveredDevices(unsigned char aui8AdapterIdx, BTRMGR_DiscoveredDevicesList_t* pDiscoveredDevices) {
    return mockBluetoothManagerInstance->BTRMGR_GetDiscoveredDevices(aui8AdapterIdx, pDiscoveredDevices);
}

BTRMGR_Result_t BTRMGR_ConnectToDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs) {
    return mockBluetoothManagerInstance->BTRMGR_ConnectToDevice(aui8AdapterIdx, ahBTRMgrDevHdl, connectAs);
}

BTRMGR_Result_t BTRMGR_SetAudioStreamingOutType(unsigned char aui8AdapterIdx, BTRMGR_StreamOut_Type_t type) {
    return mockBluetoothManagerInstance->BTRMGR_SetAudioStreamingOutType(aui8AdapterIdx, type);
}

BTRMGR_Result_t BTRMGR_GetAdapterPowerStatus(unsigned char aui8AdapterIdx, unsigned char* pPowerStatus) {
    return mockBluetoothManagerInstance->BTRMGR_GetAdapterPowerStatus(aui8AdapterIdx, pPowerStatus);
}

BTRMGR_Result_t BTRMGR_MediaControl(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaControlCommand_t mediaCtrlCmd) {
    return mockBluetoothManagerInstance->BTRMGR_MediaControl(aui8AdapterIdx, ahBTRMgrDevHdl, mediaCtrlCmd);
}

BTRMGR_Result_t BTRMGR_SetAdapterDiscoverable(unsigned char aui8AdapterIdx, unsigned char discoverable, int timeout) {
    return mockBluetoothManagerInstance->BTRMGR_SetAdapterDiscoverable(aui8AdapterIdx, discoverable, timeout);
}

BTRMGR_Result_t BTRMGR_StartAudioStreamingIn(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs) {
    return mockBluetoothManagerInstance->BTRMGR_StartAudioStreamingIn(aui8AdapterIdx, ahBTRMgrDevHdl, connectAs);
}

BTRMGR_Result_t BTRMGR_GetMediaTrackInfo(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaTrackInfo_t* mediaTrackInfo) {
    return mockBluetoothManagerInstance->BTRMGR_GetMediaTrackInfo(aui8AdapterIdx, ahBTRMgrDevHdl, mediaTrackInfo);
}

BTRMGR_Result_t BTRMGR_UnpairDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl) {
    return mockBluetoothManagerInstance->BTRMGR_UnpairDevice(aui8AdapterIdx, ahBTRMgrDevHdl);
}

BTRMGR_Result_t BTRMGR_RegisterEventCallback(BTRMGR_EventCallback afpcBBTRMgrEventOut) {
    if(mockBluetoothManagerInstance != nullptr) {
        mockBluetoothManagerInstance->evBluetoothHandler = afpcBBTRMgrEventOut;
    }
    //return mockBluetoothManagerInstance->BTRMGR_RegisterEventCallback(afpcBBTRMgrEventOut);
    return BTRMGR_RESULT_SUCCESS;
}

//BTRMGR_RegisterForCallbacks method not required to be mocked
BTRMGR_Result_t BTRMGR_RegisterForCallbacks(const char* apcProcessName) {
    return BTRMGR_RESULT_SUCCESS;
}

BTRMGR_Result_t BTRMGR_GetConnectedDevices(unsigned char aui8AdapterIdx, BTRMGR_ConnectedDevicesList_t* pConnectedDevices) {
    return mockBluetoothManagerInstance->BTRMGR_GetConnectedDevices(aui8AdapterIdx, pConnectedDevices);
}

BTRMGR_Result_t BTRMGR_DisconnectFromDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl) {
    return mockBluetoothManagerInstance->BTRMGR_DisconnectFromDevice(aui8AdapterIdx, ahBTRMgrDevHdl);
}

BTRMGR_Result_t BTRMGR_GetDeviceVolumeMute(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char* pui8Volume, unsigned char* pui8Mute) {
    return mockBluetoothManagerInstance->BTRMGR_GetDeviceVolumeMute(aui8AdapterIdx, ahBTRMgrDevHdl, deviceOpType, pui8Volume, pui8Mute);
}

BTRMGR_Result_t BTRMGR_GetDeviceProperties(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DevicesProperty_t* pDeviceProperty) {
    return mockBluetoothManagerInstance->BTRMGR_GetDeviceProperties(aui8AdapterIdx, ahBTRMgrDevHdl, pDeviceProperty);
}

BTRMGR_Result_t BTRMGR_PairDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl) {
    return mockBluetoothManagerInstance->BTRMGR_PairDevice(aui8AdapterIdx, ahBTRMgrDevHdl);
}

BTRMGR_Result_t BTRMGR_SetAdapterName(unsigned char adapterIndex, const char* adapterName) {
    return mockBluetoothManagerInstance->BTRMGR_SetAdapterName(adapterIndex, adapterName);
}

}

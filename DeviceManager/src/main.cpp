#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <Windows.h>
#include <cfgmgr32.h>

#include "CRError.h"

#pragma comment(lib,"Cfgmgr32.lib")

typedef struct _Device {
    DEVINST     DeviceInst;
    std::string DeviceId;
    std::string DriverKey;
    std::string FrindlyName;
    std::string Description;
    std::string Manufacturer;
} DEVICE;

typedef std::vector<DEVICE> DEVICE_LIST;
typedef std::vector<std::string> DEVICE_ID_LIST;

bool CreateDeviceIDList(
    DEVICE_ID_LIST& DeviceIdList,
    const PTCHAR pszFilter = nullptr
) {
    PTCHAR pszBuffer{ nullptr };
    ULONG  BufferSize{ 0 };
    ULONG  uFlags = pszFilter ?
        CM_GETIDLIST_FILTER_ENUMERATOR : CM_GETIDLIST_FILTER_NONE;
    SIZE_T szLen{ 0 };

    // Get the size of the device id list
    if (CM_Get_Device_ID_List_Size(&BufferSize, nullptr, 0) != CR_SUCCESS)
        return false;

    // Allocate some memory to hold device id list
    pszBuffer = (PTCHAR)calloc(BufferSize, sizeof(TCHAR));
    if (!pszBuffer)
        return false;

    // Get the device id list and store into buffer
    if (CM_Get_Device_ID_List(pszFilter, pszBuffer, BufferSize, uFlags) != CR_SUCCESS)
        return false;

    // Iterate through buffer and split string into multiple
    for (SIZE_T i = 0; i < BufferSize; i += lstrlen(&pszBuffer[i]) + (SIZE_T)1) {
        if ((szLen = lstrlen(&pszBuffer[i])) < 1)
            break;

        DeviceIdList.push_back(&pszBuffer[i]);
    }

    // Free buffer memory and return
    free(pszBuffer);
    return true;
}
bool CreateDeviceList(
    const DEVICE_ID_LIST& DeviceIdList,
    DEVICE_LIST& DeviceList
) {
    // Iterate through device list and get the dev node
    for (auto& Device : DeviceIdList) {

        // Skip devices without a device id
        if (Device.length() == 0)
            continue;

        // Locate the dev node and store in device
        DEVINST DeviceInst{ 0 };
        CM_Locate_DevNode(
            &DeviceInst,
            const_cast<TCHAR*>(Device.c_str()),
            CM_LOCATE_DEVNODE_NORMAL
        );

        if (DeviceInst > 0) {
            DeviceList.push_back({});
            DeviceList.back().DeviceId.assign(Device);
            DeviceList.back().DeviceInst = DeviceInst;
        }

    }

    return true;
}

bool GetDeviceDriverKey(
    DEVICE& Device
) {
    ULONG ulProperty{ CM_DRP_DRIVER };
    ULONG ulRegDataType{ 0 };
    ULONG ulLength{ 256 };
    ULONG ulFlags{ 0 };
    PVOID Buffer = calloc(ulLength, sizeof(TCHAR));

    // Check if buffer memory allocated
    if (!Buffer)
        return false;

    // Get the registry property
    if (auto Result = CM_Get_DevNode_Registry_Property(
        Device.DeviceInst, ulProperty, &ulRegDataType,
        Buffer, &ulLength, ulFlags))
    {
        std::stringstream ss;
        ss <<"CM_FAILED{" << CMGetError(Result) << "}";
        Device.DriverKey.assign(ss.str());
        return false;
    }

    if (lstrlen((TCHAR*)Buffer) > 0)
        Device.DriverKey.assign((TCHAR*)Buffer);

    free(Buffer);
    return true;

}

bool GetDeviceDriverFriendlyName(
    DEVICE& Device
) {
    ULONG ulProperty{ CM_DRP_FRIENDLYNAME };
    ULONG ulRegDataType{ 0 };
    ULONG ulLength{ 256 };
    ULONG ulFlags{ 0 };
    PVOID Buffer = calloc(ulLength, sizeof(TCHAR));

    // Check if buffer memory allocated
    if (!Buffer)
        return false;

    // Get the registry property
    if (auto Result = CM_Get_DevNode_Registry_Property(
        Device.DeviceInst, ulProperty, &ulRegDataType,
        Buffer, &ulLength, ulFlags))
    {
        std::stringstream ss;
        ss <<"CM_FAILED{" << CMGetError(Result) << "}";
        Device.FrindlyName.assign(ss.str());
        return false;
    }

    if (lstrlen((TCHAR*)Buffer) > 0)
        Device.FrindlyName.assign((TCHAR*)Buffer);

    free(Buffer);
    return true;

}

bool GetDeviceDriverDesc(
    DEVICE& Device
) {
    ULONG ulProperty{ CM_DRP_DEVICEDESC };
    ULONG ulRegDataType{ 0 };
    ULONG ulLength{ 256 };
    ULONG ulFlags{ 0 };
    PVOID Buffer = calloc(ulLength, sizeof(TCHAR));

    // Check if buffer memory allocated
    if (!Buffer)
        return false;

    // Get the registry property
    if (auto Result = CM_Get_DevNode_Registry_Property(
        Device.DeviceInst, ulProperty, &ulRegDataType,
        Buffer, &ulLength, ulFlags))
    {
        std::stringstream ss;
        ss <<"CM_FAILED{" << CMGetError(Result) << "}";
        Device.Description.assign(ss.str());
        return false;
    }

    if (lstrlen((TCHAR*)Buffer) > 0)
        Device.Description.assign((TCHAR*)Buffer);

    free(Buffer);
    return true;

}
bool GetDeviceDriverManufacturer(
    DEVICE& Device
) {
    ULONG ulProperty{ CM_DRP_MFG };
    ULONG ulRegDataType{ 0 };
    ULONG ulLength{ 512 };
    ULONG ulFlags{ 0 };
    PVOID Buffer = calloc(ulLength, sizeof(TCHAR));

    // Check if buffer memory allocated
    if (!Buffer)
        return false;

    // Get the registry property
    if (auto Result = CM_Get_DevNode_Registry_Property(
        Device.DeviceInst, ulProperty, &ulRegDataType,
        Buffer, &ulLength, ulFlags))
    {
        std::stringstream ss;
        ss <<"CM_FAILED{" << CMGetError(Result) << "}";
        Device.Manufacturer.assign(ss.str());
        return false;
    }

    if (lstrlen((TCHAR*)Buffer) > 0)
        Device.Manufacturer.assign((TCHAR*)Buffer);

    free(Buffer);
    return true;

    CR_SUCCESS;

}

void GetDeviceInfo(DEVICE_LIST& DeviceList) {
    for (auto& Device : DeviceList) {
        GetDeviceDriverKey(Device);
        GetDeviceDriverFriendlyName(Device);
        GetDeviceDriverDesc(Device);
        GetDeviceDriverManufacturer(Device);
    }
}


int main() {
    DEVICE_LIST DeviceList = {};
    DEVICE_ID_LIST DeviceIdList = {};

    CreateDeviceIDList(DeviceIdList);
    CreateDeviceList(DeviceIdList, DeviceList);
    GetDeviceInfo(DeviceList);

    for (const auto& Device : DeviceList) {
        std::cout << Device.DeviceId << '\n'
            <<"|------>" << Device.DeviceInst << '\n'
            <<"|------>" << Device.DriverKey << '\n'
            <<"|------>" << Device.FrindlyName << '\n'
            <<"|------>" << Device.Description << '\n'
            <<"|------>" << Device.Manufacturer << '\n'
            << '\n';

    }

    std::wcin.get();
    return 0;
}
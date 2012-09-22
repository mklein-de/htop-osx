
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IOKit/IOKitLib.h>

#include "smc.h"

static io_connect_t conn = 0;

uint32_t
key_to_int(const char *str, int size)
{
    uint32_t total = 0;
    int c;
     
    for (c = 0; c < size; c++) {
        total += str[c] << (size - 1 - c) * 8;
    }
    return total;
}

int SMCOpen(void)
{
    IOReturn       result;
    io_service_t   device;

    device = IOServiceGetMatchingService( kIOMasterPortDefault, IOServiceMatching("AppleSMC"));
    if (IO_OBJECT_NULL == device)
        return -1;
     
    result = IOServiceOpen(device, mach_task_self(), 1, &conn);
    IOObjectRelease(device);
    return 0;
}

io_connect_t SMCClose(void)
{
    io_connect_t result;

    if (conn != 0) {
        result = IOServiceClose(conn);
        if (result == 0) 
            conn = 0;
        return result;
    }
    return -1;
}

IOReturn
SMCCall(int index,   SMCParamStruct* input,  SMCParamStruct* output)
{
    IOReturn result;

    if (conn == 0 && (result = SMCOpen()) != 0)
        return result;

    result = IOConnectCallMethod(
                                          conn, 
                                          kSMCUserClientOpen, 
                                          NULL, 
                                          0, 
                                          NULL, 
                                          0, 
                                          NULL, 
                                          NULL, 
                                          NULL, 
                                          NULL);
    if (kIOReturnSuccess != result)
        return result;
     
    size_t outSize = sizeof(SMCParamStruct);
    result = IOConnectCallStructMethod(
                                       conn, 
                                       index, 
                                       input, 
                                       sizeof(SMCParamStruct), 
                                       output, 
                                       &outSize);
    IOConnectCallMethod(
                        conn, 
                        kSMCUserClientClose, 
                        NULL, 
                        0, 
                        NULL, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL);
    return result;
}

IOReturn
SMCReadKey(uint32_t     key, 
         uint8_t*     out, 
         uint8_t*     size)
{
    SMCParamStruct input;
    SMCParamStruct output;
     
    bzero(out,     *size);
    bzero(&input,  sizeof(SMCParamStruct));
    bzero(&output, sizeof(SMCParamStruct));
     
    // Advise the SMC to get meta-info about a key
    input.data8 = kSMCGetKeyInfo;
    input.key   = key;

    IOReturn result = SMCCall( kSMCHandleYPCEvent, 
                               &input, 
                               &output);
    if (kSMCSuccess != result)
        return result;
     
    // Get the value for the key
    input.data8 = kSMCReadKey;
    input.key   = key;
    input.keyInfo.dataSize = output.keyInfo.dataSize;
     
    bzero(&output, sizeof(SMCParamStruct));
     
    result = SMCCall(kSMCHandleYPCEvent, &input, &output);
    if (kSMCSuccess != result)
        return result;
     
    // Copy the data to out
    if (*size > input.keyInfo.dataSize)
        *size = input.keyInfo.dataSize;
     
    memcpy(out, output.bytes, *size);
    return kSMCSuccess;
}

uint16_t
SMCGetFanRPM(const char *key_text) {
    uint8_t buffer[256];
    uint8_t size=255;
    uint16_t result;
    int i;
    uint32_t key;

    key = key_to_int(key_text, 4);
    result = SMCReadKey(key, buffer, &size);
    if (result != 0) 
        return -1;
    
    result = ((uint16_t)buffer[1] + ((uint16_t)buffer[0] << 8)) >> 2;
    return 0;
}

double
SMCGetTemperature(const char *key_text) {
    uint8_t buffer[256];
    uint8_t size=255;
    double result;
    uint32_t key;
    char bin[256];

    key = key_to_int(key_text, 4);
    result = SMCReadKey(key, buffer, &size);
    if (result != 0) 
        return -10000.0;

    result = ((uint16_t)buffer[1] + ((uint16_t)buffer[0] << 8)) / 256.0;
    SMCClose();
    return result;
}

double
SMCGetCPUTemp(void) {
  double result;
  result = SMCGetTemperature("TC0D");
  return result;
}

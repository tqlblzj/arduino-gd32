/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef _GD_HARDWARERTC_H_
#define _GD_HARDWARERTC_H_

#include "rtc.h"
extern class HWRTC rtc;
typedef void(*RTCCallback_t)(void);

class HWRTC {
public:
    HWRTC(void);                                                  //RTC construct
    void setUTCTime(UTCTimeStruct *utcTime);                      //set UTC time
    void getUTCTime(UTCTimeStruct *utcTime);                      //get UTC time from base time
    void setSecTime(uint32_t secTime);                            //set second time from base time
    uint32_t getSecTime(void);                                    //get second time from base time
    void setAlarmTime(uint32_t offset, ALARM_OFFSET_FORMAT mode); //set alarm clock time base time
    void attachInterrupt(RTCCallback_t callback, INT_MODE mode);  //attach RTC interrupt
    void detachInterrupt(INT_MODE mode);                          //detach RTC interrupt
    void interruptHandler(INT_MODE mode);
private:
    UTCTimeStruct UTCTime;//time base
    RTCCallback_t callback[2] = {0};
};

#endif

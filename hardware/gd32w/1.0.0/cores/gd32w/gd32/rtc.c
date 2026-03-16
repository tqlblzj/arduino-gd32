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

#include <stdlib.h>
#include <time.h>
#include "gd32vw55x_platform.h"
#include "rtc.h"

/*!
    \brief      rtc init
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_hal_init(void)
{
    // eclic_irq_enable(RTC_WKUP_IRQn, 8, 0);
    // eclic_irq_enable(RTC_Alarm_IRQn, 8, 0);
}

extern uint32_t prescaler_a;
extern uint32_t prescaler_s;

/*!
    \brief      rtc set UTC time
    \param[in]  utcTime: point to UTC format time
    \param[out] none
    \retval     none
*/
void rtc_setUTCTime(UTCTimeStruct *utcTime)
{
    uint32_t secTime = mkTimtoStamp(utcTime);
    rtc_setSecTime(secTime);
}

/*!
    \brief      rtc get UTC time
    \param[in]  utcTime: point to UTC format time
    \param[out] none
    \retval     none
*/
void rtc_getUTCTime(UTCTimeStruct *utcTime)
{
    rtc_parameter_struct rtc_time = {0};
    prescaler_s = 0x61a7;  // 25000;
    prescaler_a = 0x31;  // 50;

    rtc_time.factor_asyn = prescaler_a;

    rtc_time.factor_syn = prescaler_s;

    rtc_current_time_get(&rtc_time);

    // BCD to decimal conversion
    utcTime->year = ((rtc_time.year >> 4) * 10) + (rtc_time.year & 0x0F) + 1900;

    utcTime->month = ((rtc_time.month >> 4) * 10) + (rtc_time.month & 0x0F);

    utcTime->day = ((rtc_time.date >> 4) * 10) + (rtc_time.date & 0x0F);
    utcTime->hour = ((rtc_time.hour >> 4) * 10) + (rtc_time.hour & 0x0F);

    utcTime->minutes = ((rtc_time.minute >> 4) * 10) + (rtc_time.minute & 0x0F);

    utcTime->seconds = ((rtc_time.second >> 4) * 10) + (rtc_time.second & 0x0F);
}

/*!
    \brief      rtc set second time
    \param[in]  secTime: second counts
    \param[out] none
    \retval     none
*/
void rtc_setSecTime(uint32_t secTime)
{
    time_t time1 = secTime;
    struct tm *p_tm1 = gmtime(&time1);
    rtc_parameter_struct rtc_initpara = {0};

    /* setup RTC time value */
    prescaler_s = 0x61a7;  // 25000;
    prescaler_a = 0x31;  // 50;
    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;

    // Decimal to BCD conversion
    rtc_initpara.year = ((p_tm1->tm_year / 10) << 4) | (p_tm1->tm_year % 10);
    rtc_initpara.day_of_week = (p_tm1->tm_wday == 0) ? RTC_SUNDAY : (p_tm1->tm_wday);
    rtc_initpara.month = (((p_tm1->tm_mon + 1) / 10) << 4) | (p_tm1->tm_mon + 1) % 10;
    rtc_initpara.date = ((p_tm1->tm_mday / 10) << 4) | (p_tm1->tm_mday % 10);
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_PM;
    rtc_initpara.hour = ((p_tm1->tm_hour / 10) << 4) | (p_tm1->tm_hour % 10);
    rtc_initpara.minute = ((p_tm1->tm_min / 10) << 4) | (p_tm1->tm_min % 10);
    rtc_initpara.second = ((p_tm1->tm_sec / 10) << 4) | (p_tm1->tm_sec % 10);

    if (ERROR == rtc_init(&rtc_initpara)) {
        printf("RTC time configuration failed!\r\n");
    }
}

/*!
    \brief      rtc get second time
    \param[in]  none
    \param[out] none
    \retval     second counts
*/
uint32_t rtc_getSecTime(void)
{
    UTCTimeStruct utc;
    rtc_getUTCTime(&utc);
    return mkTimtoStamp(&utc);
}

/*!
    \brief      rtc set alarm time
    \param[in]  alarmTime: alarm time
    \param[out] none
    \retval     second counts
*/
void rtc_setAlarmTime(uint32_t alarmTime)
{
    rtc_alarm_struct alarm;
    time_t time1 = alarmTime;
    struct tm *p_tm1 = gmtime(&time1);


    // Decimal to BCD conversion
    alarm.alarm_mask = RTC_ALARM_NONE_MASK;
    alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
    alarm.alarm_day = ((p_tm1->tm_mday / 10) << 4) | (p_tm1->tm_mday % 10);
    // alarm.alarm_day = p_tm1->tm_mday;
    alarm.alarm_hour = ((p_tm1->tm_hour / 10) << 4) | (p_tm1->tm_hour % 10);
    alarm.alarm_minute = ((p_tm1->tm_min / 10) << 4) | (p_tm1->tm_min % 10);
    alarm.alarm_second = ((p_tm1->tm_sec / 10) << 4) | (p_tm1->tm_sec % 10);
    alarm.am_pm = RTC_PM;

    rtc_alarm_config(RTC_ALARM0, &alarm);
}

/*!
    \brief      rtc irq handler
    \param[in]  mode
    \param[out] none
    \retval     none
*/
void RTC_WKUP_IRQ_Handler(void)
{
    /* clear pending interrupt bit */
    if (SET == exti_flag_get(RTC_WAKEUP_EXTI_LINE)) {
        exti_interrupt_flag_clear(RTC_WAKEUP_EXTI_LINE);
    }
    RTC_Handler(INT_SECOND_MODE);
}

/*!
    \brief      rtc alarm irq handler
    \param[in]  mode
    \param[out] none
    \retval     none
*/
void RTC_Alarm_IRQ_Handler(void)
{
    if(rtc_flag_get(RTC_STAT_ALRM0F) != RESET) {
        rtc_flag_clear(RTC_STAT_ALRM0F);
        exti_flag_clear(EXTI_17);
        exti_interrupt_flag_clear(EXTI_17);
    }
    RTC_Handler(INT_ALARM_MODE);
}

/*!
    \brief      rtc attach interrupt
    \param[in]  mode: interrupt mode
    \param[out] none
    \retval     none
*/
void rtc_attachInterrupt(INT_MODE mode)
{
    uint32_t interrupt = 0;
    switch(mode) {
        case INT_SECOND_MODE:
            interrupt = RTC_INT_WAKEUP;
            exti_init(RTC_WAKEUP_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
            rtc_flag_clear(RTC_STAT_WTF);
            rtc_wakeup_disable();
            rtc_wakeup_timer_set(1 * 2000);  /* unit: 500us */
            rtc_wakeup_isr_register(RTC_WKUP_IRQ_Handler);
            rtc_wakeup_enable();
            eclic_irq_enable(RTC_WKUP_IRQn, 8, 0);
            break;
        case INT_ALARM_MODE:
            interrupt = RTC_INT_ALARM0;
            exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
            rtc_flag_clear(RTC_STAT_ALRM0F);
            rtc_alarm_disable(RTC_ALARM0);
            rtc_alarm_isr_register(RTC_Alarm_IRQ_Handler);
            rtc_alarm_enable(RTC_ALARM0);
            eclic_irq_enable(RTC_Alarm_IRQn, 8, 0);
            break;
        default:
            return;
    }
    rtc_interrupt_enable(interrupt);
}

/*!
    \brief      rtc detach interrupt
    \param[in]  mode: interrupt mode
    \param[out] none
    \retval     none
*/
void rtc_detachInterrupt(INT_MODE mode)
{
    uint32_t interrupt = 0;
    switch(mode) {
        case INT_SECOND_MODE:
            interrupt = RTC_INT_WAKEUP;
            break;
        case INT_ALARM_MODE:
            interrupt = RTC_INT_ALARM0;
            break;
        default:
            return;
    }
    rtc_interrupt_disable(interrupt);
}

/*!
    \brief      get month length
    \param[in]  lpyr: is leap year
    \param[in]  mon: month
    \param[out] none
    \retval     month lenth
*/
uint8_t monthLength(uint8_t lpyr, uint8_t mon)
{
    uint8_t days = 30;

    if(mon == 2) {  // feb
        days = (28 + lpyr);
    } else {
        if(mon > 7) {  // aug-dec
            mon--;
        }

        if(mon & 1) {
            days = 31;
        }
    }

    return (days);
}

/*!
    \brief      make utcTime to second counts
    \param[in]  alarmTime: alarm time
    \param[out] none
    \retval     second counts
*/
uint32_t mkTimtoStamp(UTCTimeStruct *utcTime)
{
    uint16_t year = utcTime->year;
    uint8_t mon = utcTime->month;
    uint8_t day = utcTime->day;
    uint32_t numDays = 0;
    uint32_t timestamp = 0;
    while(1969 != --year) {
        if(IsLeapYear(year)) {
            numDays += 366;
        } else {
            numDays += 365;
        }
    }
    while(0 != --mon) {
        numDays += monthLength(IsLeapYear(utcTime->year), mon);
    }
    numDays = numDays + day - 1;
    timestamp = numDays * SECONDS_PER_DAY + (utcTime->hour * 3600 + utcTime->minutes * 60 +
                                             utcTime->seconds);
    return timestamp;
}

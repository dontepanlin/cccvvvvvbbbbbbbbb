/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <kern/kclock.h>
#include <inc/time.h>
#include <inc/vsyscall.h>
#include <kern/vsyscall.h>
#include <inc/string.h>

#define RTC_GETTIME_REG_NUM (6)

static void wait_while_updating()
{
	uint8_t reg_a = ~0;
	while (reg_a & RTC_UPDATE_IN_PROGRESS) {
		outb(IO_RTC_CMND, RTC_AREG);
		reg_a = inb(IO_RTC_DATA);
	}
}

static void read_once(uint8_t bcd_data_out[RTC_GETTIME_REG_NUM])
{
	uint8_t cmnd[RTC_GETTIME_REG_NUM] = {
		RTC_SEC,
		RTC_MIN,
		RTC_HOUR,
		RTC_DAY,
		RTC_MON,
		RTC_YEAR
	};

	for (int32_t i = 0; i < RTC_GETTIME_REG_NUM; i++) {
		outb(IO_RTC_CMND, cmnd[i]);
		bcd_data_out[i] = inb(IO_RTC_DATA);
	}
}

int gettime(void)
{
	nmi_disable();
	// LAB 12: your code here
	int32_t tries = 0;
	uint8_t bcd_data_one[RTC_GETTIME_REG_NUM];
	while (tries++ < 3) {
		uint8_t bcd_data_two[RTC_GETTIME_REG_NUM];
		wait_while_updating();
		read_once(bcd_data_two);
		if (memcmp(bcd_data_one, bcd_data_two, RTC_GETTIME_REG_NUM) == 0) {
			break;
		}
		memmove(bcd_data_one, bcd_data_two, RTC_GETTIME_REG_NUM);
	}
	nmi_enable();

	if (tries >= 3) {
		panic("failed to read clock");
	}

	struct tm time = {
		BCD2BIN(bcd_data_one[0]),
		BCD2BIN(bcd_data_one[1]),
		BCD2BIN(bcd_data_one[2]),
		BCD2BIN(bcd_data_one[3]),
		BCD2BIN(bcd_data_one[4]) -1,
		BCD2BIN(bcd_data_one[5])
	};
	int32_t unixtime = timestamp(&time);
	return unixtime;
}

void
rtc_init(void)
{
	nmi_disable();
	// LAB 4: your code here

	outb(IO_RTC_CMND, RTC_BREG);

	uint8_t reg_b = inb(IO_RTC_DATA);
	reg_b |= RTC_PIE;
	outb(IO_RTC_CMND, RTC_BREG);
	outb(IO_RTC_DATA, reg_b);
	outb(IO_RTC_CMND, RTC_BREG);
	uint8_t reg_a = inb(IO_RTC_DATA);

	reg_a |= 0x0F;
	outb(IO_RTC_CMND, RTC_AREG);
	outb(IO_RTC_DATA, reg_a);
	nmi_enable();
	
	vsys[VSYS_gettime] = gettime();
}

uint8_t
rtc_check_status(void)
{
	uint8_t status = 0;
	// LAB 4: your code here
	nmi_disable();

	outb(IO_RTC_CMND, RTC_CREG);
	status = inb(IO_RTC_DATA);

	nmi_enable();


	return status;
}

unsigned
mc146818_read(unsigned reg)
{
	outb(IO_RTC_CMND, reg);
	return inb(IO_RTC_DATA);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(IO_RTC_CMND, reg);
	outb(IO_RTC_DATA, datum);
}


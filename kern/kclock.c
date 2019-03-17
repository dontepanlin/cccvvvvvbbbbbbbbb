/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <kern/kclock.h>

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

/*	$NetBSD: linux_ioctl.h,v 1.2 1995/08/16 04:14:58 mycroft Exp $	*/

/*
 * Copyright (c) 1995 Frank van der Linden
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project
 *      by Frank van der Linden
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LINUX_IOCTL_H
#define _LINUX_IOCTL_H

#define _LINUX_IO(c,n) (((c)<< 8) | (n))

#define LINUX_TCGETS		_LINUX_IO('T',1)
#define LINUX_TCSETS		_LINUX_IO('T',2)
#define LINUX_TCSETSW		_LINUX_IO('T',3)
#define LINUX_TCSETSF		_LINUX_IO('T',4)
#define LINUX_TCGETA		_LINUX_IO('T',5)
#define LINUX_TCSETA		_LINUX_IO('T',6)
#define LINUX_TCSETAW		_LINUX_IO('T',7)
#define LINUX_TCSETAF		_LINUX_IO('T',8)
#define LINUX_TCSBRK		_LINUX_IO('T',9)
#define LINUX_TCXONC		_LINUX_IO('T',10)
#define LINUX_TCFLSH		_LINUX_IO('T',11)
#define LINUX_TIOCEXCL		_LINUX_IO('T',12)
#define LINUX_TIOCNXCL		_LINUX_IO('T',13)
#define LINUX_TIOCSCTTY		_LINUX_IO('T',14)
#define LINUX_TIOCGPGRP		_LINUX_IO('T',15)
#define LINUX_TIOCSPGRP		_LINUX_IO('T',16)
#define LINUX_TIOCOUTQ		_LINUX_IO('T',17)
#define LINUX_TIOCSTI		_LINUX_IO('T',18)
#define LINUX_TIOCGWINSZ	_LINUX_IO('T',19)
#define LINUX_TIOCSWINSZ	_LINUX_IO('T',20)
#define LINUX_TIOCMGET		_LINUX_IO('T',21)
#define LINUX_TIOCMBIS		_LINUX_IO('T',22)
#define LINUX_TIOCMBIC		_LINUX_IO('T',23)
#define LINUX_TIOCMSET		_LINUX_IO('T',24)
#define LINUX_TIOCGSOFTCAR	_LINUX_IO('T',25)
#define LINUX_TIOCSSOFTCAR	_LINUX_IO('T',26)
#define LINUX_FIONREAD		_LINUX_IO('T',27)
#define LINUX_TIOCINQ		LINUX_FIONREAD
#define LINUX_TIOCLINUX		_LINUX_IO('T',28)
#define LINUX_TIOCCONS		_LINUX_IO('T',29)
#define LINUX_TIOCGSERIAL	_LINUX_IO('T',30)
#define LINUX_TIOCSSERIAL	_LINUX_IO('T',31)
#define LINUX_TIOCPKT		_LINUX_IO('T',32)
#define LINUX_FIONBIO		_LINUX_IO('T',33)
#define LINUX_TIOCNOTTY		_LINUX_IO('T',34)
#define LINUX_TIOCSETD		_LINUX_IO('T',35)
#define LINUX_TIOCGETD		_LINUX_IO('T',36)
#define LINUX_TCSBRKP		_LINUX_IO('T',37)
#define LINUX_TIOCTTYGSTRUCT	_LINUX_IO('T',38)

#define LINUX_FIONCLEX		_LINUX_IO('T',80)
#define LINUX_FIOCLEX		_LINUX_IO('T',81)
#define LINUX_FIOASYNC		_LINUX_IO('T',82)
#define LINUX_TIOCSERCONFIG	_LINUX_IO('T',83)
#define LINUX_TIOCSERGWILD	_LINUX_IO('T',84)
#define LINUX_TIOCSERSWILD	_LINUX_IO('T',85)
#define LINUX_TIOCGLCKTRMIOS	_LINUX_IO('T',86)
#define LINUX_TIOCSLCKTRMIOS	_LINUX_IO('T',87)
#define LINUX_TIOCSERGSTRUCT	_LINUX_IO('T',88)
#define LINUX_TIOCSERGETLSR	_LINUX_IO('T',89)


#define LINUX_NCC 8
struct linux_termio {
	unsigned short c_iflag;
	unsigned short c_oflag;
	unsigned short c_cflag;
	unsigned short c_lflag;
	unsigned char c_line;
	unsigned char c_cc[LINUX_NCC];
};

typedef unsigned char linux_cc_t;
typedef unsigned long linux_tcflag_t;

#define LINUX_NCCS 19
struct linux_termios {
	linux_tcflag_t c_iflag;
	linux_tcflag_t c_oflag;
	linux_tcflag_t c_cflag;
	linux_tcflag_t c_lflag;
	linux_cc_t c_line;
	linux_cc_t c_cc[LINUX_NCCS];
};

/* Just in old style linux_termio struct */
#define LINUX_VINTR 0
#define LINUX_VQUIT 1
#define LINUX_VERASE 2
#define LINUX_VKILL 3
#define LINUX_VEOF 4
#define LINUX_VTIME 5
#define LINUX_VMIN 6
#define LINUX_VSWTC 7

/* In the termios struct too */
#define LINUX_VSTART 8
#define LINUX_VSTOP 9
#define LINUX_VSUSP 10
#define LINUX_VEOL 11
#define LINUX_VREPRINT 12
#define LINUX_VDISCARD 13
#define LINUX_VWERASE 14
#define LINUX_VLNEXT 15
#define LINUX_VEOL2 16

/* Linux c_iflag masks */
#define LINUX_IGNBRK	0x0000001
#define LINUX_BRKINT	0x0000002
#define LINUX_IGNPAR	0x0000004
#define LINUX_PARMRK	0x0000008
#define LINUX_INPCK	0x0000010
#define LINUX_ISTRIP	0x0000020
#define LINUX_INLCR	0x0000040
#define LINUX_IGNCR	0x0000080
#define LINUX_ICRNL	0x0000100
#define LINUX_IUCLC	0x0000200
#define LINUX_IXON	0x0000400
#define LINUX_IXANY	0x0000800
#define LINUX_IXOFF	0x0001000
#define LINUX_IMAXBEL	0x0002000

/* Linux c_oflag masks */
#define LINUX_OPOST	0x0000001
#define LINUX_OLCUC	0x0000002
#define LINUX_ONLCR	0x0000004
#define LINUX_OCRNL	0x0000008
#define LINUX_ONOCR	0x0000010
#define LINUX_ONLRET	0x0000020
#define LINUX_OFILL	0x0000040
#define LINUX_OFDEL	0x0000080
#define LINUX_NLDLY	0x0000100

#define LINUX_NL0	0x0000000
#define LINUX_NL1	0x0000100
#define LINUX_CRDLY	0x0000600
#define LINUX_CR0	0x0000000
#define LINUX_CR1	0x0000200
#define LINUX_CR2	0x0000400
#define LINUX_CR3	0x0000600
#define LINUX_TABDLY	0x0001800
#define LINUX_TAB0	0x0000000
#define LINUX_TAB1	0x0000800
#define LINUX_TAB2	0x0001000
#define LINUX_TAB3	0x0001800
#define LINUX_XTABS	0x0001800
#define LINUX_BSDLY	0x0002000
#define LINUX_BS0	0x0000000
#define LINUX_BS1	0x0002000
#define LINUX_VTDLY	0x0004000
#define LINUX_VT0	0x0000000
#define LINUX_VT1	0x0004000
#define LINUX_FFDLY	0x0008000
#define LINUX_FF0	0x0000000
#define LINUX_FF1	0x0008000

/* Linux c_cflag bit masks */

#define LINUX_NSPEEDS   16
#define LINUX_NXSPEEDS   2

#define LINUX_CBAUD	0x0000100f

#define LINUX_B0	0x00000000
#define LINUX_B50	0x00000001
#define LINUX_B75	0x00000002
#define LINUX_B110	0x00000003
#define LINUX_B134	0x00000004
#define LINUX_B150	0x00000005
#define LINUX_B200	0x00000006
#define LINUX_B300	0x00000007
#define LINUX_B600	0x00000008
#define LINUX_B1200	0x00000009
#define LINUX_B1800	0x0000000a
#define LINUX_B2400	0x0000000b
#define LINUX_B4800	0x0000000c
#define LINUX_B9600	0x0000000d
#define LINUX_B19200	0x0000000e
#define LINUX_B38400	0x0000000f
#define LINUX_EXTA	LINUX_B19200
#define LINUX_EXTB	LINUX_B38400
#define LINUX_CBAUDEX	0x00001000
#define LINUX_B57600	0x00001001
#define LINUX_B115200	0x00001002
#define LINUX_B230400	0x00001003

#define LINUX_CSIZE	0x00000030
#define LINUX_CS5	0x00000000
#define LINUX_CS6	0x00000010
#define LINUX_CS7	0x00000020
#define LINUX_CS8	0x00000030
#define LINUX_CSTOPB	0x00000040
#define LINUX_CREAD	0x00000080
#define LINUX_PARENB	0x00000100
#define LINUX_PARODD	0x00000200
#define LINUX_HUPCL	0x00000400
#define LINUX_CLOCAL	0x00000800

#define LINUX_CRTSCTS	0x80000000

/* Linux c_lflag masks */
#define LINUX_ISIG	0x00000001
#define LINUX_ICANON	0x00000002
#define LINUX_XCASE	0x00000004
#define LINUX_ECHO	0x00000008
#define LINUX_ECHOE	0x00000010
#define LINUX_ECHOK	0x00000020
#define LINUX_ECHONL	0x00000040
#define LINUX_NOFLSH	0x00000080
#define LINUX_TOSTOP	0x00000100
#define LINUX_ECHOCTL	0x00000200
#define LINUX_ECHOPRT	0x00000400
#define LINUX_ECHOKE	0x00000800
#define LINUX_FLUSHO	0x00001000
#define LINUX_PENDIN	0x00002000
#define LINUX_IEXTEN	0x00008000

/* Linux modem line defines.. not sure if they'll be used */
#define LINUX_TIOCM_LE		0x0001
#define LINUX_TIOCM_DTR		0x0002
#define LINUX_TIOCM_RTS		0x0004
#define LINUX_TIOCM_ST		0x0008
#define LINUX_TIOCM_SR		0x0010
#define LINUX_TIOCM_CTS		0x0020
#define LINUX_TIOCM_CAR		0x0040
#define LINUX_TIOCM_RNG		0x0080
#define LINUX_TIOCM_DSR		0x0100
#define LINUX_TIOCM_CD		LINUX_TIOCM_CAR
#define LINUX_TIOCM_RI 		LINUX_TIOCM_RNG

#define	LINUX_TCIFLUSH		0
#define	LINUX_TCOFLUSH		1
#define	LINUX_TCIOFLUSH		2

#define	LINUX_TCOOFF		0
#define	LINUX_TCOON		1
#define	LINUX_TCIOFF		2
#define	LINUX_TCION		3

#define	LINUX_TCSANOW		0
#define	LINUX_TCSADRAIN		1
#define	LINUX_TCSAFLUSH		2

/* Linux line disciplines */
#define LINUX_N_TTY		0
#define LINUX_N_SLIP		1
#define LINUX_N_MOUSE		2
#define LINUX_N_PPP		3

#endif /* !_LINUX_IOCTL_H */

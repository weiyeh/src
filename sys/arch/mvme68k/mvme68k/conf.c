/*	$NetBSD: conf.c,v 1.2 1995/08/17 17:40:54 thorpej Exp $	*/

/*-
 * Copyright (c) 1991 The Regents of the University of California.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)conf.c	7.9 (Berkeley) 5/28/91
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vnode.h>

int	ttselect	__P((dev_t, int, struct proc *));

#ifdef LKM
int	lkmenodev();
#else
#define	lkmenodev	enodev
#endif

#include "vnd.h"
bdev_decl(vnd);
bdev_decl(sw);
#include "sd.h"
bdev_decl(sd);
#include "cd.h"
bdev_decl(cd);
#include "st.h"
bdev_decl(st);
#include "ccd.h"
bdev_decl(ccd);

struct bdevsw	bdevsw[] =
{
	bdev_notdef(),			/* 0 */
	bdev_notdef(),			/* 1 */
	bdev_notdef(),			/* 2 */
	bdev_swap_init(1,sw),		/* 3: swap pseudo-device */
	bdev_disk_init(NSD,sd),		/* 4: SCSI disk */
	bdev_tape_init(NST,st),		/* 5: SCSI tape */
	bdev_disk_init(NVND,vnd),	/* 6: vnode disk driver */
	bdev_disk_init(NCD,cd),		/* 7: SCSI CD-ROM */
	bdev_notdef(),			/* 8 */
	bdev_lkm_dummy(),		/* 9 */
	bdev_lkm_dummy(),		/* 10 */
	bdev_lkm_dummy(),		/* 11 */
	bdev_lkm_dummy(),		/* 12 */
	bdev_lkm_dummy(),		/* 13 */
	bdev_lkm_dummy(),		/* 14 */
	bdev_disk_init(NCCD,ccd),	/* 15: concatenated disk driver */
};

int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

cdev_decl(cn);
cdev_decl(ctty);
#define mmread  mmrw
#define mmwrite mmrw
cdev_decl(mm);
cdev_decl(sw);

#include "pty.h"
#define ptstty		ptytty
#define	ptsioctl	ptyioctl
cdev_decl(pts);
#define ptctty		ptytty
#define	ptcioctl	ptyioctl
cdev_decl(ptc);

cdev_decl(log);
cdev_decl(fd);

#include "zs.h"
cdev_decl(zs);

cdev_decl(sd);
cdev_decl(cd);
cdev_decl(st);
cdev_decl(vnd);
cdev_decl(ccd);

#include "bpfilter.h"
cdev_decl(bpf);

#include "tun.h"
cdev_decl(tun);
#ifdef LKM
#define NLKM 1
#else
#define NLKM 0
#endif
cdev_decl(lkm);

struct cdevsw	cdevsw[] =
{
	cdev_cn_init(1,cn),		/* 0: virtual console */
	cdev_ctty_init(1,ctty),		/* 1: controlling terminal */
	cdev_mm_init(1,mm),		/* 2: /dev/{null,mem,kmem,...} */
	cdev_swap_init(1,sw),		/* 3: /dev/drum (swap pseudo-device) */
	cdev_tty_init(NPTY,pts),	/* 4: pseudo-tty slave */
	cdev_ptc_init(NPTY,ptc),	/* 5: pseudo-tty master */
	cdev_log_init(1,log),		/* 6: /dev/klog */
	cdev_notdef(),			/* 7 */
	cdev_disk_init(NSD,sd),		/* 8: SCSI disk */
	cdev_disk_init(NCD,cd),		/* 9: SCSI CD-ROM */
	cdev_notdef(),			/* 10 */
	cdev_notdef(),			/* 11: parallel interface */
	cdev_tty_init(NZS,zs),		/* 12: SCC serial ports */
	cdev_notdef(),			/* 13 */
	cdev_notdef(),			/* 14 */
	cdev_notdef(),			/* 15 */
	cdev_notdef(),			/* 16 */
	cdev_disk_init(NCCD,ccd),	/* 17: concatenated disk driver */
	cdev_notdef(),			/* 18 */
	cdev_disk_init(NVND,vnd),	/* 19: vnode disk */
	cdev_tape_init(NST,st),		/* 20: SCSI tape */
	cdev_fd_init(1,fd),		/* 21: file descriptor pseudo-dev */
	cdev_bpftun_init(NBPFILTER,bpf),/* 22: berkeley packet filter */
	cdev_bpftun_init(NTUN,tun),	/* 23: network tunnel */
	cdev_lkm_init(NLKM,lkm),	/* 24: loadable module driver */
	cdev_lkm_dummy(),		/* 25 */
	cdev_lkm_dummy(),		/* 26 */
	cdev_lkm_dummy(),		/* 27 */
	cdev_lkm_dummy(),		/* 28 */
	cdev_lkm_dummy(),		/* 29 */
	cdev_lkm_dummy(),		/* 30 */
};

int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

int	mem_no = 2; 	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(3, 0);

/*
 * Returns true if dev is /dev/mem or /dev/kmem.
 */
iskmemdev(dev)
	dev_t dev;
{

	return (major(dev) == mem_no && minor(dev) < 2);
}

/*
 * Returns true if dev is /dev/zero.
 */
iszerodev(dev)
	dev_t dev;
{

	return (major(dev) == mem_no && minor(dev) == 12);
}

/*
 * Returns true if dev is a disk device.
 */
isdisk(dev, type)
	dev_t dev;
	int type;
{

	/* XXXX This needs to be dynamic for LKMs. */
	switch (major(dev)) {
	case 1:
	case 2:
	case 4:
	case 6:
	case 15:
		return (type == VBLK);
	case 8:
	case 9:
	case 10:
	case 17:
	case 19:
		return (type == VCHR);
	default:
		return (0);
	}
}

static int chrtoblktbl[] = {
	/* XXXX This needs to be dynamic for LKMs. */
	/*VCHR*/	/*VBLK*/
	/*  0 */	NODEV,
	/*  1 */	NODEV,
	/*  2 */	NODEV,
	/*  3 */	NODEV,
	/*  4 */	NODEV,
	/*  5 */	NODEV,
	/*  6 */	NODEV,
	/*  7 */	NODEV,
	/*  8 */	4,
	/*  9 */	7,
	/* 10 */	NODEV,
	/* 11 */	NODEV,
	/* 12 */	NODEV,
	/* 13 */	NODEV,
	/* 14 */	NODEV,
	/* 15 */	NODEV,
	/* 16 */	NODEV,
	/* 17 */	15,
	/* 18 */	NODEV,
	/* 19 */	6,
	/* 20 */	NODEV,
	/* 21 */	NODEV,
	/* 22 */	NODEV,
};

/*
 * Convert a character device number to a block device number.
 */
chrtoblk(dev)
	dev_t dev;
{
	int blkmaj;

	if (major(dev) >= nchrdev)
		return (NODEV);
	blkmaj = chrtoblktbl[major(dev)];
	if (blkmaj == NODEV)
		return (NODEV);
	return (makedev(blkmaj, minor(dev)));
}

/*
 * This entire table could be autoconfig()ed but that would mean that
 * the kernel's idea of the console would be out of sync with that of
 * the standalone boot.  I think it best that they both use the same
 * known algorithm unless we see a pressing need otherwise.
 */
#include <dev/cons.h>

#define zscnpollc      nullcnpollc
cons_decl(zs);

struct	consdev constab[] = {
#if NZS > 0
	cons_init(zs),
#endif
	{ 0 },
};

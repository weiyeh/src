/*	$NetBSD: hpux_compat.c,v 1.19 1995/10/07 06:26:34 mycroft Exp $	*/

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 * from: Utah $Hdr: hpux_compat.c 1.64 93/08/05$
 *
 *	@(#)hpux_compat.c	8.4 (Berkeley) 2/13/94
 */

/*
 * Various HP-UX compatibility routines
 */

#ifndef COMPAT_43
#define COMPAT_43
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signalvar.h>
#include <sys/kernel.h>
#include <sys/filedesc.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/ioctl.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/ipc.h>
#include <sys/user.h>
#include <sys/mman.h>

#include <machine/cpu.h>
#include <machine/reg.h>
#include <machine/psl.h>
#include <machine/vmparam.h>

#include <sys/syscallargs.h>

#include <compat/hpux/hpux.h>
#include <compat/hpux/hpux_util.h>
#include <compat/hpux/hpux_termio.h>
#include <compat/hpux/hpux_syscall.h>
#include <compat/hpux/hpux_syscallargs.h>

#ifdef DEBUG
int unimpresponse = 0;
#endif

/* 6.0 and later style context */
#if defined(HP380)
char hpux_040context[] =
    "standalone HP-MC68040 HP-MC68881 HP-MC68020 HP-MC68010 localroot default";
#endif
#ifdef FPCOPROC
char hpux_context[] =
	"standalone HP-MC68881 HP-MC68020 HP-MC68010 localroot default";
#else
char hpux_context[] =
	"standalone HP-MC68020 HP-MC68010 localroot default";
#endif

#define NERR	83
#define BERR	1000

/* indexed by BSD errno */
int bsdtohpuxerrnomap[NERR] = {
/*00*/	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
/*10*/	 10,  45,  12,  13,  14,  15,  16,  17,  18,  19,
/*20*/	 20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
/*30*/	 30,  31,  32,  33,  34, 246, 245, 244, 216, 217,
/*40*/	218, 219, 220, 221, 222, 223, 224, 225, 226, 227,
/*50*/	228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
/*60*/	238, 239, 249, 248, 241, 242, 247,BERR,BERR,BERR,
/*70*/   70,  71,BERR,BERR,BERR,BERR,BERR,  46, 251,BERR,
/*80*/ BERR,BERR,  11
};

extern char sigcode[], esigcode[];
extern struct sysent hpux_sysent[];
extern char *hpux_syscallnames[];

struct emul emul_hpux = {
	"hpux",
	bsdtohpuxerrnomap,
	sendsig,
	HPUX_SYS_syscall,
	HPUX_SYS_MAXSYSCALL,
	hpux_sysent,
	hpux_syscallnames,
	0,
	copyargs,
	setregs,
	sigcode,
	esigcode,
};

/*
 * HP-UX fork and vfork need to map the EAGAIN return value appropriately.
 */
int
hpux_sys_fork(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_fork_args *uap = v;
	int error;

	error = sys_fork(p, (struct fork_args *) uap, retval);
	if (error == EAGAIN)
		error = OEAGAIN;
	return (error);
}

int
hpux_sys_vfork(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_vfork_args *uap = v;
	int error;

	error = sys_vfork(p, (struct vfork_args *) uap, retval);
	if (error == EAGAIN)
		error = OEAGAIN;
	return (error);
}

int
hpux_sys_execv(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_execv_args *uap = v;
	struct sys_execve_args ap;

	SCARG(&ap, path) = SCARG(uap, path);
	SCARG(&ap, argp) = SCARG(uap, argp);
	SCARG(&ap, envp) = NULL;

	return sys_execve(p, &ap, retval);
}

/*
 * HP-UX versions of wait and wait3 actually pass the parameters
 * (status pointer, options, rusage) into the kernel rather than
 * handling it in the C library stub.  We also need to map any
 * termination signal from BSD to HP-UX.
 */
int
hpux_sys_wait3(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_wait3_args *uap = v;

	/* rusage pointer must be zero */
	if (SCARG(uap, rusage))
		return (EINVAL);
#if __mc68k__
	p->p_md.md_regs[PS] = PSL_ALLCC;
	p->p_md.md_regs[R0] = SCARG(uap, options);
	p->p_md.md_regs[R1] = SCARG(uap, rusage);
#endif

	return (hpux_sys_wait(p, uap, retval));
}

int
hpux_sys_wait(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_wait_args *uap = v;
	struct sys_wait4_args w4;
	int error;
	int sig;
	size_t sz = sizeof(*SCARG(&w4, status));
	int status;

	SCARG(&w4, rusage) = NULL;
	SCARG(&w4, options) = 0;

	if (SCARG(uap, status) == NULL) {
		caddr_t sg = stackgap_init(p->p_emul);
		SCARG(&w4, status) = stackgap_alloc(&sg, sz);
	}
	else
		SCARG(&w4, status) = SCARG(uap, status);

	SCARG(&w4, pid) = WAIT_ANY;

	error = sys_wait4(p, &w4, retval);
	/*
	 * HP-UX wait always returns EINTR when interrupted by a signal
	 * (well, unless its emulating a BSD process, but we don't bother...)
	 */
	if (error == ERESTART)
		error = EINTR;
	if (error)
		return error;

	if ((error = copyin(SCARG(&w4, status), &status, sizeof(status))) != 0)
		return error;

	sig = status & 0xFF;
	if (sig == WSTOPPED) {
		sig = (status >> 8) & 0xFF;
		retval[1] = (bsdtohpuxsig(sig) << 8) | WSTOPPED;
	} else if (sig)
		retval[1] = (status & 0xFF00) |
			bsdtohpuxsig(sig & 0x7F) | (sig & 0x80);

	if (SCARG(uap, status) == NULL)
		return error;
	else
		return copyout(&retval[1], 
			       SCARG(uap, status), sizeof(retval[1]));
}

int
hpux_sys_waitpid(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_waitpid_args *uap = v;
	int rv, sig, xstat, error;

	SCARG(uap, rusage) = 0;
	error = sys_wait4(p, uap, retval);
	/*
	 * HP-UX wait always returns EINTR when interrupted by a signal
	 * (well, unless its emulating a BSD process, but we don't bother...)
	 */
	if (error == ERESTART)
		error = EINTR;
	if (error)
		return (error);

	if (SCARG(uap, status)) {
		/*
		 * Wait4 already wrote the status out to user space,
		 * pull it back, change the signal portion, and write
		 * it back out.
		 */
		rv = fuword((caddr_t)SCARG(uap, status));
		if (WIFSTOPPED(rv)) {
			sig = WSTOPSIG(rv);
			rv = W_STOPCODE(bsdtohpuxsig(sig));
		} else if (WIFSIGNALED(rv)) {
			sig = WTERMSIG(rv);
			xstat = WEXITSTATUS(rv);
			rv = W_EXITCODE(xstat, bsdtohpuxsig(sig)) |
				WCOREDUMP(rv);
		}
		(void)suword((caddr_t)SCARG(uap, status), rv);
	}
	return (error);
}

/*
 * Old creat system call.
 */
int
hpux_sys_creat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_creat_args *uap = v;
	struct sys_open_args oap;

	SCARG(&oap, path) = SCARG(uap, path);
	SCARG(&oap, flags) = O_WRONLY | O_CREAT | O_TRUNC;
	SCARG(&oap, mode) = SCARG(uap, mode);

	return sys_open(p, &oap, retval);
}

/*
 * XXX extensions to the fd_ofileflags flags.
 * Hate to put this there, but they do need to be per-file.
 */
#define UF_NONBLOCK_ON	0x10
#define	UF_FNDELAY_ON	0x20
#define	UF_FIONBIO_ON	0x40

/*
 * Must remap some bits in the mode mask.
 * O_CREAT, O_TRUNC, and O_EXCL must be remapped,
 * O_NONBLOCK is remapped and remembered,
 * O_FNDELAY is remembered,
 * O_SYNCIO is removed entirely.
 */
int
hpux_sys_open(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_open_args *uap = v;
	int mode, error;

	mode = SCARG(uap, mode);
	SCARG(uap, mode) &=
		~(HPUXNONBLOCK|HPUXFSYNCIO|HPUXFEXCL|HPUXFTRUNC|HPUXFCREAT);
	if (mode & HPUXFCREAT) {
		/*
		 * simulate the pre-NFS behavior that opening a
		 * file for READ+CREATE ignores the CREATE (unless
		 * EXCL is set in which case we will return the
		 * proper error).
		 */
		if ((mode & HPUXFEXCL) || (FFLAGS(mode) & FWRITE))
			SCARG(uap, mode) |= O_CREAT;
	}
	if (mode & HPUXFTRUNC)
		SCARG(uap, mode) |= O_TRUNC;
	if (mode & HPUXFEXCL)
		SCARG(uap, mode) |= O_EXCL;
	if (mode & HPUXNONBLOCK)
		SCARG(uap, mode) |= O_NDELAY;
	error = sys_open(p, (struct open_args *) uap, retval);
	/*
	 * Record non-blocking mode for fcntl, read, write, etc.
	 */
	if (error == 0 && (SCARG(uap, mode) & O_NDELAY))
		p->p_fd->fd_ofileflags[*retval] |=
			(mode & HPUXNONBLOCK) ? UF_NONBLOCK_ON : UF_FNDELAY_ON;
	return (error);
}

int
hpux_sys_fcntl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_fcntl_args *uap = v;
	int mode, error, flg = F_POSIX;
	struct file *fp;
	char *pop;
	struct hpux_flock hfl;
	struct flock fl;
	struct vnode *vp;

	if ((unsigned)SCARG(uap, fd) >= p->p_fd->fd_nfiles ||
	    (fp = p->p_fd->fd_ofiles[SCARG(uap, fd)]) == NULL)
		return (EBADF);
	pop = &p->p_fd->fd_ofileflags[SCARG(uap, fd)];
	switch (SCARG(uap, cmd)) {
	case F_SETFL:
		if (SCARG(uap, arg) & HPUXNONBLOCK)
			*pop |= UF_NONBLOCK_ON;
		else
			*pop &= ~UF_NONBLOCK_ON;
		if (SCARG(uap, arg) & HPUXNDELAY)
			*pop |= UF_FNDELAY_ON;
		else
			*pop &= ~UF_FNDELAY_ON;
		if (*pop & (UF_NONBLOCK_ON|UF_FNDELAY_ON|UF_FIONBIO_ON))
			SCARG(uap, arg) |= FNONBLOCK;
		else
			SCARG(uap, arg) &= ~FNONBLOCK;
		SCARG(uap, arg) &= ~(HPUXNONBLOCK|HPUXFSYNCIO|HPUXFREMOTE);
		break;
	case F_GETFL:
	case F_DUPFD:
	case F_GETFD:
	case F_SETFD:
		break;

	case HPUXF_SETLKW:
		flg |= F_WAIT;
		/* Fall into F_SETLK */

	case HPUXF_SETLK:
		if (fp->f_type != DTYPE_VNODE)
			return (EBADF);
		vp = (struct vnode *)fp->f_data;
		/* Copy in the lock structure */
		error = copyin((caddr_t)SCARG(uap, arg), (caddr_t)&hfl,
		    sizeof (hfl));
		if (error)
			return (error);
		fl.l_start = hfl.hl_start;
		fl.l_len = hfl.hl_len;
		fl.l_pid = hfl.hl_pid;
		fl.l_type = hfl.hl_type;
		fl.l_whence = hfl.hl_whence;
		if (fl.l_whence == SEEK_CUR)
			fl.l_start += fp->f_offset;
		switch (fl.l_type) {

		case F_RDLCK:
			if ((fp->f_flag & FREAD) == 0)
				return (EBADF);
			p->p_flag |= P_ADVLOCK;
			return (VOP_ADVLOCK(vp, (caddr_t)p, F_SETLK, &fl, flg));

		case F_WRLCK:
			if ((fp->f_flag & FWRITE) == 0)
				return (EBADF);
			p->p_flag |= P_ADVLOCK;
			return (VOP_ADVLOCK(vp, (caddr_t)p, F_SETLK, &fl, flg));

		case F_UNLCK:
			return (VOP_ADVLOCK(vp, (caddr_t)p, F_UNLCK, &fl,
				F_POSIX));

		default:
			return (EINVAL);
		}

	case F_GETLK:
		if (fp->f_type != DTYPE_VNODE)
			return (EBADF);
		vp = (struct vnode *)fp->f_data;
		/* Copy in the lock structure */
		error = copyin((caddr_t)SCARG(uap, arg), (caddr_t)&hfl,
		    sizeof (hfl));
		if (error)
			return (error);
		fl.l_start = hfl.hl_start;
		fl.l_len = hfl.hl_len;
		fl.l_pid = hfl.hl_pid;
		fl.l_type = hfl.hl_type;
		fl.l_whence = hfl.hl_whence;
		if (fl.l_whence == SEEK_CUR)
			fl.l_start += fp->f_offset;
		if (error = VOP_ADVLOCK(vp, (caddr_t)p, F_GETLK, &fl, F_POSIX))
			return (error);
		hfl.hl_start = fl.l_start;
		hfl.hl_len = fl.l_len;
		hfl.hl_pid = fl.l_pid;
		hfl.hl_type = fl.l_type;
		hfl.hl_whence = fl.l_whence;
		return (copyout((caddr_t)&hfl, (caddr_t)SCARG(uap, arg),
		    sizeof (hfl)));

	default:
		return (EINVAL);
	}
	error = sys_fcntl(p, uap, retval);
	if (error == 0 && SCARG(uap, cmd) == F_GETFL) {
		mode = *retval;
		*retval &= ~(O_CREAT|O_TRUNC|O_EXCL);
		if (mode & FNONBLOCK) {
			if (*pop & UF_NONBLOCK_ON)
				*retval |= HPUXNONBLOCK;
			if ((*pop & UF_FNDELAY_ON) == 0)
				*retval &= ~HPUXNDELAY;
		}
		if (mode & O_CREAT)
			*retval |= HPUXFCREAT;
		if (mode & O_TRUNC)
			*retval |= HPUXFTRUNC;
		if (mode & O_EXCL)
			*retval |= HPUXFEXCL;
	}
	return (error);
}

/*
 * Read and write calls.  Same as BSD except for non-blocking behavior.
 * There are three types of non-blocking reads/writes in HP-UX checked
 * in the following order:
 *
 *	O_NONBLOCK: return -1 and errno == EAGAIN
 *	O_NDELAY:   return 0
 *	FIOSNBIO:   return -1 and errno == EWOULDBLOCK
 */
int
hpux_sys_read(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_read_args *uap = v;
	int error;

	error = sys_read(p, (struct sys_read_args *) uap, retval);
	if (error == EWOULDBLOCK) {
		char *fp = &p->p_fd->fd_ofileflags[SCARG(uap, fd)];

		if (*fp & UF_NONBLOCK_ON) {
			*retval = -1;
			error = OEAGAIN;
		} else if (*fp & UF_FNDELAY_ON) {
			*retval = 0;
			error = 0;
		}
	}
	return (error);
}

int
hpux_sys_write(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_write_args *uap = v;
	int error;

	error = sys_write(p, (struct sys_write_args *) uap, retval);
	if (error == EWOULDBLOCK) {
		char *fp = &p->p_fd->fd_ofileflags[SCARG(uap, fd)];

		if (*fp & UF_NONBLOCK_ON) {
			*retval = -1;
			error = OEAGAIN;
		} else if (*fp & UF_FNDELAY_ON) {
			*retval = 0;
			error = 0;
		}
	}
	return (error);
}

int
hpux_sys_readv(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_readv_args *uap = v;
	int error;

	error = sys_readv(p, (struct sys_readv_args *) uap, retval);
	if (error == EWOULDBLOCK) {
		char *fp = &p->p_fd->fd_ofileflags[SCARG(uap, fd)];

		if (*fp & UF_NONBLOCK_ON) {
			*retval = -1;
			error = OEAGAIN;
		} else if (*fp & UF_FNDELAY_ON) {
			*retval = 0;
			error = 0;
		}
	}
	return (error);
}

int
hpux_sys_writev(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_writev_args *uap = v;
	int error;

	error = sys_writev(p, (struct sys_writev_args *) uap, retval);
	if (error == EWOULDBLOCK) {
		char *fp = &p->p_fd->fd_ofileflags[SCARG(uap, fd)];

		if (*fp & UF_NONBLOCK_ON) {
			*retval = -1;
			error = OEAGAIN;
		} else if (*fp & UF_FNDELAY_ON) {
			*retval = 0;
			error = 0;
		}
	}
	return (error);
}

/*
 * 4.3bsd dup allows dup2 to come in on the same syscall entry
 * and hence allows two arguments.  HP-UX dup has only one arg.
 */
int
hpux_sys_dup(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_dup_args *uap = v;
	register struct filedesc *fdp = p->p_fd;
	struct file *fp;
	int fd, error;

	if (((unsigned)SCARG(uap, fd)) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[SCARG(uap, fd)]) == NULL)
		return (EBADF);
	if (error = fdalloc(p, 0, &fd))
		return (error);
	fdp->fd_ofiles[fd] = fp;
	fdp->fd_ofileflags[fd] =
	    fdp->fd_ofileflags[SCARG(uap, fd)] &~ UF_EXCLOSE;
	fp->f_count++;
	if (fd > fdp->fd_lastfile)
		fdp->fd_lastfile = fd;
	*retval = fd;
	return (0);
}

/*
 * XXX: This belongs in hpux_machdep.c !!
 */
int
hpux_sys_utssys(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_utssys_args *uap = v;
	register int i;
	int error;
	struct hpux_utsname	ut;
	extern char ostype[], hostname[], osrelease[], version[], machine[];

	switch (SCARG(uap, request)) {
	/* uname */
	case 0:
		bzero(&ut, sizeof(ut));

		strncpy(ut.sysname, ostype, sizeof(ut.sysname));
		ut.sysname[sizeof(ut.sysname) - 1] = '\0';

		/* copy hostname (sans domain) to nodename */
		for (i = 0; i < 8 && hostname[i] != '.'; i++)
			ut.nodename[i] = hostname[i];
		ut.nodename[i] = '\0';

		strncpy(ut.release, osrelease, sizeof(ut.release));
		ut.release[sizeof(ut.release) - 1] = '\0';

		strncpy(ut.version, version, sizeof(ut.version));
		ut.version[sizeof(ut.version) - 1] = '\0';

		strncpy(ut.machine, "9000/3?0", sizeof(ut.machine));
		ut.machine[sizeof(ut.machine) - 1] = '\0';

		/* fill in machine type */
#ifdef HP_320
		switch (machineid) {
		case HP_320:
			ut.machine[6] = '2';
			break;
		/* includes 318 and 319 */
		case HP_330:
			ut.machine[6] = '3';
			break;
		case HP_340:
			ut.machine[6] = '4';
			break;
		case HP_350:
			ut.machine[6] = '5';
			break;
		case HP_360:
			ut.machine[6] = '6';
			break;
		case HP_370:
			ut.machine[6] = '7';
			break;
		/* includes 345 */
		case HP_375:
			ut.machine[6] = '7';
			ut.machine[7] = '5';
			break;
		/* includes 425 */
		case HP_380:
			ut.machine[6] = '8';
			break;
		case HP_433:
			ut.machine[5] = '4';
			ut.machine[6] = '3';
			ut.machine[7] = '3';
			break;
		}
#endif

		error = copyout((caddr_t)&ut,
		    (caddr_t)SCARG(uap, uts), sizeof(ut));
		break;

	/* gethostname */
	case 5:
		/* SCARG(uap, dev) is length */
		if (SCARG(uap, dev) > hostnamelen + 1)
			SCARG(uap, dev) = hostnamelen + 1;
		error = copyout((caddr_t)hostname, (caddr_t)SCARG(uap, uts),
				SCARG(uap, dev));
		break;

	case 1:	/* ?? */
	case 2:	/* ustat */
	case 3:	/* ?? */
	case 4:	/* sethostname */
	default:
		error = EINVAL;
		break;
	}
	return (error);
}

/*
 * XXX: This belongs in hpux_machdep.c !!
 */
int
hpux_sys_sysconf(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_sysconf_args *uap = v;
	switch (SCARG(uap, name)) {

	/* clock ticks per second */
	case HPUX_SYSCONF_CLKTICK:
		*retval = hz;
		break;

	/* open files */
	case HPUX_SYSCONF_OPENMAX:
		*retval = NOFILE;
		break;

	/* architecture */
	case HPUX_SYSCONF_CPUTYPE:
#ifdef HP_320
		switch (machineid) {
		case HP_320:
		case HP_330:
		case HP_350:
			*retval = HPUX_SYSCONF_CPUM020;
			break;
		case HP_340:
		case HP_360:
		case HP_370:
		case HP_375:
			*retval = HPUX_SYSCONF_CPUM030;
			break;
		case HP_380:
		case HP_433:
			*retval = HPUX_SYSCONF_CPUM040;
			break;
		}
#else
		*retval = HPUX_SYSCONF_CPUM020;
#endif
		break;
	default:
		uprintf("HP-UX sysconf(%d) not implemented\n",
		    SCARG(uap, name));
		return (EINVAL);
	}
	return (0);
}

int
hpux_sys_stat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_stat_args *uap = v;

	return (hpux_stat1(SCARG(uap, path), SCARG(uap, sb), FOLLOW, p));
}

int
hpux_sys_lstat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_lstat_args *uap = v;

	return (hpux_stat1(SCARG(uap, path), SCARG(uap, sb), NOFOLLOW, p));
}

int
hpux_sys_fstat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_fstat_args *uap = v;
	register struct filedesc *fdp = p->p_fd;
	register struct file *fp;
	struct stat sb;
	int error;

	if (((unsigned)SCARG(uap, fd)) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[SCARG(uap, fd)]) == NULL)
		return (EBADF);

	switch (fp->f_type) {

	case DTYPE_VNODE:
		error = vn_stat((struct vnode *)fp->f_data, &sb, p);
		break;

	case DTYPE_SOCKET:
		error = soo_stat((struct socket *)fp->f_data, &sb, p);
		break;

	default:
		panic("fstat");
		/*NOTREACHED*/
	}
	/* is this right for sockets?? */
	if (error == 0)
		error = bsdtohpuxstat(&sb, SCARG(uap, sb));
	return (error);
}

int
hpux_sys_ulimit(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_ulimit_args *uap = v;
	struct rlimit *limp;
	int error = 0;

	limp = &p->p_rlimit[RLIMIT_FSIZE];
	switch (SCARG(uap, cmd)) {
	case 2:
		SCARG(uap, newlimit) *= 512;
		if (SCARG(uap, newlimit) > limp->rlim_max &&
		    (error = suser(p->p_ucred, &p->p_acflag)))
			break;
		limp->rlim_cur = limp->rlim_max = SCARG(uap, newlimit);
		/* else fall into... */

	case 1:
		*retval = limp->rlim_max / 512;
		break;

	case 3:
		limp = &p->p_rlimit[RLIMIT_DATA];
		*retval = ctob(p->p_vmspace->vm_tsize) + limp->rlim_max;
		break;

	default:
		error = EINVAL;
		break;
	}
	return (error);
}

/*
 * Map "real time" priorities 0 (high) thru 127 (low) into nice
 * values -16 (high) thru -1 (low).
 */
int
hpux_sys_rtprio(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_rtprio_args *uap = v;
	struct proc *p;
	int nice, error;

	if (SCARG(uap, prio) < RTPRIO_MIN && SCARG(uap, prio) > RTPRIO_MAX &&
	    SCARG(uap, prio) != RTPRIO_NOCHG &&
	    SCARG(uap, prio) != RTPRIO_RTOFF)
		return (EINVAL);
	if (SCARG(uap, pid) == 0)
		p = cp;
	else if ((p = pfind(SCARG(uap, pid))) == 0)
		return (ESRCH);
	nice = p->p_nice;
	if (nice < NZERO)
		*retval = (nice + 16) << 3;
	else
		*retval = RTPRIO_RTOFF;
	switch (SCARG(uap, prio)) {

	case RTPRIO_NOCHG:
		return (0);

	case RTPRIO_RTOFF:
		if (nice >= NZERO)
			return (0);
		nice = NZERO;
		break;

	default:
		nice = (SCARG(uap, prio) >> 3) - 16;
		break;
	}
	error = donice(cp, p, nice);
	if (error == EACCES)
		error = EPERM;
	return (error);
}

/*
 * XXX: This belongs in hpux_machdep.c !!
 */
int
hpux_sys_advise(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_advise_args *uap = v;
	int error = 0;

#ifdef hp300
	switch (SCARG(uap, arg)) {
	case 0:
		p->p_md.md_flags |= MDP_HPUXMMAP;
		break;
	case 1:
		ICIA();
		break;
	case 2:
		DCIA();
		break;
	default:
		error = EINVAL;
		break;
	}
#endif
	return (error);
}

int
hpux_sys_ptrace(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_ptrace_args *uap = v;
	int error, isps = 0;
	struct proc *cp;

	switch (SCARG(uap, req)) {
	/* map signal */
#if defined(PT_STEP) || defined(PT_CONTINUE)
# ifdef PT_STEP
	case PT_STEP:
# endif
# ifdef PT_CONTINUE
	case PT_CONTINUE:
# endif
		if (SCARG(uap, data)) {
			SCARG(uap, data) = hpuxtobsdsig(SCARG(uap, data));
			if (SCARG(uap, data) == 0)
				SCARG(uap, data) = NSIG;
		}
		break;
#endif
	/* map u-area offset */
#if defined(PT_READ_U) || defined(PT_WRITE_U)
# ifdef PT_READ_U
	case PT_READ_U:
# endif
# ifdef PT_WRITE_U
	case PT_WRITE_U:
# endif
		/*
		 * Big, cheezy hack: hpuxtobsduoff is really intended
		 * to be called in the child context (procxmt) but we
		 * do it here in the parent context to avoid hacks in
		 * the MI sys_process.c file.  This works only because
		 * we can access the child's md_regs pointer and it
		 * has the correct value (the child has already trapped
		 * into the kernel).
		 */
		if ((cp = pfind(SCARG(uap, pid))) == 0)
			return (ESRCH);
		SCARG(uap, addr) =
		    (int *)hpuxtobsduoff(SCARG(uap, addr), &isps, cp);

		/*
		 * Since HP-UX PS is only 16-bits in ar0, requests
		 * to write PS actually contain the PS in the high word
		 * and the high half of the PC (the following register)
		 * in the low word.  Move the PS value to where BSD
		 * expects it.
		 */
		if (isps && SCARG(uap, req) == PT_WRITE_U)
			SCARG(uap, data) >>= 16;
		break;
#endif
	}

	error = sys_ptrace(p, uap, retval);
	/*
	 * Align PS as HP-UX expects it (see WRITE_U comment above).
	 * Note that we do not return the high part of PC like HP-UX
	 * would, but the HP-UX debuggers don't require it.
	 */
#ifdef PT_READ_U
	if (isps && error == 0 && SCARG(uap, req) == PT_READ_U)
		*retval <<= 16;
#endif
	return (error);
}

#ifdef SYSVSHM
#include <sys/shm.h>

int
hpux_sys_shmctl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_shmctl_args *uap = v;

	return (hpux_shmctl1(p, (struct hpux_shmctl_args *) uap, retval, 0));
}

int
hpux_sys_nshmctl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;	/* struct hpux_nshmctl_args * */
{
	struct hpux_sys_nshmctl_args *uap = v;

	return (hpux_shmctl1(p, (struct hpux_shmctl_args *) uap, retval, 1));
}

/*
 * Handle HP-UX specific commands.
 */
int
hpux_shmctl1(p, uap, retval, isnew)
	struct proc *p;
	struct hpux_sys_shmctl_args *uap;
	register_t *retval;
	int isnew;
{
	register struct shmid_ds *shp;
	register struct ucred *cred = p->p_ucred;
	struct hpux_shmid_ds sbuf;
	int error;
	extern struct shmid_ds *shm_find_segment_by_shmid __P((int));

	if ((shp = shm_find_segment_by_shmid(SCARG(uap, shmid))) == NULL)
		return EINVAL;

	switch (SCARG(uap, cmd)) {
	case SHM_LOCK:
	case SHM_UNLOCK:
		/* don't really do anything, but make them think we did */
		if (cred->cr_uid && cred->cr_uid != shp->shm_perm.uid &&
		    cred->cr_uid != shp->shm_perm.cuid)
			return (EPERM);
		return (0);

	case IPC_STAT:
		if (!isnew)
			break;
		error = ipcperm(cred, &shp->shm_perm, IPC_R);
		if (error == 0) {
			sbuf.shm_perm.uid = shp->shm_perm.uid;
			sbuf.shm_perm.gid = shp->shm_perm.gid;
			sbuf.shm_perm.cuid = shp->shm_perm.cuid;
			sbuf.shm_perm.cgid = shp->shm_perm.cgid;
			sbuf.shm_perm.mode = shp->shm_perm.mode;
			sbuf.shm_perm.seq = shp->shm_perm.seq;
			sbuf.shm_perm.key = shp->shm_perm.key;
			sbuf.shm_segsz = shp->shm_segsz;
			sbuf.shm_ptbl = shp->shm_internal;	/* XXX */
			sbuf.shm_lpid = shp->shm_lpid;
			sbuf.shm_cpid = shp->shm_cpid;
			sbuf.shm_nattch = shp->shm_nattch;
			sbuf.shm_cnattch = shp->shm_nattch;	/* XXX */
			sbuf.shm_atime = shp->shm_atime;
			sbuf.shm_dtime = shp->shm_dtime;
			sbuf.shm_ctime = shp->shm_ctime;
			error = copyout((caddr_t)&sbuf, SCARG(uap, buf),
			    sizeof sbuf);
		}
		return (error);

	case IPC_SET:
		if (!isnew)
			break;
		if (cred->cr_uid && cred->cr_uid != shp->shm_perm.uid &&
		    cred->cr_uid != shp->shm_perm.cuid) {
			return (EPERM);
		}
		error = copyin(SCARG(uap, buf), (caddr_t)&sbuf, sizeof sbuf);
		if (error == 0) {
			shp->shm_perm.uid = sbuf.shm_perm.uid;
			shp->shm_perm.gid = sbuf.shm_perm.gid;
			shp->shm_perm.mode = (shp->shm_perm.mode & ~0777)
				| (sbuf.shm_perm.mode & 0777);
			shp->shm_ctime = time.tv_sec;
		}
		return (error);
	}
	return (sys_shmctl(p, uap, retval));
}
#endif

/*
 * HP-UX mmap() emulation (mainly for shared library support).
 */
int
hpux_sys_mmap(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_mmap_args *uap = v;
	struct sys_mmap_args /* {
		syscallarg(caddr_t) addr;
		syscallarg(size_t) len;
		syscallarg(int) prot;
		syscallarg(int) flags;
		syscallarg(int) fd;
		syscallarg(long) pad;
		syscallarg(off_t) pos;
	} */ nargs;

	SCARG(&nargs, addr) = SCARG(uap, addr);
	SCARG(&nargs, len) = SCARG(uap, len);
	SCARG(&nargs, prot) = SCARG(uap, prot);
	SCARG(&nargs, flags) = SCARG(uap, flags) &
		~(HPUXMAP_FIXED|HPUXMAP_REPLACE|HPUXMAP_ANON);
	if (SCARG(uap, flags) & HPUXMAP_FIXED)
		SCARG(&nargs, flags) |= MAP_FIXED;
	if (SCARG(uap, flags) & HPUXMAP_ANON)
		SCARG(&nargs, flags) |= MAP_ANON;
	SCARG(&nargs, fd) = (SCARG(&nargs, flags) & MAP_ANON) ? -1 : SCARG(uap, fd);
	SCARG(&nargs, pos) = SCARG(uap, pos);

	return (sys_mmap(p, &nargs, retval));
}

hpux_stat1(fname, hsb, follow, p)
	char *fname;
	struct hpux_stat *hsb;
	int follow;
	struct proc *p;
{
	int error;
	struct stat sb;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, follow | LOCKLEAF, UIO_USERSPACE, fname, p);
	if (error = namei(&nd))
		return (error);
	error = vn_stat(nd.ni_vp, &sb, p);
	vput(nd.ni_vp);
	if (error == 0)
		error = bsdtohpuxstat(&sb, hsb);
	return (error);
}

#ifdef hp300
#include "grf.h"
#define	NHIL	1	/* XXX */
#endif

#if NGRF > 0
extern int grfopen __P((dev_t dev, int oflags, int devtype, struct proc *p));
#endif

#if NHIL > 0
extern int hilopen __P((dev_t dev, int oflags, int devtype, struct proc *p));
#endif

#include <sys/conf.h>

int
bsdtohpuxstat(sb, hsb)
	struct stat *sb;
	struct hpux_stat *hsb;
{
	struct hpux_stat ds;

	bzero((caddr_t)&ds, sizeof(ds));
	ds.hst_dev = (u_short)sb->st_dev;
	ds.hst_ino = (u_long)sb->st_ino;
	ds.hst_mode = sb->st_mode;
	ds.hst_nlink = sb->st_nlink;
	ds.hst_uid = (u_short)sb->st_uid;
	ds.hst_gid = (u_short)sb->st_gid;
	ds.hst_rdev = bsdtohpuxdev(sb->st_rdev);

	/* XXX: I don't want to talk about it... */
	if ((sb->st_mode & S_IFMT) == S_IFCHR)
#if NGRF > 0
		if (cdevsw[major(sb->st_rdev)].d_open == grfopen)
			ds.hst_rdev = grfdevno(sb->st_rdev);
		else
#endif
#if NHIL > 0
			if (cdevsw[major(sb->st_rdev)].d_open == hilopen)
				ds.hst_rdev = hildevno(sb->st_rdev);
#endif

	if (sb->st_size < (quad_t)1 << 32)
		ds.hst_size = (long)sb->st_size;
	else
		ds.hst_size = -2;
	ds.hst_atime = sb->st_atime;
	ds.hst_mtime = sb->st_mtime;
	ds.hst_ctime = sb->st_ctime;
	ds.hst_blksize = sb->st_blksize;
	ds.hst_blocks = sb->st_blocks;
	return(copyout((caddr_t)&ds, (caddr_t)hsb, sizeof(ds)));
}

int
hpuxtobsdioctl(com)
	u_long com;
{
	switch (com) {
	case HPUXTIOCSLTC:
		com = TIOCSLTC; break;
	case HPUXTIOCGLTC:
		com = TIOCGLTC; break;
	case HPUXTIOCSPGRP:
		com = TIOCSPGRP; break;
	case HPUXTIOCGPGRP:
		com = TIOCGPGRP; break;
	case HPUXTIOCLBIS:
		com = TIOCLBIS; break;
	case HPUXTIOCLBIC:
		com = TIOCLBIC; break;
	case HPUXTIOCLSET:
		com = TIOCLSET; break;
	case HPUXTIOCLGET:
		com = TIOCLGET; break;
	case HPUXTIOCGWINSZ:
		com = TIOCGWINSZ; break;
	case HPUXTIOCSWINSZ:
		com = TIOCSWINSZ; break;
	}
	return(com);
}

/*
 * HP-UX ioctl system call.  The differences here are:
 *	IOC_IN also means IOC_VOID if the size portion is zero.
 *	no FIOCLEX/FIONCLEX/FIOASYNC/FIOGETOWN/FIOSETOWN
 *	the sgttyb struct is 2 bytes longer
 */
int
hpux_sys_ioctl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_ioctl_args *uap = v;
	register struct filedesc *fdp = p->p_fd;
	register struct file *fp;
	register int com, error;
	register u_int size;
	caddr_t memp = 0;
#define STK_PARAMS	128
	char stkbuf[STK_PARAMS];
	caddr_t dt = stkbuf;

	com = SCARG(uap, com);

#ifdef COMPAT_HPUX_6X
	/* XXX */
	if (com == HPUXTIOCGETP || com == HPUXTIOCSETP)
		return (getsettty(p, SCARG(uap, fd), com, SCARG(uap, data)));
#endif

	if (((unsigned)SCARG(uap, fd)) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[SCARG(uap, fd)]) == NULL)
		return (EBADF);
	if ((fp->f_flag & (FREAD|FWRITE)) == 0)
		return (EBADF);

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the
	 * user's address space.
	 */
	size = IOCPARM_LEN(com);
	if (size > IOCPARM_MAX)
		return (ENOTTY);
	if (size > sizeof (stkbuf)) {
		memp = (caddr_t)malloc((u_long)size, M_IOCTLOPS, M_WAITOK);
		dt = memp;
	}
	if (com&IOC_IN) {
		if (size) {
			error = copyin(SCARG(uap, data), dt, (u_int)size);
			if (error) {
				if (memp)
					free(memp, M_IOCTLOPS);
				return (error);
			}
		} else
			*(caddr_t *)dt = SCARG(uap, data);
	} else if ((com&IOC_OUT) && size)
		/*
		 * Zero the buffer so the user always
		 * gets back something deterministic.
		 */
		bzero(dt, size);
	else if (com&IOC_VOID)
		*(caddr_t *)dt = SCARG(uap, data);

	switch (com) {

	case HPUXFIOSNBIO:
	{
		char *ofp = &fdp->fd_ofileflags[SCARG(uap, fd)];
		int tmp;

		if (*(int *)dt)
			*ofp |= UF_FIONBIO_ON;
		else
			*ofp &= ~UF_FIONBIO_ON;
		/*
		 * Only set/clear if O_NONBLOCK/FNDELAY not in effect
		 */
		if ((*ofp & (UF_NONBLOCK_ON|UF_FNDELAY_ON)) == 0) {
			tmp = *ofp & UF_FIONBIO_ON;
			error = (*fp->f_ops->fo_ioctl)(fp, FIONBIO,
						       (caddr_t)&tmp, p);
		}
		break;
	}

	case HPUXTIOCCONS:
		*(int *)dt = 1;
		error = (*fp->f_ops->fo_ioctl)(fp, TIOCCONS, dt, p);
		break;

	/* BSD-style job control ioctls */
	case HPUXTIOCLBIS:
	case HPUXTIOCLBIC:
	case HPUXTIOCLSET:
		*(int *)dt &= HPUXLTOSTOP;
		if (*(int *)dt & HPUXLTOSTOP)
			*(int *)dt = LTOSTOP;
		/* fall into */

	/* simple mapping cases */
	case HPUXTIOCLGET:
	case HPUXTIOCSLTC:
	case HPUXTIOCGLTC:
	case HPUXTIOCSPGRP:
	case HPUXTIOCGPGRP:
	case HPUXTIOCGWINSZ:
	case HPUXTIOCSWINSZ:
		error = (*fp->f_ops->fo_ioctl)
			(fp, hpuxtobsdioctl(com), dt, p);
		if (error == 0 && com == HPUXTIOCLGET) {
			*(int *)dt &= LTOSTOP;
			if (*(int *)dt & LTOSTOP)
				*(int *)dt = HPUXLTOSTOP;
		}
		break;

	/* SYS 5 termio and POSIX termios */
	case HPUXTCGETA:
	case HPUXTCSETA:
	case HPUXTCSETAW:
	case HPUXTCSETAF:
	case HPUXTCGETATTR:
	case HPUXTCSETATTR:
	case HPUXTCSETATTRD:
	case HPUXTCSETATTRF:
		error = hpux_termio(SCARG(uap, fd), com, dt, p);
		break;

	default:
		error = (*fp->f_ops->fo_ioctl)(fp, com, dt, p);
		break;
	}
	/*
	 * Copy any data to user, size was
	 * already set and checked above.
	 */
	if (error == 0 && (com&IOC_OUT) && size)
		error = copyout(dt, SCARG(uap, data), (u_int)size);
	if (memp)
		free(memp, M_IOCTLOPS);
	return (error);
}

/*
 * XXX: This should be in hpux_machdep.c !!
 */
/*
 * Man page lies, behaviour here is based on observed behaviour.
 */
int
hpux_sys_getcontext(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_getcontext_args *uap = v;
	int error = 0;
	register int len;

#ifdef HP380
	if (machineid == HP_380) {
		len = min(SCARG(uap, len), sizeof(hpux_040context));
		if (len)
			error = copyout(hpux_040context, SCARG(uap, buf), len);
		if (error == 0)
			*retval = sizeof(hpux_040context);
		return (error);
	}
#endif
	len = min(SCARG(uap, len), sizeof(hpux_context));
	if (len)
		error = copyout(hpux_context, SCARG(uap, buf), (u_int)len);
	if (error == 0)
		*retval = sizeof(hpux_context);
	return (error);
}

/*
 * This is the equivalent of BSD getpgrp but with more restrictions.
 * Note we do not check the real uid or "saved" uid.
 */
int
hpux_sys_getpgrp2(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_getpgrp2_args *uap = v;
	register struct proc *p;

	if (SCARG(uap, pid) == 0)
		SCARG(uap, pid) = cp->p_pid;
	p = pfind(SCARG(uap, pid));
	if (p == 0)
		return (ESRCH);
	if (cp->p_ucred->cr_uid && p->p_ucred->cr_uid != cp->p_ucred->cr_uid &&
	    !inferior(p))
		return (EPERM);
	*retval = p->p_pgid;
	return (0);
}

/*
 * This is the equivalent of BSD setpgrp but with more restrictions.
 * Note we do not check the real uid or "saved" uid or pgrp.
 */
int
hpux_sys_setpgrp2(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_setpgrp2_args *uap = v;

	/* empirically determined */
	if (SCARG(uap, pgid) < 0 || SCARG(uap, pgid) >= 30000)
		return (EINVAL);
	return (sys_setpgid(p, uap, retval));
}

/*
 * XXX Same as BSD setre[ug]id right now.  Need to consider saved ids.
 */
int
hpux_sys_setresuid(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_setresuid_args *uap = v;

	return (compat_43_sys_setreuid(p, uap, retval));
}

int
hpux_sys_setresgid(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_setresgid_args *uap = v;

	return (compat_43_sys_setregid(p, uap, retval));
}

int
hpux_sys_getrlimit(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_getrlimit_args *uap = v;
	struct compat_43_sys_getrlimit_args ap;

	if (SCARG(uap, which) > HPUXRLIMIT_NOFILE)
		return (EINVAL);
	if (SCARG(uap, which) == HPUXRLIMIT_NOFILE)
		SCARG(uap, which) = RLIMIT_NOFILE;

	SCARG(&ap, which) = SCARG(uap, which);
	SCARG(&ap, rlp) = SCARG(uap, rlp);

	return (compat_43_sys_getrlimit(p, uap, retval));
}

int
hpux_sys_setrlimit(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_setrlimit_args *uap = v;
	struct compat_43_sys_setrlimit_args ap;

	if (SCARG(uap, which) > HPUXRLIMIT_NOFILE)
		return (EINVAL);
	if (SCARG(uap, which) == HPUXRLIMIT_NOFILE)
		SCARG(uap, which) = RLIMIT_NOFILE;

	SCARG(&ap, which) = SCARG(uap, which);
	SCARG(&ap, rlp) = SCARG(uap, rlp);

	return (compat_43_sys_setrlimit(p, uap, retval));
}

/*
 * XXX: simple recognition hack to see if we can make grmd work.
 */
int
hpux_sys_lockf(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct hpux_sys_lockf_args *uap = v;

	return (0);
}

int
hpux_sys_getaccess(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	register struct hpux_sys_getaccess_args *uap = v;
	int lgroups[NGROUPS];
	int error = 0;
	register struct ucred *cred;
	register struct vnode *vp;
	struct nameidata nd;

	/*
	 * Build an appropriate credential structure
	 */
	cred = crdup(p->p_ucred);
	switch (SCARG(uap, uid)) {
	case 65502:	/* UID_EUID */
		break;
	case 65503:	/* UID_RUID */
		cred->cr_uid = p->p_cred->p_ruid;
		break;
	case 65504:	/* UID_SUID */
		error = EINVAL;
		break;
	default:
		if (SCARG(uap, uid) > 65504)
			error = EINVAL;
		cred->cr_uid = SCARG(uap, uid);
		break;
	}
	switch (SCARG(uap, ngroups)) {
	case -1:	/* NGROUPS_EGID */
		cred->cr_ngroups = 1;
		break;
	case -5:	/* NGROUPS_EGID_SUPP */
		break;
	case -2:	/* NGROUPS_RGID */
		cred->cr_ngroups = 1;
		cred->cr_gid = p->p_cred->p_rgid;
		break;
	case -6:	/* NGROUPS_RGID_SUPP */
		cred->cr_gid = p->p_cred->p_rgid;
		break;
	case -3:	/* NGROUPS_SGID */
	case -7:	/* NGROUPS_SGID_SUPP */
		error = EINVAL;
		break;
	case -4:	/* NGROUPS_SUPP */
		if (cred->cr_ngroups > 1)
			cred->cr_gid = cred->cr_groups[1];
		else
			error = EINVAL;
		break;
	default:
		if (SCARG(uap, ngroups) > 0 && SCARG(uap, ngroups) <= NGROUPS)
			error = copyin((caddr_t)SCARG(uap, gidset),
				       (caddr_t)&lgroups[0],
				       SCARG(uap, ngroups) *
					   sizeof(lgroups[0]));
		else
			error = EINVAL;
		if (error == 0) {
			int gid;

			for (gid = 0; gid < SCARG(uap, ngroups); gid++)
				cred->cr_groups[gid] = lgroups[gid];
			cred->cr_ngroups = SCARG(uap, ngroups);
		}
		break;
	}
	/*
	 * Lookup file using caller's effective IDs.
	 */
	if (error == 0) {
		NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_USERSPACE,
			SCARG(uap, path), p);
		error = namei(&nd);
	}
	if (error) {
		crfree(cred);
		return (error);
	}
	/*
	 * Use the constructed credentials for access checks.
	 */
	vp = nd.ni_vp;
	*retval = 0;
	if (VOP_ACCESS(vp, VREAD, cred, p) == 0)
		*retval |= R_OK;
	if (vn_writechk(vp) == 0 && VOP_ACCESS(vp, VWRITE, cred, p) == 0)
		*retval |= W_OK;
	/* XXX we return X_OK for root on VREG even if not */
	if (VOP_ACCESS(vp, VEXEC, cred, p) == 0)
		*retval |= X_OK;
	vput(vp);
	crfree(cred);
	return (error);
}

/*
 * XXX: This needs to be in hpux_machdep.c !!
 */
extern char kstack[];
#define UOFF(f)		((int)&((struct user *)0)->f)
#define HPUOFF(f)	((int)&((struct hpux_user *)0)->f)

/* simplified FP structure */
struct bsdfp {
	int save[54];
	int reg[24];
	int ctrl[3];
};

/*
 * Brutal hack!  Map HP-UX u-area offsets into BSD k-stack offsets.
 */
hpuxtobsduoff(off, isps, p)
	int *off, *isps;
	struct proc *p;
{
#ifdef hp300
	register int *ar0 = p->p_md.md_regs;
	struct hpux_fp *hp;
	struct bsdfp *bp;
	register u_int raddr;

	*isps = 0;

	/* u_ar0 field; procxmt puts in U_ar0 */
	if ((int)off == HPUOFF(hpuxu_ar0))
		return(UOFF(U_ar0));

#ifdef FPCOPROC
	/* FP registers from PCB */
	hp = (struct hpux_fp *)HPUOFF(hpuxu_fp);
	bp = (struct bsdfp *)UOFF(u_pcb.pcb_fpregs);
	if (off >= hp->hpfp_ctrl && off < &hp->hpfp_ctrl[3])
		return((int)&bp->ctrl[off - hp->hpfp_ctrl]);
	if (off >= hp->hpfp_reg && off < &hp->hpfp_reg[24])
		return((int)&bp->reg[off - hp->hpfp_reg]);
#endif

	/*
	 * Everything else we recognize comes from the kernel stack,
	 * so we convert off to an absolute address (if not already)
	 * for simplicity.
	 */
	if (off < (int *)ctob(UPAGES))
		off = (int *)((u_int)off + (u_int)kstack);

	/*
	 * General registers.
	 * We know that the HP-UX registers are in the same order as ours.
	 * The only difference is that their PS is 2 bytes instead of a
	 * padded 4 like ours throwing the alignment off.
	 */
	if (off >= ar0 && off < &ar0[18]) {
		/*
		 * PS: return low word and high word of PC as HP-UX would
		 * (e.g. &u.u_ar0[16.5]).
		 *
		 * XXX we don't do this since HP-UX adb doesn't rely on
		 * it and passing such an offset to procxmt will cause
		 * it to fail anyway.  Instead, we just set the offset
		 * to PS and let hpux_ptrace() shift up the value returned.
		 */
		if (off == &ar0[PS]) {
#if 0
			raddr = (u_int) &((short *)ar0)[PS*2+1];
#else
			raddr = (u_int) &ar0[(int)(off - ar0)];
#endif
			*isps = 1;
		}
		/*
		 * PC: off will be &u.u_ar0[16.5] since HP-UX saved PS
		 * is only 16 bits.
		 */
		else if (off == (int *)&(((short *)ar0)[PS*2+1]))
			raddr = (u_int) &ar0[PC];
		/*
		 * D0-D7, A0-A7: easy
		 */
		else
			raddr = (u_int) &ar0[(int)(off - ar0)];
		return((int)(raddr - (u_int)kstack));
	}
#endif
	/* everything else */
	return(-1);
}

/*
 * Kludge up a uarea dump so that HP-UX debuggers can find out
 * what they need.  IMPORTANT NOTE: we do not EVEN attempt to
 * convert the entire user struct.
 */
int
hpux_dumpu(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	int error = 0;
#ifdef hp300
	struct proc *p = curproc;
	struct hpux_user *faku;
	struct bsdfp *bp;
	short *foop;

	faku = (struct hpux_user *)malloc((u_long)ctob(1), M_TEMP, M_WAITOK);
	/*
	 * Make sure there is no mistake about this
	 * being a real user structure.
	 */
	bzero((caddr_t)faku, ctob(1));
	/*
	 * Fill in the process sizes.
	 */
	faku->hpuxu_tsize = p->p_vmspace->vm_tsize;
	faku->hpuxu_dsize = p->p_vmspace->vm_dsize;
	faku->hpuxu_ssize = p->p_vmspace->vm_ssize;
	/*
	 * Fill in the exec header for CDB.
	 * This was saved back in exec().  As far as I can tell CDB
	 * only uses this information to verify that a particular
	 * core file goes with a particular binary.
	 */
	bcopy((caddr_t)p->p_addr->u_md.md_exec,
	      (caddr_t)&faku->hpuxu_exdata, sizeof (struct hpux_exec));
	/*
	 * Adjust user's saved registers (on kernel stack) to reflect
	 * HP-UX order.  Note that HP-UX saves the SR as 2 bytes not 4
	 * so we have to move it up.
	 */
	faku->hpuxu_ar0 = p->p_md.md_regs;
	foop = (short *) p->p_md.md_regs;
	foop[32] = foop[33];
	foop[33] = foop[34];
	foop[34] = foop[35];
#ifdef FPCOPROC
	/*
	 * Copy 68881 registers from our PCB format to HP-UX format
	 */
	bp = (struct bsdfp *) &p->p_addr->u_pcb.pcb_fpregs;
	bcopy((caddr_t)bp->save, (caddr_t)faku->hpuxu_fp.hpfp_save,
	      sizeof(bp->save));
	bcopy((caddr_t)bp->ctrl, (caddr_t)faku->hpuxu_fp.hpfp_ctrl,
	      sizeof(bp->ctrl));
	bcopy((caddr_t)bp->reg, (caddr_t)faku->hpuxu_fp.hpfp_reg,
	      sizeof(bp->reg));
#endif
	/*
	 * Slay the dragon
	 */
	faku->hpuxu_dragon = -1;
	/*
	 * Dump this artfully constructed page in place of the
	 * user struct page.
	 */
	error = vn_rdwr(UIO_WRITE, vp, (caddr_t)faku, ctob(1), (off_t)0,
			UIO_SYSSPACE, IO_NODELOCKED|IO_UNIT, cred,
			(int *)NULL, p);
	/*
	 * Dump the remaining UPAGES-1 pages normally
	 */
	if (!error) 
		error = vn_rdwr(UIO_WRITE, vp, kstack + ctob(1),
				ctob(UPAGES-1), (off_t)ctob(1), UIO_SYSSPACE,
				IO_NODELOCKED|IO_UNIT, cred, (int *)NULL, p);
	free((caddr_t)faku, M_TEMP);
#endif
	return(error);
}

/*
 * The remaining routines are essentially the same as those in kern_xxx.c
 * and vfs_xxx.c as defined under "#ifdef COMPAT".  We replicate them here
 * to avoid COMPAT_HPUX dependencies in those files and to make sure that
 * HP-UX compatibility still works even when COMPAT is not defined.
 *
 * These are still needed as of HP-UX 7.05.
 */
#ifdef COMPAT_HPUX_6X

#define HPUX_HZ	50

#include "sys/times.h"


/*
 * SYS V style setpgrp()
 */
int
compat_hpux_6x_sys_setpgrp(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	struct compat_hpux_6x_sys_setpgrp_args *uap = v;

	if (p->p_pid != p->p_pgid)
		enterpgrp(p, p->p_pid, 0);
	*retval = p->p_pgid;
	return (0);
}

int
compat_hpux_6x_sys_time(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_time_args *uap = v;

	int error = 0;
	struct timeval tv;

	microtime(&tv);
        if (SCARG(uap, t))
		error = copyout(&tv.tv_sec, SCARG(uap, t),
				sizeof(*(SCARG(uap, t))));

	*retval = (int) tv.tv_sec;
	return error;
}

int
compat_hpux_6x_sys_stime(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_stime_args *uap = v;
	struct timeval tv;
	int s, error;

	tv.tv_sec = SCARG(uap, time);
	tv.tv_usec = 0;
	if (error = suser(p->p_ucred, &p->p_acflag))
		return (error);

	/* WHAT DO WE DO ABOUT PENDING REAL-TIME TIMEOUTS??? */
	boottime.tv_sec += tv.tv_sec - time.tv_sec;
	s = splhigh(); time = tv; splx(s);
	resettodr();
	return (0);
}

int
compat_hpux_6x_sys_ftime(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_ftime_args *uap = v;
	struct hpux_otimeb tb;
	int s;

	s = splhigh();
	tb.time = time.tv_sec;
	tb.millitm = time.tv_usec / 1000;
	splx(s);
	tb.timezone = tz.tz_minuteswest;
	tb.dstflag = tz.tz_dsttime;
	return (copyout((caddr_t)&tb, (caddr_t)SCARG(uap, tp), sizeof (tb)));
}

int
compat_hpux_6x_sys_alarm(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_alarm_args *uap = v;
	int s = splhigh();

	untimeout(realitexpire, (caddr_t)p);
	timerclear(&p->p_realtimer.it_interval);
	*retval = 0;
	if (timerisset(&p->p_realtimer.it_value) &&
	    timercmp(&p->p_realtimer.it_value, &time, >))
		*retval = p->p_realtimer.it_value.tv_sec - time.tv_sec;
	if (SCARG(uap, deltat) == 0) {
		timerclear(&p->p_realtimer.it_value);
		splx(s);
		return (0);
	}
	p->p_realtimer.it_value = time;
	p->p_realtimer.it_value.tv_sec += SCARG(uap, deltat);
	timeout(realitexpire, (caddr_t)p, hzto(&p->p_realtimer.it_value));
	splx(s);
	return (0);
}

int
compat_hpux_6x_sys_nice(p, v, retval)
	register struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_nice_args *uap = v;
	int error;

	error = donice(p, p, (p->p_nice-NZERO)+SCARG(uap, nval));
	if (error == 0)
		*retval = p->p_nice - NZERO;
	return (error);
}

int
compat_hpux_6x_sys_times(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_times_args *uap = v;
	struct timeval ru, rs;
	struct tms atms;
	int error;

	calcru(p, &ru, &rs, NULL);
	atms.tms_utime = hpux_scale(&ru);
	atms.tms_stime = hpux_scale(&rs);
	atms.tms_cutime = hpux_scale(&p->p_stats->p_cru.ru_utime);
	atms.tms_cstime = hpux_scale(&p->p_stats->p_cru.ru_stime);
	error = copyout((caddr_t)&atms, (caddr_t)SCARG(uap, tms),
	    sizeof (atms));
	if (error == 0)
		*(time_t *)retval = hpux_scale(&time) - hpux_scale(&boottime);
	return (error);
}

/*
 * Doesn't exactly do what the documentation says.
 * What we really do is return 1/HPUX_HZ-th of a second since that
 * is what HP-UX returns.
 */
int
hpux_scale(tvp)
	register struct timeval *tvp;
{
	return (tvp->tv_sec * HPUX_HZ + tvp->tv_usec * HPUX_HZ / 1000000);
}

/*
 * Set IUPD and IACC times on file.
 * Can't set ICHG.
 */
int
compat_hpux_6x_sys_utime(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_utime_args *uap = v;
	register struct vnode *vp;
	struct vattr vattr;
	time_t tv[2];
	int error;
	struct nameidata nd;

	if (SCARG(uap, tptr)) {
		error = copyin((caddr_t)SCARG(uap, tptr), (caddr_t)tv,
		    sizeof (tv));
		if (error)
			return (error);
	} else
		tv[0] = tv[1] = time.tv_sec;
	vattr_null(&vattr);
	vattr.va_atime.ts_sec = tv[0];
	vattr.va_atime.ts_nsec = 0;
	vattr.va_mtime.ts_sec = tv[1];
	vattr.va_mtime.ts_nsec = 0;
	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_USERSPACE,
	    SCARG(uap, fname), p);
	if (error = namei(&nd))
		return (error);
	vp = nd.ni_vp;
	if (vp->v_mount->mnt_flag & MNT_RDONLY)
		error = EROFS;
	else
		error = VOP_SETATTR(vp, &vattr, nd.ni_cnd.cn_cred, p);
	vput(vp);
	return (error);
}

int
compat_hpux_6x_sys_pause(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct compat_hpux_6x_sys_pause_args *uap = v;

	(void) tsleep(kstack, PPAUSE | PCATCH, "pause", 0);
	/* always return EINTR rather than ERESTART... */
	return (EINTR);
}

/*
 * The old fstat system call.
 */
int
compat_hpux_6x_sys_fstat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_fstat_args *uap = v;
	register struct filedesc *fdp = p->p_fd;
	struct file *fp;

	if (((unsigned)SCARG(uap, fd)) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[SCARG(uap, fd)]) == NULL)
		return (EBADF);
	if (fp->f_type != DTYPE_VNODE)
		return (EINVAL);
	return (compat_hpux_6x_stat1((struct vnode *)fp->f_data,
				      SCARG(uap, sb), p));
}

/*
 * Old stat system call.  This version follows links.
 */
int
compat_hpux_6x_sys_stat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_hpux_6x_sys_stat_args *uap = v;
	int error;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_USERSPACE,
	    SCARG(uap, path), p);
	if (error = namei(&nd))
		return (error);
	error = compat_hpux_6x_stat1(nd.ni_vp, SCARG(uap, sb), p);
	vput(nd.ni_vp);
	return (error);
}

int
compat_hpux_6x_stat1(vp, ub, p)
	struct vnode *vp;
	struct hpux_ostat *ub;
	struct proc *p;
{
	struct hpux_ostat ohsb;
	struct stat sb;
	int error;

	error = vn_stat(vp, &sb, p);
	if (error)
		return (error);

	ohsb.hst_dev = sb.st_dev;
	ohsb.hst_ino = sb.st_ino;
	ohsb.hst_mode = sb.st_mode;
	ohsb.hst_nlink = sb.st_nlink;
	ohsb.hst_uid = sb.st_uid;
	ohsb.hst_gid = sb.st_gid;
	ohsb.hst_rdev = sb.st_rdev;
	if (sb.st_size < (quad_t)1 << 32)
		ohsb.hst_size = sb.st_size;
	else
		ohsb.hst_size = -2;
	ohsb.hst_atime = sb.st_atime;
	ohsb.hst_mtime = sb.st_mtime;
	ohsb.hst_ctime = sb.st_ctime;
	return (copyout((caddr_t)&ohsb, (caddr_t)ub, sizeof(ohsb)));
}
#endif

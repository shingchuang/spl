/*****************************************************************************\
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Brian Behlendorf <behlendorf1@llnl.gov>.
 *  UCRL-CODE-235197
 *
 *  This file is part of the SPL, Solaris Porting Layer.
 *  For details, see <http://github.com/behlendorf/spl/>.
 *
 *  The SPL is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  The SPL is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the SPL.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _SPL_VNODE_H
#define _SPL_VNODE_H

#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/buffer_head.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>
#include <sys/kmem.h>
#include <sys/mutex.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/sunldi.h>

#define XVA_MAPSIZE     3
#define XVA_MAGIC       0x78766174

/*
 * Prior to linux-2.6.33 only O_DSYNC semantics were implemented and
 * they used the O_SYNC flag.  As of linux-2.6.33 the this behavior
 * was properly split in to O_SYNC and O_DSYNC respectively.
 */
#ifndef O_DSYNC
#define O_DSYNC		O_SYNC
#endif

#define FREAD		1
#define FWRITE		2
#define FCREAT		O_CREAT
#define FTRUNC		O_TRUNC
#define FOFFMAX		O_LARGEFILE
#define FSYNC		O_SYNC
#define FDSYNC		O_DSYNC
#define FRSYNC		O_SYNC
#define FEXCL		O_EXCL
#define FDIRECT		O_DIRECT
#define FAPPEND		O_APPEND

#define FNODSYNC	0x10000 /* fsync pseudo flag */
#define FNOFOLLOW	0x20000 /* don't follow symlinks */

#define AT_TYPE		0x00001
#define AT_MODE		0x00002
#undef  AT_UID		/* Conflicts with linux/auxvec.h */
#define AT_UID          0x00004
#undef  AT_GID		/* Conflicts with linux/auxvec.h */
#define AT_GID          0x00008
#define AT_FSID		0x00010
#define AT_NODEID	0x00020
#define AT_NLINK	0x00040
#define AT_SIZE		0x00080
#define AT_ATIME	0x00100
#define AT_MTIME	0x00200
#define AT_CTIME	0x00400
#define AT_RDEV		0x00800
#define AT_BLKSIZE	0x01000
#define AT_NBLOCKS	0x02000
#define AT_SEQ		0x08000
#define AT_XVATTR	0x10000

#define CRCREAT		0x01
#define RMFILE		0x02

#define B_INVAL		0x01
#define B_TRUNC		0x02

#define V_ACE_MASK	0x0001
#define V_APPEND	0x0002

#define LOOKUP_DIR		0x01
#define LOOKUP_XATTR		0x02
#define CREATE_XATTR_DIR	0x04
#define ATTR_NOACLCHECK		0x20

#ifdef HAVE_PATH_IN_NAMEIDATA
# define nd_dentry	path.dentry
# define nd_mnt		path.mnt
#else
# define nd_dentry	dentry
# define nd_mnt		mnt
#endif

typedef enum vtype {
	VNON		= 0,
	VREG		= 1,
	VDIR		= 2,
	VBLK		= 3,
	VCHR		= 4,
	VLNK		= 5,
	VFIFO		= 6,
	VDOOR		= 7,
	VPROC		= 8,
	VSOCK		= 9,
	VPORT		= 10,
	VBAD		= 11
} vtype_t;

typedef struct vattr {
	enum vtype	va_type;	/* vnode type */
	u_int		va_mask;	/* attribute bit-mask */
	u_short		va_mode;	/* acc mode */
	uid_t		va_uid;		/* owner uid */
	gid_t		va_gid;		/* owner gid */
	long		va_fsid;	/* fs id */
	long		va_nodeid;	/* node # */
	uint32_t	va_nlink;	/* # links */
	uint64_t	va_size;	/* file size */
	uint32_t	va_blocksize;	/* block size */
	uint64_t	va_nblocks;	/* space used */
	struct timespec	va_atime;	/* last acc */
	struct timespec	va_mtime;	/* last mod */
	struct timespec	va_ctime;	/* last chg */
	dev_t		va_rdev;	/* dev */
} vattr_t;

typedef struct xoptattr {
	timestruc_t	xoa_createtime;	/* Create time of file */
	uint8_t		xoa_archive;
	uint8_t		xoa_system;
	uint8_t		xoa_readonly;
	uint8_t		xoa_hidden;
	uint8_t		xoa_nounlink;
	uint8_t		xoa_immutable;
	uint8_t		xoa_appendonly;
	uint8_t		xoa_nodump;
	uint8_t		xoa_settable;
	uint8_t		xoa_opaque;
	uint8_t		xoa_av_quarantined;
	uint8_t		xoa_av_modified;
} xoptattr_t;

typedef struct xvattr {
	vattr_t		xva_vattr;	/* Embedded vattr structure */
	uint32_t	xva_magic;	/* Magic Number */
	uint32_t	xva_mapsize;	/* Size of attr bitmap (32-bit words) */
	uint32_t	*xva_rtnattrmapp;	/* Ptr to xva_rtnattrmap[] */
	uint32_t	xva_reqattrmap[XVA_MAPSIZE];	/* Requested attrs */
	uint32_t	xva_rtnattrmap[XVA_MAPSIZE];	/* Returned attrs */
	xoptattr_t	xva_xoptattrs;	/* Optional attributes */
} xvattr_t;

typedef struct vsecattr {
	uint_t		vsa_mask;	/* See below */
	int		vsa_aclcnt;	/* ACL entry count */
	void		*vsa_aclentp;	/* pointer to ACL entries */
	int		vsa_dfaclcnt;	/* default ACL entry count */
	void		*vsa_dfaclentp;	/* pointer to default ACL entries */
	size_t		vsa_aclentsz;	/* ACE size in bytes of vsa_aclentp */
	uint_t		vsa_aclflags;	/* ACE ACL flags */
} vsecattr_t;

typedef struct vnode {
	struct file	*v_file;
	kmutex_t	v_lock;		/* protects vnode fields */
	uint_t		v_flag;		/* vnode flags (see below) */
	uint_t		v_count;	/* reference count */
	void		*v_data;	/* private data for fs */
	struct vfs	*v_vfsp;	/* ptr to containing VFS */
	struct stdata	*v_stream;	/* associated stream */
	enum vtype	v_type;		/* vnode type */
	dev_t		v_rdev;		/* device (VCHR, VBLK) */
	gfp_t		v_gfp_mask;	/* original mapping gfp mask */
} vnode_t;

typedef struct vn_file {
	int		f_fd;		/* linux fd for lookup */
	struct file	*f_file;	/* linux file struct */
	atomic_t	f_ref;		/* ref count */
	kmutex_t	f_lock;		/* struct lock */
	loff_t		f_offset;	/* offset */
	vnode_t		*f_vnode;	/* vnode */
	struct list_head f_list;	/* list referenced file_t's */
} file_t;

typedef struct caller_context {
	pid_t		cc_pid;		/* Process ID of the caller */
	int		cc_sysid;	/* System ID, used for remote calls */
	u_longlong_t	cc_caller_id;	/* Identifier for (set of) caller(s) */
	ulong_t		cc_flags;
} caller_context_t;

extern vnode_t *vn_alloc(int flag);
void vn_free(vnode_t *vp);
extern vtype_t vn_mode_to_vtype(mode_t);
extern mode_t vn_vtype_to_mode(vtype_t);
extern int vn_open(const char *path, uio_seg_t seg, int flags, int mode,
		   vnode_t **vpp, int x1, void *x2);
extern int vn_openat(const char *path, uio_seg_t seg, int flags, int mode,
		     vnode_t **vpp, int x1, void *x2, vnode_t *vp, int fd);
extern int vn_rdwr(uio_rw_t uio, vnode_t *vp, void *addr, ssize_t len,
		   offset_t off, uio_seg_t seg, int x1, rlim64_t x2,
		   void *x3, ssize_t *residp);
extern int vn_close(vnode_t *vp, int flags, int x1, int x2, void *x3, void *x4);
extern int vn_seek(vnode_t *vp, offset_t o, offset_t *op, caller_context_t *ct);

extern int vn_remove(const char *path, uio_seg_t seg, int flags);
extern int vn_rename(const char *path1, const char *path2, int x1);
extern int vn_getattr(vnode_t *vp, vattr_t *vap, int flags, void *x3, void *x4);
extern int vn_fsync(vnode_t *vp, int flags, void *x3, void *x4);
extern file_t *vn_getf(int fd);
extern void vn_releasef(int fd);
extern int vn_set_pwd(const char *filename);

int vn_init(void);
void vn_fini(void);

#define VOP_CLOSE				vn_close
#define VOP_SEEK				vn_seek
#define VOP_GETATTR				vn_getattr
#define VOP_FSYNC				vn_fsync
#define VOP_PUTPAGE(vp, o, s, f, x1, x2)	((void)0)
#define vn_is_readonly(vp)			0
#define getf					vn_getf
#define releasef				vn_releasef

extern vnode_t *rootdir;

#endif /* SPL_VNODE_H */

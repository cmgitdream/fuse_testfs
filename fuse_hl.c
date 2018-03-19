
/*
 * fuse high level example
* 2018 chenmin
 */

#define FUSE_USE_VERSION 29

#ifdef linux
// for pread()/pwrite()
#define	_XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <attr/xattr.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>

// currently base on version 2.9.2

struct test_file_handle {
  ino_t ino;
  int ref;
  int fd;
};

//extern "C" {
static int test_getattr(const char *path, struct stat *stbuf)
{
  //std::cout << __func__ << std::endl; 
  int res;
  if (stat(path, stbuf) < 0)
    return -errno;
  return 0;
}

static int test_mkdir(const char *path, mode_t mode)
{
  //std::cout << __func__ << std::endl; 
  if (mkdir(path, mode) < 0)
    return -errno;
  return 0;
}

static int test_unlink(const char *path)
{
  //std::cout << __func__ << std::endl; 
  if (unlink(path) < 0 && errno != ENOENT)
    return -errno;
  return 0;
}

static int test_rmdir(const char *path)
{
  //std::cout << __func__ << std::endl; 
  if (rmdir(path) < 0)
    return -errno;
  return 0;
}

static int test_rename(const char *oldpath, const char *newpath)
{
  //std::cout << __func__ << std::endl; 
  if (rename(oldpath, newpath) < 0)
    return -errno;
  return 0;
}

static int test_chmod(const char *path, mode_t mode)
{
  //std::cout << __func__ << std::endl; 
  if (chmod(path, mode) < 0)
    return -errno;
  return 0;
}

static int test_chown(const char *path, uid_t uid, gid_t gid)
{
  //std::cout << __func__ << std::endl; 
  if (chown(path, uid, gid) < 0)
    return -errno;
  return 0;
}

static int test_truncate(const char *path, off_t size)
{
  //std::cout << __func__ << std::endl; 
  if (truncate(path, size) < 0)
    return -errno;
  return 0;
}

static int test_open(const char *path, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  int fd;
  if ((fd = open(path, fi->flags)) < 0) {
    //std::cout << __func__ << " open file failed" << std::endl; 
    return -errno;
  }
  struct stat st;
  if (fstat(fd, &st) < 0) { 
    return -errno;
  }
  struct test_file_handle *fh = (struct test_file_handle* ) malloc(sizeof(struct test_file_handle));
  //std::cout << __func__ << ": fh = " << fh << std::endl;
  //std::cout << __func__ << ": ino = " << st.st_ino << std::endl;
  if (fh == NULL) {
    //std::cout << __func__ << ": fh malloc NULL " << fh << std::endl;
    return -ENOENT;
  }
  fh->ino = st.st_ino;
  fh->ref = 1;
  fh->fd = fd;
  // fill fh
  fi->fh = (uint64_t)fh;
  struct test_file_handle *fh2 = (struct test_file_handle *)fh;
  
  //std::cout << __func__ << ": fh2 = " << fh2 << std::endl;
  //std::cout << __func__ << ": fi = " << fi << std::endl;
  //std::cout << __func__ << ": fi->fh = " << fi->fh << std::endl;
  //std::cout << __func__ << ": fd = " << fd << std::endl;
  ////std::cout << __func__ << ": fi->fh->ino = " << fi->fh.ino << std::endl;
  return 0;
}

static int test_read(const char *path, char *buf, size_t len,
	off_t off, struct fuse_file_info *fi)
{
  //std::cout << __func__ << " off:" << off << ", len: " << len << std::endl; 
  int ret;
  //std::cout << __func__ << ": fi = " << fi << std::endl;
  //std::cout << __func__ << ": fi->fh = " << fi->fh << std::endl;
  assert(fi != NULL);
  /*
  int fd;
  if ((fd = open(path, fi->flags)) < 0)
    return -errno;
  */
  
  struct test_file_handle *fh = (struct test_file_handle *)fi->fh;
  if ((ret = pread(fh->fd, buf, len, off)) < 0)
    return -errno;

  return ret;
}

static int test_write(const char *path, const char *buf, size_t len,
	off_t off, struct fuse_file_info *fi)
{
  //std::cout << __func__ << " off:" << off << ", len: " << len << std::endl; 
  printf("%s: %s off:%lu len:%lu\n", __func__, off, len);
  int ret;
  assert(fi != NULL);
  /*
  int fd;
  if ((fd = open(path, fi->flags)) < 0)
    return -errno;
  */
  
  struct test_file_handle *fh = (struct test_file_handle *)fi->fh;
  //std::cout << __func__ << ": fd = " << fh->fd << std::endl;
  /*
  if (lseek(fh->fd, off, SEEK_SET)) {
    //std::cout << __func__ << " lseek failed errno = " << errno << " "
	<< strerror(errno) << std::endl; 
    return -errno;
  }
  if ((ret = write(fh->fd, buf, len)) < 0) {
    //std::cout << __func__ << " write failed errno = " << errno << " "
	<< strerror(errno) << std::endl; 
    return -errno;
  }
  */
  ///*
  // xfs or vfs : direct_io.c will check if IO page align
  // or else return -EINVAL
  if ((ret = pwrite(fh->fd, buf, len, off)) < 0) {
    //std::cout << __func__ << " pwrite failed errno = " << errno << " " << strerror(errno) << std::endl; 
    return -errno;
  }
  //*/

  return ret;
}

static int test_statfs(const char *path, struct statvfs *stv)
{
  //std::cout << __func__ << std::endl; 
  struct statfs st;
  int rv = statfs(path, &st);
  if (!rv) {
    stv->f_frsize	= st.f_bsize;
    stv->f_bsize	= st.f_blocks;
    stv->f_bavail	= st.f_bavail;
    stv->f_files	= st.f_files;
    stv->f_ffree	= st.f_ffree;
    stv->f_namemax	= st.f_namelen;
  }
  
  return rv;
}

static int test_flush(const char *path, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  // do nothing
  return 0;
}

static int test_release(const char *path, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  assert(fi != NULL);
  struct test_file_handle *fh = (struct test_file_handle *)fi->fh; 
  assert(fh->ref > 0); 
  --fh->ref;
  if (fh->ref == 0) {
    close(fh->fd);
  }
  free(fh);
  return 0;
}

static int test_fsync(const char *path, int sync_dataonly, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  struct test_file_handle *fh = (struct test_file_handle *)fi->fh;
  if (!sync_dataonly)
    fsync(fh->fd);
  else
    fdatasync(fh->fd);
  return 0;
}

static int test_setxattr(const char *path, const char *name,
		const char *value, size_t size, int flags)
{
  //std::cout << __func__ << " set xattr name = " << name << std::endl; 
  if (setxattr(path, name, value, size, flags) < 0)
    return -errno;
  return 0;
}

static int test_getxattr(const char *path, const char *name, char *value, size_t size)
{
  //std::cout << __func__ << " xattr name = " << name << std::endl; 
  if (getxattr(path, name, value, size) < 0)
    return -errno;
  return 0;
}

static int test_removexattr(const char *path, const char *name)
{
  //std::cout << __func__ << " xattr name = " << name << std::endl; 
  if (removexattr(path, name) < 0)
    return -errno;
  return 0;
}

static int test_opendir(const char *path, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  DIR *dirp = NULL;
  if ((dirp = opendir(path)) == NULL) {
    return -errno;
  } 
  fi->fh = (uint64_t)dirp;
  return 0;
}

static int test_readdir(const char * path, void *buf, fuse_fill_dir_t filler,
			off_t offset , struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  DIR *dirp = (DIR *)fi->fh; 
  struct dirent *ent = (struct dirent *)malloc(sizeof(struct dirent));
  if (ent == NULL) {
    //std::cout << " malloc for dir entry failed" << std::endl;
    return -errno;
  }
  struct dirent *result = NULL;
  while (readdir_r(dirp, ent, &result) == 0) {
    if (result == NULL)
      break;
    //std::cout << result->d_name << std::endl;
    filler(buf, result->d_name, NULL, 0); 
  }
  free(ent);
  return 0;
}

static int test_releasedir(const char *path, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  DIR *dirp = (DIR *)fi->fh; 
  
  if (closedir(dirp))
    return -errno;
  return 0;
}

static int test_fsyncdir(const char *path, int fd, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  // do nothing
  return 0;
}

static void *test_init(struct fuse_conn_info *conn)
{
  //std::cout << __func__ << std::endl; 
  return NULL;
}

static void test_destroy(void *data)
{
  //std::cout << __func__ << std::endl; 
  
}

static int test_access(const char *path, int mode)
{
  //std::cout << __func__ << std::endl; 
  if (access(path, mode) < 0)
    return -errno;
  return 0;
}

static int test_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
  //std::cout << __func__ << std::endl; 
  int fd;
  if ((fd = open(path, fi->flags, mode)) < 0)
    return -errno;
  struct stat st;
  if (fstat(fd, &st) < 0) 
    return -errno;
  struct test_file_handle *fh = (struct test_file_handle *)malloc(sizeof(struct test_file_handle));
  fh->ino = st.st_ino;
  fh->ref = 1;
  fh->fd = fd;
  // fill fh
  fi->fh = (uint64_t)fh;
  return 0;
}

static int test_fallocate(const char *path, int fd, off_t off, off_t len,
                          struct fuse_file_info *fi)
{
  /*
  int fd;
  if ((fd = open(path, fi->flags, O_RDWR)) < 0)
    return -errno;
  */
  if (fallocate(fd, O_RDWR, off, len) < 0)
    return -errno;
  return 0;
}

#if 0
static struct fuse_operations fuse_ops = {
getattr: test_getattr,
readlink: NULL,
getdir: NULL,
mknod: NULL,
mkdir: test_mkdir,
unlink: test_unlink,
rmdir: test_rmdir,
symlink: NULL,
rename: test_rename,
link: NULL,
chmod: test_chmod,
chown: test_chown,
truncate: test_truncate,
utime: NULL,
open: test_open,
read: test_read,
write: test_write,
statfs: test_statfs,
flush: test_flush,
release: test_release,
fsync: test_fsync,
setxattr: test_setxattr,
getxattr: test_getxattr,
listxattr: NULL,
removexattr: test_removexattr,
opendir: test_opendir,		// >= 2.3
readdir: test_readdir,		// >= 2.3
releasedir: test_releasedir,	// >= 2.3
fsyncdir: test_fsyncdir,	// >= 2.3
init: test_init,		// >= 2.3
destroy: test_destroy,		// >= 2.3
access: test_access,		// >= 2.5
create: test_create,		// >= 2.5
ftruncate: NULL,			// >= 2.5
fgetattr: NULL,			// >= 2.5
lock: NULL,			// >= 2.6
utimens: NULL,			// >= 2.6
bmap: NULL,			// >= 2.6
/*
ioctl: NULL,			// >= 2.8
poll: NULL,			// >= 2.8
write_buf: NULL,			// >= 2.9
read_buf: NULL,			// >= 2.9
flock: NULL,			// >= 2.9
fallocate: NULL			// >= 2.9.1
*/
};
#endif

static struct fuse_operations fuse_ops = {
.getattr = test_getattr,
.readlink = NULL,
.getdir = NULL,
.mknod = NULL,
.mkdir = test_mkdir,
.unlink = test_unlink,
.rmdir = test_rmdir,
.symlink = NULL,
.rename = test_rename,
.link = NULL,
.chmod = test_chmod,
.chown = test_chown,
.truncate = test_truncate,
.utime = NULL,
.open = test_open,
.read = test_read,
.write = test_write,
.statfs = test_statfs,
.flush = test_flush,
.release = test_release,
.fsync = test_fsync,
.setxattr = test_setxattr,
.getxattr = test_getxattr,
.listxattr = NULL,
.removexattr = test_removexattr,
.opendir = test_opendir,		// >= 2.3
.readdir = test_readdir,		// >= 2.3
.releasedir = test_releasedir,	// >= 2.3
.fsyncdir = test_fsyncdir,	// >= 2.3
.init = test_init,		// >= 2.3
.destroy = test_destroy,		// >= 2.3
.access = test_access,		// >= 2.5
.create = test_create,		// >= 2.5
.ftruncate = NULL,			// >= 2.5
.fgetattr = NULL,			// >= 2.5
.lock = NULL,			// >= 2.6
.utimens = NULL,			// >= 2.6
.bmap = NULL,			// >= 2.6
.ioctl = NULL,			// >= 2.8
.poll = NULL,			// >= 2.8
.write_buf = NULL,			// >= 2.9
.read_buf = NULL,			// >= 2.9
.flock = NULL,			// >= 2.9
.fallocate = test_fallocate			// >= 2.9.1
};

int main(int argc, char *argv[])
{
  return fuse_main(argc, argv, &fuse_ops, NULL);
}

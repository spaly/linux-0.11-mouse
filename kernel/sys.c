/*
 *  linux/kernel/sys.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>

struct linux_dirent{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[14];
};
int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int
count){
	if (!count) return -1;
	struct m_inode *inode; //内存中的i节点结构
	struct buffer_head *bh; // 缓冲区头
	inode=current->filp[fd]->f_inode;
	bh=bread(inode->i_dev, inode->i_zone[0]);

	struct linux_dirent myld;
	int siz_dir=sizeof(struct dir_entry),siz_ld=sizeof(struct linux_dirent);
	struct dir_entry *dir; //文件目录项
	char *buf; int ans=0,k=0; //ans记录读取的字节数
	for(; k<inode->i_size; k+=siz_dir){
		dir=(struct dir_entry *)(bh->b_data+k); //数据块指针+偏移量
		if (dir->inode==0) continue; //节点为空
		if (ans+siz_ld>=count) return 0; //dirp满了
		
		myld.d_ino=dir->inode;
		memcpy(myld.d_name,dir->name,NAME_LEN);
		myld.d_off=0;
		myld.d_reclen=sizeof(myld);

		buf=&myld; int i;
		for(i=0; i<myld.d_reclen; ++i) //存入dirp
			put_fs_byte(*(buf+i), ((char *)dirp)+i+ans );
		ans+=myld.d_reclen;
	}
	brelse(bh);
	return ans;
}
int sys_sleep(unsigned int seconds){
	if (sys_signal(SIGALRM,SIG_IGN,14)==-1) return -1;
	sys_alarm(seconds);
	sys_pause();
	return 0;
}
long sys_getcwd(char * buf, size_t size){
	struct buffer_head *bh;
	struct dir_entry *dir,*fdir;
	char ch[256]="",tmp[256]="";
	int siz_dir=sizeof(struct dir_entry);

	struct m_inode *xi=current->pwd,*fi; //xi是当前目录的i节点
	if (xi==current->root) ch[0]="/",ch[1]=0; //特判已经是根节点的情况
	else{
		int block;
		if ( !(block=xi->i_zone[0]) )
			return NULL;
		if ( !(bh=bread(xi->i_dev,block)) ) //读取当前目录的数据块内容
			return NULL;
		
		while(xi!=current->root){ //回溯到根目录为止
			dir=(struct dir_entry *)(bh->b_data+siz_dir); //定位上一级目录
			fi=iget(xi->i_dev, dir->inode); //fi是上一级目录的i节点
			if ( !(block=fi->i_zone[0]) )
				return NULL;
			if ( !(bh=bread(fi->i_dev,block)) ) //读取上一级目录的数据块内容
				return NULL;
			
			int k=0;
			for(; k<fi->i_size; k+=siz_dir){ //遍历上一级目录的目录项
				fdir=(struct dir_entry *)(bh->b_data+k);
				if (fdir->inode == xi->i_num){ //i节点号对上了，表示找到了当前节点对应的文件目录项
					strcpy(tmp, "/"); strcat(tmp, fdir->name); //拼接目录路径字符串
					strcat(tmp, ch); strcpy(ch, tmp);
					break;
				}
			}
			xi=fi; //回溯到上一级目录
		}
	}
	int l=strlen(ch),i;
	if (l>size) return NULL;
	for(i=0;i<l;++i) put_fs_byte(ch[i],buf+i);
	return (long)buf;
};
int sys_pipe2(){return -1;}
int sys_mmap(){return -1;}
int sys_munmap(){return -1;}
int sys_clone(){return -1;}


int sys_ftime(){
	return -ENOSYS;
}

int sys_break(){
	return -ENOSYS;
}

int sys_ptrace(){
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_setregid(int rgid, int egid)
{
	if (rgid>0) {
		if ((current->gid == rgid) || 
		    suser())
			current->gid = rgid;
		else
			return(-EPERM);
	}
	if (egid>0) {
		if ((current->gid == egid) ||
		    (current->egid == egid) ||
		    suser()) {
			current->egid = egid;
			current->sgid = egid;
		} else
			return(-EPERM);
	}
	return 0;
}

int sys_setgid(int gid)
{
/*	return(sys_setregid(gid, gid)); */
	if (suser())
		current->gid = current->egid = current->sgid = gid;
	else if ((gid == current->gid) || (gid == current->sgid))
		current->egid = gid;
	else
		return -EPERM;
	return 0;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

int sys_time(long * tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc) {
		verify_area(tloc,4);
		put_fs_long(i,(unsigned long *)tloc);
	}
	return i;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid)
{
	int old_ruid = current->uid;
	
	if (ruid>0) {
		if ((current->euid==ruid) ||
                    (old_ruid == ruid) ||
		    suser())
			current->uid = ruid;
		else
			return(-EPERM);
	}
	if (euid>0) {
		if ((old_ruid == euid) ||
                    (current->euid == euid) ||
		    suser()) {
			current->euid = euid;
			current->suid = euid;
		} else {
			current->uid = old_ruid;
			return(-EPERM);
		}
	}
	return 0;
}

int sys_setuid(int uid)
{
/*	return(sys_setreuid(uid, uid)); */
	if (suser())
		current->uid = current->euid = current->suid = uid;
	else if ((uid == current->uid) || (uid == current->suid))
		current->euid = uid;
	else
		return -EPERM;
	return(0);
}

int sys_stime(long * tptr)
{
	if (!suser())
		return -EPERM;
	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
	return 0;
}

int sys_times(struct tms * tbuf)
{
	if (tbuf) {
		verify_area(tbuf,sizeof *tbuf);
		put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
		put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
		put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
		put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
	}
	return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&
	    end_data_seg < current->start_stack - 16384)
		current->brk = end_data_seg;
	return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
	int i;

	if (!pid)
		pid = current->pid;
	if (!pgid)
		pgid = current->pid;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->pid==pid) {
			if (task[i]->leader)
				return -EPERM;
			if (task[i]->session != current->session)
				return -EPERM;
			task[i]->pgrp = pgid;
			return 0;
		}
	return -ESRCH;
}

int sys_getpgrp(void)
{
	return current->pgrp;
}

int sys_setsid(void)
{
	if (current->leader && !suser())
		return -EPERM;
	current->leader = 1;
	current->session = current->pgrp = current->pid;
	current->tty = -1;
	return current->pgrp;
}

int sys_getgroups()
{
	return -ENOSYS;
}

int sys_setgroups()
{
	return -ENOSYS;
}

int sys_uname(struct utsname * name)
{
	static struct utsname thisname = {
		"linux .0","nodename","release ","version ","machine "
	};
	int i;

	if (!name) return -ERROR;
	verify_area(name,sizeof *name);
	for(i=0;i<sizeof *name;i++)
		put_fs_byte(((char *) &thisname)[i],i+(char *) name);
	return 0;
}

int sys_sethostname()
{
	return -ENOSYS;
}

int sys_getrlimit()
{
	return -ENOSYS;
}

int sys_setrlimit()
{
	return -ENOSYS;
}

int sys_getrusage()
{
	return -ENOSYS;
}

int sys_gettimeofday()
{
	return -ENOSYS;
}

int sys_settimeofday()
{
	return -ENOSYS;
}


int sys_umask(int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}

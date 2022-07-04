/*
 *  linux/mm/memory.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * demand-loading started 01.12.91 - seems it is high on the list of
 * things wanted, and it should be easy to implement. - Linus
 */

/*
 * Ok, demand-loading was easy, shared pages a little bit tricker. Shared
 * pages started 02.12.91, seems to work. - Linus.
 *
 * Tested sharing by executing about 30 /bin/sh: under the old kernel it
 * would have taken more than the 6M I have free, but it worked well as
 * far as I could see.
 *
 * Also corrected some "invalidate()"s - I wasn't doing enough of them.
 */
<<<<<<< HEAD
#include <sys/stat.h>
=======

>>>>>>> 050f6f7d1bf5d71774487b5aab969cf5690035e7
#include <signal.h>
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>

volatile void do_exit(long code);

static inline volatile void oom(void)
{
	printk("out of memory\n\r");
	do_exit(SIGSEGV);
}

#define invalidate() \
__asm__("movl %%eax,%%cr3"::"a" (0))

/* these are not to be changed without changing head.s etc */
#define LOW_MEM 0x100000
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED 100

#define CODE_SPACE(addr) ((((addr)+4095)&~4095) < \
current->start_code + current->end_code)

<<<<<<< HEAD
long HIGH_MEMORY = 0;
=======
static long HIGH_MEMORY = 0;
>>>>>>> 050f6f7d1bf5d71774487b5aab969cf5690035e7

#define copy_page(from,to) \
__asm__("cld ; rep ; movsl"::"S" (from),"D" (to),"c" (1024))

static unsigned char mem_map [ PAGING_PAGES ] = {0,};

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
unsigned long get_free_page(void)
{
register unsigned long __res asm("ax");

__asm__("std ; repne ; scasb\n\t"
	"jne 1f\n\t"
	"movb $1,1(%%edi)\n\t"
	"sall $12,%%ecx\n\t"
	"addl %2,%%ecx\n\t"
	"movl %%ecx,%%edx\n\t"
	"movl $1024,%%ecx\n\t"
	"leal 4092(%%edx),%%edi\n\t"
	"rep ; stosl\n\t"
	"movl %%edx,%%eax\n\t"
	"1:"
	"cld\n\t"       /* by wyj */
	:"=a" (__res)
	:"0" (0),"i" (LOW_MEM),"c" (PAGING_PAGES),
	"D" (mem_map+PAGING_PAGES-1)
	);
return __res;
}

/*
 * Free a page of memory at physical address 'addr'. Used by
 * 'free_page_tables()'
 */
void free_page(unsigned long addr)
{
	if (addr < LOW_MEM) return;
	if (addr >= HIGH_MEMORY)
		panic("trying to free nonexistent page");
	addr -= LOW_MEM;
	addr >>= 12;
	if (mem_map[addr]--) return;
	mem_map[addr]=0;
	panic("trying to free free page");
}

/*
 * This function frees a continuos block of page tables, as needed
 * by 'exit()'. As does copy_page_tables(), this handles only 4Mb blocks.
 */
int free_page_tables(unsigned long from,unsigned long size)
{
	unsigned long *pg_table;
	unsigned long * dir, nr;

	if (from & 0x3fffff)
		panic("free_page_tables called with wrong alignment");
	if (!from)
		panic("Trying to free up swapper memory space");
	size = (size + 0x3fffff) >> 22;
	dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */
	for ( ; size-->0 ; dir++) {
		if (!(1 & *dir))
			continue;
		pg_table = (unsigned long *) (0xfffff000 & *dir);
		for (nr=0 ; nr<1024 ; nr++) {
			if (1 & *pg_table)
				free_page(0xfffff000 & *pg_table);
			*pg_table = 0;
			pg_table++;
		}
		free_page(0xfffff000 & *dir);
		*dir = 0;
	}
	invalidate();
	return 0;
}

/*
 *  Well, here is one of the most complicated functions in mm. It
 * copies a range of linerar addresses by copying only the pages.
 * Let's hope this is bug-free, 'cause this one I don't want to debug :-)
 *
 * Note! We don't copy just any chunks of memory - addresses have to
 * be divisible by 4Mb (one page-directory entry), as this makes the
 * function easier. It's used only by fork anyway.
 *
 * NOTE 2!! When from==0 we are copying kernel space for the first
 * fork(). Then we DONT want to copy a full page-directory entry, as
 * that would lead to some serious memory waste - we just copy the
 * first 160 pages - 640kB. Even that is more than we need, but it
 * doesn't take any more memory - we don't copy-on-write in the low
 * 1 Mb-range, so the pages can be shared with the kernel. Thus the
 * special case for nr=xxxx.
 */
int copy_page_tables(unsigned long from,unsigned long to,long size)
{
	unsigned long * from_page_table;
	unsigned long * to_page_table;
	unsigned long this_page;
	unsigned long * from_dir, * to_dir;
	unsigned long nr;

	if ((from&0x3fffff) || (to&0x3fffff))
		panic("copy_page_tables called with wrong alignment");
	from_dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */
	to_dir = (unsigned long *) ((to>>20) & 0xffc);
	size = ((unsigned) (size+0x3fffff)) >> 22;
	for( ; size-->0 ; from_dir++,to_dir++) {
		if (1 & *to_dir)
			panic("copy_page_tables: already exist");
		if (!(1 & *from_dir))
			continue;
		from_page_table = (unsigned long *) (0xfffff000 & *from_dir);
		if (!(to_page_table = (unsigned long *) get_free_page()))
			return -1;	/* Out of memory, see freeing */
		*to_dir = ((unsigned long) to_page_table) | 7;
		nr = (from==0)?0xA0:1024;
		for ( ; nr-- > 0 ; from_page_table++,to_page_table++) {
			this_page = *from_page_table;
			if (!(1 & this_page))
				continue;
			this_page &= ~2;
			*to_page_table = this_page;
			if (this_page > LOW_MEM) {
				*from_page_table = this_page;
				this_page -= LOW_MEM;
				this_page >>= 12;
				mem_map[this_page]++;
			}
		}
	}
	invalidate();
	return 0;
}

/*
 * This function puts a page in memory at the wanted address.
 * It returns the physical address of the page gotten, 0 if
 * out of memory (either when trying to access page-table or
 * page.)
 */
unsigned long put_page(unsigned long page,unsigned long address)
{
	unsigned long tmp, *page_table;

/* NOTE !!! This uses the fact that _pg_dir=0 */

	if (page < LOW_MEM || page >= HIGH_MEMORY)
		printk("Trying to put page %p at %p\n",page,address);
	if (mem_map[(page-LOW_MEM)>>12] != 1)
		printk("mem_map disagrees with %p at %p\n",page,address);
	page_table = (unsigned long *) ((address>>20) & 0xffc);
	if ((*page_table)&1)
		page_table = (unsigned long *) (0xfffff000 & *page_table);
	else {
		if (!(tmp=get_free_page()))
			return 0;
		*page_table = tmp|7;
		page_table = (unsigned long *) tmp;
	}
	page_table[(address>>12) & 0x3ff] = page | 7;
/* no need for invalidate */
	return page;
}

void un_wp_page(unsigned long * table_entry)
{
	unsigned long old_page,new_page;

	old_page = 0xfffff000 & *table_entry;
	if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)]==1) {
		*table_entry |= 2;
		invalidate();
		return;
	}
	if (!(new_page=get_free_page()))
		oom();
	if (old_page >= LOW_MEM)
		mem_map[MAP_NR(old_page)]--;
	*table_entry = new_page | 7;
	invalidate();
	copy_page(old_page,new_page);
}	

/*
 * This routine handles present pages, when users try to write
 * to a shared page. It is done by copying the page to a new address
 * and decrementing the shared-page counter for the old page.
 *
 * If it's in code space we exit with a segment error.
 */
void do_wp_page(unsigned long error_code,unsigned long address)
{
#if 0
/* we cannot do this yet: the estdio library writes to code space */
/* stupid, stupid. I really want the libc.a from GNU */
	if (CODE_SPACE(address))
		do_exit(SIGSEGV);
#endif
	un_wp_page((unsigned long *)
		(((address>>10) & 0xffc) + (0xfffff000 &
		*((unsigned long *) ((address>>20) &0xffc)))));

}

void write_verify(unsigned long address)
{
	unsigned long page;

	if (!( (page = *((unsigned long *) ((address>>20) & 0xffc)) )&1))
		return;
	page &= 0xfffff000;
	page += ((address>>10) & 0xffc);
	if ((3 & *(unsigned long *) page) == 1)  /* non-writeable, present */
		un_wp_page((unsigned long *) page);
	return;
}

void get_empty_page(unsigned long address)
{
	unsigned long tmp;

	if (!(tmp=get_free_page()) || !put_page(tmp,address)) {
		free_page(tmp);		/* 0 is ok - ignored */
		oom();
	}
}

/*
 * try_to_share() checks the page at address "address" in the task "p",
 * to see if it exists, and if it is clean. If so, share it with the current
 * task.
 *
 * NOTE! This assumes we have checked that p != current, and that they
 * share the same executable.
 */
static int try_to_share(unsigned long address, struct task_struct * p)
{
	unsigned long from;
	unsigned long to;
	unsigned long from_page;
	unsigned long to_page;
	unsigned long phys_addr;

	from_page = to_page = ((address>>20) & 0xffc);
	from_page += ((p->start_code>>20) & 0xffc);
	to_page += ((current->start_code>>20) & 0xffc);
/* is there a page-directory at from? */
	from = *(unsigned long *) from_page;
	if (!(from & 1))
		return 0;
	from &= 0xfffff000;
	from_page = from + ((address>>10) & 0xffc);
	phys_addr = *(unsigned long *) from_page;
/* is the page clean and present? */
	if ((phys_addr & 0x41) != 0x01)
		return 0;
	phys_addr &= 0xfffff000;
	if (phys_addr >= HIGH_MEMORY || phys_addr < LOW_MEM)
		return 0;
	to = *(unsigned long *) to_page;
	if (!(to & 1))
		if (to = get_free_page())
			*(unsigned long *) to_page = to | 7;
		else
			oom();
	to &= 0xfffff000;
	to_page = to + ((address>>10) & 0xffc);
	if (1 & *(unsigned long *) to_page)
		panic("try_to_share: to_page already exists");
/* share them: write-protect */
	*(unsigned long *) from_page &= ~2;
	*(unsigned long *) to_page = *(unsigned long *) from_page;
	invalidate();
	phys_addr -= LOW_MEM;
	phys_addr >>= 12;
	mem_map[phys_addr]++;
	return 1;
}

/*
 * share_page() tries to find a process that could share a page with
 * the current one. Address is the address of the wanted page relative
 * to the current data space.
 *
 * We first check if it is at all feasible by checking executable->i_count.
 * It should be >1 if there are other tasks sharing this inode.
 */
static int share_page(unsigned long address)
{
	struct task_struct ** p;

	if (!current->executable)
		return 0;
	if (current->executable->i_count < 2)
		return 0;
	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p)
			continue;
		if (current == *p)
			continue;
		if ((*p)->executable != current->executable)
			continue;
		if (try_to_share(address,*p))
			return 1;
	}
	return 0;
}
// start
void do_no_page2(unsigned long error_code,unsigned long address)
{
	int nr[4];
	unsigned long tmp;
	unsigned long page;
	int block,i;
	address &= 0xfffff000;
	tmp = address - current->start_code;
	if (!current->executable || tmp >= current->end_data) {
		get_empty_page(address);
		return;
	}
	if (share_page(tmp))
		return;
	if (!(page = get_free_page()))
		oom();
/* remember that 1 block is used for header */
	block = 1 + tmp/BLOCK_SIZE;
	for (i=0 ; i<4 ; block++,i++)
		nr[i] = bmap(current->executable,block);
	bread_page(page,current->executable->i_dev,nr);
	i = tmp + 4096 - current->end_data;
	tmp = page + 4096;
	while (i-- > 0) {
		tmp--;
		*(char *)tmp = 0;
	}
	if (put_page(page,address))
		return;
	free_page(page);
	oom();
}

int do_execve2(unsigned long * eip,long tmp,char * filename,
char ** argv, char ** envp){
	int flag=0; flag=do_execve(eip,tmp,filename,argv,envp);
	if (flag==-1) return -1;
	long start=current->start_code,end=current->start_code+current->brk,j;
	//printk("#%x %x\n",start,end);
	for(j=start;j<=end;j+=4096){
		do_no_page2(4,j);
	}
}
void do_no_page(unsigned long error_code,unsigned long address)
{
	int nr[4];
	unsigned long tmp;
	unsigned long page;
	int block,i;
	/*
	if (current->pid > 5)
		printk(" --do_no_page: address=%x, pid=%d\n", address, current->pid);
	*/
	address &= 0xfffff000;
	tmp = address - current->start_code;
	if (!current->executable || tmp >= current->end_data) {
		get_empty_page(address);
		return;
	}
	if (share_page(tmp))
		return;
	if (!(page = get_free_page()))
		oom();
/* remember that 1 block is used for header */
	block = 1 + tmp/BLOCK_SIZE;
	for (i=0 ; i<4 ; block++,i++)
		nr[i] = bmap(current->executable,block);
	bread_page(page,current->executable->i_dev,nr);
	i = tmp + 4096 - current->end_data;
	tmp = page + 4096;
	while (i-- > 0) {
		tmp--;
		*(char *)tmp = 0;
	}
	if (put_page(page,address))
		return;
	free_page(page);
	oom();
}

void mem_init(long start_mem, long end_mem)
{
	int i;

	HIGH_MEMORY = end_mem;
	for (i=0 ; i<PAGING_PAGES ; i++)
		mem_map[i] = USED;
	i = MAP_NR(start_mem);
	end_mem -= start_mem;
	end_mem >>= 12;
	while (end_mem-->0)
		mem_map[i++]=0;
}

void calc_mem(void)
{
	int i,j,k,free=0;
	long * pg_tbl;

	for(i=0 ; i<PAGING_PAGES ; i++)
		if (!mem_map[i]) free++;
	printk("%d pages free (of %d)\n\r",free,PAGING_PAGES);
	for(i=2 ; i<1024 ; i++) {
		if (1&pg_dir[i]) {
			pg_tbl=(long *) (0xfffff000 & pg_dir[i]);
			for(j=k=0 ; j<1024 ; j++)
				if (pg_tbl[j]&1)
					k++;
			printk("Pg-dir[%d] uses %d pages\n",i,k);
		}
	}
}
<<<<<<< HEAD


#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")
static struct swap_info_struct {
	unsigned long flags;
	struct inode * swap_file;
	unsigned int swap_device;
	unsigned char * swap_map;
	char * swap_lockmap;
	int lowest_bit;
	int highest_bit;
} swap_info[8];
#define SWP_USED	1
static struct task_struct* tmpp[35];
void swap_free(unsigned int nr){
	struct swap_info_struct * p;
	if (!nr)
		return;
	printk("!!\n");
	/*
	if ((nr >> 24) >= nr_swapfiles) {
		printk("Trying to free nonexistent swap-page\n");
		return;
	}*/
	p = (nr >> 24) + swap_info;
	nr &= 0x00ffffff;
	if (nr >= 4096) {
		printk("swap_free: weirness\n");
		return;
	}
	if (!(p->flags & SWP_USED)) {
		printk("Trying to free swap from unused swap-device\n");
		return;
	}
	tmpp[0]=current; int l=0,i;
	while (setbit(p->swap_lockmap,nr)){
		printk("#%d ",p->swap_lockmap);
		tmpp[++l]=tmpp[0],sleep_on(&tmpp[0]);
	}
	if (nr < p->lowest_bit)
		p->lowest_bit = nr;
	if (nr > p->highest_bit)
		p->highest_bit = nr;
	if (!p->swap_map[nr])
		printk("swap_free: swap-space map bad (page %d)\n",nr);
	else
		p->swap_map[nr]--;
	if (!clrbit(p->swap_lockmap,nr))
		printk("swap_free: lock already cleared\n");
	for(i=0;i<=l;++i) wake_up(&tmpp[i]);
}

/*
 * maps a range of physical memory into the requested pages. the old
 * mappings are removed. any references to nonexistent pages results
 * in null mappings (currently treated as "copy-on-access")
 *
 * permiss is encoded as cxwr (copy,exec,write,read) where copy modifies
 * the behavior of write to be copy-on-write.
 *
 * due to current limitations, we actually have the following
 *		on		off
 * read:	yes		yes
 * write/copy:	yes/copy	copy/copy
 * exec:	yes		yes
 */
#define PAGE_ACCESSED 0x20
#define MAP_PAGE_RESERVED (1<<15)
int remap_page_range(unsigned long from, unsigned long to, unsigned long size,
		 int permiss)
{
	unsigned long *page_table, *dir;
	unsigned long poff, pcnt;
	unsigned long page;
	printk("???\n");
	if ((from & 0xfff) || (to & 0xfff))
		panic("remap_page_range called with wrong alignment");
	dir = (unsigned long *) (current->tss.cr3 + ((from >> 20) & 0xffc));
	size = (size + 0xfff) >> 12;
	poff = (from >> 12) & 0x3ff;
	if ((pcnt = 1024 - poff) > size)
		pcnt = size;
	printk("^%d\n",size);
	while (size > 0) {
		printk("^%d\n",size);
		if (!(1 & *dir)) {
			if (!(page_table = (unsigned long *)get_free_page())) { //?
				invalidate();
				return -1;
			}
			*dir++ = ((unsigned long) page_table) | PAGE_ACCESSED | 7; //?
		}
		else
			page_table = (unsigned long *)(0xfffff000 & *dir++);
		if (poff) {
			page_table += poff;
			poff = 0;
		}

		for (size -= pcnt; pcnt-- ;) {
			int mask = 4;
			if (permiss & 1)
				mask |= 1;
			if (permiss & 2) {
				if (permiss & 8)
					mask |= 1;
				else
					mask |= 3;
			}
			if (permiss & 4)
				mask |= 1;

			if ((page = *page_table) != 0) {
				*page_table = 0;
				--current->rss;
				if (1 & page)
					free_page(0xfffff000 & page);
				else
					swap_free(page >> 1);
			}

			
			if (mask == 4 || to >= HIGH_MEMORY || !mem_map[MAP_NR(to)])
				*page_table++ = 0;	//not present 
			else {
				++current->rss;
				*page_table++ = (to | mask);
				if (!(mem_map[MAP_NR(to)] & MAP_PAGE_RESERVED))
					mem_map[MAP_NR(to)]++;
			}
			to += PAGE_SIZE;
		}
		pcnt = (size > 1024 ? 1024 : size);
	}
	invalidate();
	return 0;
}


/*
 * description of effects of mapping type and prot in current implementation.
 * this is due to the current handling of page faults in memory.c. the expected
 * behavior is in parens:
 *
 * map_type	prot
 *		PROT_NONE	PROT_READ	PROT_WRITE	PROT_EXEC
 * MAP_SHARED	r: (no) yes	r: (yes) yes	r: (no) yes	r: (no) no
 *		w: (no) yes	w: (no) copy	w: (yes) yes	w: (no) no
 *		x: (no) no	x: (no) no	x: (no) no	x: (yes) no
 *		
 * MAP_PRIVATE	r: (no) yes	r: (yes) yes	r: (no) yes	r: (no) no
 *		w: (no) copy	w: (no) copy	w: (copy) copy	w: (no) no
 *		x: (no) no	x: (no) no	x: (no) no	x: (yes) no
 *
 * the permissions are encoded as cxwr (copy,exec,write,read)
 */
#define MAP_TYPE         0xf       /* Mask for type of mapping */
#define PROT_READ	1		/* page can be read */
#define PROT_WRITE	2		/* page can be written */
#define PROT_EXEC	4		/* page can be executed */
#define PROT_NONE	0		/* page can not be accessed */
#define MAP_FILE	0
#define MAP_SHARED	1		/* Share changes */
#define MAP_PRIVATE	2		/* Changes are private */
#define MTYP(T) ((T) & MAP_TYPE)
#define PREAD(T,P) (((P) & PROT_READ) ? 1 : 0)
#define PWRITE(T,P) (((P) & PROT_WRITE) ? (MTYP(T) == MAP_SHARED ? 2 : 10) : 0)
#define PEXEC(T,P) (((P) & PROT_EXEC) ? 4 : 0)
#define PERMISS(T,P) (PREAD(T,P)|PWRITE(T,P)|PEXEC(T,P))
long mmap(void *start, size_t len, int prot, int flags,
int fd, off_t off){
	printk("~~~\n");
	flags=1; fd=3; off=0;
	unsigned long base,addr=(unsigned long*)start;
	unsigned long limit;
	struct file *file;
	struct m_inode *inode;
	file = current->filp[fd]; inode = file->f_inode;

	/*
	 * do simple checking here so the lower-level routines won't have
	 * to. we assume access permissions have been handled by the open
	 * of the memory object, so we don't do any here.
	 */
	/*
	switch(flags){
		case MAP_SHARED:
			if ((prot & PROT_WRITE) && !(file->f_mode & 2)){
				printk("  %d %d %d\n",prot,PROT_WRITE,file->f_mode);
				return -1;
			}
			//fall through
		case MAP_PRIVATE:{
			if (!(file->f_mode & 1))
				return -1;
			break;
		}
		default:
			return -1;
	}
	*/
	/*
	 * obtain the address to map to. we verify (or select) it and ensure
	 * that it represents a valid section of the address space. we assume
	 * that if PROT_EXEC is specified this should be in the code segment.
	 */
	 //0.11
	if (prot&PROT_EXEC) //cs
		base=get_base(current->ldt[1]),limit=get_limit(0x0f);
	else //ds
		base=get_base(current->ldt[2]),limit=get_limit(0x17);
	if (addr+len>limit)
			return -1; // fang bu xia
	/*
	 * determine the object being mapped and call the appropriate
	 * specific mapper. the address has already been validated, but
	 * not unmapped
	 */
	 //0.11
	printk("$%d %d %d\n",len,HIGH_MEMORY,base);
	if (len>HIGH_MEMORY||off>HIGH_MEMORY-len) /* avoid overflow */
		return -1;
	addr+=base;
	if (remap_page_range(addr,off,len,PERMISS(flags,prot)))
		return -1;
	
	if ((long)addr>0) addr-=base;
	return addr;
}
int munmap(void * start, size_t len){
    unsigned long base, limit,addr=(unsigned long *)start;
	base=get_base(current->ldt[2]);	/* map into ds */
	limit=get_limit(0x17);		/* ds limit */

	if (addr+len>limit) return -1;
	if (free_page_tables(base+addr,len)) //unmap_page_range?
		return -1; /* should never happen */
	return 0;
}
=======
>>>>>>> 050f6f7d1bf5d71774487b5aab969cf5690035e7

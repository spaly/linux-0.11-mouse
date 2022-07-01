# linux-0.11 (more syscalls)
一个扩充了一些系统调用的linux 0.11

## Update_v0.1
在kernal/sys.c里添加了sys_sleep，相应更改了include/unistd.h和include/linux/sys.h

## Update_v0.2
添加了sys_execve2，修改的文件有mm/memory.c，kernal/system_call.s，lib/execve.c

## Update_v0.2.1
重新上传了完整代码到master分支, 之后都在那里更新

## v0.3
添加了sys_getdents

## v0.4
添加了sys_getcwd

## v0.4.1
添加了部分注释

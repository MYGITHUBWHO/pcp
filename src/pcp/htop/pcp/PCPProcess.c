/*
htop - PCPProcess.c
(C) 2014 Hisham H. Muhammad
(C) 2020 Red Hat, Inc.  All Rights Reserved.
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "PCPProcess.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

#include "CRT.h"
#include "Process.h"
#include "ProvideCurses.h"
#include "XUtils.h"


ProcessFieldData Process_fields[] = {
   [0] = { .name = "", .title = NULL, .description = NULL, .flags = 0, },
   [PID] = { .name = "PID", .title = "    PID ", .description = "Process/thread ID", .flags = 0, },
   [COMM] = { .name = "Command", .title = "Command ", .description = "Command line", .flags = 0, },
   [STATE] = { .name = "STATE", .title = "S ", .description = "Process state (S sleeping, R running, D disk, Z zombie, T traced, W paging, I idle)", .flags = 0, },
   [PPID] = { .name = "PPID", .title = "   PPID ", .description = "Parent process ID", .flags = 0, },
   [PGRP] = { .name = "PGRP", .title = "   PGRP ", .description = "Process group ID", .flags = 0, },
   [SESSION] = { .name = "SESSION", .title = "    SID ", .description = "Process's session ID", .flags = 0, },
   [TTY_NR] = { .name = "TTY_NR", .title = "TTY      ", .description = "Controlling terminal", .flags = 0, },
   [TPGID] = { .name = "TPGID", .title = "  TPGID ", .description = "Process ID of the fg process group of the controlling terminal", .flags = 0, },
   [FLAGS] = { .name = "FLAGS", .title = NULL, .description = NULL, .flags = 0, },
   [MINFLT] = { .name = "MINFLT", .title = "     MINFLT ", .description = "Number of minor faults which have not required loading a memory page from disk", .flags = 0, },
   [CMINFLT] = { .name = "CMINFLT", .title = "    CMINFLT ", .description = "Children processes' minor faults", .flags = 0, },
   [MAJFLT] = { .name = "MAJFLT", .title = "     MAJFLT ", .description = "Number of major faults which have required loading a memory page from disk", .flags = 0, },
   [CMAJFLT] = { .name = "CMAJFLT", .title = "    CMAJFLT ", .description = "Children processes' major faults", .flags = 0, },
   [UTIME] = { .name = "UTIME", .title = " UTIME+  ", .description = "User CPU time - time the process spent executing in user mode", .flags = 0, },
   [STIME] = { .name = "STIME", .title = " STIME+  ", .description = "System CPU time - time the kernel spent running system calls for this process", .flags = 0, },
   [CUTIME] = { .name = "CUTIME", .title = " CUTIME+ ", .description = "Children processes' user CPU time", .flags = 0, },
   [CSTIME] = { .name = "CSTIME", .title = " CSTIME+ ", .description = "Children processes' system CPU time", .flags = 0, },
   [PRIORITY] = { .name = "PRIORITY", .title = "PRI ", .description = "Kernel's internal priority for the process", .flags = 0, },
   [NICE] = { .name = "NICE", .title = " NI ", .description = "Nice value (the higher the value, the more it lets other processes take priority)", .flags = 0, },
   [ITREALVALUE] = { .name = "ITREALVALUE", .title = NULL, .description = NULL, .flags = 0, },
   [STARTTIME] = { .name = "STARTTIME", .title = "START ", .description = "Time the process was started", .flags = 0, },
   [VSIZE] = { .name = "VSIZE", .title = NULL, .description = NULL, .flags = 0, },
   [RSS] = { .name = "RSS", .title = NULL, .description = NULL, .flags = 0, },
   [RLIM] = { .name = "RLIM", .title = NULL, .description = NULL, .flags = 0, },
   [STARTCODE] = { .name = "STARTCODE", .title = NULL, .description = NULL, .flags = 0, },
   [ENDCODE] = { .name = "ENDCODE", .title = NULL, .description = NULL, .flags = 0, },
   [STARTSTACK] = { .name = "STARTSTACK", .title = NULL, .description = NULL, .flags = 0, },
   [KSTKESP] = { .name = "KSTKESP", .title = NULL, .description = NULL, .flags = 0, },
   [KSTKEIP] = { .name = "KSTKEIP", .title = NULL, .description = NULL, .flags = 0, },
   [SIGNAL] = { .name = "SIGNAL", .title = NULL, .description = NULL, .flags = 0, },
   [BLOCKED] = { .name = "BLOCKED", .title = NULL, .description = NULL, .flags = 0, },
   [SSIGIGNORE] = { .name = "SIGIGNORE", .title = NULL, .description = NULL, .flags = 0, },
   [SIGCATCH] = { .name = "SIGCATCH", .title = NULL, .description = NULL, .flags = 0, },
   [WCHAN] = { .name = "WCHAN", .title = NULL, .description = NULL, .flags = 0, },
   [NSWAP] = { .name = "NSWAP", .title = NULL, .description = NULL, .flags = 0, },
   [CNSWAP] = { .name = "CNSWAP", .title = NULL, .description = NULL, .flags = 0, },
   [EXIT_SIGNAL] = { .name = "EXIT_SIGNAL", .title = NULL, .description = NULL, .flags = 0, },
   [PROCESSOR] = { .name = "PROCESSOR", .title = "CPU ", .description = "Id of the CPU the process last executed on", .flags = 0, },
   [M_VIRT] = { .name = "M_VIRT", .title = " VIRT ", .description = "Total program size in virtual memory", .flags = 0, },
   [M_RESIDENT] = { .name = "M_RESIDENT", .title = "  RES ", .description = "Resident set size, size of the text and data sections, plus stack usage", .flags = 0, },
   [M_SHARE] = { .name = "M_SHARE", .title = "  SHR ", .description = "Size of the process's shared pages", .flags = 0, },
   [M_TRS] = { .name = "M_TRS", .title = " CODE ", .description = "Size of the text segment of the process", .flags = 0, },
   [M_DRS] = { .name = "M_DRS", .title = " DATA ", .description = "Size of the data segment plus stack usage of the process", .flags = 0, },
   [M_LRS] = { .name = "M_LRS", .title = " LIB ", .description = "The library size of the process (unused since Linux 2.6; always 0)", .flags = 0, },
   [M_DT] = { .name = "M_DT", .title = " DIRTY ", .description = "Size of the dirty pages of the process (unused since Linux 2.6; always 0)", .flags = 0, },
   [ST_UID] = { .name = "ST_UID", .title = "  UID ", .description = "User ID of the process owner", .flags = 0, },
   [PERCENT_CPU] = { .name = "PERCENT_CPU", .title = "CPU% ", .description = "Percentage of the CPU time the process used in the last sampling", .flags = 0, },
   [PERCENT_NORM_CPU] = { .name = "PERCENT_NORM_CPU", .title = "NCPU%", .description = "Normalized percentage of the CPU time the process used in the last sampling (normalized by cpu count)", .flags = 0, },
   [PERCENT_MEM] = { .name = "PERCENT_MEM", .title = "MEM% ", .description = "Percentage of the memory the process is using, based on resident memory size", .flags = 0, },
   [USER] = { .name = "USER", .title = "USER      ", .description = "Username of the process owner (or user ID if name cannot be determined)", .flags = 0, },
   [TIME] = { .name = "TIME", .title = "  TIME+  ", .description = "Total time the process has spent in user and system time", .flags = 0, },
   [NLWP] = { .name = "NLWP", .title = "NLWP ", .description = "Number of threads in the process", .flags = 0, },
   [TGID] = { .name = "TGID", .title = "   TGID ", .description = "Thread group ID (i.e. process ID)", .flags = 0, },
   [RCHAR] = { .name = "RCHAR", .title = "    RD_CHAR ", .description = "Number of bytes the process has read", .flags = PROCESS_FLAG_IO, },
   [WCHAR] = { .name = "WCHAR", .title = "    WR_CHAR ", .description = "Number of bytes the process has written", .flags = PROCESS_FLAG_IO, },
   [SYSCR] = { .name = "SYSCR", .title = "    RD_SYSC ", .description = "Number of read(2) syscalls for the process", .flags = PROCESS_FLAG_IO, },
   [SYSCW] = { .name = "SYSCW", .title = "    WR_SYSC ", .description = "Number of write(2) syscalls for the process", .flags = PROCESS_FLAG_IO, },
   [RBYTES] = { .name = "RBYTES", .title = "  IO_RBYTES ", .description = "Bytes of read(2) I/O for the process", .flags = PROCESS_FLAG_IO, },
   [WBYTES] = { .name = "WBYTES", .title = "  IO_WBYTES ", .description = "Bytes of write(2) I/O for the process", .flags = PROCESS_FLAG_IO, },
   [CNCLWB] = { .name = "CNCLWB", .title = "  IO_CANCEL ", .description = "Bytes of cancelled write(2) I/O", .flags = PROCESS_FLAG_IO, },
   [IO_READ_RATE] = { .name = "IO_READ_RATE", .title = "  DISK READ ", .description = "The I/O rate of read(2) in bytes per second for the process", .flags = PROCESS_FLAG_IO, },
   [IO_WRITE_RATE] = { .name = "IO_WRITE_RATE", .title = " DISK WRITE ", .description = "The I/O rate of write(2) in bytes per second for the process", .flags = PROCESS_FLAG_IO, },
   [IO_RATE] = { .name = "IO_RATE", .title = "   DISK R/W ", .description = "Total I/O rate in bytes per second", .flags = PROCESS_FLAG_IO, },
   [CGROUP] = { .name = "CGROUP", .title = "    CGROUP ", .description = "Which cgroup the process is in", .flags = PROCESS_FLAG_LINUX_CGROUP, },
   [OOM] = { .name = "OOM", .title = " OOM ", .description = "OOM (Out-of-Memory) killer score", .flags = PROCESS_FLAG_LINUX_OOM, },
   [PERCENT_CPU_DELAY] = { .name = "PERCENT_CPU_DELAY", .title = "CPUD% ", .description = "CPU delay %", .flags = 0, },
   [PERCENT_IO_DELAY] = { .name = "PERCENT_IO_DELAY", .title = "IOD% ", .description = "Block I/O delay %", .flags = 0, },
   [PERCENT_SWAP_DELAY] = { .name = "PERCENT_SWAP_DELAY", .title = "SWAPD% ", .description = "Swapin delay %", .flags = 0, },
   [M_PSS] = { .name = "M_PSS", .title = "  PSS ", .description = "proportional set size, same as M_RESIDENT but each page is divided by the number of processes sharing it.", .flags = PROCESS_FLAG_LINUX_SMAPS, },
   [M_SWAP] = { .name = "M_SWAP", .title = " SWAP ", .description = "Size of the process's swapped pages", .flags = PROCESS_FLAG_LINUX_SMAPS, },
   [M_PSSWP] = { .name = "M_PSSWP", .title = " PSSWP ", .description = "shows proportional swap share of this mapping, Unlike \"Swap\", this does not take into account swapped out page of underlying shmem objects.", .flags = PROCESS_FLAG_LINUX_SMAPS, },
   [CTXT] = { .name = "CTXT", .title = " CTXT ", .description = "Context switches (incremental sum of voluntary_ctxt_switches and nonvoluntary_ctxt_switches)", .flags = PROCESS_FLAG_LINUX_CTXT, },
   [SECATTR] = { .name = "SECATTR", .title = " Security Attribute ", .description = "Security attribute of the process (e.g. SELinux or AppArmor)", .flags = PROCESS_FLAG_LINUX_SECATTR, },
   [LAST_PROCESSFIELD] = { .name = "*** report bug! ***", .title = NULL, .description = NULL, .flags = 0, },
};

ProcessPidColumn Process_pidColumns[] = {
   { .id = PID, .label = "PID" },
   { .id = PPID, .label = "PPID" },
   { .id = TPGID, .label = "TPGID" },
   { .id = TGID, .label = "TGID" },
   { .id = PGRP, .label = "PGRP" },
   { .id = SESSION, .label = "SID" },
   { .id = 0, .label = NULL },
};

Process* PCPProcess_new(const Settings* settings) {
   PCPProcess* this = xCalloc(1, sizeof(PCPProcess));
   Object_setClass(this, Class(PCPProcess));
   Process_init(&this->super, settings);
   return &this->super;
}

void Process_delete(Object* cast) {
   PCPProcess* this = (PCPProcess*) cast;
   Process_done((Process*)cast);
   free(this->cgroup);
   free(this->secattr);
   free(this->ttyDevice);
   free(this);
}

void PCPProcess_printDelay(float delay_percent, char* buffer, int n) {
   if (isnan(delay_percent)) {
      xSnprintf(buffer, n, " N/A  ");
   } else {
      xSnprintf(buffer, n, "%4.1f  ", delay_percent);
   }
}

static void PCPProcess_writeField(const Process* this, RichString* str, ProcessField field) {
   const PCPProcess* lp = (const PCPProcess*) this;
   bool coloring = this->settings->highlightMegabytes;
   char buffer[256]; buffer[255] = '\0';
   int attr = CRT_colors[DEFAULT_COLOR];
   int n = sizeof(buffer) - 1;
   switch ((int)field) {
   case TTY_NR: {
      if (lp->ttyDevice) {
         xSnprintf(buffer, n, "%-9s", lp->ttyDevice + 5 /* skip "/dev/" */);
      } else {
         attr = CRT_colors[PROCESS_SHADOW];
         xSnprintf(buffer, n, "?        ");
      }
      break;
   }
   case CMINFLT: Process_colorNumber(str, lp->cminflt, coloring); return;
   case CMAJFLT: Process_colorNumber(str, lp->cmajflt, coloring); return;
   case M_DRS: Process_humanNumber(str, lp->m_drs, coloring); return;
   case M_DT: Process_humanNumber(str, lp->m_dt, coloring); return;
   case M_LRS: Process_humanNumber(str, lp->m_lrs, coloring); return;
   case M_TRS: Process_humanNumber(str, lp->m_trs, coloring); return;
   case M_SHARE: Process_humanNumber(str, lp->m_share, coloring); return;
   case M_PSS: Process_humanNumber(str, lp->m_pss, coloring); return;
   case M_SWAP: Process_humanNumber(str, lp->m_swap, coloring); return;
   case M_PSSWP: Process_humanNumber(str, lp->m_psswp, coloring); return;
   case UTIME: Process_printTime(str, lp->utime); return;
   case STIME: Process_printTime(str, lp->stime); return;
   case CUTIME: Process_printTime(str, lp->cutime); return;
   case CSTIME: Process_printTime(str, lp->cstime); return;
   case RCHAR:  Process_colorNumber(str, lp->io_rchar, coloring); return;
   case WCHAR:  Process_colorNumber(str, lp->io_wchar, coloring); return;
   case SYSCR:  Process_colorNumber(str, lp->io_syscr, coloring); return;
   case SYSCW:  Process_colorNumber(str, lp->io_syscw, coloring); return;
   case RBYTES: Process_colorNumber(str, lp->io_read_bytes, coloring); return;
   case WBYTES: Process_colorNumber(str, lp->io_write_bytes, coloring); return;
   case CNCLWB: Process_colorNumber(str, lp->io_cancelled_write_bytes, coloring); return;
   case IO_READ_RATE:  Process_outputRate(str, buffer, n, lp->io_rate_read_bps, coloring); return;
   case IO_WRITE_RATE: Process_outputRate(str, buffer, n, lp->io_rate_write_bps, coloring); return;
   case IO_RATE: {
      double totalRate = NAN;
      if (!isnan(lp->io_rate_read_bps) && !isnan(lp->io_rate_write_bps))
         totalRate = lp->io_rate_read_bps + lp->io_rate_write_bps;
      else if (!isnan(lp->io_rate_read_bps))
         totalRate = lp->io_rate_read_bps;
      else if (!isnan(lp->io_rate_write_bps))
         totalRate = lp->io_rate_write_bps;
      else
         totalRate = NAN;
      Process_outputRate(str, buffer, n, totalRate, coloring); return;
   }
   case CGROUP: xSnprintf(buffer, n, "%-10s ", lp->cgroup ? lp->cgroup : ""); break;
   case OOM: xSnprintf(buffer, n, "%4u ", lp->oom); break;
   case PERCENT_CPU_DELAY:
      PCPProcess_printDelay(lp->cpu_delay_percent, buffer, n);
      break;
   case PERCENT_IO_DELAY:
      PCPProcess_printDelay(lp->blkio_delay_percent, buffer, n);
      break;
   case PERCENT_SWAP_DELAY:
      PCPProcess_printDelay(lp->swapin_delay_percent, buffer, n);
      break;
   case CTXT:
      if (lp->ctxt_diff > 1000) {
         attr |= A_BOLD;
      }
      xSnprintf(buffer, n, "%5lu ", lp->ctxt_diff);
      break;
   case SECATTR: snprintf(buffer, n, "%-30s   ", lp->secattr ? lp->secattr : "?"); break;
   default:
      Process_writeField(this, str, field);
      return;
   }
   RichString_appendWide(str, attr, buffer);
}

static long PCPProcess_compare(const void* v1, const void* v2) {
   const PCPProcess *p1, *p2;
   const Settings *settings = ((const Process*)v1)->settings;

   if (settings->direction == 1) {
      p1 = (const PCPProcess*)v1;
      p2 = (const PCPProcess*)v2;
   } else {
      p2 = (const PCPProcess*)v1;
      p1 = (const PCPProcess*)v2;
   }

   switch ((int)settings->sortKey) {
   case M_DRS:
      return SPACESHIP_NUMBER(p2->m_drs, p1->m_drs);
   case M_DT:
      return SPACESHIP_NUMBER(p2->m_dt, p1->m_dt);
   case M_LRS:
      return SPACESHIP_NUMBER(p2->m_lrs, p1->m_lrs);
   case M_TRS:
      return SPACESHIP_NUMBER(p2->m_trs, p1->m_trs);
   case M_SHARE:
      return SPACESHIP_NUMBER(p2->m_share, p1->m_share);
   case M_PSS:
      return SPACESHIP_NUMBER(p2->m_pss, p1->m_pss);
   case M_SWAP:
      return SPACESHIP_NUMBER(p2->m_swap, p1->m_swap);
   case M_PSSWP:
      return SPACESHIP_NUMBER(p2->m_psswp, p1->m_psswp);
   case UTIME:
      return SPACESHIP_NUMBER(p2->utime, p1->utime);
   case CUTIME:
      return SPACESHIP_NUMBER(p2->cutime, p1->cutime);
   case STIME:
      return SPACESHIP_NUMBER(p2->stime, p1->stime);
   case CSTIME:
      return SPACESHIP_NUMBER(p2->cstime, p1->cstime);
   case RCHAR:
      return SPACESHIP_NUMBER(p2->io_rchar, p1->io_rchar);
   case WCHAR:
      return SPACESHIP_NUMBER(p2->io_wchar, p1->io_wchar);
   case SYSCR:
      return SPACESHIP_NUMBER(p2->io_syscr, p1->io_syscr);
   case SYSCW:
      return SPACESHIP_NUMBER(p2->io_syscw, p1->io_syscw);
   case RBYTES:
      return SPACESHIP_NUMBER(p2->io_read_bytes, p1->io_read_bytes);
   case WBYTES:
      return SPACESHIP_NUMBER(p2->io_write_bytes, p1->io_write_bytes);
   case CNCLWB:
      return SPACESHIP_NUMBER(p2->io_cancelled_write_bytes, p1->io_cancelled_write_bytes);
   case IO_READ_RATE:
      return SPACESHIP_NUMBER(p2->io_rate_read_bps, p1->io_rate_read_bps);
   case IO_WRITE_RATE:
      return SPACESHIP_NUMBER(p2->io_rate_write_bps, p1->io_rate_write_bps);
   case IO_RATE:
      return SPACESHIP_NUMBER(p2->io_rate_read_bps + p2->io_rate_write_bps, p1->io_rate_read_bps + p1->io_rate_write_bps);
   case CGROUP:
      return SPACESHIP_NULLSTR(p1->cgroup, p2->cgroup);
   case OOM:
      return SPACESHIP_NUMBER(p2->oom, p1->oom);
   case PERCENT_CPU_DELAY:
      return SPACESHIP_NUMBER(p2->cpu_delay_percent, p1->cpu_delay_percent);
   case PERCENT_IO_DELAY:
      return SPACESHIP_NUMBER(p2->blkio_delay_percent, p1->blkio_delay_percent);
   case PERCENT_SWAP_DELAY:
      return SPACESHIP_NUMBER(p2->swapin_delay_percent, p1->swapin_delay_percent);
   case CTXT:
      return SPACESHIP_NUMBER(p2->ctxt_diff, p1->ctxt_diff);
   case SECATTR:
      return SPACESHIP_NULLSTR(p1->secattr, p2->secattr);
   default:
      return Process_compare(v1, v2);
   }
}

bool Process_isThread(const Process* this) {
   return (Process_isUserlandThread(this) || Process_isKernelThread(this));
}

const ProcessClass PCPProcess_class = {
   .super = {
      .extends = Class(Process),
      .display = Process_display,
      .delete = Process_delete,
      .compare = PCPProcess_compare
   },
   .writeField = PCPProcess_writeField
};

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <mips/trapframe.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}

int sys_fork(struct trapframe* tf, pid_t* retval) {
	/* we should definitely NOT be a new process */
	KASSERT(curproc_getas() != NULL);

	struct proc* child_proc;
	struct trapframe* child_tf;
	int result;

	/* create child process */
	child_proc = proc_create_runprogram("child process");

	if (child_proc == NULL) {
		/* not enough memory for new process */
		return ENOMEM;
	}

	/* copy current process's address space */
	result = as_copy(curproc_getas(), &child_proc->p_addrspace);
	if (result) {
		proc_destroy(child_proc);
		return result;
	}

	/* copy trapframe and modify it */
	child_tf = kmalloc(sizeof(struct trapframe));
	if (child_tf == NULL) {
		proc_destroy(child_proc);
		return ENOMEM;
	}

	*child_tf = *tf;
	child_tf->tf_v0 = 0;	/* set return code to 0 */
	child_tf->tf_epc += 4;	/* prevent syscall from repeating */

	/* heeeere we go */
	thread_fork("child thread", child_proc, (void*)enter_forked_process,
		(void*)child_tf, 0);

	/* successful exit */
	*retval = child_proc->p_pid;
	return 0;
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  *retval = curproc->p_pid;
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  if (options != 0) {
	/* invalid options */
    return(EINVAL);
  }

  if (proc_table[pid] == NULL) {
	/* no such process */
	return(ESRCH);
  }

  /* wait for exit */
  if (!proc_table[pid]->exited) {
	lock_aquire(proc_table[pid]->p_exit_lock);
	cv_wait(proc_table[pid]->p_exit_cv);
	lock_release(proc_table[pid]->p_exit_lock);
  }
  
  /* return exit status */
  exitstatus = proc_table[pid]->exit_code;
  result = copyout((void *)&proc_table[pid]->exit_code, status, sizeof(int));
  if (result) {
    return(result);
  }

  /* done */
  *retval = pid;
  return(0);
}


// SPDX-License-Identifier: GPL-2.0-only
/*
 * NOTE: This example is works on x86 and powerpc.
 * Here's a sample kernel module showing the use of kprobes to dump a
 * stack trace and selected registers when _do_fork() is called.
 *
 * For more information on theory of operation of kprobes, see
 * Documentation/kprobes.txt
 *
 * You will see the trace data in /var/log/messages and on the console
 * whenever _do_fork() is invoked to create a new process.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/memory.h>
#include <linux/proc_fs.h>

#define MAX_SYMBOL_LEN	64
static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
//static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

int pid_param = 0;
module_param(pid_param, int, S_IRUGO);
MODULE_PARM_DESC(pid_param, "Process ID of an active process");

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
	.symbol_name	= symbol,
};

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	if(current->pid == pid_param)
		printk(KERN_ALERT "0x%lx", regs->si);
	return 0;
}

static int __init kprobe_init(void)
{
	int ret;
	kp.pre_handler = handler_pre;
	//kp.post_handler = handler_post;
	//kp.fault_handler = handler_fault;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("register_kprobe failed, returned %d\n", ret);
		return ret;
	}

	pr_info("Planted kprobe at %p\n", kp.addr);
	printk(KERN_ALERT "Planted kprobe at %p, pid_param=%d\n", kp.addr, pid_param);
	return 0;
}

static void __exit kprobe_exit(void)
{
	unregister_kprobe(&kp);
	pr_info("kprobe at %p unregistered\n", kp.addr);
	printk(KERN_ALERT "kprobe at %p unregestered\n", kp.addr);
}


module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");

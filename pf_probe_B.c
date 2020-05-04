#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/memory.h>

#define MAX_SYMBOL_LEN	64
#define DATA_SIZE 1024
#define COLUMNS 70
#define ROWS 30

void clear_scatterplot(void);
void populate_scatterplot(void);
void print_scatterplot(void);

int pid_param = 0;
static int counter = 0;
static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
static struct pf_data {
	long int addr;
	struct timespec time;
} data[DATA_SIZE];
char scatterplot[ROWS][COLUMNS+1];
int data_count = 0;

module_param_string(symbol, symbol, sizeof(symbol), 0645);
module_param(pid_param, int, S_IRUGO);
MODULE_PARM_DESC(pid_param, "Process ID of an active process");

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
	.symbol_name	= symbol,
};

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	if(current->pid == pid_param) {
		/* assign address at regs->si to struct data->addr */
                if(counter < DATA_SIZE) {
                        data[counter].addr = regs->si;

                        /* capture current time into struct data->time */
                        getnstimeofday(&data[counter].time);

                        ++counter;
                }

	}
	return 0;
}

static int __init kprobe_init(void)
{
	int ret;
	kp.pre_handler = handler_pre;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	return 0;
}

static void __exit kprobe_exit(void)
{
	unregister_kprobe(&kp);
        populate_scatterplot();
        print_scatterplot();
}

void populate_scatterplot(void)
{
        int i, time_idx, addr_idx;
        long elapsed_time, current_time;
        
        clear_scatterplot();

        data_count = counter >= DATA_SIZE ? DATA_SIZE : counter; 
        elapsed_time = data[data_count-1].time.tv_nsec - data[0].time.tv_nsec;
    
        for(i = 0; i < data_count; ++i) {
                current_time = data[i].time.tv_nsec - data[0].time.tv_nsec;
                time_idx = (100 * current_time / elapsed_time) * (COLUMNS-1) / 100; 
                addr_idx = (100 * (data[i].addr >> 32) / 0xffff) * ROWS / 100;
                scatterplot[addr_idx][time_idx] = '*';
        }
        for(i = 0; i < ROWS; ++i) {
            scatterplot[i][COLUMNS-1] = '\0';
        }
}

void print_scatterplot(void)
{
        int i, addr_label_idx;
        char lmargin[8] = "      ";
        char lmargin_addr_label[8] = " addr ";

	printk(KERN_ALERT "\n\n                                              Page Faults                            \n");
	printk(KERN_ALERT "                                              -----------                            \n");

	for(i = 0; i < ROWS; ++i) { 
                addr_label_idx = (100 * (data[i].addr>>32) / 0xffff) * (data_count-1) / 100;
                printk(KERN_ALERT "%s| %s\n", lmargin, scatterplot[i]);
        }

	printk(KERN_ALERT "%s|_______________________________________________________________________\n", lmargin_addr_label);
	printk(KERN_ALERT "\n          time                                 \n\n");

}

void clear_scatterplot(void)
{
	int i, j;
	for(i = 0; i < ROWS; ++i) { 
		for(j = 0; j < COLUMNS; ++j)
                        scatterplot[i][j] = ' ';
	        scatterplot[i][COLUMNS] = '\0';
	}
}



module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");

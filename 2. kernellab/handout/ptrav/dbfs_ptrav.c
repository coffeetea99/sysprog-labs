//------------------------------------------------------------------------------
/// @brief Kernel Lab: Page Table Walk (dbfs_ptrav.c)
///
/// @section license_section License
/// Copyright (c) 2018-2019, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>

MODULE_LICENSE("Dual BSD/GPL");

struct packet {
        pid_t pid;
        unsigned long pages_1g, pages_2m, pages_4k;
};

static struct dentry *dir, *output;
static struct task_struct *task;

int filevalue;

static ssize_t read_output(struct file *fp,
                        char __user *user_buffer,
                        size_t length,
                        loff_t *position)
{
        // Implement read file operations
        // Fill in the arguments below
	struct packet pckt;
	copy_from_user(&pckt, user_buffer, length);

	task = pid_task(find_vpid(pckt.pid), PIDTYPE_PID);
	pgd_t* pgd_start = (task -> mm) ->pgd;

	unsigned long _1g = 0, _2m = 0, _4k = 0;

	int i;
	for ( i = 0 ; i < 256 ; ++i ) {
		pgd_t pgd_entry = *(pgd_start + i);
		if ( pgd_val(pgd_entry) & 0x1 == 1 ) {
			
			pud_t* pud_start = (pud_t*)pgd_page_vaddr(pgd_entry);
			int j;
			for ( j = 0 ; j < 512 ; ++j ) {
				pud_t pud_entry = *(pud_start + j);
				if ( pud_val(pud_entry) & 0x1 == 1 ) {
					if ( ( pud_val(pud_entry) >> 7) & 0x1 == 1 ) ++_1g;
					else {

						pmd_t* pmd_start = (pmd_t*)pud_page_vaddr(pud_entry);
						int k;
						for ( k = 0 ; k < 512 ; ++k ) {
							pmd_t pmd_entry = *(pmd_start + k);
							if ( pmd_val(pmd_entry) & 0x1 == 1 ) {
								if ( ( pmd_val(pmd_entry) >> 7) & 0x1 == 1 ) ++_2m;
								else {
									
									pte_t* pte_start = (pte_t*)pmd_page_vaddr(pmd_entry);
									int l;
									for ( l = 0 ; l < 512 ; ++l ) {
										pte_t pte_entry = *(pte_start + l);
										if ( pte_val(pte_entry) & 0x1 == 1 ) ++_4k;
									}

								}

							}
						}
					}

				}
			}

		}
	}

	pckt.pages_1g = _1g;
	pckt.pages_2m = _2m;
	pckt.pages_4k = _4k;

	// Fill in the arguments below
	copy_to_user(user_buffer, &pckt, length);

	return length;
}

static const struct file_operations dbfs_fops = {
	// Mapping file operations with your functions
	.read = read_output,
};

static int __init dbfs_module_init(void)
{
        // Implement init module
        printk("Init Module\n");

        dir = debugfs_create_dir("ptrav", NULL);

        if (!dir) {
                printk("Cannot create paddr dir\n");
                return -1;
        }

        // Fill in the arguments below
        output = debugfs_create_file("output", S_IRWXU, dir, &filevalue, &dbfs_fops);

        return 0;
}

static void __exit dbfs_module_exit(void)
{
  // Implement exit module
	debugfs_remove_recursive(dir);
}

module_init(dbfs_module_init);
module_exit(dbfs_module_exit);

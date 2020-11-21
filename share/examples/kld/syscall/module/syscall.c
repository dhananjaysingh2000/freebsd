/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 1999 Assar Westerlund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysproto.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/vmem.h>
#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>

/*
 * The function for implementing the syscall.
 */
static int
hello(struct thread *td, void *arg)
{
	int rv;
  	vmem_size_t size = 64*1024; // 64 KB
	vmem_size_t alignment = 64*1024; // 64KB Super Page
	vmem_addr_t addrp; // The address where the memeory is allocated
	vm_page_t m;
	int pflags = VM_ALLOC_NORMAL | VM_ALLOC_NOOBJ;

	rv = vmem_xalloc(kernel_arena, size, alignment, 0, 0, VMEM_ADDR_MIN, VMEM_ADDR_MAX, M_WAITOK | M_BESTFIT, &addrp);
	
	if (rv != 0) {
		printf("virtual memory allocation failed../");
		return 1;
	}

	m = vm_page_alloc_contig(NULL, 0, pflags, size/4096, 0, ~(vm_paddr_t) 0, alignment, 0, VM_MEMATTR_DEFAULT);

	if (m == NULL) {
		printf("page allocation failed...");
		return 1;
	}	

	pmap_kenter(addrp, size, VM_PAGE_TO_PHYS(m), VM_MEMATTR_DEFAULT);
	printf("hello kernel! Testing from Dhananjay. ADDRESS = %lx\n", addrp);
#if 0
	pmap_kremove(addrp);
	printf("pmap_kremove() called\n");

	for (int i = 0; i < size/4096; i++) {	
		vm_page_free(m + i);
	}
	printf("vm_page_free() called\n");

	// TODO: VMEM_XFREE CALL
	vmem_xfree(kernel_arena, addrp, size);
	printf("vmem_xfree() called\n");
#endif

	return (0);
}

/*
 * The `sysent' for the new syscall
 */
static struct sysent hello_sysent = {
	hello			/* sy_call */
};

/*
 * The offset in sysent where the syscall is allocated.
 */
static int offset = NO_SYSCALL;

/*
 * The function called at load/unload.
 */
static int
load(struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch (cmd) {
	case MOD_LOAD :
		printf("syscall loaded at %d\n", offset);
		break;
	case MOD_UNLOAD :
		printf("syscall unloaded from %d\n", offset);
		break;
	default :
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

SYSCALL_MODULE(syscall, &offset, &hello_sysent, load, NULL);

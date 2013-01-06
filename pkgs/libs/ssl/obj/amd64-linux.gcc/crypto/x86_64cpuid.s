.text

.globl	OPENSSL_atomic_add
.type	OPENSSL_atomic_add,@function
.align	16
OPENSSL_atomic_add:
	movl	(%rdi),%eax
.Lspin:	leaq	(%rsi,%rax),%r8
lock;	cmpxchgl	%r8d,(%rdi)
	jne	.Lspin
	movl	%r8d,%eax
	.byte	0x48,0x98
	ret
.size	OPENSSL_atomic_add,.-OPENSSL_atomic_add

.globl	OPENSSL_wipe_cpu
.type	OPENSSL_wipe_cpu,@function
.align	16
OPENSSL_wipe_cpu:
	pxor	%xmm0,%xmm0
	pxor	%xmm1,%xmm1
	pxor	%xmm2,%xmm2
	pxor	%xmm3,%xmm3
	pxor	%xmm4,%xmm4
	pxor	%xmm5,%xmm5
	pxor	%xmm6,%xmm6
	pxor	%xmm7,%xmm7
	pxor	%xmm8,%xmm8
	pxor	%xmm9,%xmm9
	pxor	%xmm10,%xmm10
	pxor	%xmm11,%xmm11
	pxor	%xmm12,%xmm12
	pxor	%xmm13,%xmm13
	pxor	%xmm14,%xmm14
	pxor	%xmm15,%xmm15
	xorq	%rcx,%rcx
	xorq	%rdx,%rdx
	xorq	%rsi,%rsi
	xorq	%rdi,%rdi
	xorq	%r8,%r8
	xorq	%r9,%r9
	xorq	%r10,%r10
	xorq	%r11,%r11
	leaq	8(%rsp),%rax
	ret
.size	OPENSSL_wipe_cpu,.-OPENSSL_wipe_cpu

.section	.init
	call	OPENSSL_cpuid_setup

.text

.globl	OPENSSL_rdtsc
.type	OPENSSL_rdtsc,@function
.align	16
OPENSSL_rdtsc:
	rdtsc
	shlq	$32,%rdx
	orq	%rdx,%rax
	.byte	0xf3,0xc3
.size	OPENSSL_rdtsc,.-OPENSSL_rdtsc

.globl	OPENSSL_ia32_cpuid
.type	OPENSSL_ia32_cpuid,@function
.align	16
OPENSSL_ia32_cpuid:
	movq	%rbx,%r8

	xorl	%eax,%eax
	cpuid
	xorl	%eax,%eax
	cmpl	$1970169159,%ebx
	setne	%al
	movl	%eax,%r9d
	cmpl	$1231384169,%edx
	setne	%al
	orl	%eax,%r9d
	cmpl	$1818588270,%ecx
	setne	%al
	orl	%eax,%r9d

	movl	$1,%eax
	cpuid
	cmpl	$0,%r9d
	jne	.Lnotintel
	orl	$1048576,%edx
	andb	$15,%ah
	cmpb	$15,%ah
	je	.Lnotintel
	orl	$1073741824,%edx
.Lnotintel:
	btl	$28,%edx
	jnc	.Ldone
	shrl	$16,%ebx
	cmpb	$1,%bl
	ja	.Ldone
	andl	$4026531839,%edx
.Ldone:
	shlq	$32,%rcx
	movl	%edx,%eax
	movq	%r8,%rbx
	orq	%rcx,%rax
	.byte	0xf3,0xc3
.size	OPENSSL_ia32_cpuid,.-OPENSSL_ia32_cpuid

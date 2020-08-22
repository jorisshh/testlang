	.text
	.def	 @feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 0
	.file	"potatoscript"
	.def	 print;
	.scl	2;
	.type	32;
	.endef
	.globl	print                           # -- Begin function print
	.p2align	4, 0x90
print:                                  # @print
# %bb.0:                                # %entry
	retq
                                        # -- End function
	.def	 main;
	.scl	2;
	.type	32;
	.endef
	.globl	__real@3f800000                 # -- Begin function main
	.section	.rdata,"dr",discard,__real@3f800000
	.p2align	2
__real@3f800000:
	.long	0x3f800000                      # float 1
	.text
	.globl	main
	.p2align	4, 0x90
main:                                   # @main
.seh_proc main
# %bb.0:                                # %entry
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movss	__real@3f800000(%rip), %xmm0    # xmm0 = mem[0],zero,zero,zero
	callq	print
	nop
	addq	$40, %rsp
	retq
	.seh_handlerdata
	.text
	.seh_endproc
                                        # -- End function
	.globl	_fltused

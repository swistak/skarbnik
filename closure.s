	.file	"closure.c"
	.section	.rodata
.LC0:
	.string	"ProtoObject#print: %s\n"
	.text
.globl print
	.type	print, @function
print:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	$.LC0, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	printf
	leave
	ret
	.size	print, .-print
.globl rename_object
	.type	rename_object, @function
rename_object:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	free
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movl	%edx, (%eax)
	leave
	ret
	.size	rename_object, .-rename_object
	.section	.rodata
.LC1:
	.string	"TestName"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	movl	$12, (%esp)
	call	malloc
	movl	%eax, 28(%esp)
	movl	28(%esp), %eax
	movl	$print, 4(%eax)
	movl	28(%esp), %eax
	movl	$0, 8(%eax)
	movl	28(%esp), %eax
	movl	$.LC1, (%eax)
	movl	28(%esp), %eax
	movl	4(%eax), %edx
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	*%edx
	movl	$0, %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3"
	.section	.note.GNU-stack,"",@progbits

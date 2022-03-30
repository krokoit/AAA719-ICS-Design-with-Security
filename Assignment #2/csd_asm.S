// ------------------------------------------
//  Author: Yeonseok Jang
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   March 25, 2022
// ------------------------------------------

.extern csd_main

.global main
main:
	ldr r0, =Input_data
	ldr r1, =Output_data
	mov r2, #4
	bl copy					// copy(&Input_data, &Output_data, 4)

	ldr r0, =Output_data
	mov r1, #32
	bl sort					// sort(&Output_data, 32)

	bl csd_main				// csd_main()

	b forever

/* copy(&src, &dst, len): copy 8*`len` words from `src` to `dst`
 - src (r0): source address
 - dst (r1): destination address
 - len (r2): how many 8 words to be copied
*/
copy:
	ldmia r0!, {r3-r10}		// LOAD 8 words from `src`
	stmia r1!, {r3-r10}		// STORE 8 words to `dst`
 	subs r2, r2, #1			// DECR `len` (8 words copied)

 	bne copy				// IF 8*`len` words copied, then DONE
 	mov pc, lr

/* sort(&list, len): sort `len` elements of list in ascending order (bubble sort)
 - list(r0): address of list to be sorted
 - len (r1): # of elements to be sorted
*/
sort:
	mov r2, r0				// COPY `list` address
	eor r3, r3, r3			// SET inner loop counter j = 0
	subs r1, r1, #1			// for (i = n - 1; i > 0; i--)
	bne sort_inner			// 	sort_inner(&list, i, j)
	mov pc, lr

/* sort_inner(outer_ctr, &list, inner_ctr): put largest element in list[0..i] at &list[i]
 - i   (r1): outer loop counter
 - list(r2): list address
 - j   (r3): inner loop counter
*/
sort_inner:
	ldr r4, [r2]				// LOAD list[j]
	ldr	r5, [r2, #4]			// LOAD list[j+1]
	cmp r4, r5					// COMPARE list[j], list[j+1]
		strgt r4, [r2, #4]		// IF list[j] > list[j+1], then STORE list[j]'s value to list[j+1]
		strgt r5, [r2]			// IF list[j] > list[j+1], then STORE list[j+1]'s value to list[j]
	add r2, r2, #4				// INCR `list` pointer by 4 (pointing to next element)
	add r3, r3, #1				// INCR inner loop counter `j`
	cmp r3, r1					// CONTINUE while `j` < `i`
	bne sort_inner
	b sort

forever:
	nop
 	b forever

.data
.align 4
Input_data:
 	.word 2, 0, -7, -1, 3, 8, -4, 10
	.word -9, -16, 15, 13, 1, 4, -3, 14
	.word -8, -10, -15, 6, -13, -5, 9, 12
	.word -11, -14, -6, 11, 5, 7, -2, -12
Output_data:
 	.word 0, 0, 0, 0, 0, 0, 0, 0
	.word 0, 0, 0, 0, 0, 0, 0, 0
	.word 0, 0, 0, 0, 0, 0, 0, 0
	.word 0, 0, 0, 0, 0, 0, 0, 0

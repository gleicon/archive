/* bgr2rgb -> flips the frame as fast as possible (1 bgr field @ time) */
/* Gleicon S. Moraes (gleicon@terra.com.br) */
/* __asm__() fix  Marco Aurelio Casaroli (nada@onda.com.br) */

/*  

	usage:  bgr2rgb(unsigned char *buffer, unsigned int size);
	compiler keys: NONE m0f0
	

*/


#define bgr2rgb(a, b) __bgr2rgb_16(a, b) 

void __bgr2rgb_16 (unsigned char *b, unsigned int tam) {

	
	__asm__	__volatile__ (" 
		xorl %%ebx, %%ebx 
	label:
		movb (%1, %%ebx), %%al 
		movb 2(%1, %%ebx), %%ah 
		movb %%al, 2(%0, %%ebx) 
		movb %%ah,(%0, %%ebx)
		addl $3, %%ebx
		decl %2
		jnz label"
		: "=r"(b)  		
		: "r" (b), "r" (tam) 
		: "al", "ah", "ebx"); /* modify */
}



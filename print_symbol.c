/* 
 *
 * print_symbol: 
 * if the char isn't 0-9 A-Z a-z, print the encoded form ( %FF - FF == char code )
 */

print_symbol (char *wot) {
	
	while (*wot!=0) {
		if ((*wot >='A') || (*wot <= 'Z') || (*wot >= 'a') || (*wot <= 'z') || ( *wot >='0') || ( *wot <='9')) {
			printf ("%c", *wot) 
			}
		else printf ("%%%x",*wot);
		}
	wot++;
	}



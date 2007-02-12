#include <stdio.h>

int main(int argc, char **argv) {
  printf("__GNUC__=%d, __GNUC_MINOR__=%d\n",
#ifdef __GNUC__
	 __GNUC__, __GNUC_MINOR__
#else
	 0,0
#endif /*__GNUC__*/
	 );

#if __GNUC__ >= 3
# if __GNUC_MINOR__ == 0
  printf("+ (old) detected gcc v3.0\n");
# else /* !( __GNUC_MINOR__ == 0 ) */
  printf("+ (old) detected gcc >v3.0\n");
# endif /* __GNUC_MINOR__ == 0 */
#endif /* __GNUC__ >= 3 */


#if __GNUC__ >= 3
# if __GNUC__ == 3 && __GNUC_MINOR__ == 0
  printf("+ (new) detected gcc v3.0\n");
# else /* !( __GNUC__ == 3 && __GNUC_MINOR__ == 0 ) */
  printf("+ (new) detected gcc >v3.0\n");
# endif /*  __GNUC__ == 3 && __GNUC_MINOR__ == 0 */
#endif /* __GNUC__ >= 3 */

  return  0;
}

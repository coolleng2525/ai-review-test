/*
 * to test 64 bit issues
 *
 * write all known 64bit issues here, to check how many issues could be found out by tools.
 * ref https://jira-wiki.ruckuswireless.com/display/ZFP/64bit-migration
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


//4.1). Interchange between "int" and "pointer"
//Casting pointer to int, would lead to assignment truncation in 64-bit platform.
//Example1
short * pointer_to_int_assignment_truncation_1_01(short *ptr)
{
    int n = (int) ptr + 2; //Casting pointer to int, would lead to assignment truncation in 64-bit platform.
    ptr = (short *) n;
    return ptr;
}

//Example2:
#define PAGEOFFSET 0xffff8000UL
//#define PAGEOFFSET 0xffff800000000000UL
char * pointer_to_int_assignment_truncation_2_02(char *p)
{
    p = (char *) ((int)p & PAGEOFFSET);//Casting pointer to int, would lead to assignment truncation in 64-bit platform.
    return p;
}

//solution:
//using intptr_t/uintptr_t instead of int.



//4.2). Interchange between "int" and "long"
//4.2.1). Assignment Truncation
//long -> int
//examples:
static long retlong(long n)
{
    return n+1;
}

void long_to_int_assignment_truncation_03(long long1, long long2, long long3)
{
    int int1, int2=2, int3;

    int1 = long1; //direct assignment
    int2 = int2 * long2; //expression
    int3 = retlong(long3); //argument pass, function return.
    printf("int1:%d, int2:%d, int3:%d\n", int1, int2, int3);
}

//Solution: avoid such case.


//4.2.2). Loss of sign
//unsigned + signed -> unsigned
//int + long -> long
//If the first conversion (loss of sign) is carried out before the second (promotion to long) then the result may be incorrect when assigned to a signed long.
//
//Example:
void int_to_long_Loss_of_sign_04(void)
{
    long a;
    int b;
    unsigned int c;

    b = -2;
    c = 1U;
    a = b + c; //a is 0x00000000FFFFFFFF instead of -1, because b is converted to unsigned int firstly.
    printf("a:%ld, b:%d, c:%u\n", a, b, c);
}

//Solution: convert int to long explicitly to avoid loss of sign:
//a = (long) b + c; //a is -1L now.


//4.2.3). sign extension
//int -> long
//example:
struct my_bitint_foo {
    unsigned int bitint1:19, bitint2:13;
};
unsigned long int_to_long_sign_extension_05(void)
{
    unsigned long ulong1, ulong2;
    struct my_bitint_foo bitint;
    unsigned short usint1 = 0x4000U;
    ulong1 = usint1 << 17; //usint1 is converted to int, the expression result is int, then converted to long, sign extension occurs here, then converted to unsigned long. The final result is: 0xffffffff80000000.
    bitint.bitint1 = 0x40000U;
    ulong2 = bitint.bitint1 << 13; //bitint1 is converted to int, the expression result is int, then converted to long, sign extension occurs here, then converted to unsigned long. The final result is: 0xffffffff80000000.
    printf("ulong1:%0lx, ulong2:%0lx\n", ulong1, ulong2);
    return ulong1;
}

//Solution:
//convert "unsigned int" to "unsigned long" explicitly to avoid sign extension:
//ulong1 = (unsigned long) uint1 << 13; // the final result is: 0x80000000


//4.2.4). Overflow of int
//int -> long
//example:
long int_to_long_overflow_of_int_06(int int1, int int2)
{
    long long1, long2;
    long1 = int1 * int2; // 32bit multiply
    long2 = (long)(int1 * int2); // 32bit multiply
    printf("long1:%ld, long2:%ld\n", long1, long2);
    return long1;
}

//Solution: use a cast.:
//long1 = (long)int1 * int2; // 64bit multiply
//long1 = int1 * (long)int2; // 64bit multiply



//4.3). constant
//Integer constants (unless modified by a suffix, e.g. 0x8L) are treated as the smallest size that can hold the value.
//Numbers written in hexadecimal may be treated by the compiler as int, long, or long long types, and may be either signed or unsigned types.
//Decimal numbers are always treated as signed types.
//Note: There are no negative integer constants in C.
//All integers constants in C are numerals with nonnegative values. Where a minus sign appears with a constant in C code, as in -34, it is two separate tokens: a unary negation operator followed by a constant.
//4.3.1). Expression truncated at 32 bits
//example:
void constant_expression_truncated_at_32_bits_07(int int1, int int2)
{
    long long1, long2, long3;
    long1 = 20000000 * 30000000; // expression truncated at 32 bits
    long2 = int1 * int2;// expression truncated at 32 bits
    long3 = 1 << 32; // "1" is int, so the result of "1<<32" is 0.
    printf("long1:%ld, long2:%ld, long3:%ld\n", long1, long2, long3);
}

//Solution: use wider constant:
//long1 = 20000000L * 30000000;
//long1 = 1L << 32;
/*
other suffixes:

1LL // (long long)
1U // (unsigned)
1UL // (unsigned long)
1ULL // (unsigned long long)
*/

//4.3.2). Expression depends on 32 bit truncation
//example:
long constant_expression_depends_on_32_bit_truncation_08(long long1)
{
    long1 += 0xffffffff; //0xffffffff is unsigned int. the result is: long1 - 1 for ILP32, long1 + 4294967295 for LP64
    return long1;
}

//Solution: use wider constant:
//long1 += -1L;


//4.3.3). Constant has int size, not "full size", leading zeroes do not increase the size
//example:
long constant_leading_zeroes_do_not_increase_the_size_09(long long1, long long2)
{
    long1 &= ~0xffff0000; //clears 48 bits
    long2 &= ~0x00000000ffff0000; //clears 48 bits. leading zeroes do not increase the size!
    printf("long1:%ld, long2:%ld\n", long1, long2);
    return (long1 + long2);
}

//solution: use a cast or suffix:
//long1 &= ~(long)0xffff0000; //clears 16 bits
//long1 &= ~0xffff0000L; //clears 16 bits


//4.3.4). Shifts expecting 32 bit operands
//Example:
void constant_shifts_expecting_32_bit_operands_10(unsigned long ulong1, long long2)
{
    ulong1 = (ulong1 << 5) >> 16; //ILP32: keeps bits 11-26, LP64: bits 11-58
    long2 = (long2 << 5) >> 16; //ILP32: might sign ext.11-26, LP64: bits 11-58
    printf("ulong1:%lu, long2:%ld\n", ulong1, long2);
}

//solution: don't assumes that long or unsigned long data is 32 bits:
//ulong1 = (ulong1 & 0x7fff800) >> 11;
//long1 = (long1 << (CHAR_BIT * sizeof(long) – 27)) >> (CHAR_BIT * sizeof(long) – 16);


//4.4). Interchange between pointer of "int" and "long"
//For *long -> *int, there may be truncation issue when dereferencing the latter.
//For *int -> *long, there may be undefined behavior, since the latter may access unexpected memory area.
//Example:

static void fint(int *pint1)
{
    printf("pint1:%p, value:%d\n", pint1, *pint1);
}

static void flong(long *plong1)
{
    printf("plong1:%p, value:%ld\n", plong1, *plong1);
}

int pointer_long_to_int_truncation_when_dereferencing_11(long *plong1)
{
    int *pint1;
    pint1 = (int *)plong1;//*long -> *int, there may be truncation issue when dereferencing the latter.
    fint((int *)plong1);//*long -> *int, there may be truncation issue when dereferencing the latter.
    return *pint1;
}

long pointer_int_to_long_may_access_unexpected_memory_area_12(int *pint1)
{
    long *plong1;
    plong1 = (long *)pint1;//*int -> *long, there may be undefined behavior, since the latter may access unexpected memory area.
    flong((long *)pint1);//*int -> *long, there may be undefined behavior, since the latter may access unexpected memory area.
    return *plong1;
}



//4.5). Alignment/padding/size of structure
//Example:
struct alignment_bad
{
    int a;
    void* p;
    int b;
};
/*
32-bit layout 64-bit layout
0:a                0:a
4:p                4:<padding>
8:b                8:p
                    12:
                    16:b
*/

/*
    Solution: To be better performance, this is better:
*/
struct alignment_better
{
    void* p;
    int a;
    int b;
};
/*
32-bit layout    64-bit layout
0:p                 0:p
4:a                 4:
8:b                 8:a
                     12:b
*/

void alignment_padding_size_of_structure_13(void)
{
    struct alignment_bad bad;
    struct alignment_better better;
    printf("sizeof(struct alignment_bad):%zu\n", sizeof(bad));
    printf("sizeof(struct alignment_better):%zu\n", sizeof(better));
}



//4.6). Implicit Declarations
//4.6.1). Implicit type of int
/*
The compiler would assume the type int for any function or variable that is used in a module and not defined or declared externally.
If a variable is used without declaration, its type is int and may lead to problem if it's not compatible with the real type.
If a function is called in C without function prototype, the return value is "int".
For example, if malloc() is used without a prototype, the resulting binary might break because of:
malloc() The return value is a 32-bit entity and therefore only half of the bits of the returned address might be stored in the variable that holds the return value making the pointer invalid.
*/
void *implicit_type_of_int_14(int size)
{
    void *p;
    p = myalloc(size);//If a function is called in C without function prototype, the return value is "int" and therefore only half of the bits of the returned pointer address might be stored in the variable
    return p;
}


//4.6.2). implicit parameter declaration
//default argument promotions
//If the called function has not a prototype, the integer promotions are performed on each argument smaller than int.

//Truncation
//If passing a "long" to an "int" argument, the value would be truncated.

//Undefined behavior
//If passing a small size value to a wider size argument, undefined behavior would happen:
//For example, if you pass in a 32-bit value, it is normally passed in 64-bit registers or on the stack as 64-bit value. The question now is what to do with the unused 32 bits? The 32-bit value can be zero-extended so that the unused bits are all zero, it can be sign-extended giving all zeros or all ones, and it can be left unspecified. If the called function expects now a 64-bit value where it gets a 32-bit value, the function might not work as expected.

//Note that 0 is not the same as a NULL pointer since those have different sizes.



//4.7). Magic Numbers
//4.7.1). using "4" for pointer size
//Example:
int **magic_number_using_4_for_pointer_size_15()
{
    int **pointerArray;
    size_t num= 10U;
    pointerArray = (int **) malloc(num * 4);//using "4" for pointer size
    return pointerArray;
}

//solution: using "sizeof" instead of magic number "4":
//int **pointerArray = (int **) malloc(num * sizeof(int *));


//4.7.2). using 0xFFFFFFFF as "-1" to return an error
//Example:

#define ERROR_CODE 0xffffffff
size_t mysize(int size)
{
    if (size < 1)
        return (size_t)ERROR_CODE;//using 0xFFFFFFFF as "-1" to return an error
    else
        return (size_t)size;
}

void magic_using_8f_as_negative_one_16(int size)
{
    if (mysize(size) == (size_t)-1) //0xFFFFFFFF is unsigned int, after converted to size_t, still the same value no matter at 32bit or 64bit platform
        printf("(size_t)0xFFFFFFFF == -1\n"); // -1 is 0xffffffffffffffff on 64bit platform, so won't hit here.
    else
        printf("(size_t)0xFFFFFFFF != -1\n"); // -1 is 0xffffffffffffffff on 64bit platform, so hit here.
}

//Solution: using "(size_t)(-1)" instead of "0xffffffff" for this use case:
//#define ERROR_CODE ((size_t)(-1))
//for moere info about Integer constants, see 4.3). constant



//4.8). format specifier
//The format strings for printf, sprintf, scanf, and sscanf could have problem in 64 bit platform.
//4.8.1). Truncation
//example
void format_specifier_truncation_17(void *p)
{
    printf("size:%u", sizeof(int)); //size_t is 64 bit now, but "%u" is still 32-bit.
    printf("address:%x", (void *)p); //pointer is 64 bit now, but "%x" is still 32-bit.
}

/*
Solution:
printf("size:%zu", sizeof(foo)); // For size_t type, use the z format modifier.
printf("address:%p", (void *)p); // For pointers, use %p instead of %x.

For ptrdiff_t, intptr_t, and uintptr_t data types, use the t format modifier.
You can use both z and t with the following specifiers: d, i, o, u, x, and X. If you omit z or t and use only %d, for example, your value is truncated to the low 32 bits.
*/


//4.8.2). Overflow
//example
void format_specifier_overflow_18(void *p)
{
    char buf[9] = {0};
    sprintf(buf, "%p", p); //This code will cause buffer overflow on the 64-bit platform.
    printf("buf:%s\n", buf);
}


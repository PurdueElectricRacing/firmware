Irving Wang (irvingw@purdue.edu)

## About

This document defines the general coding style and rules for PER-owned projects. Most of these rules are C-specific but the essence of the rules can still be applied to other languages.

The goal is to maintain a consistent and easily maintainable codebase. Following these rules reduces opportunities for hidden bugs and makes the code more readable and reviewable for others.

Additionally, the compiler standard for `firmware` is set to C23, enabling several important modern features.

## Rules and Suggestions

1. Avoid nesting where possible
    1. Try using early returns and condition inversion
        - remember to free resources (like semaphore/mutex) before returning
    2. No more than 4 indentations deep
    
    ```c
    void nesting_example_bad() {
    	if (condition1) {
    		if (condition2) {
    			// code here
    		}
    	}
    }
    
    void nesting_example_better() {
    	if (!condition1) {
    		return;
    	}
    	
    	if (!condition2) {
    		return;
    	}
    	// code here
    }
    ```
    
2. Use of dynamic memory allocation is forbidden
    1. Absolutely no `malloc()`, this will open the risk for memory fragmentation
    2. Instead, you must either allocate memory at compile time, or use stack-based local variables
3. Avoid magic numbers
    1. Define an `enum`, `static constexpr`, or macro
    2. Prefer `static constexpr` over macros for inherent type safety
4. Variable names
    1. Use `snake_case`
    2. Try to use full words wherever possible
    3. Keep it to a reasonable length
    4. Global variables should be prefixed with `g_`
    5. Variables with units should have their unit at the end of the variable name
    6. The exception to this rule is temporary variables in for loops like `i` or `j`
    
    ```c
    g_global_var = 0;
    g_last_rx_time_ms = 0;
    
    void main() {
    	local_var = 0;
    }
    ```
    
5. Function names
    1. Use `snake_case`
    2. Try to use full words wherever possible
    3. Keep it to a reasonable length
    4. Library functions should be prefixed with the library name in caps
    
    ```c
    void NXT_set_command();
    void poll_shock_pots();
    ```
    
6. Macros
    1. Use `SCREAMING_SNAKE_CASE`
    2. Same rules as function names
    3. Wrap values with parenthesis to prevent expansion edge cases
        
        ```c
        // Example pulled from firmware/PR(#147)
        
        // bad
        #define ABS(x) ((x) < 0 ? (-1 * x) : x)
        // good
        #define ABS(x) ((x) < 0 ? (-1 * (x)) : (x))
        ```
        
7. Inline functions
    1. Use inline functions over macros wherever possible, they offer type safety
8. Don’t use brace-less control statements
    
    ```c
    // bad
    if (condition)
    	code();
    	
    // good
    if (condition) {
    	code();
    }
    ```
    
9. Allowed data types
    1. `float`
    2. `char`
    3. `bool` (standard keyword in C23)
    4. From `<stdint.h>` 
    
    ```c
    // bad, size depends on architecture
    int num;
    short num2;
    
    // good, explicit sizes
    int32_t num;
    int16_t num2;
    ```
    
10.  Always initialize variables with a value
    1. global variables should explicitly be zero’d out (even though the compiler does this for you)
    
    ```c
    type_t g_global_struct = {0};
    uint8_t g_global_val = 0;
    ```
    
11. `struct` declarations
    1. Always use `typedef` and suffix your new type with `_t`
    2. Order your variables in descending size (this saves space from padding)
    3. However exceptions can be made for logical/clarity purposes
    4. Remember that we target 32-bit architecture for firmware
        - pointers are 4 bytes
    
    ```c
    // bad, 24 bytes of padding
    typedef struct {
    	uint16_t small_num;
    	uint32_t medium_num;
    	uint64_t big_num;
    	uint16_t small_num2;
    } bad_struct_t
    
    // good, 0 bytes of padding
    typedef struct {
    	uint64_t big_num;
    	uint32_t medium_num;
    	uint16_t small_num;
    	uint16_t small_num2;
    } good_struct_t;
    ```
    
12. `enum` declaration
    1. Always use `typedef` and suffix your new type with `_t`
    2. Enum members should be prefixed with the name of the enum (example below)
    3. Always explicitly declare the base type of the enum, ESPECIALLY when it is a member of a `struct` 
    4. It is recommended to assign explicit values to each enum member
    
    ```c
    // example enum
    typedef enum : uint8_t {
        FAULT_STATE_OK = 0,
        FAULT_STATE_PENDING = 1,
        FAULT_STATE_LATCHED = 2,
        FAULT_STATE_RECOVERING = 3
    } fault_state_t;
    ```
    
13. Check null pointers before use
    
    ```c
    // bad
    void pass_by_reference(uint16_t *ptr) {
    	*ptr = 10
    }
    
    // good
    void pass_by_reference(uint16_t *ptr) {
    	if (ptr == NULL) {
    		return; // or return error code
    	}
    	*ptr = 10
    }
    ```
    
14. Run da formatter before merging your PR
    1. Use the command `clang-format -i file.c`
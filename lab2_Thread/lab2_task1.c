#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       
#include <pthread.h>    

/*  Global data shared by all threads  */
static int *g_input  = NULL;   // pointer to the original array 
static int *g_output = NULL;   // global pointer to the destination array used by the merge thread at last
static int  g_n      = 0;      // number of elements in the arrays must be even. Stated in Print

/* structure for merging and sorting  */
typedef struct {
    int *arr;     // pointer to the array to be sorted 
    int start;    // start index of the subarray this thread will sort
    int end;      // end index of the subarray this thread will sort
} SortArgs;

typedef struct {
    int *src;         // pointer to the source array that holds two sorted halves did in g_input
    int left_lo;      // left half start index
    int left_hi;      // left half end   index  
    int right_lo;     // right half start index 
    int right_hi;     // right half end   index 
    int *dst;         // pointer to destination array to write the merged result 
} MergeArgs;

/* 
   bubble_sort_range cunction shame as given example in class
   Inputs array start and end index (as half)
   Output : sorted array
   */
static void bubble_sort_range(int *a, int start, int end)
{
    int len = end - start;                 // number of elements in this slice

    for (int pass = 0; pass < len - 1; ++pass) {   
        int swapped = 0;                           
        int upto = (len - 1) - pass;               
        for (int i = start; i < start + upto; ++i) {  
            if (a[i] > a[i + 1]) {                
                int tmp = a[i];
                a[i] = a[i + 1];
                a[i + 1] = tmp;
                swapped = 1;                      
            }
        }
        if (!swapped) break;                      
    }
}

/* 
   sort_thread calling sort function for threads
   Input  : vp structure describing which subarray to sort
   Output : returns NULL to pthreads and arr[start..end)
            becomes sorted in place.
   */
static void *sort_thread(void *vp)
{
    SortArgs *args = (SortArgs *)vp;               // unpack the struct
    bubble_sort_range(args->arr, args->start, args->end);  // sort that slice
    return NULL;                                    // pthreads expects a void* return; we return nothing
}

/* 
   merge_thread that merges two sorted thred
   Input  : vp structure describing two sorted halves in src and a dst
   Output : returns NULL and dst[0..(right_hi-left_lo)) is the
            fully merged globally sorted array.
    */
static void *merge_thread(void *vp)
{
    MergeArgs *m = (MergeArgs *)vp;      

    int i = m->left_lo;                  // pointer for left  half
    int j = m->right_lo;                 // pointer for right half
    int k = 0;                           // pointer for destination 

    // merge by always copy the smaller head element to dst
    while (i < m->left_hi && j < m->right_hi) {
        if (m->src[i] <= m->src[j]) {    // take from left 
            m->dst[k++] = m->src[i++];
        } else {
            m->dst[k++] = m->src[j++];
        }
    }

    // Copy any leftovers from the left half
    while (i < m->left_hi) {
        m->dst[k++] = m->src[i++];
    }

    // Copy any leftovers from the right half
    while (j < m->right_hi) {
        m->dst[k++] = m->src[j++];
    }

    return NULL;                         
}

/* 
   function for printing
    */
static void print_array(const char *label, const int *a, int n)
{
    printf("%s", label);                 
    for (int i = 0; i < n; ++i)          
        printf("%s%d", (i ? " " : ""), a[i]);
    printf("\n");                        
}


int main(int argc, char **argv)
{
    // Check for exactly one command-line argument if not give error messgae
    if (argc != 2) {
        fprintf(stderr, "To Run: %s <even_count>\n", argv[0]);
        return 1;
    }

    g_n = atoi(argv[1]);                 // read N from argv[1]
    if (g_n <= 0 || (g_n % 2) != 0) {    // N must be positive and even if not give error message
        fprintf(stderr, "Error: count must be a positive even number.\n");
        return 1;
    }

    // Allocate the two arrays  input and output 
    g_input  = (int *)malloc((size_t)g_n * sizeof(int));
    g_output = (int *)malloc((size_t)g_n * sizeof(int));
    if (!g_input || !g_output) {         // check allocation
        fprintf(stderr, "Error: malloc failed.\n");
        free(g_input);
        free(g_output);
        return 1;
    }

    // Fill input with pseudorandom values within 1000
    for (int i = 0; i < g_n; ++i)
        g_input[i] = rand() % 1000;

    // Show the unsorted array using print function
    print_array("Original: ", g_input, g_n);

    // Create two sorting threads, each working on one half of g_input
    pthread_t t_sort_left, t_sort_right;

    int mid = g_n / 2;                               // split input

    SortArgs left_args  = { g_input, 0,   mid   };   // left  half is [0, mid)
    SortArgs right_args = { g_input, mid, g_n   };   // right half is [mid, g_n)

    // thread creation
    if (pthread_create(&t_sort_left,  NULL, sort_thread, &left_args) != 0) {
        fprintf(stderr, "Error: pthread_create (left) failed.\n");
        free(g_input); free(g_output); return 1;
    }
    if (pthread_create(&t_sort_right, NULL, sort_thread, &right_args) != 0) {
        fprintf(stderr, "Error: pthread_create (right) failed.\n");
        free(g_input); free(g_output); return 1;
    }

    // Wait until both halves are sorted 
    pthread_join(t_sort_left,  NULL);
    pthread_join(t_sort_right, NULL);

    // Prepare and start the merge thread to combine the two sorted halves into g_output
    pthread_t t_merge;
    MergeArgs margs = {
        .src      = g_input,  // read from g_input as contains two sorted halves now
        .left_lo  = 0,
        .left_hi  = mid,
        .right_lo = mid,
        .right_hi = g_n,
        .dst      = g_output  // write the fully merged, sorted array here
    };
    //creating merge thread
    if (pthread_create(&t_merge, NULL, merge_thread, &margs) != 0) {
        fprintf(stderr, "Error: pthread_create (merge) failed.\n");
        free(g_input); free(g_output); return 1;
    }

    // Wait for the merge to complete
    pthread_join(t_merge, NULL);

    // Print the final sorted array
    print_array("Sorted:   ", g_output, g_n);

    
    return 0;
}



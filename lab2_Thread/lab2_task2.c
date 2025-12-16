#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       
#include <pthread.h>   
/* This code only works for two or four threads*/ 

/*  Global data shared by all threads*/
static int *g_input  = NULL;   // pointer to original array
static int *g_output = NULL;   // pointer to output array
static int  g_n      = 0;      // total number of elements must be multiple of 2
static int  g_T      = 0;      // number of sorting threads 2 or 4

/* Structure for sorting thread */
typedef struct {
    int *arr;   // array pointer (g_input)
    int start;  // start index of this thread's slice
    int end;    // end index of this thread's slice
} SortArgs;

/* Structure for the merge thread */
typedef struct {
    const int *src;  // pointer to g_input
    int       *dst;  // pointer to g_output
    int        n;    // total length of the whole array
    int        T;    // number of sorted chunks either 2 or 4
    int       *tmp;  // buffer used only when Thread==4 
} MergeArgs;

/* 
   bubble_sort_range cunction shame as given example in class
   Inputs array start and end index (as half)
   Output : sorted array
   */

static void bubble_sort_range(int *a, int start, int end)
{
    int len = end - start;                       
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
    SortArgs *args = (SortArgs *)vp;              
    bubble_sort_range(args->arr, args->start, args->end); 
    return NULL;                                   
}

/* 
   merge_two Threads
   Inputs : src ; source array that holds two sorted ranges
            a_lo : left  range start 
            a_hi : left  range end  
            b_lo : right range start 
            b_hi : right range end  
            dst  : destination array to write merged output
            k_lo : starting index in dst 
   Output : writes merged (sorted) contents of the two ranges into dst
            beginning at dst[k_lo]
  */
static void merge_two(const int *src,
                      int a_lo, int a_hi,
                      int b_lo, int b_hi,
                      int *dst, int k_lo)
{
    int i = a_lo;                 // POINTER for left  range
    int j = b_lo;                 // POINTER for right range
    int k = k_lo;                 // POINTER for destination

    // Merge until one of the ranges is done
    while (i < a_hi && j < b_hi)
        dst[k++] = (src[i] <= src[j]) ? src[i++] : src[j++];

    // Copy any leftovers from the left range
    while (i < a_hi) dst[k++] = src[i++];

    // Copy any leftovers from the right range
    while (j < b_hi) dst[k++] = src[j++];
}

/* 
   merge_thread 2 or 4
   Inputs : vp structure describing how many chunks exist Thread=2 or Thread==4,
                  the source array (src), destination (dst), and scratch tmp
   Output : returns NULL and dst[0..n) becomes globally sorted
   Logic  :
     - T==2 : merge left half [0..n/2) and right half [n/2..n) into dst
     - T==4 : pairwise merge quarters (0,1) and (2,3) into tmp, then
              merge the two halves from tmp into dst
   */
static void *merge_thread(void *vp)
{
    MergeArgs *m = (MergeArgs *)vp;  

    if (m->T == 2) {
        int mid = m->n / 2;                              
        merge_two(m->src, 0, mid, mid, m->n, m->dst, 0); // merge the two halves into dst[0..n)
    } else {//four thread case
        // quarters [0,q), [q,2q), [2q,3q), [3q,n)
        int q = m->n / 4;

        // Stage 1 merge (Q0, Q1) → tmp[0..2q)
        merge_two(m->src, 0,     q,   q,   2*q, m->tmp, 0);

        // Stage 1: merge (Q2, Q3) → tmp[2q..n)
        merge_two(m->src, 2*q, 3*q, 3*q,   m->n, m->tmp, 2*q);

        // Stage 2: merge the two halves from tmp → dst[0..n)
        merge_two(m->tmp, 0,   2*q, 2*q,   m->n, m->dst, 0);
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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <even_count> <threads:2|4>\n", argv[0]);
        return 1;
    }

    g_n = atoi(argv[1]);   // number of elements
    g_T = atoi(argv[2]);   // number of sorting threads (2 or 4)

    // N must be positive and even if not give error message threads must be 2 or 4 and N divisible by threads
    if (g_n <= 0 || (g_n % 2) != 0) {
        fprintf(stderr, "Error: <even_count> must be a positive even number.\n");
        return 1;
    }
    if (!(g_T == 2 || g_T == 4)) {
        fprintf(stderr, "Error: <threads> must be 2 or 4.\n");
        return 1;
    }
    if (g_n % g_T != 0) {
        fprintf(stderr, "Error: N must be divisible by <threads> (e.g., N%%%d==0).\n", g_T);
        return 1;
    }

    // Allocating the two global arrays dynamically
    g_input  = (int *)malloc((size_t)g_n * sizeof(int));
    g_output = (int *)malloc((size_t)g_n * sizeof(int));
    if (!g_input || !g_output) {
        fprintf(stderr, "Error: malloc failed.\n");
        free(g_input);
        free(g_output);
        return 1;
    }

    // input with pseudorandom integers in [0, 1000)
    for (int i = 0; i < g_n; ++i) g_input[i] = rand() % 1000;

    // Show unsorted list
    print_array("Original: ", g_input, g_n);

    //  g_T sorting threads over equal-size chunks 
    int chunk = g_n / g_T;  // size of each chunk
    pthread_t *tids = (pthread_t *)malloc((size_t)g_T * sizeof(pthread_t)); 
    SortArgs  *sarg = (SortArgs  *)malloc((size_t)g_T * sizeof(SortArgs));  
    if (!tids || !sarg) {
        fprintf(stderr, "Error: malloc failed.\n");
        free(tids); free(sarg); free(g_input); free(g_output);
        return 1;
    }

    // Creating each sorting thread with its own slice
    for (int t = 0; t < g_T; ++t) {
        sarg[t].arr   = g_input;
        sarg[t].start = t * chunk;
        sarg[t].end   = (t + 1) * chunk;
        if (pthread_create(&tids[t], NULL, sort_thread, &sarg[t]) != 0) {
            fprintf(stderr, "Error: pthread_create (sort %d) failed.\n", t);
            free(tids); free(sarg); free(g_input); free(g_output);
            return 1;
        }
    }

    // Wait for all sorting threads to finish
    for (int t = 0; t < g_T; ++t)
        pthread_join(tids[t], NULL);

    // merge thread to combine the sorted chunks 
    pthread_t t_merge;    
    int *tmp = NULL;      // temp buffer need only for 4-way merge
    if (g_T == 4) {
        tmp = (int *)malloc((size_t)g_n * sizeof(int));
        if (!tmp) {
            fprintf(stderr, "Error: malloc failed (tmp).\n");
            free(tids); free(sarg); free(g_input); free(g_output);
            return 1;
        }
    }

    MergeArgs margs;      
    margs.src = g_input;  // read from g_input now contains T sorted chunks
    margs.dst = g_output; // output fully merged result 
    margs.n   = g_n;      // total length
    margs.T   = g_T;      // 2 or 4 chunks
    margs.tmp = tmp;      // scratch buffer only used when T==4

    if (pthread_create(&t_merge, NULL, merge_thread, &margs) != 0) {
        fprintf(stderr, "Error: pthread_create (merge) failed.\n");
        free(tmp); free(tids); free(sarg); free(g_input); free(g_output);
        return 1;
    }

    // Wait for merge to complete
    pthread_join(t_merge, NULL);

    // Print sorted array
    print_array("Sorted:   ", g_output, g_n);

    
    return 0;
}


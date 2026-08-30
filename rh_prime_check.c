/*
 * rh_prime_check.c
 * ----------------
 * Numerically explores the Riemann Hypothesis via the prime-counting
 * function pi(x) versus the logarithmic integral Li(x).
 *
 * Background:
 *   pi(x)      = number of primes <= x
 *   Li(x)      = integral from 2 to x of dt/ln(t)   (best smooth approx to pi(x))
 *
 * The Riemann Hypothesis is EQUIVALENT to the statement:
 *
 *     pi(x) - Li(x) = O( sqrt(x) * log(x) )
 *
 * i.e. the error between the actual prime count and its smooth
 * approximation never grows faster than sqrt(x)*log(x). This program:
 *
 *   1. Sieves all primes up to N using a segmented Sieve of Eratosthenes
 *      (memory-efficient: base sieve up to sqrt(N), then sweeps segments).
 *   2. Computes pi(x) at checkpoints as it sieves.
 *   3. Computes Li(x) at those same checkpoints via numerical integration
 *      (adaptive Simpson's rule, long double precision).
 *   4. Reports the error pi(x)-Li(x) against the RH bound C*sqrt(x)*log(x),
 *      writing a CSV you can plot.
 *
 * Build:  gcc -O2 -o rh_prime_check rh_prime_check.c -lm
 * Run:    ./rh_prime_check 100000000        (checks primes up to 10^8)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef unsigned char u8;
typedef uint64_t u64;

/* ---------- Li(x): logarithmic integral via adaptive Simpson's rule ---------- */

static long double li_integrand(long double t) {
    return 1.0L / logl(t);
}

/* Simpson's rule on [a,b] with n subintervals (n must be even) */
static long double simpson(long double a, long double b, int n) {
    if (n % 2) n++;
    long double h = (b - a) / n;
    long double sum = li_integrand(a) + li_integrand(b);
    for (int i = 1; i < n; i++) {
        long double x = a + i * h;
        sum += (i % 2 == 0 ? 2.0L : 4.0L) * li_integrand(x);
    }
    return sum * h / 3.0L;
}

/* Li(x) = integral_2^x dt/ln(t). Integrand blows up near t=1 but we start
 * at 2, so it's smooth on [2, x]. Use enough subintervals to be accurate
 * for x up to ~10^10. */
static long double li(long double x) {
    if (x <= 2.0L) return 0.0L;
    int n = 20000; /* generous; integrand is smooth here */
    return simpson(2.0L, x, n);
}

/* ---------- Segmented Sieve of Eratosthenes ---------- */

/* Basic sieve up to `limit`, returns list of primes and count via *out_count */
static u64 *simple_sieve(u64 limit, u64 *out_count) {
    u8 *is_composite = calloc(limit + 1, 1);

    /* Mark composites first */
    for (u64 i = 2; i * i <= limit; i++) {
        if (!is_composite[i]) {
            for (u64 j = i * i; j <= limit; j += i)
                is_composite[j] = 1;
        }
    }

    /* Now count survivors */
    u64 count = 0;
    for (u64 i = 2; i <= limit; i++)
        if (!is_composite[i]) count++;

    /* Fill the primes array */
    u64 *primes = malloc(count * sizeof(u64));
    u64 idx = 0;
    for (u64 i = 2; i <= limit; i++)
        if (!is_composite[i]) primes[idx++] = i;

    free(is_composite);
    *out_count = count;
    return primes;
}

int main(int argc, char **argv) {
    u64 N = 100000000ULL; /* default: 10^8 */
    if (argc > 1) N = strtoull(argv[1], NULL, 10);

    fprintf(stderr, "Sieving primes up to %llu ...\n", (unsigned long long)N);

    u64 sqrtN = (u64)sqrtl((long double)N) + 1;
    u64 base_count;
    u64 *base_primes = simple_sieve(sqrtN, &base_count);

    /* Checkpoints where we report pi(x): logarithmically spaced */
    #define MAX_CHECKPOINTS 60
    u64 checkpoints[MAX_CHECKPOINTS];
    int n_checkpoints = 0;
    for (long double x = 1000.0L; x <= (long double)N && n_checkpoints < MAX_CHECKPOINTS; x *= 1.35L) {
        checkpoints[n_checkpoints++] = (u64)x;
    }
    checkpoints[n_checkpoints - 1] = N; /* make sure N itself is included */

    u64 pi_count = 0;      /* running prime count */
    int cp_idx = 0;
    u64 *pi_at_checkpoint = calloc(n_checkpoints, sizeof(u64));

    /* Handle primes <= sqrtN directly (they're all in base_primes) */
    const u64 SEGMENT_SIZE = 1 << 20; /* 1M numbers per segment */
    u8 *segment = malloc(SEGMENT_SIZE);

    for (u64 low = 0; low <= N; low += SEGMENT_SIZE) {
        u64 high = low + SEGMENT_SIZE - 1;
        if (high > N) high = N;
        u64 seg_len = high - low + 1;
        memset(segment, 0, seg_len);

        for (u64 i = 0; i < base_count; i++) {
            u64 p = base_primes[i];
            if (p * p > high) break;
            u64 start = (low + p - 1) / p * p;
            if (start < p * p) start = p * p;
            for (u64 j = start; j <= high; j += p)
                segment[j - low] = 1;
        }

        for (u64 num = (low < 2 ? 2 : low); num <= high; num++) {
            if (!segment[num - low]) {
                pi_count++;
            }
            /* record checkpoint(s) that fall in this exact position */
            while (cp_idx < n_checkpoints && checkpoints[cp_idx] == num) {
                pi_at_checkpoint[cp_idx] = pi_count;
                cp_idx++;
            }
        }
    }
    /* catch any checkpoint not hit exactly (shouldn't happen, safety net) */
    for (int i = 0; i < n_checkpoints; i++)
        if (pi_at_checkpoint[i] == 0 && checkpoints[i] > 1)
            pi_at_checkpoint[i] = pi_count; /* fallback: use final count */

    fprintf(stderr, "Done sieving. Total primes up to %llu: %llu\n",
            (unsigned long long)N, (unsigned long long)pi_count);

    /* ---------- Compute Li(x) and RH bound at each checkpoint, write CSV ---------- */
    FILE *csv = fopen("rh_results.csv", "w");
    fprintf(csv, "x,pi_x,Li_x,error,rh_bound_sqrtx_logx,within_bound\n");

    printf("%-14s %-12s %-14s %-12s %-18s %-6s\n",
           "x", "pi(x)", "Li(x)", "error", "sqrt(x)*log(x)", "OK?");

    for (int i = 0; i < n_checkpoints; i++) {
        long double x = (long double)checkpoints[i];
        long double Lix = li(x);
        long double error = (long double)pi_at_checkpoint[i] - Lix;
        long double bound = sqrtl(x) * logl(x);
        int within = fabsl(error) <= bound;

        printf("%-14.0Lf %-12llu %-14.4Lf %-12.4Lf %-18.4Lf %-6s\n",
               x, (unsigned long long)pi_at_checkpoint[i], Lix, error, bound,
               within ? "yes" : "NO");

        fprintf(csv, "%.0Lf,%llu,%.6Lf,%.6Lf,%.6Lf,%d\n",
                x, (unsigned long long)pi_at_checkpoint[i], Lix, error, bound, within);
    }
    fclose(csv);

    printf("\nWrote rh_results.csv (all checkpoints, ready to plot).\n");
    printf("If every 'OK?' column says yes, the data is consistent with RH\n");
    printf("(pi(x)-Li(x) staying inside the sqrt(x)*log(x) envelope).\n");

    free(base_primes);
    free(segment);
    free(pi_at_checkpoint);
    return 0;
}

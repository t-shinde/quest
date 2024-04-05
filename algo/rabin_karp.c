#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define HASH_PRIME	227
#define HASH_MOD	1000000009
//#define HASH_MOD	1000005

static inline long hash(char *p, long len)
{
	long ret = 0;

	for(long i = 0; i < len; i++) {
		ret = (ret * HASH_PRIME) + p[i];
		ret = ret % HASH_MOD;
	}

	return ret;
}

static inline long hash_power(long pat_len)
{
	long p = 1;

	for (long i = 0; i < pat_len; i++) {
		p *= HASH_PRIME;
		p %= HASH_MOD;
	}

	return p;
}

long hash_roll(long power, long old_hash,
		  unsigned long prev_char,
		  unsigned long next_char)
{
	long ret = old_hash;

	/* Add new char */
	ret = (ret * HASH_PRIME) + next_char;
	ret = ret % HASH_MOD;

	/* remove old char */
	ret = ret - (power * prev_char);
	ret = ret % HASH_MOD;
	if (ret < 0) {
		ret += HASH_MOD;
	}

	return ret;
}

long find_pattern_bf(char *haystack, long hlen, char *pat, long plen)
{
	long i, j, len = hlen - plen;

	for (i = 0; i < len; i++) {
		for (j = 0; j < plen; j++) {
			if (pat[j] != haystack[i+j]) {
				break;
			}
		}
		if (j == plen) {
			printf("Found matching pattern at offset %ld\n", i);
			return i;
		}
	}

	return i;
}

long find_pattern_rk(char *haystack, long hlen, char *pat, long plen)
{
	long h, x, off = 0, ret = -1;
	long power = hash_power(plen);
	unsigned long pchar, nchar;

	if (plen > hlen) {
		return ret;
	}

	h = hash(pat, plen);
	x = hash(&haystack[off], plen);

	while (1) {
		if (x == h) {
			if (memcmp(&haystack[off], pat, plen) == 0) {
				printf("Found matching pattern at offset %ld\n", off);
				ret = off;
				return ret;
			}
		}

		off++;

		if ((off+plen-1) >= hlen) {
			break;
		}

		pchar = haystack[off-1];
		nchar = haystack[off+plen-1];
		x = hash_roll(power, x, pchar, nchar);
	}

	return ret;
}

void test1()
{
	char *buf = "ABCABCaasdfsafsdfllereklABCrtnbmlqpwerbdlthnABCasdfjadslfABC";
	char *pat = "ABC";
	long n = strlen(buf), m = strlen(pat);

	find_pattern_rk(buf, n, pat, m);
}

void fill_random(char *buf, long len)
{
#if 0
	/* Dont work with binary data */
	long ret, fd = open("/dev/urandom", O_RDONLY);
	long off = 0;
	if (fd < 0) {
		perror("Open");
		abort();
	}

	while (off < len) {
		ret = read(fd, &buf[off], len - off);
		if (ret <= 0) {
			printf("Failed to read complete bytes, Read %d\n", ret);
			abort();
		}
		off += ret;
	}

	close(fd);
	return;
#else
	long i, n, r;
	char *s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890abcdefghijklmnopqrstuvwxyz";

	n = strlen(s);
	for (i = 0; i < len; i++) {
		r = rand() % n;
		buf[i] = s[r];
	}

#endif
}

void test2()
{
	char *buf = NULL;
	char pat[100];
	long cnt = 10;
	long i, ret, hlen = 1024;
	hlen *= 1024;
	hlen *= 1024;
	hlen *= 4;

	buf = malloc(hlen);
	printf("Filling buffer...\n");
	fill_random(buf, hlen);

	while (cnt--) {
		ret = rand() % hlen;
		ret -= 100;
		if (ret < 0) {
			ret = 0;
		}

		for (i = 0; i < 100; i++) {
			pat[i] = buf[ret+i];
		}

		//printf("searching pattern...\n");
		ret = find_pattern_rk(buf, hlen, pat, 100);
		if (ret == -1) {
			printf("Failed!!\n");
			exit(1);
		}

		ret = find_pattern_bf(buf, hlen, pat, 100);
		if (ret == -1) {
			printf("Failed!!\n");
			exit(1);
		}
	}

	free(buf);
}

int main()
{
	//test1();
	test2();
	return 0;
}

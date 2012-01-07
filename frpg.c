#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define pow2(n) 1 << n
#define LEN 1024

const char * help = "Formatting random password generator\n"
"-l n: lowercase letters (a-z) of n times\n"
"-u n: uppercase letters (A-Z)of n times\n"
"-n n: digital number (1-9) of n times\n"
"-s n: signs such !#$%&... of n times\n"
"-h: print help\n"
"-v: print version";

const char * version = "Formatting random password generator\n"
"version: 0.1\n"
"author: yang@haipo.me";

enum char_type {
    digit = 1,
    lower = 2,
    upper = 4,
    sign = 8
};

struct memory {
	char * p;
	int i;
	size_t len;
};

struct passmask {
    int digits;
    int lowers;
    int uppers;
    int signs;
    int types;
};

int getrand(int max)
{
	return rand() % max;
}

char getlower()
{
	return (char)(getrand('z' - 'a') + 'a');
}

char getupper()
{
	return (char)(getrand('Z' - 'A') + 'A');
}

char getdigit()
{
	return (char)(getrand('9' - '0') + '0');
}

char getsign()
{
	int i = getrand(3);
	switch(i) {
		case 0:
			return (char)(getrand(15) + 33);
		case 1:
			return (char)(getrand(6) + 91);
		case 2:
			return (char)(getrand(4) + 123);
	}
}

struct passmask * passmasknew(struct passmask * mask)
{
    mask = calloc(1, sizeof(struct passmask));
    if (!mask) {
        fprintf(stderr, "Error while allocating memory for mask\n");
        return NULL;
    }
    
    mask->digits = 0;
    mask->lowers = 0;
    mask->uppers = 0;
    mask->signs = 0;
    mask->types = 0;
    
    return mask;
}
void passmaskdestruct(struct passmask * mask)
{
    if (mask)
        free(mask);
    else
        fprintf(stderr, "Nothing to free in passmaskdestroy\n");
}
struct passmask * passmaskupdate (struct passmask * mask, int type, int n)
{
    switch (type) {
        case digit:
            mask->digits = n;
            if (!(mask->types & digit))
                mask->types += digit;
            break;
        case lower:
            mask->lowers = n;
            if (!(mask->types & lower))
                mask->types += lower;
            break;
        case upper:
            mask->uppers = n;
            if (!(mask->types & upper))
                mask->types += upper;
            break;
        case sign:
            mask->signs = n;
            if (!(mask->types & sign))
                mask->types += sign;
            break;
        default:
            break;
    }
    return mask;
}
void dumpmask(struct passmask * mask) {
    printf("digits - %d\n"
           "lowers - %d\n"
           "uppers - %d\n"
           "signs - %d\n"
           "types - %d\n\n",
           mask->digits, mask->lowers, mask->uppers, mask->signs, mask->types);
}
int createrandmask(struct passmask * mask, int * randmask) {
    int randn = ((mask->types & digit) ? 1 : 0) +
    ((mask->types & lower) ? 1 : 0) +
    ((mask->types & upper) ? 1 : 0) +
    ((mask->types & sign) ? 1 : 0);
    int i = 0;
    
    if(mask->types & digit)
        randmask[i++] = digit;
    if(mask->types & lower)
        randmask[i++] = lower;
    if(mask->types & upper)
        randmask[i++] = upper;
    if(mask->types & sign)
        randmask[i++] = sign;
    
    return getrand(randn);
}
void process(struct memory * m, struct passmask * mask)
{
	int n = 0, i = 0, randn = 0, randtype = 0;
    int randmask[4] = {};
    
	while (mask->types) {
        
        if (m->i >= m->len) {
            m->len *= 2;
            m->p = realloc(m->p, m->len);
        }
        
        randn = createrandmask(mask, randmask);
        randtype = randmask[randn];
        
        switch (randtype) {
            case digit:
                (m->p)[m->i] = getdigit();
                mask->digits--;
                break;
            case lower:
                (m->p)[m->i] = getlower();
                mask->lowers--;
                break;
            case upper:
                (m->p)[m->i] = getupper();
                mask->uppers--;
                break;
            case sign:
                (m->p)[m->i] = getsign();
                mask->signs--;
                break;
            default:
                break;
        }
        if (mask->digits == 0 && mask->types & digit)
            mask->types -= digit;
        if (mask->lowers == 0 && mask->types & lower)
            mask->types -= lower;
        if (mask->uppers == 0 && mask->types & upper)
            mask->types -= upper;
        if (mask->signs == 0 && mask->types & sign)
            mask->types -= sign;
        
        m->i += 1;
	}
}

int main(int argc, char *argv[])
{
    FILE * f;
	unsigned int seed;
	if ((f = fopen("randseed.txt", "r"))) {
		fscanf(f, "%u", &seed);
		fclose(f);
	} else {
		seed = (unsigned int)time(NULL);
	}
	srand(seed);
    
	struct memory m;
	m.len = LEN;
	m.i = 0;
	m.p = malloc(m.len);
    
    struct passmask * mask = passmasknew(mask);
	
	int c;
	while ((c = getopt(argc, argv, "l:u:n:s:hv")) != -1) {
		switch(c) {
			case 'l':
                passmaskupdate(mask, lower, atoi(optarg));
				break;
			case 'u':
                passmaskupdate(mask, upper, atoi(optarg));
				break;
			case 'n':
                passmaskupdate(mask, digit, atoi(optarg));
				break;
			case 's':
                passmaskupdate(mask, sign, atoi(optarg));
				break;
			case 'h':
				printf("%s", help);
				break;
			case 'v':
				printf("%s", version);
				break;
			case '?':
				printf("There Must got some thing wrong\n");
				printf("%s", help);
				return opterr;
			default:
				abort();
		}
	}
    
    process(&m, mask);
    passmaskdestruct(mask);
	
	(m.p)[m.i] = '\0';
	printf("%s\n", m.p);
    
	if ((f = fopen("randseed.txt", "w+")) != NULL) {
		fprintf(f, "%d", rand());
		fclose(f);
	}
    
	return 0;
}

#ifndef __CASS_REG__
#define __CASS_REG__

typedef struct {
	void *p;
	const char *n;
} cass_reg_entry_t;

typedef ARRAY_TYPE(cass_reg_entry_t) cass_reg_t;

int cass_reg_init (cass_reg_t *reg);

int cass_reg_init_size (cass_reg_t *reg, cass_size_t size);

int cass_reg_cleanup (cass_reg_t *reg);

int32_t cass_reg_lookup (cass_reg_t *reg, const char *n);

int32_t cass_reg_find (cass_reg_t *reg, const void *n);

void *cass_reg_get (cass_reg_t *, uint32_t i);


int cass_reg_add (cass_reg_t *, const char *name, void *);

//void cass_reg_del (cass_reg_t *, void *);

#endif


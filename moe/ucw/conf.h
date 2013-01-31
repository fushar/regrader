/*
 *	UCW Library -- Configuration files
 *
 *	(c) 2001--2006 Robert Spalek <robert@ucw.cz>
 *	(c) 2003--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef	_UCW_CONF_H
#define	_UCW_CONF_H

/*** === Data types [[conf_types]] ***/

enum cf_class {				/** Class of the configuration item. **/
  CC_END,				// end of list
  CC_STATIC,				// single variable or static array
  CC_DYNAMIC,				// dynamically allocated array
  CC_PARSER,				// arbitrary parser function
  CC_SECTION,				// section appears exactly once
  CC_LIST,				// list with 0..many nodes
  CC_BITMAP				// of up to 32 items
};

enum cf_type {				/** Type of a single value. **/
  CT_INT, CT_U64, CT_DOUBLE,		// number types
  CT_IP,				// IP address
  CT_STRING,				// string type
  CT_LOOKUP,				// in a string table
  CT_USER				// user-defined type
};

struct fastbuf;

/**
 * A parser function gets an array of (strdup'ed) strings and a pointer with
 * the customized information (most likely the target address).  It can store
 * the parsed value anywhere in any way it likes, however it must first call
 * @cf_journal_block() on the overwritten memory block.  It returns an error
 * message or NULL if everything is all right.
 **/
typedef char *cf_parser(uns number, char **pars, void *ptr);
/**
 * A parser function for user-defined types gets a string and a pointer to
 * the destination variable.  It must store the value within [ptr,ptr+size),
 * where size is fixed for each type.  It should not call @cf_journal_block().
 **/
typedef char *cf_parser1(char *string, void *ptr);
/**
 * An init- or commit-hook gets a pointer to the section or NULL if this
 * is the global section.  It returns an error message or NULL if everything
 * is all right.  The init-hook should fill in default values (needed for
 * dynamically allocated nodes of link lists or for filling global variables
 * that are run-time dependent).  The commit-hook should perform sanity
 * checks and postprocess the parsed values.  Commit-hooks must call
 * @cf_journal_block() too.  Caveat! init-hooks for static sections must not
 * use @cf_malloc() but normal <<memory:xmalloc()>>.
 **/
typedef char *cf_hook(void *ptr);
/**
 * Dumps the contents of a variable of a user-defined type.
 **/
typedef void cf_dumper1(struct fastbuf *fb, void *ptr);
/**
 * Similar to init-hook, but it copies attributes from another list node
 * instead of setting the attributes to default values.  You have to provide
 * it if your node contains parsed values and/or sub-lists.
 **/
typedef char *cf_copier(void *dest, void *src);

struct cf_user_type {			/** Structure to store information about user-defined variable type. **/
  uns size;				// of the parsed attribute
  char *name;				// name of the type (for dumping)
  cf_parser1 *parser;			// how to parse it
  cf_dumper1 *dumper;			// how to dump the type
};

struct cf_section;
struct cf_item {			/** Single configuration item. **/
  const char *name;			// case insensitive
  int number;				// length of an array or #parameters of a parser (negative means at most)
  void *ptr;				// pointer to a global variable or an offset in a section
  union cf_union {
    struct cf_section *sec;		// declaration of a section or a list
    cf_parser *par;			// parser function
    const char * const *lookup;		// NULL-terminated sequence of allowed strings for lookups
    struct cf_user_type *utype;		// specification of the user-defined type
  } u;
  enum cf_class cls:16;			// attribute class
  enum cf_type type:16;			// type of a static or dynamic attribute
};

struct cf_section {			/** A section. **/
  uns size;				// 0 for a global block, sizeof(struct) for a section
  cf_hook *init;			// fills in default values (no need to bzero)
  cf_hook *commit;			// verifies parsed data (optional)
  cf_copier *copy;			// copies values from another instance (optional, no need to copy basic attributes)
  struct cf_item *cfg;			// CC_END-terminated array of items
  uns flags;				// for internal use only
};

/***
 * [[conf_macros]]
 * Convenience macros
 * ~~~~~~~~~~~~~~~~~~
 *
 * You could create the structures manually, but you can use these macros to
 * save some typing.
 */

/***
 * Declaration of <<struct_cf_section,`cf_section`>>
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * These macros can be used to configure the <<struct_cf_section,`cf_section`>>
 * structure.
 ***/

/**
 * Data type of a section.
 * If you store the section into a structure, use this macro.
 *
 * Storing a section into a structure is useful mostly when you may have multiple instances of the
 * section (eg. <<conf_multi,array or list>>).
 *
 * Example:
 *
 *   struct list_node {
 *     cnode n;		// This one is for the list itself
 *     char *name;
 *     uns value;
 *   };
 *
 *   static struct clist nodes;
 *
 *   static struct cf_section node = {
 *     CF_TYPE(struct list_node),
 *     CF_ITEMS {
 *       CF_STRING("name", PTR_TO(struct list_node, name)),
 *       CF_UNS("value", PTR_TO(struct list_node, value)),
 *       CF_END
 *     }
 *   };
 *
 *   static struct cf_section section = {
 *     CF_LIST("node", &nodes, &node),
 *     CF_END
 *   };
 *
 * You could use <<def_CF_STATIC,`CF_STATIC`>> or <<def_CF_DYNAMIC,`CF_DYNAMIC`>>
 * macros to create arrays.
 */
#define CF_TYPE(s)	.size = sizeof(s)
/**
 * An init <<hooks,hook>>.
 * You can use this to initialize dynamically allocated items (for a dynamic array or list).
 * The hook returns an error message or NULL if everything was OK.
 */
#define CF_INIT(f)	.init = (cf_hook*) f
/**
 * A commit <<hooks,hook>>.
 * You can use this one to check sanity of loaded data and postprocess them.
 * You must call @cf_journal_block() if you change anything.
 *
 * Return error message or NULL if everything went OK.
 **/
#define CF_COMMIT(f)	.commit = (cf_hook*) f
/**
 * A <<hooks,copy function>>.
 * You need to provide one for too complicated sections where a memcpy is not
 * enough to copy it properly. It happens, for example, when you have a dynamically
 * allocated section containing a list of other sections.
 *
 * You return an error message or NULL if you succeed.
 **/
#define CF_COPY(f)	.copy = (cf_copier*) f		/**  **/
#define CF_ITEMS	.cfg = ( struct cf_item[] )	/** List of sub-items. **/
#define CF_END		{ .cls = CC_END }		/** End of the structure. **/
/***
 * Declaration of a configuration item
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * Each of these describe single <<struct_cf_item,configuration item>>. They are mostly
 * for internal use, do not use them directly unless you really know what you are doing.
 ***/

/**
 * Static array of items.
 * Expects you to allocate the memory and provide pointer to it.
 **/
#define CF_STATIC(n,p,T,t,c)	{ .cls = CC_STATIC, .type = CT_##T, .name = n, .number = c, .ptr = CHECK_PTR_TYPE(p,t*) }
/**
 * Dynamic array of items.
 * Expects you to provide pointer to your pointer to data and it will allocate new memory for it
 * and set your pointer to it.
 **/
#define CF_DYNAMIC(n,p,T,t,c)	{ .cls = CC_DYNAMIC, .type = CT_##T, .name = n, .number = c, .ptr = CHECK_PTR_TYPE(p,t**) }
#define CF_PARSER(n,p,f,c)	{ .cls = CC_PARSER, .name = n, .number = c, .ptr = p, .u.par = (cf_parser*) f }					/** A low-level parser. **/
#define CF_SECTION(n,p,s)	{ .cls = CC_SECTION, .name = n, .number = 1, .ptr = p, .u.sec = s }						/** A sub-section. **/
#define CF_LIST(n,p,s)		{ .cls = CC_LIST, .name = n, .number = 1, .ptr = CHECK_PTR_TYPE(p,clist*), .u.sec = s }				/** A list with sub-items. **/
#define CF_BITMAP_INT(n,p)	{ .cls = CC_BITMAP, .type = CT_INT, .name = n, .number = 1, .ptr = CHECK_PTR_TYPE(p,u32*) }			/** A bitmap. **/
#define CF_BITMAP_LOOKUP(n,p,t)	{ .cls = CC_BITMAP, .type = CT_LOOKUP, .name = n, .number = 1, .ptr = CHECK_PTR_TYPE(p,u32*), .u.lookup = t }	/** A bitmap with named bits. **/
/***
 * Basic configuration items
 * ^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * They describe basic data types used in the configuration. This should be enough for
 * most real-life purposes.
 *
 * The parameters are as follows:
 *
 * * @n -- name of the item.
 * * @p -- pointer to the variable where it shall be stored.
 * * @c -- count.
 **/
#define CF_INT(n,p)		CF_STATIC(n,p,INT,int,1)		/** Single `int` value. **/
#define CF_INT_ARY(n,p,c)	CF_STATIC(n,p,INT,int,c)		/** Static array of integers. **/
#define CF_INT_DYN(n,p,c)	CF_DYNAMIC(n,p,INT,int,c)		/** Dynamic array of integers. **/
#define CF_UNS(n,p)		CF_STATIC(n,p,INT,uns,1)		/** Single `uns` (`unsigned`) value. **/
#define CF_UNS_ARY(n,p,c)	CF_STATIC(n,p,INT,uns,c)		/** Static array of unsigned integers. **/
#define CF_UNS_DYN(n,p,c)	CF_DYNAMIC(n,p,INT,uns,c)		/** Dynamic array of unsigned integers. **/
#define CF_U64(n,p)		CF_STATIC(n,p,U64,u64,1)		/** Single unsigned 64bit integer (`u64`). **/
#define CF_U64_ARY(n,p,c)	CF_STATIC(n,p,U64,u64,c)		/** Static array of u64s. **/
#define CF_U64_DYN(n,p,c)	CF_DYNAMIC(n,p,U64,u64,c)		/** Dynamic array of u64s. **/
#define CF_DOUBLE(n,p)		CF_STATIC(n,p,DOUBLE,double,1)		/** Single instance of `double`. **/
#define CF_DOUBLE_ARY(n,p,c)	CF_STATIC(n,p,DOUBLE,double,c)		/** Static array of doubles. **/
#define CF_DOUBLE_DYN(n,p,c)	CF_DYNAMIC(n,p,DOUBLE,double,c)		/** Dynamic array of doubles. **/
#define CF_IP(n,p)		CF_STATIC(n,p,IP,u32,1)			/** Single IPv4 address. **/
#define CF_IP_ARY(n,p,c)	CF_STATIC(n,p,IP,u32,c)			/** Static array of IP addresses. **/.
#define CF_IP_DYN(n,p,c)	CF_DYNAMIC(n,p,IP,u32,c)		/** Dynamic array of IP addresses. **/
/**
 * A string.
 * You provide a pointer to a `char *` variable and it will fill it with
 * dynamically allocated string. For example:
 *
 *   static char *string = "Default string";
 *
 *   static struct cf_section section = {
 *     CF_ITEMS {
 *       CF_STRING("string", &string),
 *       CF_END
 *     }
 *   };
 **/
#define CF_STRING(n,p)		CF_STATIC(n,p,STRING,char*,1)
#define CF_STRING_ARY(n,p,c)	CF_STATIC(n,p,STRING,char*,c)		/** Static array of strings. **/
#define CF_STRING_DYN(n,p,c)	CF_DYNAMIC(n,p,STRING,char*,c)		/** Dynamic array of strings. **/
/**
 * One string out of a predefined set.
 * You provide the set as an array of strings terminated by NULL (similar to @argv argument
 * of main()) as the @t parameter.
 *
 * The configured variable (pointer to `int`) is set to index of the string.
 * So, it works this way:
 *
 *   static *strings[] = { "First", "Second", "Third", NULL };
 *
 *   static int variable;
 *
 *   static struct cf_section section = {
 *     CF_ITEMS {
 *       CF_LOOKUP("choice", &variable, strings),
 *       CF_END
 *     }
 *   };
 *
 * Now, if the configuration contains `choice "Second"`, `variable` will be set to 1.
 **/
#define CF_LOOKUP(n,p,t)	{ .cls = CC_STATIC, .type = CT_LOOKUP, .name = n, .number = 1, .ptr = CHECK_PTR_TYPE(p,int*), .u.lookup = t }
/**
 * Static array of strings out of predefined set.
 **/
#define CF_LOOKUP_ARY(n,p,t,c)	{ .cls = CC_STATIC, .type = CT_LOOKUP, .name = n, .number = c, .ptr = CHECK_PTR_TYPE(p,int*), .u.lookup = t }
/**
 * Dynamic array of strings out of predefined set.
 **/
#define CF_LOOKUP_DYN(n,p,t,c)	{ .cls = CC_DYNAMIC, .type = CT_LOOKUP, .name = n, .number = c, .ptr = CHECK_PTR_TYPE(p,int**), .u.lookup = t }
/**
 * A user-defined type.
 * See <<custom_parser,creating custom parsers>> section if you want to know more.
 **/
#define CF_USER(n,p,t)		{ .cls = CC_STATIC, .type = CT_USER, .name = n, .number = 1, .ptr = p, .u.utype = t }
/**
 * Static array of user-defined types (all of the same type).
 * See <<custom_parser,creating custom parsers>> section.
 **/
#define CF_USER_ARY(n,p,t,c)	{ .cls = CC_STATIC, .type = CT_USER, .name = n, .number = c, .ptr = p, .u.utype = t }
/**
 * Dynamic array of user-defined types.
 * See <<custom_parser,creating custom parsers>> section.
 **/
#define CF_USER_DYN(n,p,t,c)	{ .cls = CC_DYNAMIC, .type = CT_USER, .name = n, .number = c, .ptr = p, .u.utype = t }

/**
 * Any number of dynamic array elements
 **/
#define CF_ANY_NUM		-0x7fffffff

#define DARY_LEN(a) ((uns*)a)[-1]	/** Length of an dynamic array. **/
#define DARY_ALLOC(type,len,val...) ((struct { uns l; type a[len]; }) { .l = len, .a = { val } }).a
  // creates a static instance of a dynamic array

/***
 * [[alloc]]
 * Memory allocation
 * ~~~~~~~~~~~~~~~~~
 *
 * Uses <<mempool:,memory pools>> for efficiency and journal recovery.
 * You should use these routines when implementing custom parsers.
 ***/
struct mempool;
/**
 * A <<mempool:type_mempool,memory pool>> for configuration parser needs.
 * Memory allocated from here is valid as long as the current config is loaded
 * (if you allocate some memory and rollback the transaction or you load some
 * other configuration, it gets lost).
 **/
extern struct mempool *cf_pool;
void *cf_malloc(uns size);	/** Returns @size bytes of memory. Allocates from <<var_cf_pool,`cf_pool`>>. **/
void *cf_malloc_zero(uns size);	/** Like @cf_malloc(), but zeroes the memory. **/
char *cf_strdup(const char *s);	/** Copy a string into @cf_malloc()ed memory. **/
char *cf_printf(const char *fmt, ...) FORMAT_CHECK(printf,1,2); /** printf() into @cf_malloc()ed memory. **/

/***
 * [[journal]]
 * Undo journal
 * ~~~~~~~~~~~~
 *
 * For error recovery when <<reload,reloading configuration>>.
 ***/
extern uns cf_need_journal;	/** Is the journal needed? If you do not reload configuration, you set this to 0 and gain a little more performance and free memory. **/
/**
 * When a block of memory is about to be changed, put the old value
 * into journal with this function. You need to call it from a <<hooks,commit hook>>
 * if you change anything. It is used internally by low-level parsers.
 * <<custom_parser,Custom parsers>> do not need to call it, it is called
 * before them.
 **/
void cf_journal_block(void *ptr, uns len);
#define CF_JOURNAL_VAR(var) cf_journal_block(&(var), sizeof(var))	// Store single value into journal.

/***
 * [[declare]]
 * Section declaration
 * ~~~~~~~~~~~~~~~~~~~
 **/

/**
 * Plug another top-level section into the configuration system.
 * @name is the name in the configuration file,
 * @sec is pointer to the section description.
 * If @allow_unknown is set to 0 and a variable not described in @sec
 * is found in the configuration file, it produces an error.
 * If you set it to 1, all such variables are ignored.
 **/
void cf_declare_section(const char *name, struct cf_section *sec, uns allow_unknown);
/**
 * If you have a section in a structure and you want to initialize it
 * (eg. if you want a copy of default values outside the configuration),
 * you can use this. It initializes it recursively.
 *
 * This is used mostly internally. You probably do not need it.
 **/
void cf_init_section(const char *name, struct cf_section *sec, void *ptr, uns do_bzero);

/***
 * [[bparser]]
 * Parsers for basic types
 * ~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Each of them gets a string to parse and pointer to store the value.
 * It returns either NULL or error message.
 *
 * The parsers support units. See <<config:units,their list>>.
 ***/
char *cf_parse_int(const char *str, int *ptr);		/** Parser for integers. **/
char *cf_parse_u64(const char *str, u64 *ptr);		/** Parser for 64 unsigned integers. **/
char *cf_parse_double(const char *str, double *ptr);	/** Parser for doubles. **/
char *cf_parse_ip(const char *p, u32 *varp);		/** Parser for IP addresses. **/

#endif


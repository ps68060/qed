#ifndef hilite_h
#define hilite_h

/* cache line flags; originally intended for easy use
 * with GEM VDI, change these for your own purposes, if
 * needed (within a HL_ELEM)
 */
#define HL_BOLD 1         /* set bold text fx */
#define HL_LIGHT 2        /* set light text fx */
#define HL_ITALIC 4       /* set italic text fx */
#define HL_COLOR 0x20     /* set color */
#define HL_SELCOLOR 0x40  /* set selected color */
#define HL_CACHEEND 0x80  /* end of the cache line */

typedef void * HL_HANDLE;        /* Handle of a complete Text */
typedef void * HL_LINEHANDLE;    /* Handle of a line */
typedef unsigned char HL_ELEM;   /* one element of a cache line - change it if you need sth bigger */
typedef HL_ELEM * HL_LINE;       /* Cache line */

/* rule info; for getting and changing rule */
typedef struct {
	char *name;        /* rule name, ignored when setting (Hl_ChangeRule()) */
	HL_ELEM attribs;   /* text attributes and HL_COLOR, HL_SELCOLOR if colors valid */
	long color;        /* text color */
	long selcolor;     /* selected text color */
} HL_RULEINFO;

/* error numbers */
typedef enum {
	E_HL_WRONGFILE,    /* syntax file of wrong type */ 
	E_HL_MEMORY,       /* memory error */
	E_HL_TOFROM,       /* "to" or "while" keyword before "from" */
	E_HL_MIXED,        /* "from"-"to"/"while" and "keyword" mixed */        
	E_HL_SYNTAX,       /* syntax error */
	E_HL_WRONGVAL,     /* wrong numerical value */
	E_HL_UNKNTEXT,     /* unknown text when reading settings only */
	E_HL_UNKNRULE,     /* unknown rule when reading settings only */
	E_HL_DUPTEXT       /* doubly defined text definition */
} HL_ERRTYPE;



/* chained list string */
typedef struct stringentry {
	int len;
	char *name;
	struct stringentry *next;
} STRINGENTRY;


/* rule types */
typedef enum {
	RULE_KEYWORD,  /* rule is a keyword, e.g. C keywords: <if>,<while>, ... */
	RULE_FROM,     /* start of a to- or while-rule, e.g. Pascal-Comment: "(*", c-hex-constants: "0x"  */
	RULE_TO,       /* to-rule (eg. Pascal-Comment: "*)") */
	RULE_WHILE     /* while-rule (e.g. c-hex-constants: <0-9>,<a-f>) */
} RULETYPE;


/* style struct; attribs and colors */
typedef struct {
	HL_ELEM attribs;
	long color;
	long selcolor;
} HL_STYLEINFO;


/* resolution type */
typedef enum {
	COLOR2,
	COLOR16,
	COLOR256,
	NUM_RESTYPES
} RESTYPE;

/* text rule flags */
#define TXTRULEF_CASE 1      /* text is case-sensitive */
#define TXTRULEF_ACTIVE 2

#ifndef BOOLEAN
typedef short BOOLEAN;
#endif

/* a rule; description for detecting keywords/chars and setting the corresponding colors and flags */
typedef struct rule {
	char *name;                       /* name of the rule, e.g. "Comment" */
	BOOLEAN dup;                      /* duplicate rule; another rule of the same name already exists, only the attributes of the 1st rule are valid */
	RULETYPE type;                    /* rule type, RULE_... */
	int flags;                        /* rule flags, RULEF_... */
	HL_STYLEINFO style[NUM_RESTYPES]; /* attributes & colors for different resolutions */
	HL_ELEM attribs;                  /* actual text attributes */
	long color;                       /* actual text color */
	long selcolor;                    /* actual selected text color */
	STRINGENTRY *kwstring[256];       /* all strings to be searched for, indexed by the first char */
	char kwsinglechar[256];           /* all single chars of this rule */
	char kwchar[256];                 /* all beginning chars of kwsinglechar and kwstring (pre filter) */
	unsigned char quotechar;          /* quote char, as e.g. \ in C strings */
	struct rule *link;                /* stoprule, if type == RULE_FROM, or link to startrule if nested rule */
	struct rule *next;                /* next rule struct */
} RULE;

/* a cache entry */
typedef struct cacheline {
	char *line;                  /* source text row */
	RULE *startrule;             /* rule which is active at the beginning of the row */
	RULE *endrule;               /* rule which is active at the end of the row */
	HL_LINE cachetok;            /* the encoded text format */
	struct cacheline *next;      /* next cache entry */
	int startcount;              /* for nested rules: count at the start of the cache line */
	int endcount;                /* count at the end of the cache line */
} CACHE;

/* Rule for a text type, e.g. "c"/"h", "pas", "s"... */
typedef struct txtrule {
	char *name;                  /* name of the text type, e.g. "C source" */
	char *filename;              /* filename of this rule */
	STRINGENTRY *txttypes;       /* txttypes of the text type, e.g "c" and "h" for c files */
	char token[256];             /* all chars this text type uses in its rules */
	char kwstartchar[256];       /* pre filter for start rules */
	char kwendchar[256];         /* pre filter for end rules */
	int flags;                   /* text flags, TXTRULEF_... */
	RULE *rules;                 /* the rules for this text type */
	BOOLEAN checked;             /* this textrule is checked already in check_rules() */
	struct txtrule *next;        /* next text type */
} TXTRULE;

/* the base for the cache of a text */
typedef struct cacheb {
	CACHE *cache;                /* cache entries */
	TXTRULE *txtrule;            /* text rule (is it c, pascal...? */
	struct cacheb *next;
} CACHEBASE;

/* init and exit routines */
void Hl_Init( void (*err_callback)( char *currfile, HL_ERRTYPE err, int linenr ) );
void Hl_Exit( void );

static CACHEBASE *cacheanchor = NULL; /* anchor for syntax cache */

/* read and write syntax file */
int Hl_ReadSyn( char *filename, int curr_planes, int settingsonly );
int Hl_WriteSyn( char *filename, int settingsonly );

/* handle syntax cache (main functions) */
HL_HANDLE     Hl_New( char *txttype, int resvd );
long Hl_InsertLine( HL_HANDLE anchor, HL_LINEHANDLE prev, char *line,
                    HL_LINEHANDLE *inserted, int update );
long Hl_Update( HL_HANDLE anchor, HL_LINEHANDLE handle, char *line, int updatenext );
long Hl_RemoveLine( HL_HANDLE anchor, HL_LINEHANDLE prev, HL_LINEHANDLE *curr, int update );
HL_LINE Hl_GetLine( HL_LINEHANDLE handle );
void Hl_Free( HL_HANDLE anchor );

/* get, set and temporarily save settings */
int Hl_EnumTxtNames( char **txtname, int *idx );
int Hl_EnumTxtTypes( int txtidx, char **txttype, int *idx );
int Hl_EnumRules(  int txtidx, HL_RULEINFO *ruleinfo, int *idx );
int Hl_IsActive( int txtidx );
void Hl_SetActive( int txtidx, int active );
void Hl_RestoreSettings( void );
void Hl_DeleteSaveSettings( void );
int Hl_SaveSettings( void );
int Hl_ChangeRule( int txtidx, int idx, HL_RULEINFO *ri );
int Hl_TxtIndexByTxttype( char *txttype );

#endif
/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 15 "gram.y"

#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "defaults.h"

#define YYERROR_VERBOSE 1

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ADD_TO_LL(type, list, value) \
	do { \
		if (iface->list == NULL) \
			iface->list = value; \
		else { \
			type *current = iface->list; \
			while (current->next != NULL) \
				current = current->next; \
			current->next = value; \
		} \
	} while (0)



/* Line 268 of yacc.c  */
#line 103 "gram.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_INTERFACE = 258,
     T_PREFIX = 259,
     T_ROUTE = 260,
     T_RDNSS = 261,
     T_DNSSL = 262,
     T_CLIENTS = 263,
     T_LOWPANCO = 264,
     T_ABRO = 265,
     T_RASRCADDRESS = 266,
     STRING = 267,
     NUMBER = 268,
     SIGNEDNUMBER = 269,
     DECIMAL = 270,
     SWITCH = 271,
     IPV6ADDR = 272,
     INFINITY = 273,
     T_IgnoreIfMissing = 274,
     T_AdvSendAdvert = 275,
     T_MaxRtrAdvInterval = 276,
     T_MinRtrAdvInterval = 277,
     T_MinDelayBetweenRAs = 278,
     T_AdvManagedFlag = 279,
     T_AdvOtherConfigFlag = 280,
     T_AdvLinkMTU = 281,
     T_AdvRAMTU = 282,
     T_AdvReachableTime = 283,
     T_AdvRetransTimer = 284,
     T_AdvCurHopLimit = 285,
     T_AdvDefaultLifetime = 286,
     T_AdvDefaultPreference = 287,
     T_AdvSourceLLAddress = 288,
     T_AdvOnLink = 289,
     T_AdvAutonomous = 290,
     T_AdvValidLifetime = 291,
     T_AdvPreferredLifetime = 292,
     T_DeprecatePrefix = 293,
     T_DecrementLifetimes = 294,
     T_AdvRouterAddr = 295,
     T_AdvHomeAgentFlag = 296,
     T_AdvIntervalOpt = 297,
     T_AdvHomeAgentInfo = 298,
     T_Base6Interface = 299,
     T_Base6to4Interface = 300,
     T_UnicastOnly = 301,
     T_AdvRASolicitedUnicast = 302,
     T_HomeAgentPreference = 303,
     T_HomeAgentLifetime = 304,
     T_AdvRoutePreference = 305,
     T_AdvRouteLifetime = 306,
     T_RemoveRoute = 307,
     T_AdvRDNSSPreference = 308,
     T_AdvRDNSSOpenFlag = 309,
     T_AdvRDNSSLifetime = 310,
     T_FlushRDNSS = 311,
     T_AdvDNSSLLifetime = 312,
     T_FlushDNSSL = 313,
     T_AdvMobRtrSupportFlag = 314,
     T_AdvContextLength = 315,
     T_AdvContextCompressionFlag = 316,
     T_AdvContextID = 317,
     T_AdvLifeTime = 318,
     T_AdvContextPrefix = 319,
     T_AdvVersionLow = 320,
     T_AdvVersionHigh = 321,
     T_AdvValidLifeTime = 322,
     T_Adv6LBRaddress = 323,
     T_BAD_TOKEN = 324
   };
#endif
/* Tokens.  */
#define T_INTERFACE 258
#define T_PREFIX 259
#define T_ROUTE 260
#define T_RDNSS 261
#define T_DNSSL 262
#define T_CLIENTS 263
#define T_LOWPANCO 264
#define T_ABRO 265
#define T_RASRCADDRESS 266
#define STRING 267
#define NUMBER 268
#define SIGNEDNUMBER 269
#define DECIMAL 270
#define SWITCH 271
#define IPV6ADDR 272
#define INFINITY 273
#define T_IgnoreIfMissing 274
#define T_AdvSendAdvert 275
#define T_MaxRtrAdvInterval 276
#define T_MinRtrAdvInterval 277
#define T_MinDelayBetweenRAs 278
#define T_AdvManagedFlag 279
#define T_AdvOtherConfigFlag 280
#define T_AdvLinkMTU 281
#define T_AdvRAMTU 282
#define T_AdvReachableTime 283
#define T_AdvRetransTimer 284
#define T_AdvCurHopLimit 285
#define T_AdvDefaultLifetime 286
#define T_AdvDefaultPreference 287
#define T_AdvSourceLLAddress 288
#define T_AdvOnLink 289
#define T_AdvAutonomous 290
#define T_AdvValidLifetime 291
#define T_AdvPreferredLifetime 292
#define T_DeprecatePrefix 293
#define T_DecrementLifetimes 294
#define T_AdvRouterAddr 295
#define T_AdvHomeAgentFlag 296
#define T_AdvIntervalOpt 297
#define T_AdvHomeAgentInfo 298
#define T_Base6Interface 299
#define T_Base6to4Interface 300
#define T_UnicastOnly 301
#define T_AdvRASolicitedUnicast 302
#define T_HomeAgentPreference 303
#define T_HomeAgentLifetime 304
#define T_AdvRoutePreference 305
#define T_AdvRouteLifetime 306
#define T_RemoveRoute 307
#define T_AdvRDNSSPreference 308
#define T_AdvRDNSSOpenFlag 309
#define T_AdvRDNSSLifetime 310
#define T_FlushRDNSS 311
#define T_AdvDNSSLLifetime 312
#define T_FlushDNSSL 313
#define T_AdvMobRtrSupportFlag 314
#define T_AdvContextLength 315
#define T_AdvContextCompressionFlag 316
#define T_AdvContextID 317
#define T_AdvLifeTime 318
#define T_AdvContextPrefix 319
#define T_AdvVersionLow 320
#define T_AdvVersionHigh 321
#define T_AdvValidLifeTime 322
#define T_Adv6LBRaddress 323
#define T_BAD_TOKEN 324




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 138 "gram.y"

	unsigned int		num;
	int			snum;
	double			dec;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
	struct AdvRDNSS		*rdnssinfo;
	struct AdvDNSSL		*dnsslinfo;
	struct Clients		*ainfo;
	struct AdvLowpanCo	*lowpancoinfo;
	struct AdvAbro		*abroinfo;
	struct AdvRASrcAddress	*rasrcaddressinfo;



/* Line 293 of yacc.c  */
#line 295 "gram.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 343 of yacc.c  */
#line 154 "gram.y"

extern int num_lines;
static char const * filename;
static struct Interface *iface;
static struct Interface *IfaceList;
static struct AdvPrefix *prefix;
static struct AdvRoute *route;
static struct AdvRDNSS *rdnss;
static struct AdvDNSSL *dnssl;
static struct AdvLowpanCo *lowpanco;
static struct AdvAbro  *abro;
static void cleanup(void);
#define ABORT	do { cleanup(); YYABORT; } while (0);
static void yyerror(char const * msg);


/* Line 343 of yacc.c  */
#line 324 "gram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   248

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  49
/* YYNRULES -- Number of rules.  */
#define YYNRULES  122
/* YYNRULES -- Number of states.  */
#define YYNSTATES  267

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    72,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,     2,    71,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    14,    17,    19,    22,    23,
      25,    27,    29,    31,    33,    35,    37,    39,    41,    45,
      49,    53,    57,    61,    65,    69,    73,    77,    81,    85,
      89,    93,    97,   101,   105,   109,   113,   117,   121,   125,
     129,   133,   137,   141,   145,   151,   154,   158,   164,   167,
     171,   175,   180,   181,   184,   188,   191,   193,   197,   201,
     205,   209,   213,   217,   221,   225,   229,   235,   240,   241,
     243,   246,   248,   252,   256,   260,   266,   269,   271,   273,
     276,   277,   279,   282,   284,   288,   292,   296,   300,   306,
     309,   311,   313,   316,   317,   319,   322,   324,   328,   332,
     338,   340,   341,   343,   346,   348,   352,   356,   360,   364,
     370,   372,   374,   377,   382,   383,   385,   388,   390,   394,
     398,   402,   404
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      75,     0,    -1,    75,    76,    -1,    76,    -1,    77,    70,
      79,    71,    72,    -1,     3,    78,    -1,    12,    -1,    79,
      80,    -1,    -1,    81,    -1,    86,    -1,    82,    -1,    91,
      -1,    96,    -1,   103,    -1,   110,    -1,   115,    -1,    84,
      -1,    22,    13,    72,    -1,    21,    13,    72,    -1,    23,
      13,    72,    -1,    22,    15,    72,    -1,    21,    15,    72,
      -1,    23,    15,    72,    -1,    19,    16,    72,    -1,    20,
      16,    72,    -1,    24,    16,    72,    -1,    25,    16,    72,
      -1,    26,    13,    72,    -1,    27,    13,    72,    -1,    28,
      13,    72,    -1,    29,    13,    72,    -1,    31,    13,    72,
      -1,    32,    14,    72,    -1,    30,    13,    72,    -1,    33,
      16,    72,    -1,    42,    16,    72,    -1,    43,    16,    72,
      -1,    41,    16,    72,    -1,    48,    13,    72,    -1,    49,
      13,    72,    -1,    46,    16,    72,    -1,    47,    16,    72,
      -1,    59,    16,    72,    -1,     8,    70,    83,    71,    72,
      -1,    17,    72,    -1,    83,    17,    72,    -1,    11,    70,
      85,    71,    72,    -1,    17,    72,    -1,    85,    17,    72,
      -1,    87,    88,    72,    -1,     4,    17,    73,    13,    -1,
      -1,    70,    71,    -1,    70,    89,    71,    -1,    89,    90,
      -1,    90,    -1,    34,    16,    72,    -1,    35,    16,    72,
      -1,    40,    16,    72,    -1,    36,   122,    72,    -1,    37,
     122,    72,    -1,    38,    16,    72,    -1,    39,    16,    72,
      -1,    44,    78,    72,    -1,    45,    78,    72,    -1,    92,
      70,    93,    71,    72,    -1,     5,    17,    73,    13,    -1,
      -1,    94,    -1,    94,    95,    -1,    95,    -1,    50,    14,
      72,    -1,    51,   122,    72,    -1,    52,    16,    72,    -1,
      99,    70,   100,    71,    72,    -1,    97,    98,    -1,    98,
      -1,    17,    -1,     6,    97,    -1,    -1,   101,    -1,   101,
     102,    -1,   102,    -1,    53,    13,    72,    -1,    54,    16,
      72,    -1,    55,   122,    72,    -1,    56,    16,    72,    -1,
     106,    70,   107,    71,    72,    -1,   104,   105,    -1,   105,
      -1,    12,    -1,     7,   104,    -1,    -1,   108,    -1,   108,
     109,    -1,   109,    -1,    57,   122,    72,    -1,    58,    16,
      72,    -1,   111,    70,   112,    71,    72,    -1,     9,    -1,
      -1,   113,    -1,   113,   114,    -1,   114,    -1,    60,    13,
      72,    -1,    61,    16,    72,    -1,    62,    13,    72,    -1,
      63,    13,    72,    -1,   116,    70,   119,    71,    72,    -1,
     117,    -1,   118,    -1,    10,    17,    -1,    10,    17,    73,
      13,    -1,    -1,   120,    -1,   120,   121,    -1,   121,    -1,
      65,    13,    72,    -1,    66,    13,    72,    -1,    67,    13,
      72,    -1,    13,    -1,    18,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   174,   177,   187,   216,   223,   224,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   238,   242,
     246,   250,   254,   258,   262,   266,   270,   274,   278,   282,
     288,   292,   296,   300,   304,   308,   312,   316,   320,   324,
     328,   332,   336,   340,   346,   352,   363,   377,   383,   394,
     408,   434,   463,   464,   465,   468,   469,   472,   478,   484,
     490,   497,   504,   510,   516,   530,   545,   553,   577,   578,
     581,   582,   586,   590,   594,   600,   607,   608,   611,   646,
     655,   656,   659,   660,   664,   668,   672,   687,   693,   700,
     701,   704,   746,   755,   756,   759,   760,   764,   776,   782,
     789,   802,   803,   806,   807,   810,   814,   818,   822,   828,
     835,   835,   837,   851,   871,   872,   875,   876,   879,   883,
     887,   893,   897
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_INTERFACE", "T_PREFIX", "T_ROUTE",
  "T_RDNSS", "T_DNSSL", "T_CLIENTS", "T_LOWPANCO", "T_ABRO",
  "T_RASRCADDRESS", "STRING", "NUMBER", "SIGNEDNUMBER", "DECIMAL",
  "SWITCH", "IPV6ADDR", "INFINITY", "T_IgnoreIfMissing", "T_AdvSendAdvert",
  "T_MaxRtrAdvInterval", "T_MinRtrAdvInterval", "T_MinDelayBetweenRAs",
  "T_AdvManagedFlag", "T_AdvOtherConfigFlag", "T_AdvLinkMTU", "T_AdvRAMTU",
  "T_AdvReachableTime", "T_AdvRetransTimer", "T_AdvCurHopLimit",
  "T_AdvDefaultLifetime", "T_AdvDefaultPreference", "T_AdvSourceLLAddress",
  "T_AdvOnLink", "T_AdvAutonomous", "T_AdvValidLifetime",
  "T_AdvPreferredLifetime", "T_DeprecatePrefix", "T_DecrementLifetimes",
  "T_AdvRouterAddr", "T_AdvHomeAgentFlag", "T_AdvIntervalOpt",
  "T_AdvHomeAgentInfo", "T_Base6Interface", "T_Base6to4Interface",
  "T_UnicastOnly", "T_AdvRASolicitedUnicast", "T_HomeAgentPreference",
  "T_HomeAgentLifetime", "T_AdvRoutePreference", "T_AdvRouteLifetime",
  "T_RemoveRoute", "T_AdvRDNSSPreference", "T_AdvRDNSSOpenFlag",
  "T_AdvRDNSSLifetime", "T_FlushRDNSS", "T_AdvDNSSLLifetime",
  "T_FlushDNSSL", "T_AdvMobRtrSupportFlag", "T_AdvContextLength",
  "T_AdvContextCompressionFlag", "T_AdvContextID", "T_AdvLifeTime",
  "T_AdvContextPrefix", "T_AdvVersionLow", "T_AdvVersionHigh",
  "T_AdvValidLifeTime", "T_Adv6LBRaddress", "T_BAD_TOKEN", "'{'", "'}'",
  "';'", "'/'", "$accept", "grammar", "ifacedef", "ifacehead", "name",
  "ifaceparams", "ifaceparam", "ifaceval", "clientslist",
  "v6addrlist_clients", "rasrcaddresslist", "v6addrlist_rasrcaddress",
  "prefixdef", "prefixhead", "optional_prefixplist", "prefixplist",
  "prefixparms", "routedef", "routehead", "optional_routeplist",
  "routeplist", "routeparms", "rdnssdef", "rdnssaddrs", "rdnssaddr",
  "rdnsshead", "optional_rdnssplist", "rdnssplist", "rdnssparms",
  "dnssldef", "dnsslsuffixes", "dnsslsuffix", "dnsslhead",
  "optional_dnsslplist", "dnsslplist", "dnsslparms", "lowpancodef",
  "lowpancohead", "optional_lowpancoplist", "lowpancoplist",
  "lowpancoparms", "abrodef", "abrohead", "abrohead_new", "abrohead_dep",
  "optional_abroplist", "abroplist", "abroparms", "number_or_infinity", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     123,   125,    59,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    76,    77,    78,    79,    79,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    82,    83,    83,    84,    85,    85,
      86,    87,    88,    88,    88,    89,    89,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    91,    92,    93,    93,
      94,    94,    95,    95,    95,    96,    97,    97,    98,    99,
     100,   100,   101,   101,   102,   102,   102,   102,   103,   104,
     104,   105,   106,   107,   107,   108,   108,   109,   109,   110,
     111,   112,   112,   113,   113,   114,   114,   114,   114,   115,
     116,   116,   117,   118,   119,   119,   120,   120,   121,   121,
     121,   122,   122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     5,     2,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     2,     3,     5,     2,     3,
       3,     4,     0,     2,     3,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     0,     1,
       2,     1,     3,     3,     3,     5,     2,     1,     1,     2,
       0,     1,     2,     1,     3,     3,     3,     3,     5,     2,
       1,     1,     2,     0,     1,     2,     1,     3,     3,     5,
       1,     0,     1,     2,     1,     3,     3,     3,     3,     5,
       1,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     3,     0,     6,     5,     1,     2,     8,
       0,     0,     0,     0,     0,     0,   100,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     9,    11,    17,    10,    52,    12,
       0,    13,     0,    14,     0,    15,     0,    16,     0,   110,
     111,     0,     0,    78,    79,    77,    91,    92,    90,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     0,
       0,    68,    80,    93,   101,   114,     0,     0,    76,    89,
       0,     0,     0,     0,     0,    24,    25,    19,    22,    18,
      21,    20,    23,    26,    27,    28,    29,    30,    31,    34,
      32,    33,    35,    38,    36,    37,    41,    42,    39,    40,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      53,     0,    56,    50,     0,     0,     0,     0,    69,    71,
       0,     0,     0,     0,     0,    81,    83,     0,     0,     0,
      94,    96,     0,     0,     0,     0,     0,   102,   104,     0,
       0,     0,     0,   115,   117,    51,    67,    45,     0,     0,
     113,    48,     0,     0,     0,     0,   121,   122,     0,     0,
       0,     0,     0,     0,     0,    54,    55,     0,     0,     0,
       0,    70,     0,     0,     0,     0,     0,    82,     0,     0,
       0,    95,     0,     0,     0,     0,     0,   103,     0,     0,
       0,     0,   116,    46,    44,    49,    47,    57,    58,    60,
      61,    62,    63,    59,    64,    65,    72,    73,    74,    66,
      84,    85,    86,    87,    75,    97,    98,    88,   105,   106,
     107,   108,    99,   118,   119,   120,   109
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     6,    10,    43,    44,    45,   111,
      46,   114,    47,    48,   100,   151,   152,    49,    50,   157,
     158,   159,    51,    64,    65,    52,   164,   165,   166,    53,
      67,    68,    54,   169,   170,   171,    55,    56,   176,   177,
     178,    57,    58,    59,    60,   182,   183,   184,   198
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -96
static const yytype_int16 yypact[] =
{
      11,    24,    58,   -96,   -11,   -96,   -96,   -96,   -96,   -96,
      -4,    43,    48,    49,    64,     7,   -96,    63,    26,    65,
      83,    78,    79,    82,    84,    85,    89,    90,    91,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   106,
     107,   105,    37,   -96,   -96,   -96,   -96,   -96,    28,   -96,
      41,   -96,    42,   -96,    52,   -96,    53,   -96,    54,   -96,
     -96,    55,    56,   -96,    49,   -96,   -96,    64,   -96,   108,
      57,   109,    59,    60,    61,    62,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    80,    81,
      86,    87,    88,    92,   103,   104,   110,   111,   -96,    12,
     112,    38,   -43,   -17,   -29,    19,   114,   122,   -96,   -96,
     113,    -9,   123,   115,    -8,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,
     -96,   121,   134,    17,    17,   135,   138,   139,    24,    24,
     -96,    34,   -96,   -96,   142,    17,   141,   117,    38,   -96,
     148,   146,    17,   147,   118,   -43,   -96,    17,   149,   119,
     -17,   -96,   153,   151,   155,   156,   120,   -29,   -96,   157,
     158,   159,   124,    19,   -96,   -96,   -96,   -96,   125,   126,
     -96,   -96,   127,   128,   129,   130,   -96,   -96,   131,   132,
     133,   136,   137,   140,   143,   -96,   -96,   144,   145,   150,
     152,   -96,   154,   160,   161,   162,   163,   -96,   164,   165,
     166,   -96,   167,   168,   169,   170,   171,   -96,   172,   173,
     174,   175,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,   -96
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -96,   -96,   176,   -96,   -95,   -96,   -96,   -96,   -96,   -96,
     -96,   -96,   -96,   -96,   -96,   -96,    22,   -96,   -96,   -96,
     -96,    16,   -96,   -96,   116,   -96,   -96,   -96,    14,   -96,
     -96,   181,   -96,   -96,   -96,    23,   -96,   -96,   -96,   -96,
       0,   -96,   -96,   -96,   -96,   -96,   -96,    -2,   -80
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      11,    12,    13,    14,    15,    16,    17,    18,   188,   192,
     160,   161,   162,   163,     1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     196,   172,   173,   174,   175,   197,     5,    34,    35,    36,
     167,   168,    37,    38,    39,    40,   141,   142,   143,   144,
     145,   146,   147,   203,   204,    41,   148,   149,     7,     9,
      61,     1,   189,   193,   199,    62,    63,    42,   141,   142,
     143,   144,   145,   146,   147,   208,    66,    69,   148,   149,
      70,    72,   214,   150,   179,   180,   181,   218,   154,   155,
     156,    74,    76,    75,    77,    78,    71,    79,    99,    73,
      80,    81,    82,    83,    84,   205,    85,    86,    87,    98,
      88,   101,   102,    89,    90,    91,    92,    93,    94,    95,
      96,    97,   103,   104,   105,   110,   113,   185,   106,   107,
     112,   115,   116,   117,   118,   186,   190,   194,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     195,   200,   131,   132,   201,   202,   207,   209,   133,   134,
     135,   212,   213,   215,   136,   219,   222,   223,   224,   225,
     228,   229,   230,   206,   211,   137,   138,   227,     8,   217,
     108,   232,   139,   140,   153,   187,     0,   191,   210,   216,
     220,   226,     0,   221,     0,   231,     0,   233,   234,   235,
     236,   237,   238,   239,   240,   241,     0,     0,   242,   243,
       0,     0,   244,     0,     0,   245,   246,   247,     0,     0,
       0,     0,   248,     0,   249,     0,   250,     0,     0,     0,
       0,     0,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   109
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-96))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       4,     5,     6,     7,     8,     9,    10,    11,    17,    17,
      53,    54,    55,    56,     3,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      13,    60,    61,    62,    63,    18,    12,    41,    42,    43,
      57,    58,    46,    47,    48,    49,    34,    35,    36,    37,
      38,    39,    40,   148,   149,    59,    44,    45,     0,    70,
      17,     3,    71,    71,   144,    17,    17,    71,    34,    35,
      36,    37,    38,    39,    40,   155,    12,    70,    44,    45,
      17,    16,   162,    71,    65,    66,    67,   167,    50,    51,
      52,    13,    13,    15,    15,    13,    70,    15,    70,    16,
      16,    16,    13,    13,    13,    71,    13,    13,    13,    72,
      14,    70,    70,    16,    16,    16,    16,    16,    16,    13,
      13,    16,    70,    70,    70,    17,    17,    13,    73,    73,
      73,    72,    72,    72,    72,    13,    13,    16,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      16,    16,    72,    72,    16,    16,    14,    16,    72,    72,
      72,    13,    16,    16,    72,    16,    13,    16,    13,    13,
      13,    13,    13,   151,   158,    72,    72,   177,     2,   165,
      64,   183,    72,    72,    72,    72,    -1,    72,    71,    71,
      71,    71,    -1,   170,    -1,    71,    -1,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    -1,    -1,    72,    72,
      -1,    -1,    72,    -1,    -1,    72,    72,    72,    -1,    -1,
      -1,    -1,    72,    -1,    72,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    67
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    75,    76,    77,    12,    78,     0,    76,    70,
      79,     4,     5,     6,     7,     8,     9,    10,    11,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    41,    42,    43,    46,    47,    48,
      49,    59,    71,    80,    81,    82,    84,    86,    87,    91,
      92,    96,    99,   103,   106,   110,   111,   115,   116,   117,
     118,    17,    17,    17,    97,    98,    12,   104,   105,    70,
      17,    70,    16,    16,    13,    15,    13,    15,    13,    15,
      16,    16,    13,    13,    13,    13,    13,    13,    14,    16,
      16,    16,    16,    16,    16,    13,    13,    16,    72,    70,
      88,    70,    70,    70,    70,    70,    73,    73,    98,   105,
      17,    83,    73,    17,    85,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    34,    35,    36,    37,    38,    39,    40,    44,    45,
      71,    89,    90,    72,    50,    51,    52,    93,    94,    95,
      53,    54,    55,    56,   100,   101,   102,    57,    58,   107,
     108,   109,    60,    61,    62,    63,   112,   113,   114,    65,
      66,    67,   119,   120,   121,    13,    13,    72,    17,    71,
      13,    72,    17,    71,    16,    16,    13,    18,   122,   122,
      16,    16,    16,    78,    78,    71,    90,    14,   122,    16,
      71,    95,    13,    16,   122,    16,    71,   102,   122,    16,
      71,   109,    13,    16,    13,    13,    71,   114,    13,    13,
      13,    71,   121,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:

/* Line 1806 of yacc.c  */
#line 178 "gram.y"
    {
			dlog(LOG_DEBUG, 4, "%s interface definition ok", iface->props.name);

			iface->next = IfaceList;
			IfaceList = iface;

			iface = NULL;
		}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 188 "gram.y"
    {
			iface = IfaceList;

			while (iface)
			{
				if (!strcmp((yyvsp[(2) - (2)].str), iface->props.name))
				{
					flog(LOG_ERR, "duplicate interface "
						"definition for %s", (yyvsp[(2) - (2)].str));
					ABORT;
				}
				iface = iface->next;
			}

			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->props.name, (yyvsp[(2) - (2)].str), IFNAMSIZ-1);
			iface->props.name[IFNAMSIZ-1] = '\0';
			iface->lineno = num_lines;
		}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 217 "gram.y"
    {
			/* check vality */
			(yyval.str) = (yyvsp[(1) - (1)].str);
		}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 228 "gram.y"
    { ADD_TO_LL(struct AdvPrefix, AdvPrefixList, (yyvsp[(1) - (1)].pinfo)); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 229 "gram.y"
    { ADD_TO_LL(struct Clients, ClientList, (yyvsp[(1) - (1)].ainfo)); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 230 "gram.y"
    { ADD_TO_LL(struct AdvRoute, AdvRouteList, (yyvsp[(1) - (1)].rinfo)); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 231 "gram.y"
    { ADD_TO_LL(struct AdvRDNSS, AdvRDNSSList, (yyvsp[(1) - (1)].rdnssinfo)); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 232 "gram.y"
    { ADD_TO_LL(struct AdvDNSSL, AdvDNSSLList, (yyvsp[(1) - (1)].dnsslinfo)); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 233 "gram.y"
    { ADD_TO_LL(struct AdvLowpanCo, AdvLowpanCoList, (yyvsp[(1) - (1)].lowpancoinfo)); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 234 "gram.y"
    { ADD_TO_LL(struct AdvAbro, AdvAbroList, (yyvsp[(1) - (1)].abroinfo)); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 235 "gram.y"
    { ADD_TO_LL(struct AdvRASrcAddress, AdvRASrcAddressList, (yyvsp[(1) - (1)].rasrcaddressinfo)); }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 239 "gram.y"
    {
			iface->MinRtrAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 243 "gram.y"
    {
			iface->MaxRtrAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 247 "gram.y"
    {
			iface->MinDelayBetweenRAs = (yyvsp[(2) - (3)].num);
		}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 251 "gram.y"
    {
			iface->MinRtrAdvInterval = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 255 "gram.y"
    {
			iface->MaxRtrAdvInterval = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 259 "gram.y"
    {
			iface->MinDelayBetweenRAs = (yyvsp[(2) - (3)].dec);
		}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 263 "gram.y"
    {
			iface->IgnoreIfMissing = (yyvsp[(2) - (3)].num);
		}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 267 "gram.y"
    {
			iface->AdvSendAdvert = (yyvsp[(2) - (3)].num);
		}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 271 "gram.y"
    {
			iface->ra_header_info.AdvManagedFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 275 "gram.y"
    {
			iface->ra_header_info.AdvOtherConfigFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 279 "gram.y"
    {
			iface->AdvLinkMTU = (yyvsp[(2) - (3)].num);
		}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 283 "gram.y"
    {
			iface->AdvRAMTU = (yyvsp[(2) - (3)].num);
			iface->AdvRAMTU = MAX(MIN_AdvLinkMTU, iface->AdvRAMTU);
			iface->AdvRAMTU = MIN(MAX_AdvLinkMTU, iface->AdvRAMTU);
		}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 289 "gram.y"
    {
			iface->ra_header_info.AdvReachableTime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 293 "gram.y"
    {
			iface->ra_header_info.AdvRetransTimer = (yyvsp[(2) - (3)].num);
		}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 297 "gram.y"
    {
			iface->ra_header_info.AdvDefaultLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 301 "gram.y"
    {
			iface->ra_header_info.AdvDefaultPreference = (yyvsp[(2) - (3)].snum);
		}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 305 "gram.y"
    {
			iface->ra_header_info.AdvCurHopLimit = (yyvsp[(2) - (3)].num);
		}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 309 "gram.y"
    {
			iface->AdvSourceLLAddress = (yyvsp[(2) - (3)].num);
		}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 313 "gram.y"
    {
			iface->mipv6.AdvIntervalOpt = (yyvsp[(2) - (3)].num);
		}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 317 "gram.y"
    {
			iface->mipv6.AdvHomeAgentInfo = (yyvsp[(2) - (3)].num);
		}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 321 "gram.y"
    {
			iface->ra_header_info.AdvHomeAgentFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 325 "gram.y"
    {
			iface->mipv6.HomeAgentPreference = (yyvsp[(2) - (3)].num);
		}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 329 "gram.y"
    {
			iface->mipv6.HomeAgentLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 333 "gram.y"
    {
			iface->UnicastOnly = (yyvsp[(2) - (3)].num);
		}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 337 "gram.y"
    {
			iface->AdvRASolicitedUnicast = (yyvsp[(2) - (3)].num);
		}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 341 "gram.y"
    {
			iface->mipv6.AdvMobRtrSupportFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 347 "gram.y"
    {
			(yyval.ainfo) = (yyvsp[(3) - (5)].ainfo);
		}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 353 "gram.y"
    {
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), (yyvsp[(1) - (2)].addr), sizeof(struct in6_addr));
			(yyval.ainfo) = new;
		}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 364 "gram.y"
    {
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), (yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
			new->next = (yyvsp[(1) - (3)].ainfo);
			(yyval.ainfo) = new;
		}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 378 "gram.y"
    {
			(yyval.rasrcaddressinfo) = (yyvsp[(3) - (5)].rasrcaddressinfo);
		}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 384 "gram.y"
    {
			struct AdvRASrcAddress *new = calloc(1, sizeof(struct AdvRASrcAddress));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->address), (yyvsp[(1) - (2)].addr), sizeof(struct in6_addr));
			(yyval.rasrcaddressinfo) = new;
		}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 395 "gram.y"
    {
			struct AdvRASrcAddress *new = calloc(1, sizeof(struct AdvRASrcAddress));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->address), (yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
			new->next = (yyvsp[(1) - (3)].rasrcaddressinfo);
			(yyval.rasrcaddressinfo) = new;
		}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 409 "gram.y"
    {
			if (prefix) {
				unsigned int dst;

				if (prefix->AdvPreferredLifetime > prefix->AdvValidLifetime)
				{
					flog(LOG_ERR, "AdvValidLifeTime must be "
						"greater than AdvPreferredLifetime in %s, line %d",
						filename, num_lines);
					ABORT;
				}

				if ( prefix->if6[0] )
				{
					if (prefix->PrefixLen != 64) {
						flog(LOG_ERR, "only /64 is allowed with Base6Interface.  %s:%d", filename, num_lines);
						ABORT;
					}
				}
			}
			(yyval.pinfo) = prefix;
			prefix = NULL;
		}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 435 "gram.y"
    {
			struct in6_addr zeroaddr;
			memset(&zeroaddr, 0, sizeof(zeroaddr));

			if (!memcmp((yyvsp[(2) - (4)].addr), &zeroaddr, sizeof(struct in6_addr))) {
				flog(LOG_WARNING, "invalid all-zeros prefix in %s, line %d", filename, num_lines);
			}
			prefix = malloc(sizeof(struct AdvPrefix));

			if (prefix == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if ((yyvsp[(4) - (4)].num) > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid prefix length in %s, line %d", filename, num_lines);
				ABORT;
			}

			prefix->PrefixLen = (yyvsp[(4) - (4)].num);

			memcpy(&prefix->Prefix, (yyvsp[(2) - (4)].addr), sizeof(struct in6_addr));
		}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 473 "gram.y"
    {
			if (prefix) {
				prefix->AdvOnLinkFlag = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 479 "gram.y"
    {
			if (prefix) {
				prefix->AdvAutonomousFlag = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 485 "gram.y"
    {
			if (prefix) {
				prefix->AdvRouterAddr = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 491 "gram.y"
    {
			if (prefix) {
				prefix->AdvValidLifetime = (yyvsp[(2) - (3)].num);
				prefix->curr_validlft = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 498 "gram.y"
    {
			if (prefix) {
				prefix->AdvPreferredLifetime = (yyvsp[(2) - (3)].num);
				prefix->curr_preferredlft = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 505 "gram.y"
    {
			if (prefix) {
				prefix->DeprecatePrefixFlag = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 511 "gram.y"
    {
			if (prefix) {
				prefix->DecrementLifetimesFlag = (yyvsp[(2) - (3)].num);
			}
		}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 517 "gram.y"
    {
#ifndef HAVE_IFADDRS_H
			flog(LOG_ERR, "Base6Interface not supported in %s, line %d", filename, num_lines);
			ABORT;
#else
			if (prefix) {
				dlog(LOG_DEBUG, 4, "using prefixes on interface %s for prefixes on interface %s", (yyvsp[(2) - (3)].str), iface->props.name);
				strncpy(prefix->if6, (yyvsp[(2) - (3)].str), IFNAMSIZ-1);
				prefix->if6[IFNAMSIZ-1] = '\0';
			}
#endif
		}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 531 "gram.y"
    {
#ifndef HAVE_IFADDRS_H
			flog(LOG_ERR, "Base6to4Interface not supported in %s, line %d", filename, num_lines);
			ABORT;
#else
			if (prefix) {
				dlog(LOG_DEBUG, 4, "using interface %s for 6to4 prefixes on interface %s", (yyvsp[(2) - (3)].str), iface->props.name);
				strncpy(prefix->if6to4, (yyvsp[(2) - (3)].str), IFNAMSIZ-1);
				prefix->if6to4[IFNAMSIZ-1] = '\0';
			}
#endif
		}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 546 "gram.y"
    {
			(yyval.rinfo) = route;
			route = NULL;
		}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 554 "gram.y"
    {
			route = malloc(sizeof(struct AdvRoute));

			if (route == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			route_init_defaults(route, iface);

			if ((yyvsp[(4) - (4)].num) > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid route prefix length in %s, line %d", filename, num_lines);
				ABORT;
			}

			route->PrefixLen = (yyvsp[(4) - (4)].num);

			memcpy(&route->Prefix, (yyvsp[(2) - (4)].addr), sizeof(struct in6_addr));
		}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 587 "gram.y"
    {
			route->AdvRoutePreference = (yyvsp[(2) - (3)].snum);
		}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 591 "gram.y"
    {
			route->AdvRouteLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 595 "gram.y"
    {
			route->RemoveRouteFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 601 "gram.y"
    {
			(yyval.rdnssinfo) = rdnss;
			rdnss = NULL;
		}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 612 "gram.y"
    {
			if (!rdnss) {
				/* first IP found */
				rdnss = malloc(sizeof(struct AdvRDNSS));

				if (rdnss == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				rdnss_init_defaults(rdnss, iface);
			}

			switch (rdnss->AdvRDNSSNumber) {
				case 0:
					memcpy(&rdnss->AdvRDNSSAddr1, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 1:
					memcpy(&rdnss->AdvRDNSSAddr2, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 2:
					memcpy(&rdnss->AdvRDNSSAddr3, (yyvsp[(1) - (1)].addr), sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				default:
					flog(LOG_CRIT, "too many addresses in RDNSS section");
					ABORT;
			}

		}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 647 "gram.y"
    {
			if (!rdnss) {
				flog(LOG_CRIT, "no address specified in RDNSS section");
				ABORT;
			}
		}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 665 "gram.y"
    {
			flog(LOG_WARNING, "ignoring deprecated RDNSS preference");
		}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 669 "gram.y"
    {
			flog(LOG_WARNING, "ignoring deprecated RDNSS open flag");
		}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 673 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster");
			if ((yyvsp[(2) - (3)].num) < iface->MaxRtrAdvInterval && (yyvsp[(2) - (3)].num) != 0) {
				flog(LOG_ERR, "AdvRDNSSLifetime must be at least MaxRtrAdvInterval");
				rdnss->AdvRDNSSLifetime = iface->MaxRtrAdvInterval;
			} else {
				rdnss->AdvRDNSSLifetime = (yyvsp[(2) - (3)].num);
			}
			if ((yyvsp[(2) - (3)].num) > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: (%s:%d) AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster", filename, num_lines);

			rdnss->AdvRDNSSLifetime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 688 "gram.y"
    {
			rdnss->FlushRDNSSFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 694 "gram.y"
    {
			(yyval.dnsslinfo) = dnssl;
			dnssl = NULL;
		}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 705 "gram.y"
    {
			char *ch;
			for (ch = (yyvsp[(1) - (1)].str);*ch != '\0';ch++) {
				if (*ch >= 'A' && *ch <= 'Z')
					continue;
				if (*ch >= 'a' && *ch <= 'z')
					continue;
				if (*ch >= '0' && *ch <= '9')
					continue;
				if (*ch == '-' || *ch == '.')
					continue;

				flog(LOG_CRIT, "invalid domain suffix specified");
				ABORT;
			}

			if (!dnssl) {
				/* first domain found */
				dnssl = malloc(sizeof(struct AdvDNSSL));

				if (dnssl == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				dnssl_init_defaults(dnssl, iface);
			}

			dnssl->AdvDNSSLNumber++;
			dnssl->AdvDNSSLSuffixes =
				realloc(dnssl->AdvDNSSLSuffixes,
					dnssl->AdvDNSSLNumber * sizeof(char*));
			if (dnssl->AdvDNSSLSuffixes == NULL) {
				flog(LOG_CRIT, "realloc failed: %s", strerror(errno));
				ABORT;
			}

			dnssl->AdvDNSSLSuffixes[dnssl->AdvDNSSLNumber - 1] = strdup((yyvsp[(1) - (1)].str));
		}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 747 "gram.y"
    {
			if (!dnssl) {
				flog(LOG_CRIT, "no domain specified in DNSSL section");
				ABORT;
			}
		}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 765 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: AdvDNSSLLifetime <= 2*MaxRtrAdvInterval would allow stale DNS suffixes to be deleted faster");
			if ((yyvsp[(2) - (3)].num) < iface->MaxRtrAdvInterval && (yyvsp[(2) - (3)].num) != 0) {
				flog(LOG_ERR, "AdvDNSSLLifetime must be at least MaxRtrAdvInterval");
				dnssl->AdvDNSSLLifetime = iface->MaxRtrAdvInterval;
			} else {
				dnssl->AdvDNSSLLifetime = (yyvsp[(2) - (3)].num);
			}

		}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 777 "gram.y"
    {
			dnssl->FlushDNSSLFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 783 "gram.y"
    {
			(yyval.lowpancoinfo) = lowpanco;
			lowpanco = NULL;
		}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 790 "gram.y"
    {
			lowpanco = malloc(sizeof(struct AdvLowpanCo));

			if (lowpanco == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(lowpanco, 0, sizeof(struct AdvLowpanCo));
		}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 811 "gram.y"
    {
			lowpanco->ContextLength = (yyvsp[(2) - (3)].num);
		}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 815 "gram.y"
    {
			lowpanco->ContextCompressionFlag = (yyvsp[(2) - (3)].num);
		}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 819 "gram.y"
    {
			lowpanco->AdvContextID = (yyvsp[(2) - (3)].num);
		}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 823 "gram.y"
    {
			lowpanco->AdvLifeTime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 829 "gram.y"
    {
			(yyval.abroinfo) = abro;
			abro = NULL;
		}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 838 "gram.y"
    {
			abro = malloc(sizeof(struct AdvAbro));

			if (abro == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(abro, 0, sizeof(struct AdvAbro));
			memcpy(&abro->LBRaddress, (yyvsp[(2) - (2)].addr), sizeof(struct in6_addr));
		}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 852 "gram.y"
    {
			flog(LOG_WARNING
				, "%s:%d abro prefix length deprecated, remove trailing '/%d'"
				, filename
				, num_lines
				, (yyvsp[(4) - (4)].num)
			);
			abro = malloc(sizeof(struct AdvAbro));

			if (abro == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(abro, 0, sizeof(struct AdvAbro));
			memcpy(&abro->LBRaddress, (yyvsp[(2) - (4)].addr), sizeof(struct in6_addr));
		}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 880 "gram.y"
    {
			abro->Version[1] = (yyvsp[(2) - (3)].num);
		}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 884 "gram.y"
    {
			abro->Version[0] = (yyvsp[(2) - (3)].num);
		}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 888 "gram.y"
    {
			abro->ValidLifeTime = (yyvsp[(2) - (3)].num);
		}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 894 "gram.y"
    {
				(yyval.num) = (yyvsp[(1) - (1)].num);
			}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 898 "gram.y"
    {
				(yyval.num) = (uint32_t)~0;
			}
    break;



/* Line 1806 of yacc.c  */
#line 2827 "gram.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 903 "gram.y"


static void cleanup(void)
{
	if (iface) {
		free_ifaces(iface);
		iface = 0;
	}

	if (prefix) {
		free(prefix);
		prefix = 0;
	}

	if (route) {
		free(route);
		route = 0;
	}

	if (rdnss) {
		free(rdnss);
		rdnss = 0;
	}

	if (dnssl) {
		int i;
		for (i = 0;i < dnssl->AdvDNSSLNumber;i++)
			free(dnssl->AdvDNSSLSuffixes[i]);
		free(dnssl->AdvDNSSLSuffixes);
		free(dnssl);
		dnssl = 0;
	}

	if (lowpanco) {
		free(lowpanco);
		lowpanco = 0;
	}

	if (abro) {
		free(abro);
		abro = 0;
	}
}

struct Interface * readin_config(char const *path)
{
	FILE * in = fopen(path, "r");
	if (in) {
		filename = path;
		IfaceList = 0;
		num_lines = 1;
		iface = 0;

		yyset_in(in);
		if (yyparse() != 0) {
			free_ifaces(iface);
			iface = 0;
		} else {
			dlog(LOG_DEBUG, 1, "config file, %s, syntax ok", path);
		}
		yylex_destroy();
		fclose(in);
	}

	return IfaceList;
}

static void yyerror(char const * msg)
{
	fprintf(stderr, "%s:%d error: %s\n",
		filename,
		num_lines,
		msg);
}



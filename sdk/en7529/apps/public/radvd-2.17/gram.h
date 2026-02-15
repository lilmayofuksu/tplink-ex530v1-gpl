/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
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

/* Line 2068 of yacc.c  */
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



/* Line 2068 of yacc.c  */
#line 206 "gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;



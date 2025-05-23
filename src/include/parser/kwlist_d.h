/*-------------------------------------------------------------------------
 *
 * kwlist_d.h
 *    List of keywords represented as a ScanKeywordList.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * NOTES
 *  ******************************
 *  *** DO NOT EDIT THIS FILE! ***
 *  ******************************
 *
 *  It has been GENERATED by src/tools/gen_keywordlist.pl
 *
 *-------------------------------------------------------------------------
 */

#ifndef KWLIST_D_H
#define KWLIST_D_H

#include "parser/kwlookup.h"

static const char ScanKeywords_kw_string[] =
	"abort\0"
	"absolute\0"
	"access\0"
	"account\0"
	"action\0"
	"add\0"
	"admin\0"
	"after\0"
	"aggregate\0"
	"algorithm\0"
	"all\0"
	"also\0"
	"alter\0"
	"always\0"
	"analyse\0"
	"analyze\0"
	"and\0"
	"any\0"
	"app\0"
	"append\0"
	"archive\0"
	"array\0"
	"as\0"
	"asc\0"
	"assertion\0"
	"assignment\0"
	"asymmetric\0"
	"at\0"
	"attribute\0"
	"audit\0"
	"authid\0"
	"authorization\0"
	"auto_increment\0"
	"autoextend\0"
	"automapped\0"
	"backward\0"
	"barrier\0"
	"before\0"
	"begin\0"
	"begin_non_anoyblock\0"
	"between\0"
	"bigint\0"
	"binary\0"
	"binary_double\0"
	"binary_double_infinity\0"
	"binary_double_nan\0"
	"binary_integer\0"
	"bit\0"
	"blanks\0"
	"blob\0"
	"blockchain\0"
	"body\0"
	"boolean\0"
	"both\0"
	"bucketcnt\0"
	"buckets\0"
	"build\0"
	"by\0"
	"byte\0"
	"byteawithoutorder\0"
	"byteawithoutorderwithequal\0"
	"cache\0"
	"call\0"
	"called\0"
	"cancelable\0"
	"cascade\0"
	"cascaded\0"
	"case\0"
	"cast\0"
	"catalog\0"
	"catalog_name\0"
	"chain\0"
	"change\0"
	"char\0"
	"character\0"
	"characteristics\0"
	"characterset\0"
	"charset\0"
	"check\0"
	"checkpoint\0"
	"class\0"
	"class_origin\0"
	"clean\0"
	"client\0"
	"client_master_key\0"
	"client_master_keys\0"
	"clob\0"
	"close\0"
	"cluster\0"
	"coalesce\0"
	"collate\0"
	"collation\0"
	"column\0"
	"column_encryption_key\0"
	"column_encryption_keys\0"
	"column_name\0"
	"columns\0"
	"comment\0"
	"comments\0"
	"commit\0"
	"committed\0"
	"compact\0"
	"compatible_illegal_chars\0"
	"compile\0"
	"complete\0"
	"completion\0"
	"compress\0"
	"concurrently\0"
	"condition\0"
	"configuration\0"
	"connect\0"
	"connection\0"
	"consistent\0"
	"constant\0"
	"constraint\0"
	"constraint_catalog\0"
	"constraint_name\0"
	"constraint_schema\0"
	"constraints\0"
	"content\0"
	"continue\0"
	"contview\0"
	"conversion\0"
	"convert\0"
	"coordinator\0"
	"coordinators\0"
	"copy\0"
	"cost\0"
	"create\0"
	"cross\0"
	"csn\0"
	"csv\0"
	"cube\0"
	"current\0"
	"current_catalog\0"
	"current_date\0"
	"current_role\0"
	"current_schema\0"
	"current_time\0"
	"current_timestamp\0"
	"current_user\0"
	"cursor\0"
	"cursor_name\0"
	"cycle\0"
	"data\0"
	"database\0"
	"datafile\0"
	"datanode\0"
	"datanodes\0"
	"datatype_cl\0"
	"date\0"
	"date_format\0"
	"day\0"
	"day_hour\0"
	"day_minute\0"
	"day_second\0"
	"dbcompatibility\0"
	"deallocate\0"
	"dec\0"
	"decimal\0"
	"declare\0"
	"decode\0"
	"default\0"
	"defaults\0"
	"deferrable\0"
	"deferred\0"
	"definer\0"
	"delete\0"
	"delimiter\0"
	"delimiters\0"
	"delta\0"
	"deltamerge\0"
	"desc\0"
	"deterministic\0"
	"diagnostics\0"
	"dictionary\0"
	"direct\0"
	"directory\0"
	"disable\0"
	"discard\0"
	"disconnect\0"
	"distinct\0"
	"distribute\0"
	"distribution\0"
	"do\0"
	"document\0"
	"domain\0"
	"double\0"
	"drop\0"
	"dumpfile\0"
	"duplicate\0"
	"each\0"
	"elastic\0"
	"else\0"
	"enable\0"
	"enclosed\0"
	"encoding\0"
	"encrypted\0"
	"encrypted_value\0"
	"encryption\0"
	"encryption_type\0"
	"end\0"
	"ends\0"
	"enforced\0"
	"enum\0"
	"eol\0"
	"error\0"
	"errors\0"
	"escape\0"
	"escaped\0"
	"escaping\0"
	"event\0"
	"events\0"
	"every\0"
	"except\0"
	"exchange\0"
	"exclude\0"
	"excluded\0"
	"excluding\0"
	"exclusive\0"
	"execute\0"
	"exists\0"
	"expired\0"
	"explain\0"
	"extension\0"
	"external\0"
	"extract\0"
	"false\0"
	"family\0"
	"fast\0"
	"features\0"
	"fenced\0"
	"fetch\0"
	"fields\0"
	"fileheader\0"
	"fill_missing_fields\0"
	"filler\0"
	"filter\0"
	"first\0"
	"fixed\0"
	"float\0"
	"following\0"
	"follows\0"
	"for\0"
	"force\0"
	"foreign\0"
	"formatter\0"
	"forward\0"
	"freeze\0"
	"from\0"
	"full\0"
	"function\0"
	"functions\0"
	"generated\0"
	"get\0"
	"global\0"
	"grant\0"
	"granted\0"
	"greatest\0"
	"group\0"
	"grouping\0"
	"groupparent\0"
	"handler\0"
	"having\0"
	"hdfsdirectory\0"
	"header\0"
	"hold\0"
	"hour\0"
	"hour_minute\0"
	"hour_second\0"
	"identified\0"
	"identity\0"
	"if\0"
	"ignore\0"
	"ignore_extra_data\0"
	"ilike\0"
	"immediate\0"
	"immutable\0"
	"implicit\0"
	"in\0"
	"include\0"
	"including\0"
	"increment\0"
	"incremental\0"
	"index\0"
	"indexes\0"
	"infile\0"
	"infinite\0"
	"inherit\0"
	"inherits\0"
	"initial\0"
	"initially\0"
	"initrans\0"
	"inline\0"
	"inner\0"
	"inout\0"
	"input\0"
	"insensitive\0"
	"insert\0"
	"instead\0"
	"int\0"
	"integer\0"
	"internal\0"
	"intersect\0"
	"interval\0"
	"into\0"
	"invisible\0"
	"invoker\0"
	"ip\0"
	"is\0"
	"isnull\0"
	"isolation\0"
	"join\0"
	"key\0"
	"key_path\0"
	"key_store\0"
	"kill\0"
	"label\0"
	"language\0"
	"large\0"
	"last\0"
	"lc_collate\0"
	"lc_ctype\0"
	"leading\0"
	"leakproof\0"
	"least\0"
	"left\0"
	"less\0"
	"level\0"
	"like\0"
	"limit\0"
	"lines\0"
	"list\0"
	"listen\0"
	"load\0"
	"local\0"
	"localtime\0"
	"localtimestamp\0"
	"location\0"
	"lock\0"
	"locked\0"
	"log\0"
	"logging\0"
	"login_any\0"
	"login_failure\0"
	"login_success\0"
	"logout\0"
	"loop\0"
	"mapping\0"
	"masking\0"
	"master\0"
	"match\0"
	"matched\0"
	"materialized\0"
	"maxextents\0"
	"maxsize\0"
	"maxtrans\0"
	"maxvalue\0"
	"merge\0"
	"message_text\0"
	"method\0"
	"minextents\0"
	"minus\0"
	"minute\0"
	"minute_second\0"
	"minvalue\0"
	"mode\0"
	"model\0"
	"modify\0"
	"month\0"
	"move\0"
	"movement\0"
	"mysql_errno\0"
	"name\0"
	"names\0"
	"nan\0"
	"national\0"
	"natural\0"
	"nchar\0"
	"next\0"
	"no\0"
	"nocompress\0"
	"nocycle\0"
	"node\0"
	"nologging\0"
	"nomaxvalue\0"
	"nominvalue\0"
	"none\0"
	"not\0"
	"nothing\0"
	"notify\0"
	"notnull\0"
	"novalidate\0"
	"nowait\0"
	"null\0"
	"nullcols\0"
	"nullif\0"
	"nulls\0"
	"number\0"
	"numeric\0"
	"numstr\0"
	"nvarchar\0"
	"nvarchar2\0"
	"nvl\0"
	"object\0"
	"of\0"
	"off\0"
	"offset\0"
	"oids\0"
	"on\0"
	"only\0"
	"operator\0"
	"optimization\0"
	"option\0"
	"optionally\0"
	"options\0"
	"or\0"
	"order\0"
	"out\0"
	"outer\0"
	"outfile\0"
	"over\0"
	"overlaps\0"
	"overlay\0"
	"owned\0"
	"owner\0"
	"package\0"
	"packages\0"
	"parallel_enable\0"
	"parser\0"
	"partial\0"
	"partition\0"
	"partitions\0"
	"passing\0"
	"password\0"
	"pctfree\0"
	"per\0"
	"percent\0"
	"performance\0"
	"perm\0"
	"pipelined\0"
	"placing\0"
	"plan\0"
	"plans\0"
	"policy\0"
	"pool\0"
	"position\0"
	"precedes\0"
	"preceding\0"
	"precision\0"
	"predict\0"
	"preferred\0"
	"prefix\0"
	"prepare\0"
	"prepared\0"
	"preserve\0"
	"primary\0"
	"prior\0"
	"priorer\0"
	"private\0"
	"privilege\0"
	"privileges\0"
	"procedural\0"
	"procedure\0"
	"profile\0"
	"publication\0"
	"publish\0"
	"purge\0"
	"query\0"
	"quote\0"
	"randomized\0"
	"range\0"
	"ratio\0"
	"raw\0"
	"read\0"
	"real\0"
	"reassign\0"
	"rebuild\0"
	"recheck\0"
	"recursive\0"
	"recyclebin\0"
	"redisanyvalue\0"
	"ref\0"
	"references\0"
	"refresh\0"
	"reindex\0"
	"reject\0"
	"relative\0"
	"release\0"
	"reloptions\0"
	"remote\0"
	"remove\0"
	"rename\0"
	"repeat\0"
	"repeatable\0"
	"replace\0"
	"replica\0"
	"reset\0"
	"resize\0"
	"resource\0"
	"restart\0"
	"restrict\0"
	"return\0"
	"returned_sqlstate\0"
	"returning\0"
	"returns\0"
	"reuse\0"
	"revoke\0"
	"right\0"
	"role\0"
	"roles\0"
	"rollback\0"
	"rollup\0"
	"rotate\0"
	"rotation\0"
	"row\0"
	"row_count\0"
	"rownum\0"
	"rows\0"
	"rowtype\0"
	"rule\0"
	"sample\0"
	"savepoint\0"
	"schedule\0"
	"schema\0"
	"schema_name\0"
	"scroll\0"
	"search\0"
	"second\0"
	"security\0"
	"select\0"
	"separator\0"
	"sequence\0"
	"sequences\0"
	"serializable\0"
	"server\0"
	"session\0"
	"session_user\0"
	"set\0"
	"setof\0"
	"sets\0"
	"share\0"
	"shippable\0"
	"show\0"
	"shrink\0"
	"shutdown\0"
	"siblings\0"
	"similar\0"
	"simple\0"
	"size\0"
	"skip\0"
	"slave\0"
	"slice\0"
	"smalldatetime\0"
	"smalldatetime_format\0"
	"smallint\0"
	"snapshot\0"
	"some\0"
	"source\0"
	"space\0"
	"specification\0"
	"spill\0"
	"split\0"
	"sql\0"
	"stable\0"
	"stacked\0"
	"standalone\0"
	"start\0"
	"starting\0"
	"starts\0"
	"statement\0"
	"statement_id\0"
	"statistics\0"
	"stdin\0"
	"stdout\0"
	"storage\0"
	"store\0"
	"stored\0"
	"stratify\0"
	"stream\0"
	"strict\0"
	"strip\0"
	"subclass_origin\0"
	"subpartition\0"
	"subpartitions\0"
	"subscription\0"
	"substring\0"
	"symmetric\0"
	"synonym\0"
	"sys_refcursor\0"
	"sysdate\0"
	"sysid\0"
	"system\0"
	"table\0"
	"table_name\0"
	"tables\0"
	"tablesample\0"
	"tablespace\0"
	"target\0"
	"temp\0"
	"template\0"
	"temporary\0"
	"terminated\0"
	"text\0"
	"than\0"
	"then\0"
	"time\0"
	"time_format\0"
	"timecapsule\0"
	"timestamp\0"
	"timestamp_format\0"
	"timestampdiff\0"
	"tinyint\0"
	"to\0"
	"trailing\0"
	"transaction\0"
	"transform\0"
	"treat\0"
	"trigger\0"
	"trim\0"
	"true\0"
	"truncate\0"
	"trusted\0"
	"tsfield\0"
	"tstag\0"
	"tstime\0"
	"type\0"
	"types\0"
	"unbounded\0"
	"uncommitted\0"
	"unencrypted\0"
	"union\0"
	"unique\0"
	"unknown\0"
	"unlimited\0"
	"unlisten\0"
	"unlock\0"
	"unlogged\0"
	"until\0"
	"unusable\0"
	"update\0"
	"use\0"
	"useeof\0"
	"user\0"
	"using\0"
	"vacuum\0"
	"valid\0"
	"validate\0"
	"validation\0"
	"validator\0"
	"value\0"
	"values\0"
	"varchar\0"
	"varchar2\0"
	"variables\0"
	"variadic\0"
	"varying\0"
	"vcgroup\0"
	"verbose\0"
	"verify\0"
	"version\0"
	"view\0"
	"visible\0"
	"volatile\0"
	"wait\0"
	"warnings\0"
	"weak\0"
	"when\0"
	"where\0"
	"while\0"
	"whitespace\0"
	"window\0"
	"with\0"
	"within\0"
	"without\0"
	"work\0"
	"workload\0"
	"wrapper\0"
	"write\0"
	"xml\0"
	"xmlattributes\0"
	"xmlconcat\0"
	"xmlelement\0"
	"xmlexists\0"
	"xmlforest\0"
	"xmlparse\0"
	"xmlpi\0"
	"xmlroot\0"
	"xmlserialize\0"
	"year\0"
	"year_month\0"
	"yes\0"
	"zone";

static const uint16 ScanKeywords_kw_offsets[] = {
	0,
	6,
	15,
	22,
	30,
	37,
	41,
	47,
	53,
	63,
	73,
	77,
	82,
	88,
	95,
	103,
	111,
	115,
	119,
	123,
	130,
	138,
	144,
	147,
	151,
	161,
	172,
	183,
	186,
	196,
	202,
	209,
	223,
	238,
	249,
	260,
	269,
	277,
	284,
	290,
	310,
	318,
	325,
	332,
	346,
	369,
	387,
	402,
	406,
	413,
	418,
	429,
	434,
	442,
	447,
	457,
	465,
	471,
	474,
	479,
	497,
	524,
	530,
	535,
	542,
	553,
	561,
	570,
	575,
	580,
	588,
	601,
	607,
	614,
	619,
	629,
	645,
	658,
	666,
	672,
	683,
	689,
	702,
	708,
	715,
	733,
	752,
	757,
	763,
	771,
	780,
	788,
	798,
	805,
	827,
	850,
	862,
	870,
	878,
	887,
	894,
	904,
	912,
	937,
	945,
	954,
	965,
	974,
	987,
	997,
	1011,
	1019,
	1030,
	1041,
	1050,
	1061,
	1080,
	1096,
	1114,
	1126,
	1134,
	1143,
	1152,
	1163,
	1171,
	1183,
	1196,
	1201,
	1206,
	1213,
	1219,
	1223,
	1227,
	1232,
	1240,
	1256,
	1269,
	1282,
	1297,
	1310,
	1328,
	1341,
	1348,
	1360,
	1366,
	1371,
	1380,
	1389,
	1398,
	1408,
	1420,
	1425,
	1437,
	1441,
	1450,
	1461,
	1472,
	1488,
	1499,
	1503,
	1511,
	1519,
	1526,
	1534,
	1543,
	1554,
	1563,
	1571,
	1578,
	1588,
	1599,
	1605,
	1616,
	1621,
	1635,
	1647,
	1658,
	1665,
	1675,
	1683,
	1691,
	1702,
	1711,
	1722,
	1735,
	1738,
	1747,
	1754,
	1761,
	1766,
	1775,
	1785,
	1790,
	1798,
	1803,
	1810,
	1819,
	1828,
	1838,
	1854,
	1865,
	1881,
	1885,
	1890,
	1899,
	1904,
	1908,
	1914,
	1921,
	1928,
	1936,
	1945,
	1951,
	1958,
	1964,
	1971,
	1980,
	1988,
	1997,
	2007,
	2017,
	2025,
	2032,
	2040,
	2048,
	2058,
	2067,
	2075,
	2081,
	2088,
	2093,
	2102,
	2109,
	2115,
	2122,
	2133,
	2153,
	2160,
	2167,
	2173,
	2179,
	2185,
	2195,
	2203,
	2207,
	2213,
	2221,
	2231,
	2239,
	2246,
	2251,
	2256,
	2265,
	2275,
	2285,
	2289,
	2296,
	2302,
	2310,
	2319,
	2325,
	2334,
	2346,
	2354,
	2361,
	2375,
	2382,
	2387,
	2392,
	2404,
	2416,
	2427,
	2436,
	2439,
	2446,
	2464,
	2470,
	2480,
	2490,
	2499,
	2502,
	2510,
	2520,
	2530,
	2542,
	2548,
	2556,
	2563,
	2572,
	2580,
	2589,
	2597,
	2607,
	2616,
	2623,
	2629,
	2635,
	2641,
	2653,
	2660,
	2668,
	2672,
	2680,
	2689,
	2699,
	2708,
	2713,
	2723,
	2731,
	2734,
	2737,
	2744,
	2754,
	2759,
	2763,
	2772,
	2782,
	2787,
	2793,
	2802,
	2808,
	2813,
	2824,
	2833,
	2841,
	2851,
	2857,
	2862,
	2867,
	2873,
	2878,
	2884,
	2890,
	2895,
	2902,
	2907,
	2913,
	2923,
	2938,
	2947,
	2952,
	2959,
	2963,
	2971,
	2981,
	2995,
	3009,
	3016,
	3021,
	3029,
	3037,
	3044,
	3050,
	3058,
	3071,
	3082,
	3090,
	3099,
	3108,
	3114,
	3127,
	3134,
	3145,
	3151,
	3158,
	3172,
	3181,
	3186,
	3192,
	3199,
	3205,
	3210,
	3219,
	3231,
	3236,
	3242,
	3246,
	3255,
	3263,
	3269,
	3274,
	3277,
	3288,
	3296,
	3301,
	3311,
	3322,
	3333,
	3338,
	3342,
	3350,
	3357,
	3365,
	3376,
	3383,
	3388,
	3397,
	3404,
	3410,
	3417,
	3425,
	3432,
	3441,
	3451,
	3455,
	3462,
	3465,
	3469,
	3476,
	3481,
	3484,
	3489,
	3498,
	3511,
	3518,
	3529,
	3537,
	3540,
	3546,
	3550,
	3556,
	3564,
	3569,
	3578,
	3586,
	3592,
	3598,
	3606,
	3615,
	3631,
	3638,
	3646,
	3656,
	3667,
	3675,
	3684,
	3692,
	3696,
	3704,
	3716,
	3721,
	3731,
	3739,
	3744,
	3750,
	3757,
	3762,
	3771,
	3780,
	3790,
	3800,
	3808,
	3818,
	3825,
	3833,
	3842,
	3851,
	3859,
	3865,
	3873,
	3881,
	3891,
	3902,
	3913,
	3923,
	3931,
	3943,
	3951,
	3957,
	3963,
	3969,
	3980,
	3986,
	3992,
	3996,
	4001,
	4006,
	4015,
	4023,
	4031,
	4041,
	4052,
	4066,
	4070,
	4081,
	4089,
	4097,
	4104,
	4113,
	4121,
	4132,
	4139,
	4146,
	4153,
	4160,
	4171,
	4179,
	4187,
	4193,
	4200,
	4209,
	4217,
	4226,
	4233,
	4251,
	4261,
	4269,
	4275,
	4282,
	4288,
	4293,
	4299,
	4308,
	4315,
	4322,
	4331,
	4335,
	4345,
	4352,
	4357,
	4365,
	4370,
	4377,
	4387,
	4396,
	4403,
	4415,
	4422,
	4429,
	4436,
	4445,
	4452,
	4462,
	4471,
	4481,
	4494,
	4501,
	4509,
	4522,
	4526,
	4532,
	4537,
	4543,
	4553,
	4558,
	4565,
	4574,
	4583,
	4591,
	4598,
	4603,
	4608,
	4614,
	4620,
	4634,
	4655,
	4664,
	4673,
	4678,
	4685,
	4691,
	4705,
	4711,
	4717,
	4721,
	4728,
	4736,
	4747,
	4753,
	4762,
	4769,
	4779,
	4792,
	4803,
	4809,
	4816,
	4824,
	4830,
	4837,
	4846,
	4853,
	4860,
	4866,
	4882,
	4895,
	4909,
	4922,
	4932,
	4942,
	4950,
	4964,
	4972,
	4978,
	4985,
	4991,
	5002,
	5009,
	5021,
	5032,
	5039,
	5044,
	5053,
	5063,
	5074,
	5079,
	5084,
	5089,
	5094,
	5106,
	5118,
	5128,
	5145,
	5159,
	5167,
	5170,
	5179,
	5191,
	5201,
	5207,
	5215,
	5220,
	5225,
	5234,
	5242,
	5250,
	5256,
	5263,
	5268,
	5274,
	5284,
	5296,
	5308,
	5314,
	5321,
	5329,
	5339,
	5348,
	5355,
	5364,
	5370,
	5379,
	5386,
	5390,
	5397,
	5402,
	5408,
	5415,
	5421,
	5430,
	5441,
	5451,
	5457,
	5464,
	5472,
	5481,
	5491,
	5500,
	5508,
	5516,
	5524,
	5531,
	5539,
	5544,
	5552,
	5561,
	5566,
	5575,
	5580,
	5585,
	5591,
	5597,
	5608,
	5615,
	5620,
	5627,
	5635,
	5640,
	5649,
	5657,
	5663,
	5667,
	5681,
	5691,
	5702,
	5712,
	5722,
	5731,
	5737,
	5745,
	5758,
	5763,
	5774,
	5778,
};

#define SCANKEYWORDS_NUM_KEYWORDS 693

static int
ScanKeywords_hash_func(const void *key, size_t keylen)
{
	static const int16 h[1387] = {
		-36,   0,     210,   32767, 512,   965,   32767, 0,
		32767, 32767, 32767, 412,   32767, 32767, 85,    209,
		561,   -376,  32767, 32767, 25,    0,     422,   113,
		32767, 302,   32767, 32767, 538,   652,   12,    32767,
		50,    32767, 32767, 610,   86,    32767, 32767, 32767,
		711,   32767, 0,     32767, 0,     32767, 0,     73,
		264,   585,   470,   88,    627,   32767, -255,  562,
		444,   32767, -51,   32767, 457,   337,   32767, -11,
		1418,  32767, 32767, 567,   60,    -319,  32767, 32767,
		-319,  0,     32767, 32767, 718,   32767, 1299,  27,
		32767, 228,   32767, 32767, 32767, 207,   32767, 32767,
		-151,  0,     112,   0,     307,   5,     32767, 32767,
		438,   -168,  32767, 204,   447,   183,   32767, -368,
		32767, -32,   685,   32767, 32767, 195,   0,     32767,
		-230,  32767, 255,   32767, 32767, 0,     32767, 32767,
		0,     409,   32767, 32767, 143,   -163,  32767, -53,
		-286,  32767, 797,   0,     517,   -434,  463,   32767,
		200,   0,     0,     381,   32767, 32767, 267,   0,
		387,   32767, 32767, 32767, 32767, 484,   32767, 32767,
		32767, 599,   32767, 32767, -163,  0,     371,   495,
		0,     -416,  0,     128,   255,   32767, 32767, 0,
		525,   627,   421,   -745,  32767, -5,    32767, 0,
		376,   -184,  468,   32767, 589,   32767, 32767, -68,
		32767, 32767, 32767, 32767, 32767, -408,  478,   32767,
		-76,   578,   170,   245,   -514,  32767, 470,   -198,
		0,     145,   155,   313,   1281,  378,   494,   32767,
		0,     32767, 439,   -461,  32767, 0,     374,   0,
		0,     -580,  32767, 0,     -339,  32767, 635,   32767,
		-100,  422,   149,   32767, -77,   689,   215,   149,
		32767, 32767, -1300, -508,  381,   0,     -220,  -254,
		628,   32767, 41,    506,   32767, 556,   -35,   32767,
		438,   32767, 32767, 858,   0,     643,   314,   189,
		73,    0,     176,   32767, 0,     114,   -953,  32767,
		0,     398,   32767, 32767, 130,   32767, 433,   32767,
		32767, 281,   32767, 32767, 32767, 0,     32767, 0,
		414,   32767, 7,     32767, -451,  32767, 0,     471,
		233,   32767, 165,   32767, -373,  32767, 32767, -428,
		32767, 32767, 32767, 32767, -390,  450,   0,     32767,
		99,    32767, 235,   -138,  251,   651,   0,     32767,
		32767, 474,   0,     361,   547,   0,     121,   32767,
		185,   32767, 0,     618,   -923,  301,   0,     32767,
		0,     325,   32767, 453,   469,   554,   776,   436,
		32767, 356,   -52,   32767, -357,  766,   648,   32767,
		0,     -9,    32767, 32767, -1277, 0,     347,   32767,
		32767, 32767, 0,     -911,  521,   -56,   208,   32767,
		1111,  682,   -107,  -236,  35,    231,   32767, 32767,
		16,    32767, 249,   32767, -2,    40,    205,   32767,
		32767, 271,   -936,  32767, 385,   689,   28,    -4,
		501,   -709,  32767, 554,   -282,  -329,  649,   137,
		375,   32767, 483,   32767, 0,     424,   32767, 32767,
		-339,  43,    32767, 1435,  0,     0,     536,   0,
		-61,   -787,  32767, 425,   979,   777,   0,     32767,
		32767, 73,    32767, 0,     32767, 399,   216,   268,
		-125,  32767, 32767, 865,   -221,  32767, 1308,  54,
		0,     32767, 32767, -83,   331,   0,     -347,  -27,
		-195,  32767, 236,   32767, 32767, 188,   32767, 32767,
		0,     32767, 32767, 32767, 32767, 32767, 0,     32767,
		-620,  32767, 32767, -352,  32767, -37,   32767, 224,
		1206,  -182,  -44,   661,   74,    515,   -433,  -177,
		417,   0,     700,   442,   32767, 617,   0,     1409,
		676,   -169,  32767, 32767, 32767, 0,     481,   -49,
		264,   0,     32767, 585,   529,   83,    283,   570,
		306,   32767, 0,     32767, 357,   482,   -236,  48,
		460,   32767, 32767, -340,  32767, 600,   32767, 466,
		32767, 143,   32767, 196,   0,     251,   32767, 460,
		32767, 32767, 104,   285,   32767, -336,  522,   32767,
		32767, 486,   32767, 32767, 32767, 579,   32767, 155,
		-166,  -229,  -128,  258,   -58,   32767, 32767, 0,
		284,   0,     601,   32767, 24,    -3,    32767, 0,
		-216,  686,   32767, 782,   32767, -560,  578,   32767,
		0,     32767, 171,   32767, 32767, 32767, 32767, 32767,
		32767, 369,   42,    11,    -178,  -164,  194,   -66,
		1434,  32767, 32767, -152,  0,     203,   481,   272,
		-492,  -8,    -89,   32767, 109,   266,   -478,  0,
		-98,   54,    -164,  32767, 360,   -249,  32767, 293,
		110,   367,   1251,  33,    -406,  0,     -98,   32767,
		-208,  32767, 32767, 32767, 0,     32767, 32767, 0,
		32767, 680,   0,     32767, -64,   32767, 479,   0,
		-429,  32767, 99,    437,   0,     0,     584,   523,
		382,   901,   803,   536,   410,   0,     32767, 32767,
		32767, 431,   32767, 179,   384,   32767, 32767, 681,
		32767, 822,   0,     0,     32767, -120,  32767, 0,
		677,   -380,  32767, 502,   32767, 0,     32767, 32767,
		0,     4,     0,     0,     32767, 32767, 353,   32767,
		262,   32767, 480,   0,     0,     -151,  32767, 0,
		195,   463,   32767, 240,   -387,  588,   32767, 106,
		133,   398,   1719,  173,   0,     556,   32767, 32767,
		369,   -515,  -1171, 32767, 476,   32767, 0,     32767,
		-553,  681,   32767, 6,     75,    400,   0,     32767,
		0,     32767, 0,     32767, -266,  32767, 0,     32767,
		32767, 32767, -250,  32767, 510,   32767, 32767, 23,
		558,   32767, 32767, 795,   600,   337,   0,     608,
		11,    274,   -348,  32767, 627,   -462,  279,   32767,
		254,   263,   0,     -51,   0,     32767, 660,   40,
		593,   546,   -96,   0,     32767, 32767, 646,   -293,
		32767, 32767, 32767, 32767, 32767, 478,   32767, 0,
		0,     0,     0,     345,   32767, -1206, 210,   506,
		548,   0,     32767, 656,   134,   0,     104,   -483,
		32767, 208,   32767, 32767, 514,   32767, 37,    -236,
		280,   -406,  32767, -139,  -699,  32767, -395,  192,
		269,   32767, 312,   32767, 252,   -41,   32767, 32767,
		-108,  32767, 0,     587,   141,   472,   32767, 656,
		32,    32767, -259,  0,     0,     32767, 622,   285,
		0,     623,   32767, 0,     32767, 52,    16,    32767,
		36,    32767, 32767, 32767, 720,   431,   649,   -242,
		32767, -238,  -17,   678,   32767, 184,   0,     346,
		156,   32767, 292,   -153,  669,   455,   32767, 0,
		32767, -561,  32767, -45,   113,   -33,   32767, 123,
		681,   0,     32767, 32767, 502,   1122,  206,   -977,
		482,   449,   1290,  32767, -112,  32767, -626,  163,
		-51,   32767, 0,     32767, 333,   0,     709,   -147,
		96,    405,   32767, 597,   32767, 0,     338,   32767,
		-69,   409,   -38,   32767, 32767, 416,   32767, 32767,
		349,   -882,  36,    0,     289,   0,     0,     144,
		-249,  328,   32767, 365,   32767, 32767, 0,     254,
		32767, 32767, 149,   32767, -562,  32767, 0,     32767,
		32767, 32767, 32767, 32767, 32767, 1312,  258,   32767,
		32767, 32767, 32767, 160,   32767, 809,   32767, 928,
		32767, 32767, 152,   611,   0,     465,   107,   32767,
		32767, 0,     610,   32767, 551,   752,   175,   500,
		-252,  32767, 32767, 0,     644,   300,   323,   299,
		32767, 366,   933,   32767, 254,   32767, 0,     475,
		32767, -99,   32767, 0,     32767, 32767, 0,     288,
		0,     32767, 322,   32767, 32767, 454,   531,   340,
		-219,  430,   32767, 32767, 0,     354,   32767, -307,
		-611,  32767, -131,  0,     32767, 0,     0,     32767,
		259,   -193,  32767, 239,   307,   0,     258,   32767,
		32767, 164,   32767, 0,     -592,  32767, 32767, 7,
		0,     242,   487,   32767, 0,     32767, 32767, 0,
		-81,   663,   5,     15,    32767, 32767, 32767, 239,
		32767, 32767, 32767, 256,   565,   170,   32767, 32767,
		446,   32767, 32767, 32767, 32767, -166,  636,   32767,
		32767, 531,   32767, -365,  32767, 495,   583,   854,
		0,     -140,  0,     32767, 373,   32767, -593,  -226,
		396,   0,     282,   -374,  32767, 32767, 644,   140,
		32767, 388,   32767, 0,     32767, 1641,  32767, 811,
		32767, 32767, 32767, 32767, 96,    32767, 683,   -27,
		-297,  438,   -109,  219,   32767, 278,   175,   445,
		32767, 32767, -530,  513,   0,     297,   0,     596,
		32767, 32767, -410,  32767, 32767, 32767, 32767, 1341,
		32767, 306,   193,   169,   36,    0,     -36,   32767,
		68,    32767, 1152,  32767, -263,  0,     32767, 0,
		0,     144,   0,     -156,  0,     135,   0,     0,
		32767, 0,     82,    0,     7,     32767, 32767, 926,
		32767, -453,  0,     32767, 217,   -14,   32767, 605,
		32767, 0,     355,   32767, 32767, 32767, 32767, 0,
		32767, 32767, 595,   1108,  171,   32767, 301,   105,
		385,   32767, 0,     0,     -104,  32767, 178,   -67,
		32767, 32767, 1119,  0,     0,     32767, 32767, 32767,
		32767, -328,  32767, 201,   0,     544,   0,     32767,
		32767, 32767, 177,   32767, 32767, 32767, 329,   156,
		32767, 187,   -221,  32767, 32767, 560,   321,   702,
		32767, 0,     32767, 321,   115,   32767, 32767, 614,
		32767, 61,    151,   484,   32767, 0,     -164,  32767,
		275,   -95,   608,   0,     348,   0,     604,   186,
		243,   0,     0,     32767, 516,   32767, -1185, 32767,
		216,   190,   54,    32767, 387,   428,   0,     32767,
		-8,    422,   83,    32767, 32767, 156,   -168,  32767,
		155,   32767, 0,     0,     32767, 7,     32767, 0,
		481,   0,     115,   132,   29,    0,     569,   32767,
		-211,  32767, 329,   32767, 0,     641,   193,   32767,
		0,     -7,    -878,  32767, 32767, 32767, 32767, 37,
		32767, 32767, 421,   509,   190,   32767, 287,   32767,
		0,     32767, 46,    66,    32767, 0,     0,     667,
		0,     0,     117,   32767, 619,   32767, 32767, 32767,
		32767, 0,     32767, 32767, 70,    662,   0,     435,
		32767, 220,   0,     0,     451,   569,   32767, 32767,
		32767, 32767, 0,     32767, 0,     32767, 0,     -1051,
		69,    0,     530,   0,     0,     391,   32767, 32767,
		62,    32767, 32767, 32767, 127,   32767, 32767, 0,
		32767, 32767, 32767, -454,  833,   32767, 44,    0,
		624,   535,   32767, 32767, 32767, 648,   32767, 501,
		32767, 1060,  0,     -735,  234,   55,    325,   0,
		324,   94,    0
	};

	const unsigned char *k = (const unsigned char *) key;
	uint32		a = 0;
	uint32		b = 0;

	while (keylen--)
	{
		unsigned char c = *k++ | 0x20;

		a = a * 257 + c;
		b = b * 31 + c;
	}
	return h[a % 1387] + h[b % 1387];
}

const ScanKeywordList ScanKeywords = {
	ScanKeywords_kw_string,
	ScanKeywords_kw_offsets,
	ScanKeywords_hash_func,
	SCANKEYWORDS_NUM_KEYWORDS,
	26
};

#endif							/* KWLIST_D_H */

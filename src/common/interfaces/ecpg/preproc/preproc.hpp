/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_BASE_YY_PREPROC_HPP_INCLUDED
# define YY_BASE_YY_PREPROC_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int base_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SQL_ALLOCATE = 258,
    SQL_AUTOCOMMIT = 259,
    SQL_BOOL = 260,
    SQL_BREAK = 261,
    SQL_CALL = 262,
    SQL_CARDINALITY = 263,
    SQL_COUNT = 264,
    SQL_DATETIME_INTERVAL_CODE = 265,
    SQL_DATETIME_INTERVAL_PRECISION = 266,
    SQL_DESCRIBE = 267,
    SQL_DESCRIPTOR = 268,
    SQL_FOUND = 269,
    SQL_FREE = 270,
    SQL_GET = 271,
    SQL_GO = 272,
    SQL_GOTO = 273,
    SQL_IDENTIFIED = 274,
    SQL_INDICATOR = 275,
    SQL_KEY_MEMBER = 276,
    SQL_LENGTH = 277,
    SQL_LONG = 278,
    SQL_NULLABLE = 279,
    SQL_OCTET_LENGTH = 280,
    SQL_OPEN = 281,
    SQL_OUTPUT = 282,
    SQL_REFERENCE = 283,
    SQL_RETURNED_LENGTH = 284,
    SQL_RETURNED_OCTET_LENGTH = 285,
    SQL_SCALE = 286,
    SQL_SECTION = 287,
    SQL_SHORT = 288,
    SQL_SIGNED = 289,
    SQL_SQL = 290,
    SQL_SQLERROR = 291,
    SQL_SQLPRINT = 292,
    SQL_SQLWARNING = 293,
    SQL_START = 294,
    SQL_STOP = 295,
    SQL_STRUCT = 296,
    SQL_UNSIGNED = 297,
    SQL_VAR = 298,
    SQL_WHENEVER = 299,
    S_ADD = 300,
    S_AND = 301,
    S_ANYTHING = 302,
    S_AUTO = 303,
    S_CONST = 304,
    S_DEC = 305,
    S_DIV = 306,
    S_DOTPOINT = 307,
    S_EQUAL = 308,
    S_EXTERN = 309,
    S_INC = 310,
    S_LSHIFT = 311,
    S_MEMPOINT = 312,
    S_MEMBER = 313,
    S_MOD = 314,
    S_MUL = 315,
    S_NEQUAL = 316,
    S_OR = 317,
    S_REGISTER = 318,
    S_RSHIFT = 319,
    S_STATIC = 320,
    S_SUB = 321,
    S_VOLATILE = 322,
    S_TYPEDEF = 323,
    CSTRING = 324,
    CVARIABLE = 325,
    CPP_LINE = 326,
    SQL_IP = 327,
    DOLCONST = 328,
    ECONST = 329,
    NCONST = 330,
    UCONST = 331,
    UIDENT = 332,
    IDENT = 333,
    FCONST = 334,
    SCONST = 335,
    BCONST = 336,
    VCONST = 337,
    XCONST = 338,
    Op = 339,
    CmpOp = 340,
    CmpNullOp = 341,
    COMMENTSTRING = 342,
    SET_USER_IDENT = 343,
    SET_IDENT = 344,
    UNDERSCORE_CHARSET = 345,
    FCONST_F = 346,
    FCONST_D = 347,
    ICONST = 348,
    PARAM = 349,
    TYPECAST = 350,
    ORA_JOINOP = 351,
    DOT_DOT = 352,
    COLON_EQUALS = 353,
    PARA_EQUALS = 354,
    SET_IDENT_SESSION = 355,
    SET_IDENT_GLOBAL = 356,
    ABORT_P = 357,
    ABSOLUTE_P = 358,
    ACCESS = 359,
    ACCOUNT = 360,
    ACTION = 361,
    ADD_P = 362,
    ADMIN = 363,
    AFTER = 364,
    AGGREGATE = 365,
    ALGORITHM = 366,
    ALL = 367,
    ALSO = 368,
    ALTER = 369,
    ALWAYS = 370,
    ANALYSE = 371,
    ANALYZE = 372,
    AND = 373,
    ANY = 374,
    APP = 375,
    APPEND = 376,
    ARCHIVE = 377,
    ARRAY = 378,
    AS = 379,
    ASC = 380,
    ASSERTION = 381,
    ASSIGNMENT = 382,
    ASYMMETRIC = 383,
    AT = 384,
    ATTRIBUTE = 385,
    AUDIT = 386,
    AUTHID = 387,
    AUTHORIZATION = 388,
    AUTOEXTEND = 389,
    AUTOMAPPED = 390,
    AUTO_INCREMENT = 391,
    BACKWARD = 392,
    BARRIER = 393,
    BEFORE = 394,
    BEGIN_NON_ANOYBLOCK = 395,
    BEGIN_P = 396,
    BETWEEN = 397,
    BIGINT = 398,
    BINARY = 399,
    BINARY_DOUBLE = 400,
    BINARY_DOUBLE_INF = 401,
    BINARY_DOUBLE_NAN = 402,
    BINARY_INTEGER = 403,
    BIT = 404,
    BLANKS = 405,
    BLOB_P = 406,
    BLOCKCHAIN = 407,
    BODY_P = 408,
    BOGUS = 409,
    BOOLEAN_P = 410,
    BOTH = 411,
    BUCKETCNT = 412,
    BUCKETS = 413,
    BUILD = 414,
    BY = 415,
    BYTE_P = 416,
    BYTEAWITHOUTORDER = 417,
    BYTEAWITHOUTORDERWITHEQUAL = 418,
    CACHE = 419,
    CALL = 420,
    CALLED = 421,
    CANCELABLE = 422,
    CASCADE = 423,
    CASCADED = 424,
    CASE = 425,
    CAST = 426,
    CATALOG_P = 427,
    CATALOG_NAME = 428,
    CHAIN = 429,
    CHANGE = 430,
    CHAR_P = 431,
    CHARACTER = 432,
    CHARACTERISTICS = 433,
    CHARACTERSET = 434,
    CHARSET = 435,
    CHECK = 436,
    CHECKPOINT = 437,
    CLASS = 438,
    CLASS_ORIGIN = 439,
    CLEAN = 440,
    CLIENT = 441,
    CLIENT_MASTER_KEY = 442,
    CLIENT_MASTER_KEYS = 443,
    CLOB = 444,
    CLOSE = 445,
    CLUSTER = 446,
    COALESCE = 447,
    COLLATE = 448,
    COLLATION = 449,
    COLUMN = 450,
    COLUMN_ENCRYPTION_KEY = 451,
    COLUMN_ENCRYPTION_KEYS = 452,
    COLUMN_NAME = 453,
    COLUMNS = 454,
    COMMENT = 455,
    COMMENTS = 456,
    COMMIT = 457,
    COMMITTED = 458,
    COMPACT = 459,
    COMPATIBLE_ILLEGAL_CHARS = 460,
    COMPILE = 461,
    COMPLETE = 462,
    COMPLETION = 463,
    COMPRESS = 464,
    CONCURRENTLY = 465,
    CONDITION = 466,
    CONFIGURATION = 467,
    CONNECTION = 468,
    CONSISTENT = 469,
    CONSTANT = 470,
    CONSTRAINT = 471,
    CONSTRAINT_CATALOG = 472,
    CONSTRAINT_NAME = 473,
    CONSTRAINT_SCHEMA = 474,
    CONSTRAINTS = 475,
    CONTENT_P = 476,
    CONTINUE_P = 477,
    CONTVIEW = 478,
    CONVERSION_P = 479,
    CONVERT_P = 480,
    CONNECT = 481,
    COORDINATOR = 482,
    COORDINATORS = 483,
    COPY = 484,
    COST = 485,
    CREATE = 486,
    CROSS = 487,
    CSN = 488,
    CSV = 489,
    CUBE = 490,
    CURRENT_P = 491,
    CURRENT_CATALOG = 492,
    CURRENT_DATE = 493,
    CURRENT_ROLE = 494,
    CURRENT_SCHEMA = 495,
    CURRENT_TIME = 496,
    CURRENT_TIMESTAMP = 497,
    CURRENT_USER = 498,
    CURSOR = 499,
    CURSOR_NAME = 500,
    CYCLE = 501,
    SHRINK = 502,
    USE_P = 503,
    DATA_P = 504,
    DATABASE = 505,
    DATAFILE = 506,
    DATANODE = 507,
    DATANODES = 508,
    DATATYPE_CL = 509,
    DATE_P = 510,
    DATE_FORMAT_P = 511,
    DAY_P = 512,
    DAY_HOUR_P = 513,
    DAY_MINUTE_P = 514,
    DAY_SECOND_P = 515,
    DBCOMPATIBILITY_P = 516,
    DEALLOCATE = 517,
    DEC = 518,
    DECIMAL_P = 519,
    DECLARE = 520,
    DECODE = 521,
    DEFAULT = 522,
    DEFAULTS = 523,
    DEFERRABLE = 524,
    DEFERRED = 525,
    DEFINER = 526,
    DELETE_P = 527,
    DELIMITER = 528,
    DELIMITERS = 529,
    DELTA = 530,
    DELTAMERGE = 531,
    DESC = 532,
    DETERMINISTIC = 533,
    DIAGNOSTICS = 534,
    DICTIONARY = 535,
    DIRECT = 536,
    DIRECTORY = 537,
    DISABLE_P = 538,
    DISCARD = 539,
    DISTINCT = 540,
    DISTRIBUTE = 541,
    DISTRIBUTION = 542,
    DO = 543,
    DOCUMENT_P = 544,
    DOMAIN_P = 545,
    DOUBLE_P = 546,
    DROP = 547,
    DUPLICATE = 548,
    DISCONNECT = 549,
    DUMPFILE = 550,
    EACH = 551,
    ELASTIC = 552,
    ELSE = 553,
    ENABLE_P = 554,
    ENCLOSED = 555,
    ENCODING = 556,
    ENCRYPTED = 557,
    ENCRYPTED_VALUE = 558,
    ENCRYPTION = 559,
    ENCRYPTION_TYPE = 560,
    END_P = 561,
    ENDS = 562,
    ENFORCED = 563,
    ENUM_P = 564,
    ERROR_P = 565,
    ERRORS = 566,
    ESCAPE = 567,
    EOL = 568,
    ESCAPING = 569,
    EVENT = 570,
    EVENTS = 571,
    EVERY = 572,
    EXCEPT = 573,
    EXCHANGE = 574,
    EXCLUDE = 575,
    EXCLUDED = 576,
    EXCLUDING = 577,
    EXCLUSIVE = 578,
    EXECUTE = 579,
    EXISTS = 580,
    EXPIRED_P = 581,
    EXPLAIN = 582,
    EXTENSION = 583,
    EXTERNAL = 584,
    EXTRACT = 585,
    ESCAPED = 586,
    FALSE_P = 587,
    FAMILY = 588,
    FAST = 589,
    FENCED = 590,
    FETCH = 591,
    FIELDS = 592,
    FILEHEADER_P = 593,
    FILL_MISSING_FIELDS = 594,
    FILLER = 595,
    FILTER = 596,
    FIRST_P = 597,
    FIXED_P = 598,
    FLOAT_P = 599,
    FOLLOWING = 600,
    FOLLOWS_P = 601,
    FOR = 602,
    FORCE = 603,
    FOREIGN = 604,
    FORMATTER = 605,
    FORWARD = 606,
    FEATURES = 607,
    FREEZE = 608,
    FROM = 609,
    FULL = 610,
    FUNCTION = 611,
    FUNCTIONS = 612,
    GENERATED = 613,
    GET = 614,
    GLOBAL = 615,
    GRANT = 616,
    GRANTED = 617,
    GREATEST = 618,
    GROUP_P = 619,
    GROUPING_P = 620,
    GROUPPARENT = 621,
    HANDLER = 622,
    HAVING = 623,
    HDFSDIRECTORY = 624,
    HEADER_P = 625,
    HOLD = 626,
    HOUR_P = 627,
    HOUR_MINUTE_P = 628,
    HOUR_SECOND_P = 629,
    IDENTIFIED = 630,
    IDENTITY_P = 631,
    IF_P = 632,
    IGNORE = 633,
    IGNORE_EXTRA_DATA = 634,
    ILIKE = 635,
    IMMEDIATE = 636,
    IMMUTABLE = 637,
    IMPLICIT_P = 638,
    IN_P = 639,
    INCLUDE = 640,
    INCLUDING = 641,
    INCREMENT = 642,
    INCREMENTAL = 643,
    INDEX = 644,
    INDEXES = 645,
    INFILE = 646,
    INFINITE_P = 647,
    INHERIT = 648,
    INHERITS = 649,
    INITIAL_P = 650,
    INITIALLY = 651,
    INITRANS = 652,
    INLINE_P = 653,
    INNER_P = 654,
    INOUT = 655,
    INPUT_P = 656,
    INSENSITIVE = 657,
    INSERT = 658,
    INSTEAD = 659,
    INT_P = 660,
    INTEGER = 661,
    INTERNAL = 662,
    INTERSECT = 663,
    INTERVAL = 664,
    INTO = 665,
    INVISIBLE = 666,
    INVOKER = 667,
    IP = 668,
    IS = 669,
    ISNULL = 670,
    ISOLATION = 671,
    JOIN = 672,
    KEY = 673,
    KILL = 674,
    KEY_PATH = 675,
    KEY_STORE = 676,
    LABEL = 677,
    LANGUAGE = 678,
    LARGE_P = 679,
    LAST_P = 680,
    LC_COLLATE_P = 681,
    LC_CTYPE_P = 682,
    LEADING = 683,
    LEAKPROOF = 684,
    LINES = 685,
    LEAST = 686,
    LESS = 687,
    LEFT = 688,
    LEVEL = 689,
    LIKE = 690,
    LIMIT = 691,
    LIST = 692,
    LISTEN = 693,
    LOAD = 694,
    LOCAL = 695,
    LOCALTIME = 696,
    LOCALTIMESTAMP = 697,
    LOCATION = 698,
    LOCK_P = 699,
    LOCKED = 700,
    LOG_ON = 701,
    LOG_P = 702,
    LOGGING = 703,
    LOGIN_ANY = 704,
    LOGIN_FAILURE = 705,
    LOGIN_SUCCESS = 706,
    LOGOUT = 707,
    LOOP = 708,
    MAPPING = 709,
    MASKING = 710,
    MASTER = 711,
    MATCH = 712,
    MATERIALIZED = 713,
    MATCHED = 714,
    MAXEXTENTS = 715,
    MAXSIZE = 716,
    MAXTRANS = 717,
    MAXVALUE = 718,
    MERGE = 719,
    MESSAGE_TEXT = 720,
    METHOD = 721,
    MINUS_P = 722,
    MINUTE_P = 723,
    MINUTE_SECOND_P = 724,
    MINVALUE = 725,
    MINEXTENTS = 726,
    MODE = 727,
    MODEL = 728,
    MODIFY_P = 729,
    MONTH_P = 730,
    MOVE = 731,
    MOVEMENT = 732,
    MYSQL_ERRNO = 733,
    NAME_P = 734,
    NAMES = 735,
    NAN_P = 736,
    NATIONAL = 737,
    NATURAL = 738,
    NCHAR = 739,
    NEXT = 740,
    NO = 741,
    NOCOMPRESS = 742,
    NOCYCLE = 743,
    NODE = 744,
    NOLOGGING = 745,
    NOMAXVALUE = 746,
    NOMINVALUE = 747,
    NONE = 748,
    NOT = 749,
    NOTHING = 750,
    NOTIFY = 751,
    NOTNULL = 752,
    NOVALIDATE = 753,
    NOWAIT = 754,
    NULL_P = 755,
    NULLCOLS = 756,
    NULLIF = 757,
    NULLS_P = 758,
    NUMBER_P = 759,
    NUMERIC = 760,
    NUMSTR = 761,
    NVARCHAR = 762,
    NVARCHAR2 = 763,
    NVL = 764,
    OBJECT_P = 765,
    OF = 766,
    OFF = 767,
    OFFSET = 768,
    OIDS = 769,
    ON = 770,
    ONLY = 771,
    OPERATOR = 772,
    OPTIMIZATION = 773,
    OPTION = 774,
    OPTIONALLY = 775,
    OPTIONS = 776,
    OR = 777,
    ORDER = 778,
    OUT_P = 779,
    OUTER_P = 780,
    OVER = 781,
    OVERLAPS = 782,
    OVERLAY = 783,
    OWNED = 784,
    OWNER = 785,
    OUTFILE = 786,
    PACKAGE = 787,
    PACKAGES = 788,
    PARALLEL_ENABLE = 789,
    PARSER = 790,
    PARTIAL = 791,
    PARTITION = 792,
    PARTITIONS = 793,
    PASSING = 794,
    PASSWORD = 795,
    PCTFREE = 796,
    PER_P = 797,
    PERCENT = 798,
    PERFORMANCE = 799,
    PERM = 800,
    PLACING = 801,
    PLAN = 802,
    PLANS = 803,
    POLICY = 804,
    POSITION = 805,
    PIPELINED = 806,
    POOL = 807,
    PRECEDING = 808,
    PRECISION = 809,
    PREDICT = 810,
    PREFERRED = 811,
    PREFIX = 812,
    PRESERVE = 813,
    PREPARE = 814,
    PREPARED = 815,
    PRIMARY = 816,
    PRECEDES_P = 817,
    PRIVATE = 818,
    PRIOR = 819,
    PRIORER = 820,
    PRIVILEGES = 821,
    PRIVILEGE = 822,
    PROCEDURAL = 823,
    PROCEDURE = 824,
    PROFILE = 825,
    PUBLICATION = 826,
    PUBLISH = 827,
    PURGE = 828,
    QUERY = 829,
    QUOTE = 830,
    RANDOMIZED = 831,
    RANGE = 832,
    RATIO = 833,
    RAW = 834,
    READ = 835,
    REAL = 836,
    REASSIGN = 837,
    REBUILD = 838,
    RECHECK = 839,
    RECURSIVE = 840,
    RECYCLEBIN = 841,
    REDISANYVALUE = 842,
    REF = 843,
    REFERENCES = 844,
    REFRESH = 845,
    REINDEX = 846,
    REJECT_P = 847,
    RELATIVE_P = 848,
    RELEASE = 849,
    RELOPTIONS = 850,
    REMOTE_P = 851,
    REMOVE = 852,
    RENAME = 853,
    REPEAT = 854,
    REPEATABLE = 855,
    REPLACE = 856,
    REPLICA = 857,
    RESET = 858,
    RESIZE = 859,
    RESOURCE = 860,
    RESTART = 861,
    RESTRICT = 862,
    RETURN = 863,
    RETURNED_SQLSTATE = 864,
    RETURNING = 865,
    RETURNS = 866,
    REUSE = 867,
    REVOKE = 868,
    RIGHT = 869,
    ROLE = 870,
    ROLES = 871,
    ROLLBACK = 872,
    ROLLUP = 873,
    ROTATE = 874,
    ROTATION = 875,
    ROW = 876,
    ROW_COUNT = 877,
    ROWNUM = 878,
    ROWS = 879,
    ROWTYPE_P = 880,
    RULE = 881,
    SAMPLE = 882,
    SAVEPOINT = 883,
    SCHEDULE = 884,
    SCHEMA = 885,
    SCHEMA_NAME = 886,
    SCROLL = 887,
    SEARCH = 888,
    SECOND_P = 889,
    SECURITY = 890,
    SELECT = 891,
    SEPARATOR_P = 892,
    SEQUENCE = 893,
    SEQUENCES = 894,
    SERIALIZABLE = 895,
    SERVER = 896,
    SESSION = 897,
    SESSION_USER = 898,
    SET = 899,
    SETS = 900,
    SETOF = 901,
    SHARE = 902,
    SHIPPABLE = 903,
    SHOW = 904,
    SHUTDOWN = 905,
    SIBLINGS = 906,
    SIMILAR = 907,
    SIMPLE = 908,
    SIZE = 909,
    SKIP = 910,
    SLAVE = 911,
    SLICE = 912,
    SMALLDATETIME = 913,
    SMALLDATETIME_FORMAT_P = 914,
    SMALLINT = 915,
    SNAPSHOT = 916,
    SOME = 917,
    SOURCE_P = 918,
    SPACE = 919,
    SPECIFICATION = 920,
    SPILL = 921,
    SPLIT = 922,
    STABLE = 923,
    STACKED_P = 924,
    STANDALONE_P = 925,
    START = 926,
    STARTS = 927,
    STARTWITH = 928,
    STATEMENT = 929,
    STATEMENT_ID = 930,
    STATISTICS = 931,
    STDIN = 932,
    STDOUT = 933,
    STORAGE = 934,
    STORE_P = 935,
    STORED = 936,
    STRATIFY = 937,
    STREAM = 938,
    STRICT_P = 939,
    STRIP_P = 940,
    SUBCLASS_ORIGIN = 941,
    SUBPARTITION = 942,
    SUBPARTITIONS = 943,
    SUBSCRIPTION = 944,
    SUBSTRING = 945,
    SYMMETRIC = 946,
    SYNONYM = 947,
    SYSDATE = 948,
    SYSID = 949,
    SYSTEM_P = 950,
    SYS_REFCURSOR = 951,
    STARTING = 952,
    SQL_P = 953,
    TABLE = 954,
    TABLE_NAME = 955,
    TABLES = 956,
    TABLESAMPLE = 957,
    TABLESPACE = 958,
    TARGET = 959,
    TEMP = 960,
    TEMPLATE = 961,
    TEMPORARY = 962,
    TERMINATED = 963,
    TEXT_P = 964,
    THAN = 965,
    THEN = 966,
    TIME = 967,
    TIME_FORMAT_P = 968,
    TIMECAPSULE = 969,
    TIMESTAMP = 970,
    TIMESTAMP_FORMAT_P = 971,
    TIMESTAMPDIFF = 972,
    TINYINT = 973,
    TO = 974,
    TRAILING = 975,
    TRANSACTION = 976,
    TRANSFORM = 977,
    TREAT = 978,
    TRIGGER = 979,
    TRIM = 980,
    TRUE_P = 981,
    TRUNCATE = 982,
    TRUSTED = 983,
    TSFIELD = 984,
    TSTAG = 985,
    TSTIME = 986,
    TYPE_P = 987,
    TYPES_P = 988,
    UNBOUNDED = 989,
    UNCOMMITTED = 990,
    UNENCRYPTED = 991,
    UNION = 992,
    UNIQUE = 993,
    UNKNOWN = 994,
    UNLIMITED = 995,
    UNLISTEN = 996,
    UNLOCK = 997,
    UNLOGGED = 998,
    UNTIL = 999,
    UNUSABLE = 1000,
    UPDATE = 1001,
    USEEOF = 1002,
    USER = 1003,
    USING = 1004,
    VACUUM = 1005,
    VALID = 1006,
    VALIDATE = 1007,
    VALIDATION = 1008,
    VALIDATOR = 1009,
    VALUE_P = 1010,
    VALUES = 1011,
    VARCHAR = 1012,
    VARCHAR2 = 1013,
    VARIABLES = 1014,
    VARIADIC = 1015,
    VARRAY = 1016,
    VARYING = 1017,
    VCGROUP = 1018,
    VERBOSE = 1019,
    VERIFY = 1020,
    VERSION_P = 1021,
    VIEW = 1022,
    VISIBLE = 1023,
    VOLATILE = 1024,
    WAIT = 1025,
    WARNINGS = 1026,
    WEAK = 1027,
    WHEN = 1028,
    WHERE = 1029,
    WHILE_P = 1030,
    WHITESPACE_P = 1031,
    WINDOW = 1032,
    WITH = 1033,
    WITHIN = 1034,
    WITHOUT = 1035,
    WORK = 1036,
    WORKLOAD = 1037,
    WRAPPER = 1038,
    WRITE = 1039,
    XML_P = 1040,
    XMLATTRIBUTES = 1041,
    XMLCONCAT = 1042,
    XMLELEMENT = 1043,
    XMLEXISTS = 1044,
    XMLFOREST = 1045,
    XMLPARSE = 1046,
    XMLPI = 1047,
    XMLROOT = 1048,
    XMLSERIALIZE = 1049,
    YEAR_P = 1050,
    YEAR_MONTH_P = 1051,
    YES_P = 1052,
    ZONE = 1053,
    NULLS_FIRST = 1054,
    NULLS_LAST = 1055,
    WITH_TIME = 1056,
    INCLUDING_ALL = 1057,
    RENAME_PARTITION = 1058,
    PARTITION_FOR = 1059,
    SUBPARTITION_FOR = 1060,
    ADD_PARTITION = 1061,
    DROP_PARTITION = 1062,
    REBUILD_PARTITION = 1063,
    MODIFY_PARTITION = 1064,
    ADD_SUBPARTITION = 1065,
    DROP_SUBPARTITION = 1066,
    NOT_ENFORCED = 1067,
    VALID_BEGIN = 1068,
    DECLARE_CURSOR = 1069,
    ON_UPDATE_TIME = 1070,
    START_WITH = 1071,
    CONNECT_BY = 1072,
    SHOW_ERRORS = 1073,
    END_OF_INPUT = 1074,
    END_OF_INPUT_COLON = 1075,
    END_OF_PROC = 1076,
    EVENT_TRIGGER = 1077,
    NOT_IN = 1078,
    NOT_BETWEEN = 1079,
    NOT_LIKE = 1080,
    NOT_ILIKE = 1081,
    NOT_SIMILAR = 1082,
    FORCE_INDEX = 1083,
    USE_INDEX = 1084,
    IGNORE_INDEX = 1085,
    CURSOR_EXPR = 1086,
    PARTIAL_EMPTY_PREC = 1087,
    POSTFIXOP = 1088,
    lower_than_index = 1089,
    higher_than_rotate = 1090,
    UMINUS = 1091,
    EMPTY_FROM_CLAUSE = 1092
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 564 "preproc.y" /* yacc.c:1909  */

	double	dval;
	char	*str;
	int		ival;
	struct	when		action;
	struct	index		index;
	int		tagname;
	struct	this_type	type;
	enum	ECPGttype	type_enum;
	enum	ECPGdtype	dtype_enum;
	struct	fetch_desc	descriptor;
	struct  su_symbol	struct_union;
	struct	prep		prep;

#line 907 "preproc.hpp" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE base_yylval;
extern YYLTYPE base_yylloc;
int base_yyparse (void);

#endif /* !YY_BASE_YY_PREPROC_HPP_INCLUDED  */

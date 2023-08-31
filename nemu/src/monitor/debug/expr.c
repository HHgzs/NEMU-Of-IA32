#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum
{
	NOTYPE = 256,
	EQUAL,
	NUM,
	NOTEQUAL,
	OR,
	AND,
	REG,
	REF,
	NEG

};

static struct rule
{
	char *regex;
	int symbol_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	// {" +",	NOTYPE},
	// {"0x[0-9,a-f]+", HEXNUM},
	// {"[0-9]+", NUM},
	// {"\\$[a-z]{2,3}", REGNAME},
	// {"\\(", '('},
	// {"\\)", ')'},
	// {"\\*", '*'},
	// {"\\/", '/'},
	// {"\\+", '+'},
	// {"\\-", '-'},
	// {"==", EQUAL},
	// {"!=", NOTEQUAL},
	// {"&&", AND},
	// {"\\|\\|", OR},
	// {"!", '!'}

	{" +", NOTYPE},
	{"\\+", '+'},
	{"==", EQUAL},
	{"0x[0-9a-fA-F]{1,8}", NUM},
	{"[0-9]{1,10}", NUM},
	{"\\$[a-z]{1,31}", REG},
	{"-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"!=", NOTEQUAL},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", '!'},
	{"\\(", '('},
	{"\\)", ')'}

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct symbol
{
	int type;
	char str[32];
} Token;

Token symbols[32];
int symbol_;

static bool make_symbol(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	symbol_ = 0;

	while (e[position] != '\0')
	{

		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("regular expressions \"%s\" fit at %d, whitch length is %d: %.*s", rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				switch (rules[i].symbol_type)
				{
				case NOTYPE:
					break;
				case NUM:
				case REG:
					sprintf(symbols[symbol_].str, "%.*s", substr_len, substr_start);
				default:
					symbols[symbol_].type = rules[i].symbol_type;
					symbol_++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static int op2code(int t)
{
	switch (t)
	{
	case '!':
	case NEG:
	case REF:
		return 0;
	case '*':
	case '/':
	case '%':
		return 1;
	case '+':
	case '-':
		return 2;
	case EQUAL:
	case NOTEQUAL:
		return 4;
	case AND:
		return 8;
	case OR:
		return 9;
	default:
		assert(0);
	}
}

static inline int op_compare(int t1, int t2)
{
	return op2code(t1) - op2code(t2);
}

static int master_op_finder(int s, int e, bool *success)
{
	int i;
	int bracket_level = 0;
	int master_op = -1;
	for (i = s; i <= e; i++)
	{
		switch (symbols[i].type)
		{
		case REG:
		case NUM:
			break;

		case '(':
			bracket_level++;
			break;

		case ')':
			bracket_level--;
			if (bracket_level < 0)
			{
				*success = false;
				return 0;
			}
			break;

		default:
			if (bracket_level == 0)
			{

				if (master_op == -1 ||
					op_compare(symbols[master_op].type, symbols[i].type) < 0 ||
					(op_compare(symbols[master_op].type, symbols[i].type) == 0 &&
					 symbols[i].type != '!' && symbols[i].type != '~' &&
					 symbols[i].type != NEG && symbols[i].type != REF))
				{
					master_op = i;
				}
			}
			break;
		}
	}

	*success = (master_op != -1);
	return master_op;
}

uint32_t get_reg_val(const char *, bool *);

static uint32_t eval(int s, int e, bool *success)
{
	if (s > e)
	{
		*success = false;
		return 0;
	}
	else if (s == e)
	{
		uint32_t val;
		switch (symbols[s].type)
		{
		case REG:
			val = get_reg_val(symbols[s].str + 1, success); // skip '$'
			if (!*success)
			{
				return 0;
			}
			break;

		case NUM:
			val = strtol(symbols[s].str, NULL, 0);
			break;

		default:
			assert(0);
		}

		*success = true;
		return val;
	}
	else if (symbols[s].type == '(' && symbols[e].type == ')')
	{
		return eval(s + 1, e - 1, success);
	}
	else
	{









		int master_op = master_op_finder(s, e, success);




















		
		if (!*success)
		{
			return 0;
		}

		int op_type = symbols[master_op].type;
		if (op_type == '!' || op_type == NEG || op_type == REF)
		{
			uint32_t val = eval(master_op + 1, e, success);
			if (!*success)
			{
				return 0;
			}

			switch (op_type)
			{
			case '!':
				return !val;
			case NEG:
				return -val;
			case REF:
				return swaddr_read(val, 4);
			default:
				assert(0);
			}
		}

		uint32_t val1 = eval(s, master_op - 1, success);
		if (!*success)
		{
			return 0;
		}
		uint32_t val2 = eval(master_op + 1, e, success);
		if (!*success)
		{
			return 0;
		}

		switch (op_type)
		{
		case '+':
			return val1 + val2;
		case '-':
			return val1 - val2;
		case '*':
			return val1 * val2;
		case '/':
			return val1 / val2;
		case '%':
			return val1 % val2;
		case EQUAL:
			return val1 == val2;
		case NOTEQUAL:
			return val1 != val2;
		case AND:
			return val1 && val2;
		case OR:
			return val1 || val2;
		default:
			assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success)
{
	if (!make_symbol(e))
	{
		*success = false;
		return 0;
	}

	int i;
	int prev_type;
	for (i = 0; i < symbol_; i++)
	{
		if (symbols[i].type == '-')
		{
			if (i == 0)
			{
				symbols[i].type = NEG;
				continue;
			}

			prev_type = symbols[i - 1].type;
			if (!(prev_type == ')' || prev_type == NUM || prev_type == REG))
			{
				symbols[i].type = NEG;
			}
		}

		else if (symbols[i].type == '*')
		{
			if (i == 0)
			{
				symbols[i].type = REF;
				continue;
			}

			prev_type = symbols[i - 1].type;
			if (!(prev_type == ')' || prev_type == NUM || prev_type == REG))
			{
				symbols[i].type = REF;
			}
		}
	}

	return eval(0, symbol_ - 1, success);
}

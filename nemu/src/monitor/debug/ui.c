#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

char *rl_gets()
{
	static char *line_read = NULL;

	if (line_read)
	{
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read)
	{
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args)
{
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args)
{
	return -1;
}

static int cmd_si(char *args)
{
	char *arg = strtok(NULL, " ");
	int steps = 0;
	if (arg == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	sscanf(arg, "%d", &steps);
	if (steps < -1)
	{
		printf("Error, N is an integer greater than or equal to -1\n");
		return 0;
	}
	cpu_exec(steps);

	return 0;
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");

	if (arg != NULL)
	{
		if (strcmp(arg, "r") == 0)
		{
			int i;
			for (i = 0; i < 4; i++)
			{
				printf("$%s\t(0x%08x)\n", regsl[i], cpu.gpr[i]._32);
			}

			// printf("%s\t\t0x%08x\t\t%d\n", "eip", cpu.eip, cpu.eip);
		}
		else if (strcmp(arg, "w") == 0)
		{
			wp_display();
		}
	}
	return 0;
}

static int cmd_p(char *args)
{
	bool success;

	if (args)
	{
		uint32_t r = expr(args, &success);
		if (success)
		{
			printf("0x%08x(%d)\n", r, r);
		}
		else
		{
			printf("Bad expression\n");
		}
	}
	return 0;
}

static int cmd_x(char *args)
{
	// printf("args: %s\n", args);
    // char* N = NULL;
    char* EXPR = NULL;

    // 查找第一个空格的位置
    char* spacePos = strchr(args, ' ');
    if (spacePos != NULL)
    {
        *spacePos = '\0'; // 将空格替换为字符串结束符'\0'
        // N = args;  第一个空格前的字符串
        EXPR = spacePos + 1; // 第一个空格后的字符串
    }
    else
    {
        // N = args;  如果没有空格，则整个字符串为第一个字符串
        EXPR = ""; // 第二个字符串为空字符串
    }

    // printf("N: %s\n", N);
    // printf("EXPR: %s\n", EXPR);

	int len;

	sscanf(args, "%d", &len);
	swaddr_t address;
	bool success;
	address = expr(EXPR, &success);
	if (!success)
	{
		printf("Bad expression\n");
		return 0;
	}
	// printf("address: 0x%08x: ", address);

	int i;
	for (i = 0; i < len; i++)
	{
		printf("0x%08x ", swaddr_read(address, 4));
		address += 4;
		if(i % 4 == 3) printf("\n");
	}
	printf("\n");

	return 0;
}


static int cmd_w(char *args)
{
	if (args)
	{
		int NO = wp_set(args);
		if (NO != -1)
		{
			printf("Set watchpoint #%d\n", NO);
		}
		else
		{
			printf("Bad expression\n");
		}
	}
	return 0;
}

static int cmd_d(char *args)
{
	int NO;
	sscanf(args, "%d", &NO);
	if (!wp_remove(NO))
	{
		printf("Watchpoint #%d does not exist\n", NO);
	}

	return 0;
}

static int cmd_help(char *args);

static struct
{
	char *name;
	char *description;
	int (*handler)(char *);
} cmd_table[] = {
	{"help", "Display informations about all supported commands", cmd_help},
	{"c", "Continue the execution of the program", cmd_c},
	{"q", "Exit NEMU", cmd_q},

	/* TODO: Add more commands */
	{"si", "Single step execution", cmd_si},
	{"info", "r to print register values\n       w to show watch point state", cmd_info},
	{"x", "examine memory", cmd_x},
	{"p", "calculate expression", cmd_p},
	{"w", "set a new watchpoint", cmd_w},
	{"d", "delete watchpoint", cmd_d}

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL)
	{
		/* no argument given */
		for (i = 0; i < NR_CMD; i++)
		{
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else
	{
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(arg, cmd_table[i].name) == 0)
			{
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop()
{
	while (1)
	{
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first symbol as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL)
		{
			continue;
		}

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end)
		{
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(cmd, cmd_table[i].name) == 0)
			{
				if (cmd_table[i].handler(args) < 0)
				{
					return;
				}
				break;
			}
		}

		if (i == NR_CMD)
		{
			printf("Unknown command '%s'\n", cmd);
		}
	}
}

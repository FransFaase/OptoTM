#include <stdio.h>

struct TM_Rule
{
	int col;
	char dir;
	int state;
	bool rep;
};

struct TM_State
{
	TM_Rule *rules;
};

struct TM
{
	int nr_states;
	int nr_cols;
	TM_State *states;
	
	TM(const char *input)
	{
		nr_cols = 0;
		nr_states = 0;
		for (const char *s = input; *s >= ' ';)
		{
			int col = s[0] - '0';
			if (col >= nr_cols)
				nr_cols = col + 1;
			int state = s[2] - 'A';
			if (state >= nr_states)
				nr_states = state + 1;
			s += 3;
			while (*s == ' ')
				s++;
		}

		states = new TM_State[nr_states];
		for (int i = 0; i < nr_states; i++)
			states[i].rules = new TM_Rule[nr_cols];
	
		int col_nr = 0;
		int state_nr = 0;
		for (const char *s = input; *s >= ' ';)
		{
			TM_Rule* rule = &states[state_nr].rules[col_nr];
			rule->col = s[0] - '0';
			rule->dir = s[1];
			rule->state = s[2] - 'A';
			if (++col_nr == nr_cols)
			{
				state_nr++;
				col_nr = 0;
			}
			s += 3;
			while (*s == ' ')
				s++;
		}
	}
	void print(FILE *f)
	{
		for (int s = 0; s < nr_states; s++)
		{
			for (int c = 0; c < nr_cols; c++)
				printf("%c%c%c ",
						states[s].rules[c].col + '0',
						states[s].rules[c].dir,
						states[s].rules[c].state + 'A');
			printf("\n");
		}
	}
};

struct Cell
{
	Cell *next;
	int col;
	long long mul;
};

Cell *old_cells = 0;

struct Tape
{
	long long step;
	Cell* left_tape;
	Cell* right_tape;
	int col;
	int state;
	
	Tape() : step(0), left_tape(0), right_tape(0), col(0), state(0) {}
	
	void popCell(Cell *&tape)
	{
		Cell *top = tape;
		tape = tape->next;
		top->next = old_cells;
		old_cells = top;
	}
	
	void push(Cell *&tape, int new_col, long long mul = 1)
	{
		if (tape != 0 && tape->col == new_col)
		{
			tape->mul += mul;
		}
		else if (new_col != 0 || tape != 0)
		{
			Cell *r;
			if (old_cells != 0)
			{
				r = old_cells;
				old_cells = old_cells->next;
			}
			else
				r = new Cell;
			r->next = tape;
			r->col = new_col;
			r->mul = mul;
			tape = r;
		}
	}
	
	void pop(Cell *&tape)
	{
		col = tape != 0 ? tape->col : 0;
		if (tape != 0 && --tape->mul == 0)
			popCell(tape);
	}
	
	/*
	void popN(Cell *&tape, long long n)
	{
		while (n > 0 && tape != 0 && tape->mul <= n)
		{
			col = tape->col;
			n -= tape->mul;
			popCell(tape);
		}
		if (n > 0 && tape != 0)
		{
			col = tape->col;
			tape->mul -= n;
		}
	}
	
	*/	
	long long popCol(Cell *&tape)
	{
		if (tape == 0 || tape->col != col)
			return 0;
		long long m = tape->mul;
		popCell(tape);
		col = tape != 0 ? tape->col : 0;
		return m;
	}

	bool print()
	{
		if (step > 1367361263051LL)
			return false;
		
		printf("%13lld %c%c ", step, 'A'+state, '0'+col);
		for (Cell *c = left_tape; c != 0; c = c->next)
		{
			printf("%c", '0'+c->col);
			if (c->mul > 1)
				printf("[%lld]", c->mul);
		}
		printf(" ");
		for (Cell *c = right_tape; c != 0; c = c->next)
		{
			printf("%c", '0'+c->col);
			if (c->mul > 1)
				printf("[%lld]", c->mul);
		}
		printf("\n");
		return true;
	}
	
	void moveRight(int new_col)
	{
		push(left_tape, new_col);
		pop(right_tape);
		step++;
	}
	
	void moveLeft(int new_col)
	{
		push(right_tape, new_col);
		pop(left_tape);
		step++;
	}

	void moveRightN(int new_col)
	{
		long long m = popCol(right_tape);
		push(left_tape, new_col, m+1);
		pop(right_tape);
		step += m+1;
	}
	
	void moveLeftN(int new_col)
	{
		long long m = popCol(left_tape);
		push(right_tape, new_col, m+1);
		pop(left_tape);
		step += m+1;
	}
};

void interpretter(TM *tm)
{
	Tape tape;
	
	for (int i = 0; i < 100000; i++)
	{
		tape.print();
		
		TM_Rule *rule = &tm->states[tape.state].rules[tape.col];
		if (tape.state != rule->state)
		{
			tape.state = rule->state;
			if (rule->dir == 'R')
				tape.moveRight(rule->col);
			else
				tape.moveLeft(rule->col);
		}
		else
		{
			if (rule->dir == 'R')
				tape.moveRightN(rule->col);
			else
				tape.moveLeftN(rule->col);
		}
	}
}

int main(int argc, char *argv[])
{
	TM *tm = new TM("1RB 2RA 1RA 2RB 2LB 3LA 0RB 0RA");
	tm->print(stdout);
	
	interpretter(tm);
}
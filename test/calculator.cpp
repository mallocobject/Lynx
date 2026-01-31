/**
 * Expression calculator
 * @Author Rizhong Li<lirizhong97@163.com>
 * @Date 2024-01-09
 */

#include <stack>
#include <string>

using namespace std;

typedef enum em_op_type_t
{
	OP_NONE,
	OP_PLUS,
	OP_MINUS,
	OP_MULTI,
	OP_DIV,
} op_type_t;
typedef enum em_token_type_t
{
	TOKEN_NUM,
	TOKEN_OP
} token_type_t;
typedef struct st_num_t
{
	double num;
	int ok;
} num_t;
typedef struct st_op_t
{
	op_type_t op;
} op_t;

void trim_left(char*& ps, char*& pe);
void trim_right(char*& ps, char*& pe);
num_t evaluate(const string& expr);
num_t expect_num(const string& expr, char*& ps, char*& pe);
op_t expect_op(char*& ps, char*& pe);

void trim_left(char*& ps, char*& pe)
{
	while (ps <= pe && isspace(*ps))
		++ps;
}

void trim_right(char*& ps, char*& pe)
{
	while (pe >= ps && isspace(*pe))
		--pe;
}

num_t expect_num(const string& expr, char*& ps, char*& pe)
{
	trim_left(ps, pe);
	if (ps > pe)
	{
		return {0, -1};
	}

	double sign = 1;
	if (ps[0] == '-')
	{
		ps++;
		sign = -1;
	}
	else if (ps[0] == '+')
	{
		ps++;
		sign = 1;
	}

	if (ps[0] == '(')
	{
		char* s = ps;
		char* e = ps;
		while (ps <= pe)
		{
			if (ps[0] == ')')
			{
				e = ps;
			}
			else
			{
				if (ps == pe)
					e = pe;
			}
			ps++;
			if (s != e)
			{
				string tmp_expr(expr, s - expr.c_str() + 1, e - s - 1);
				num_t num = evaluate(tmp_expr);
				num.num *= sign;
				return num;
			}
		}
	}
	else
	{
		string token;
		while (ps <= pe)
		{
			trim_left(ps, pe);
			if (ps > pe)
				break;
			if (std::isdigit(ps[0]) || ps[0] == '.')
			{
				token += ps[0];
			}
			else
			{
				break;
			}
			ps++;
		}

		if (!token.empty())
		{
			try
			{
				return {sign * std::stod(token), 0}; // Fixme: maybe overflow
			}
			catch (...)
			{
				return {0, -1};
			}
		}
	}

	return {0, -1};
}

op_t expect_op(char*& ps, char*& pe)
{
	trim_left(ps, pe);
	if (ps > pe)
		return {OP_NONE};
	char op = ps[0];
	ps++;
	switch (op)
	{
	case '+':
		return {OP_PLUS};
	case '-':
		return {OP_MINUS};
	case '*':
		return {OP_MULTI};
	case '/':
		return {OP_DIV};
	}
	return {OP_NONE};
}

num_t evaluate(const string& expr)
{
	num_t res = {0, -1};
	char* ps = (char*)expr.c_str();
	char* pe = (char*)expr.c_str() + expr.length() - 1;
	token_type_t state = TOKEN_NUM;
	stack<op_t> ops;
	stack<num_t> nums;

	trim_left(ps, pe);
	trim_right(ps, pe);
	if (ps > pe)
	{
		return res;
	}

	while (ps <= pe)
	{
		switch (state)
		{ /* state machine */
		case TOKEN_NUM:
		{
			num_t num = expect_num(expr, ps, pe);
			if (num.ok)
				return res;
			nums.push(num);
			state = TOKEN_OP;
			break;
		}
		case TOKEN_OP:
		{
			op_t op = expect_op(ps, pe);
			if (op.op == OP_NONE)
				return res;
			if (op.op == OP_PLUS || op.op == OP_MINUS)
			{
				ops.push(op);
				state = TOKEN_NUM;
			}
			else
			{
				num_t num = expect_num(expr, ps, pe);
				if (num.ok)
					return res;
				if (nums.size() <= ops.size())
					return res;
				num_t mid = nums.top();
				nums.pop();
				if (op.op == OP_MULTI)
				{
					mid.num *= num.num;
				}
				else
				{
					if (num.num == 0)
						return res;
					mid.num /= num.num;
				}
				nums.push(mid);
				state = TOKEN_OP;
			}
			break;
		}
		}
	}

	while (ops.size() > 0)
	{
		op_t op = ops.top();
		ops.pop();

		num_t n1 = nums.top();
		nums.pop();
		num_t n2 = nums.top();
		nums.pop();

		if (op.op == OP_PLUS)
		{
			n2.num += n1.num;
		}
		else if (op.op == OP_MINUS)
		{
			n2.num -= n1.num;
		}
		if (ops.empty())
		{
			res.num += n2.num;
		}
		else
		{
			nums.push(n2);
		}
	}

	if (ops.empty() && nums.empty())
	{
		res.ok = 0;
	}

	return res;
}
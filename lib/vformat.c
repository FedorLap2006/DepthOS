#include <depthos/string.h>
#include <depthos/tools.h>
#include <depthos/vformat.h>

enum {
	SIGN_DEFAULT,
	SIGN_SPACE,
	SIGN_PLUS,
};

enum {
	LENGTH_DEFAULT,
	LENGTH_CHAR,
	LENGTH_SHORT,
	LENGTH_LONG,
	LENGTH_LONG_LONG,
	LENGTH_INTMAX_T,
	LENGTH_SIZE_T,
	LENGTH_PTRDIFF_T,
};

struct vformat_flags {
	unsigned alternate : 1;
	unsigned padding : 1;
	unsigned left : 1;
	unsigned width : 1;
	unsigned precision : 1;
	unsigned signed_value : 1;

	char padding_value;
	int width_value;
	int precision_value;
	int length;
	int sign;
	int radix;
};

static void output_copy(void (*output)(void *context, const char *data, size_t sz),
			void *context, int c, size_t n)
{
	char buf[64];

	if (n < sizeof(buf))
		memset(buf, c, n);
	else
		memset(buf, c, sizeof(buf));

	while (n > 0) {
		size_t w = n < sizeof(buf) ? n : sizeof(buf);
		output(context, buf, w);
		n -= w;
	}
}

static int format_int(void (*output)(void *context, const char *data, size_t sz),
		      void *context, unsigned long v, unsigned int base)
{
	char c[sizeof(v) * 8];
	int i = ARRAY_SIZE(c);

	if (v == 0 || base < 2 || base > 16) {
		c[--i] = '0';
	} else {
		while (v > 0) {
			c[--i] = "0123456789abcdef"[v % base];
			v /= base;
		}
	}
	if (output)
		output(context, c + i, ARRAY_SIZE(c) - i);
	return ARRAY_SIZE(c) - i;
}

static int format_rich_int(void (*output)(void *context, const char *data, size_t sz),
			   void *context, unsigned long v,
			   struct vformat_flags flags)
{
	int sw = 0;
	int aw = 0;
	int vw = 0;

	int w0 = 0;
	int w1 = 0;
	int w2 = 0;

	if (flags.signed_value &&
	    (flags.sign != SIGN_DEFAULT || (long)v < 0))
			sw = 1;
	if (flags.alternate) {
		if (flags.radix == 8 && v)
			aw = 1;
		else if (flags.radix == 16)
			aw = 2;
	}

	if (flags.precision && flags.precision_value == 0 && v == 0) {
		vw = 0;
	} else {
		if (flags.signed_value && (long)v < 0)
			vw = format_int(NULL, NULL, -v, flags.radix);
		else
			vw = format_int(NULL, NULL, v, flags.radix);
		if (flags.precision_value > vw)
			w1 = flags.precision_value - vw;
	}

	if (flags.width &&
	    sw + aw + w1 + vw < flags.width_value) {
		int dw = flags.width_value - (sw + aw + w1 + vw);

		if (flags.left)
			w2 = dw;
		else if (flags.precision || flags.padding_value == ' ')
			w0 = dw;
		else
			w1 = dw;
	}

	if (w0)
		output_copy(output, context, ' ', w0);

	if (flags.signed_value) {
		if ((long)v < 0) {
			output(context, "-", 1);
			v = -v;
		} else if (flags.sign != SIGN_DEFAULT) {
			output(context,
			       flags.sign == SIGN_PLUS ? "+" : " ", 1);
		}
	}

	if (flags.alternate) {
		if (flags.radix == 8 && v)
			output(context, "0", 1);
		else if (flags.radix == 16)
			output(context, "0x", 2);
	}

	if (w1)
		output_copy(output, context, '0', w1);

	format_int(output, context, v, flags.radix);

	if (w2)
		output_copy(output, context, ' ', w2);

	return w0 + sw + aw + w1 + vw + w2;
}

static void format_string(void (*output)(void *context, const char *data, size_t sz),
			  void *context, const char *p,
			  struct vformat_flags flags)
{
	int w0 = 0;
	int w1 = 0;

	if (flags.width &&
	    flags.width_value > flags.precision_value) {
		int dw = flags.width_value - flags.precision_value;

		if (flags.left)
			w1 = dw;
		else
			w0 = dw;
	}
	if (w0)
		output_copy(output, context, ' ', w0);
	output(context, p, flags.precision_value);
	if (w1)
		output_copy(output, context, ' ', w1);
}

static const char *fetch_int(const char *start, int *p)
{
	int v;

	for (v = 0; *start >= '0' && *start <= '9'; ++start) {
		v = v * 10 + (*start - '0');
	}
	*p = v;
	return start;
}

void vformat(void (*output)(void *context, const char *data, size_t sz),
	     void *context, const char *fmt, va_list ap)
{
	const char *p;
	const char *percent_start;
	enum {
		LITERAL,
		PERCENT,
		FLAGS,
		WIDTH,
		DOT,
		PRECISION,
		LENGTH,
		TYPE,
	} state = LITERAL;
	struct vformat_flags flags;
	unsigned long ui;
	long si;
	void *pvoid;
	const char *pchar;
	char c;

	while (*fmt) {
		if (state == LITERAL) {
			for (p = fmt; *p && *p != '%'; ++p)
				;
			output(context, fmt, p - fmt);
			state = PERCENT;
			fmt = p;
			percent_start = fmt;
			if (*fmt == '%')
				++fmt;
			else
				break;
			flags = (struct vformat_flags){
				.length = LENGTH_DEFAULT,
				.sign = SIGN_DEFAULT,
			};
		}
		if (state == PERCENT) {
			switch (*fmt++) {
			case '%':
				output(context, fmt - 1, 1);
				state = LITERAL;
				break;
			case '#':
				flags.alternate = 1;
				break;
			case '0':
				flags.padding_value = '0';
				flags.padding = 1;
				break;
			case ' ':
				if (flags.sign == SIGN_DEFAULT)
					flags.sign = SIGN_SPACE;
				break;
			case '-':
				flags.left = 1;
				break;
			case '+':
				flags.sign = SIGN_PLUS;
				break;
			default:
				--fmt;
				state = FLAGS;
				break;
			}
		}
		if (state == FLAGS) {
			if (*fmt == '*') {
				++fmt;
				flags.width = 1;
				flags.width_value = va_arg(ap, int);
			} else {
				p = fetch_int(fmt, &flags.width_value);
				if (p != fmt) {
					fmt = p;
					flags.width = 1;
				}
			}
			state = WIDTH;
		}
		if (state == WIDTH) {
			if (*fmt == '.') {
				++fmt;
				state = DOT;
			} else {
				state = PRECISION;
			}
		}
		if (state == DOT) {
			if (*fmt == '*') {
				++fmt;
				flags.precision = 1;
				flags.precision_value = va_arg(ap, int);
			} else {
				p = fetch_int(fmt, &flags.precision_value);
				if (p != fmt) {
					fmt = p;
					flags.precision = 1;
				}
			}
			state = PRECISION;
		}
		if (state == PRECISION) {
			switch (*fmt++) {
			case 'h':
				if (*fmt == 'h') {
					++fmt;
					flags.length = LENGTH_CHAR;
				} else {
					flags.length = LENGTH_SHORT;
				}
				break;
			case 'l':
				if (*fmt == 'l') {
					++fmt;
					flags.length = LENGTH_LONG_LONG;
				} else {
					flags.length = LENGTH_LONG;
				}
				break;
			case 'j':
				flags.length = LENGTH_INTMAX_T;
				break;
			case 'z':
				flags.length = LENGTH_SIZE_T;
				break;
			case 't':
				flags.length = LENGTH_PTRDIFF_T;
				break;
			default:
				--fmt;
				break;
			}
			state = LENGTH;
		}
		if (state == LENGTH) {
			if (flags.left && flags.padding)
				flags.padding = 0;
			if (!flags.padding)
				flags.padding_value = ' ';

			switch (*fmt) {
			case 'd':
			case 'i':
				switch (flags.length) {
				case LENGTH_CHAR:
				case LENGTH_SHORT:
				case LENGTH_DEFAULT:
					si = va_arg(ap, int);
					break;
				case LENGTH_LONG:
					si = va_arg(ap, long);
					break;
				case LENGTH_LONG_LONG:
					si = va_arg(ap, long long);
					break;
				case LENGTH_SIZE_T:
					si = va_arg(ap, size_t);
					break;
				}
				flags.signed_value = 1;
				flags.radix = 10;
				format_rich_int(output, context, si, flags);
				break;
			case 'u':
			case 'o':
			case 'x':
				switch (flags.length) {
				case LENGTH_CHAR:
				case LENGTH_SHORT:
					break;
				case LENGTH_DEFAULT:
					ui = va_arg(ap, unsigned int);
					break;
				case LENGTH_LONG:
					ui = va_arg(ap, unsigned long);
					break;
				case LENGTH_LONG_LONG:
					ui = va_arg(ap, unsigned long long);
					break;
				case LENGTH_SIZE_T:
					ui = va_arg(ap, size_t);
					break;
				}
				if (*fmt == 'u')
					flags.radix = 10;
				else if (*fmt == 'o')
					flags.radix = 8;
				else
					flags.radix = 16;
				format_rich_int(output, context, ui, flags);
				break;
			case 'p':
				pvoid = va_arg(ap, void *);
				flags.alternate = 1;
				flags.radix = 16;
				format_rich_int(output, context,
						(unsigned long)pvoid, flags);
				break;
			case 'c':
				c = va_arg(ap, int);
				flags.precision = 1;
				flags.precision_value = 1;
				format_string(output, context, &c, flags);
				break;
			case 's':
				pchar = va_arg(ap, void *);
				if (flags.precision) {
					p = memchr(pchar, 0, flags.precision_value);
					if (p)
						flags.precision_value = p - pchar;
				} else {
					flags.precision_value = strlen(pchar);
				}
				format_string(output, context, pchar, flags);
				break;
			case 0:
				--fmt;
			default:
				output(context, percent_start - 1, fmt - percent_start + 2);
				break;
			}
			++fmt;
			state = LITERAL;
		}
	}
}

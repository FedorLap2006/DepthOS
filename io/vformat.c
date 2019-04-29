#include <depthos/tools.h>
#include <depthos/vformat.h>

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

void vformat(void (*output)(void *context, const char *data, size_t sz),
	     void *context, const char *fmt, va_list ap)
{
	const char *p;
	const char *percent_start;
	enum {
		LITERAL,
		PERCENT,
		TYPE,
	} state = LITERAL;
	unsigned int ui;
	void *pvoid;
	const char *pchar;
	size_t i;

	while (*fmt) {
		switch (state) {
		case LITERAL:
			for (p = fmt; *p && *p != '%'; ++p)
				;
			output(context, fmt, p - fmt);
			fmt = p;
			state = PERCENT;
			break;

		case PERCENT:
			++fmt;
			if (*fmt == '%') {
				state = LITERAL;
				output(context, fmt, 1);
				++fmt;
			} else {
				percent_start = fmt;
				state = TYPE;
			}
			break;

		case TYPE:
			switch (*fmt) {
			case 'd':
			case 'i':
			case 'u':
				ui = va_arg(ap, unsigned int);
				format_int(output, context, ui, 10);
				break;

			case 'o':
				ui = va_arg(ap, unsigned int);
				format_int(output, context, ui, 8);
				break;

			case 'x':
				ui = va_arg(ap, unsigned int);
				format_int(output, context, ui, 16);
				break;

			case 'p':
				pvoid = va_arg(ap, void *);
				output(context, "0x", 2);
				format_int(output, context,
					   (unsigned long)pvoid, 16);
				break;

			case 's':
				pchar = va_arg(ap, void *);
				for (i = 0; pchar[i]; ++i)
					;
				output(context, pchar, i);
				break;

			default:
				output(context, percent_start - 1, fmt - percent_start + 2);
				break;
			}
			++fmt;
			state = LITERAL;
			break;
		}
	}
}

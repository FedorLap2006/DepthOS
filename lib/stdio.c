#include <depthos/stdio.h>
#include <depthos/string.h>
#include <depthos/vformat.h>

struct str_output_context {
	char *str;
	size_t limit;
};

static void output_str(void *p, const char *data, size_t sz)
{
	struct str_output_context *context = p;

	if (context->limit <= 1) {
	} else if (context->limit < sz) {
		memcpy(context->str, data, context->limit - 1);
		context->limit = 1;
	} else {
		memcpy(context->str, data, sz);
		context->limit -= sz;
	}
	context->str += sz;
}

int vsprintf(char *str, const char *fmt, va_list ap)
{
	struct str_output_context context = {
		.str = str,
		.limit = -1,
	};
	vformat(output_str, &context, fmt, ap);
	*context.str = 0;
	return context.str - str;
}

int sprintf(char *str, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = vsprintf(str, fmt, ap);
	va_end(ap);
	return rv;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	struct str_output_context context = {
		.str = str,
		.limit = size,
	};
	vformat(output_str, &context, fmt, ap);
	if (size)
		str[size - context.limit] = 0;
	return context.str - str;
}

int snprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = vsnprintf(str, size, fmt, ap);
	va_end(ap);
	return rv;
}

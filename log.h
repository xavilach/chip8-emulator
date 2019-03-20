#ifndef LOG_H_
#define LOG_H_

#ifndef NDEBUG
#define DEBUG_PRINT(fmt, ...)	printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)	(void)0
#endif /*NDEBUG*/

#define ERROR_PRINT(fmt)			printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__)
#define ERROR_PRINT_ARGS(fmt, ...)	printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif /*LOG_H_*/

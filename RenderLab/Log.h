#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#ifndef RELEASE
#ifndef NODEBUG
#define LOG_D(name, format) printf("\n[debug] %s = " #format " [from %s(...) at %s:%u]\n", #name, name, __FUNCTION__, __FILE__, __LINE__)
#define LOG_D_FOR(iterator, start, end, ...) do { \
		printf("\n[debug] each " #iterator " from " #start " to " #end ":\n" ); \
		for (auto iterator = start; iterator != end; iterator++) {\
			__VA_ARGS__	\
		} \
		printf("\n[from %s(...) at %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
	} while (0)
#define LOG_P(val, format) printf(#format ", ", val);
#define ASSERT_LOG_D(logger, value) do { \
		if (!(value)) { \
			logger << #value << " FAIL!"; \
			printf("\n[from %s(...) at %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
		} \
	} while (0)
#define ASSERT_LOG_D_RETURN(logger, returnv, value) do { \
		if (!(value)) { \
			logger << #value << " FAIL!"; \
			printf("\n[from %s(...) at %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
			return returnv; \
		} \
	} while (0)

#define ASSERT_LOG_D_RUN(logger, value, ...) do { \
		if (!(value)) { \
			logger << #value << " FAIL!"; \
			printf("\n[from %s(...) at %s:%u]\n", __FUNCTION__, __FILE__, __LINE__); \
			__VA_ARGS__ \
		} \
	} while (0)

#else
#define LOG_D(name, format) 
#define LOG_D_FOR(iterator, start, end, ...)
#define LOG_P(val, format)
#define ASSERT_LOG_D(logger, ...)
#define ASSERT_LOG_D_RETURN(logger, returnv, ...) 

#endif
#endif

#define ASSERT_LOG(logger, msg, ...) do { \
		if (!(__VA_ARGS__)) { \
			logger << msg; \
		} \
	} while (0)

#define ASSERT_LOG_RETURN(logger, msg, returnv, ...) do { \
		if (!(__VA_ARGS__)) { \
			logger << msg; \
			return returnv; \
		} \
	} while (0)


class Log {
public:
	enum class Type {
		Console,
		ConsoleError,
		File
	};

	Log();
	Log& use(Type type = Type::Console);
	std::ostream& operator ()(Type type = Type::Console);
	std::ostream& info(bool wrap = true);
	std::ostream& warn(bool wrap = true);
	std::ostream& err(bool wrap = true);
	void push(std::ostream&);

public:
	std::string infoTitle = "[info] ";
	std::string warnTitle = "\033[0m\033[1;33m[warn]\033[0m ";
	std::string errTitle = "\033[0m\033[1;31m[err]\033[0m ";
	std::string wrapstr = "\n";
	std::ostream* file;
	std::ofstream openedFile;

#ifndef RELEASE
	bool pushError = true;
	bool validationEnable = true;
#else
	bool pushError = false;
	bool validationEnable = false;
#endif
};


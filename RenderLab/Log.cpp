#include "Log.h"

std::ostream& Log::operator()(Type type) {
	switch (type) {
	case Type::Console:
		return std::cout;
		break;
	case Type::File:
		return openedFile;
		break;
	case Type::ConsoleError:
		return std::cerr;
		break;
	default:
		return std::cerr;
		break;
	}
}

std::ostream& Log::info(bool wrap) {
	return *file << (wrap ? wrapstr : "") << infoTitle;
}

std::ostream& Log::warn(bool wrap) {
	return *file << (wrap ? wrapstr : "") << warnTitle;
}

std::ostream& Log::err(bool wrap) {
	return *file << (wrap ? wrapstr : "") << errTitle;
}

void Log::push(std::ostream& re) {
	if (pushError) {
		std::ostringstream ss;
		ss << re.rdbuf();
		throw std::runtime_error(ss.str());
	}
}

Log& Log::use(Log::Type type) {
	switch (type) {
	case Type::Console:
		file = &std::cout;
		break;
	case Type::ConsoleError:
		file = &std::cerr;
		break;
	case Type::File:
		file = &openedFile;
		break;
	default:
		break;
	}
	return *this;
}

Log::Log() {
	use(Type::Console);
}
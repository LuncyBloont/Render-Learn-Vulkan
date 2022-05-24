#include "Core.h"

void Core::init()
{
	log.info() << "Initializing...";
	for (auto f = initFunctions.begin(); f != initFunctions.end(); f++)
	{
		log.info() << " - call " << f->name << "(...)";
		if (!f->function())
		{
			log.push(log.err() << "Failed at " << f->name);
		}
	}
	log.info() << "Done";
}

void Core::run()
{
	log.info() << "Running...";
	for (auto f = runFunctions.begin(); f != runFunctions.end(); f++)
	{
		log.info() << " - call " << f->name << "(...)";
		if (!f->function())
		{
			log.push(log.err() << "Failed at " << f->name);
		}
	}
	log.info() << "Stop";
}

void Core::exit()
{
	log.info() << "Cleanup...";
	for (auto f = exitFunctions.begin(); f != exitFunctions.end(); f++)
	{
		log.info() << " - call " << f->name << "(...)";
		if (!f->function())
		{
			log.push(log.err() << "Failed at " << f->name);
		}
	}
	log.info() << "Exit";
}

std::ostream& operator <<(std::ostream& os, Core::Steps step)
{
	switch (step)
	{
	case Core::Steps::Init:
		return os << "Init";
		break;
	case Core::Steps::Run:
		return os << "Run";
		break;
	case Core::Steps::Exit:
		return os << "Exit";
		break;
	default:
		return os << static_cast<int>(step);
		break;
	}
}

CoreFunc Core::addFunc(CoreFunc f, Core::Steps step) 
{
	switch (step)
	{
	case Steps::Init:
		initFunctions.push_back(f);
		break;
	case Steps::Run:
		runFunctions.push_back(f);
		break;
	case Steps::Exit:
		exitFunctions.push_back(f);
		break;
	default:
		log.push(log.err() << "Unknown step type " << step);
		return TO_COREFUNC(nullptr);
		break;
	}
	return f;
}

void Core::clearFunc(Core::Steps step)
{
	switch (step)
	{
	case Steps::Init:
		initFunctions.clear();
		break;
	case Steps::Run:
		runFunctions.clear();
		break;
	case Steps::Exit:
		exitFunctions.clear();
		break;
	default:
		log.push(log.err() << "Unknown step type " << step);
	}
}

std::vector<const char*> VKHelper::strs2pp(const std::vector<std::string>& strs) {
	std::vector<const char*> pp;
	for (size_t i = 0; i < strs.size(); i++) {
		pp.push_back(strs[i].data());
	}
	return pp;
}

uint32_t CG(uint64_t number) { 
	return static_cast<uint32_t>(number);
}

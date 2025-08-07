#include "doobius/core/root.h"
#include "doobius/common/code_timer.h"

int main(int argc, char* argv[]) {
	// TODO: Do I need to pass $(TargetPath) here instead?
	std::filesystem::path logDir = std::filesystem::absolute(argv[0]).parent_path() / "EngineDriverLogs";
	DOOBIUS_LOG_MNG().initLogging(logDir);
	Doobius::Perf::CodeTimer mainFunc("mainFunc");
	Doobius::Root::DoobiusRootConfig rootConfig{};
	DOOBIUS_ROOT().init(rootConfig);
	mainFunc.end();
}

#include <testsuite/dump_control.hpp>

#include <utils/assert.hpp>

namespace testsuite {

void DumpControl::WriteCacheDumps(
    const std::vector<std::string>& dumper_names) {
  for (const auto& name : dumper_names) {
    FindDumper(name).WriteDumpSyncDebug();
  }
}

void DumpControl::ReadCacheDumps(const std::vector<std::string>& dumper_names) {
  for (const auto& name : dumper_names) {
    FindDumper(name).ReadDumpDebug();
  }
}

void DumpControl::RegisterDumper(dump::Dumper& dumper) {
  auto dumpers = dumpers_.Lock();
  const auto [_, success] = dumpers->try_emplace(dumper.Name(), dumper);
  YTX_INVARIANT(success,
                fmt::format("Dumper already registered: {}", dumper.Name()));
}

void DumpControl::UnregisterDumper(dump::Dumper& dumper) {
  auto dumpers = dumpers_.Lock();
  const auto removed_count = dumpers->erase(dumper.Name());
  YTX_INVARIANT(removed_count != 0,
                fmt::format("Trying to remove a non-registered dumper: {}",
                            dumper.Name()));
}

dump::Dumper& DumpControl::FindDumper(const std::string& name) const {
  const auto dumpers = dumpers_.Lock();
  const auto iter = dumpers->find(name);
  YTX_INVARIANT(iter != dumpers->end(),
                fmt::format("The requested dumper does not exist: {}", name));
  return iter->second.get();
}

DumperRegistrationHolder::DumperRegistrationHolder(DumpControl& control,
                                                   dump::Dumper& dumper)
    : control_(control), dumper_(dumper) {
  control_.RegisterDumper(dumper_);
}

DumperRegistrationHolder::~DumperRegistrationHolder() {
  control_.UnregisterDumper(dumper_);
}

}  // namespace testsuite

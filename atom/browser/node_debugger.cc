// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/node_debugger.h"

#include <memory>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "libplatform/libplatform.h"
#include "native_mate/dictionary.h"

#include "atom/common/node_includes.h"

namespace atom {

NodeDebugger::NodeDebugger(node::Environment* env) : env_(env) {}

NodeDebugger::~NodeDebugger() {}

void NodeDebugger::Start() {
  auto* inspector = env_->inspector_agent();
  if (inspector == nullptr)
    return;

  std::vector<std::string> args;
  for (auto& arg : base::CommandLine::ForCurrentProcess()->argv()) {
#if defined(OS_WIN)
    args.push_back(base::UTF16ToUTF8(arg));
#else
    args.push_back(arg);
#endif
  }

  auto options = std::make_shared<node::DebugOptions>();
  std::vector<std::string> exec_args;
  std::vector<std::string> v8_args;
  std::string error;

  node::options_parser::DebugOptionsParser::instance.Parse(
      &args, &exec_args, &v8_args, options.get(),
      node::options_parser::kDisallowedInEnvironment, &error);

  if (!error.empty()) {
    // TODO(jeremy): what's the appropriate behaviour here?
    LOG(ERROR) << "Error parsing node options: " << error;
  }

  // Set process._debugWaitConnect if --inspect-brk was specified to stop
  // the debugger on the first line
  if (options->wait_for_connect()) {
    mate::Dictionary process(env_->isolate(), env_->process_object());
    process.Set("_breakFirstLine", true);
  }

  const char* path = "";
  inspector->Start(path, options);
  // FIXME
  // DCHECK(env_->inspector_agent()->IsListening());
}

}  // namespace atom

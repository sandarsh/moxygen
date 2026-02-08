/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <moxygen/MoQClient.h>
#include <moxygen/events/MoQFollyExecutorImpl.h>
#include <folly/coro/Task.h>
#include <chrono>
#include <string>

namespace moxygen {

struct InteropTestResult {
  bool passed{false};
  std::string testName;
  std::chrono::milliseconds duration{0};
  std::string message;
};

class MoQInteropClient {
 public:
  MoQInteropClient(
      folly::EventBase* evb,
      proxygen::URL url,
      bool useQuicTransport,
      bool tlsDisableVerify,
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds transactionTimeout);

  ~MoQInteropClient() = default;

  MoQInteropClient(const MoQInteropClient&) = delete;
  MoQInteropClient& operator=(const MoQInteropClient&) = delete;

  // T0.1: Connect, complete SETUP exchange, close gracefully
  folly::coro::Task<InteropTestResult> testSetupOnly();

 private:
  folly::EventBase* evb_;
  proxygen::URL url_;
  bool useQuicTransport_;
  bool tlsDisableVerify_;
  std::chrono::milliseconds connectTimeout_;
  std::chrono::milliseconds transactionTimeout_;
};

} // namespace moxygen

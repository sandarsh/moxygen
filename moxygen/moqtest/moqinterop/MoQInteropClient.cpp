/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "moxygen/moqtest/moqinterop/MoQInteropClient.h"

#include <moxygen/MoQWebTransportClient.h>
#include <moxygen/util/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Timeout.h>

namespace moxygen {

MoQInteropClient::MoQInteropClient(
    folly::EventBase* evb,
    proxygen::URL url,
    bool useQuicTransport,
    bool tlsDisableVerify,
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds transactionTimeout)
    : evb_(evb),
      url_(std::move(url)),
      useQuicTransport_(useQuicTransport),
      tlsDisableVerify_(tlsDisableVerify),
      connectTimeout_(connectTimeout),
      transactionTimeout_(transactionTimeout) {}

folly::coro::Task<InteropTestResult> MoQInteropClient::testSetupOnly() {
  InteropTestResult result;
  result.testName = "setup-only";
  auto start = std::chrono::steady_clock::now();

  try {
    auto moqExecutor = std::make_unique<MoQFollyExecutorImpl>(evb_);

    std::shared_ptr<fizz::CertificateVerifier> verifier;
    if (tlsDisableVerify_) {
      verifier = std::make_shared<
          test::InsecureVerifierDangerousDoNotUseInProduction>();
    }

    auto moqClient = useQuicTransport_
        ? std::make_unique<MoQClient>(
              moqExecutor->keepAlive(), url_, std::move(verifier))
        : std::make_unique<MoQWebTransportClient>(
              moqExecutor->keepAlive(), url_, std::move(verifier));

    quic::TransportSettings ts;
    ts.orderedReadCallbacks = true;

    co_await moqClient->setupMoQSession(
        connectTimeout_,
        transactionTimeout_,
        nullptr, // no publish handler
        nullptr, // no subscribe handler
        ts);

    // SETUP exchange completed successfully - close gracefully
    if (moqClient->moqSession_) {
      moqClient->moqSession_->close(SessionCloseErrorCode::NO_ERROR);
    }

    auto end = std::chrono::steady_clock::now();
    result.passed = true;
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);
    result.message = "SETUP exchange completed successfully";
  } catch (const std::exception& ex) {
    auto end = std::chrono::steady_clock::now();
    result.passed = false;
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);
    result.message = ex.what();
  }

  co_return result;
}

} // namespace moxygen

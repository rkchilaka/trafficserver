/** @file
 *
 *  A brief file description
 *
 *  @section license License
 *
 *  Licensed to the Apache Software Foundation (ASF) under one
 *  or more contributor license agreements.  See the NOTICE file
 *  distributed with this work for additional information
 *  regarding copyright ownership.  The ASF licenses this file
 *  to you under the Apache License, Version 2.0 (the
 *  "License"); you may not use this file except in compliance
 *  with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include "QUICTypes.h"
#include "QUICFrame.h"

class QUICFlowController
{
public:
  uint64_t credit();
  QUICOffset current_offset();
  QUICOffset current_limit();

  /*
   * Returns 0 if succeed
   */
  virtual int update(QUICOffset offset);
  virtual void forward_limit(QUICOffset limit);

  void set_threshold(uint64_t threshold);

  /**
   * This is only for flow controllers initialized without a limit (== UINT64_MAX).
   * Once a limit is set, it should be updated with forward_limit().
   */
  void set_limit(QUICOffset limit);

  QUICFrameUPtr generate_frame();

protected:
  QUICFlowController(uint64_t initial_limit) : _limit(initial_limit) {}
  virtual QUICFrameUPtr _create_frame() = 0;

  QUICOffset _offset    = 0;
  QUICOffset _limit     = 0;
  QUICOffset _threshold = 1024;
  QUICFrameUPtr _frame  = QUICFrameFactory::create_null_frame();
};

class QUICRemoteFlowController : public QUICFlowController
{
public:
  QUICRemoteFlowController(uint64_t initial_limit) : QUICFlowController(initial_limit) {}
  int update(QUICOffset offset) override;
  void forward_limit(QUICOffset limit) override;

private:
  bool _blocked = false;
};

class QUICLocalFlowController : public QUICFlowController
{
public:
  QUICLocalFlowController(uint64_t initial_limit) : QUICFlowController(initial_limit) {}
  void forward_limit(QUICOffset limit) override;
};

class QUICRemoteConnectionFlowController : public QUICRemoteFlowController
{
public:
  QUICRemoteConnectionFlowController(uint64_t initial_limit) : QUICRemoteFlowController(initial_limit) {}
  QUICFrameUPtr _create_frame() override;
};

class QUICLocalConnectionFlowController : public QUICLocalFlowController
{
public:
  QUICLocalConnectionFlowController(uint64_t initial_limit) : QUICLocalFlowController(initial_limit) {}
  QUICFrameUPtr _create_frame() override;
};

class QUICRemoteStreamFlowController : public QUICRemoteFlowController
{
public:
  QUICRemoteStreamFlowController(uint64_t initial_limit, QUICStreamId stream_id)
    : QUICRemoteFlowController(initial_limit), _stream_id(stream_id)
  {
  }
  QUICFrameUPtr _create_frame() override;

private:
  QUICStreamId _stream_id = 0;
};

class QUICLocalStreamFlowController : public QUICLocalFlowController
{
public:
  QUICLocalStreamFlowController(uint64_t initial_limit, QUICStreamId stream_id)
    : QUICLocalFlowController(initial_limit), _stream_id(stream_id)
  {
  }
  QUICFrameUPtr _create_frame() override;

private:
  QUICStreamId _stream_id = 0;
};

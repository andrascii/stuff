#include "thread.h"

#include "adopted_thread.h"
#include "objects_registry.h"
#include "set_thread_name_message.h"

//
// WARN: Проблемы
// 1. Создание объектов выполняется в одном отдельно взятом потоке
// 2. Разрушается объект также только в одном потоке (в каком?)
// 3. Уничтожающийся объект может получить сообщение от другого объекта, т.к. поток продолжает свою работу
//   и ObjectsRegistry не спасает от этого, потому, что он не дает уничтожить только самую базовую часть объекта типа Object.
// 4. Сейчас код устроен так, что сперва нужно остановить поток, потом уже разрушать объект, приаттаченный к этому потоку. См. бенчмарки.
//

#if defined(USE_WINDOWS_SET_THREAD_NAME_HACK)

namespace {

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;    // Must be 0x1000.
  LPCSTR szName;   // Pointer to name (in user addr space).
  DWORD dwThreadID;// Thread ID (-1=caller thread).
  DWORD dwFlags;   // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD thread_id, const std::string& thread_name) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = thread_name.data();
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*) &info);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
}

}// namespace

#endif

namespace mdo {

thread_local std::shared_ptr<ThreadData> current_thread_data = nullptr;

const std::shared_ptr<ThreadData>&
GetThreadData(const Thread* thread) noexcept {
  return thread->data_;
}

Thread* Thread::Current() {
  if (current_thread_data) {
    return current_thread_data->Thread();
  }

  current_thread_data = std::make_unique<ThreadData>();
  current_thread_data->SetId(std::this_thread::get_id());
  current_thread_data->SetIsAdopted(true);
  current_thread_data->SetThread(new AdoptedThread(current_thread_data));// how to delete it?

  return current_thread_data->Thread();
}

void Thread::YieldThread() { std::this_thread::yield(); }

void Thread::Sleep(const std::chrono::milliseconds& ms) {
  std::this_thread::sleep_for(ms);
}

void Thread::SetCurrentThreadName(const std::string& name) noexcept {
#ifdef USE_WINDOWS_SET_THREAD_NAME_HACK
  SetThreadName(static_cast<DWORD>(-1), name);
#elif defined(__APPLE__)
  pthread_setname_np(name.data());
#else
  prctl(PR_SET_NAME, name.data(), 0, 0, 0);
#endif
}

std::unique_ptr<Thread> Thread::Create(const char* name) {
  struct NewEnabler : Thread {
    explicit NewEnabler(std::function<void()> alternative_entry_point)
        : Thread(std::move(alternative_entry_point)) {}
  };

  auto thread = std::make_unique<NewEnabler>(std::function<void()>{});

  thread->SetName(name);

  Initialize(*thread);

  return thread;
}

Thread::~Thread() {
  StopImpl();

  const auto thread_name =
    name_.empty() ? ToString(std::this_thread::get_id()) : name_;

  LOG_INFO("thread '{}' destroyed", thread_name);

  data_->SetThread(nullptr);
}

const std::string& Thread::Name() const noexcept { return name_; }

void Thread::SetName(const std::string& name) {
  name_ = name;

  if (data_->IsAdopted()) {
    SetCurrentThreadName(name_);
  } else if (IsRunning()) {
    data_->Queue().Post(SetThreadNameMessage(name));
  }
}

void Thread::Start() {
  if (future_.valid()) {
    LOG_WARNING("attempt to start already started thread");
    return;
  }

  future_ = std::async(std::launch::async, [this] {
    SetCurrentThreadName(name_);
    current_thread_data = data_;
    current_thread_data->SetId(std::this_thread::get_id());

    LOG_TRACE("starting '{}' thread", CurrentThreadId());

    if (alternative_entry_point_) {
      alternative_entry_point_();
    } else {
      Run();
    }
  });
}

void Thread::Wait() const noexcept {
  if (!future_.valid()) {
    return;
  }

  future_.wait();
}

bool Thread::WaitFor(const std::chrono::milliseconds& ms) const noexcept {
  if (!future_.valid()) {
    return true;
  }

  const auto await_result = future_.wait_for(ms);

  if (await_result == std::future_status::deferred) {
    LOG_CRITICAL("the thread function must not be in a deferred state");
    std::terminate();
  }

  if (await_result == std::future_status::timeout) {
    return false;
  }

  return true;
}

void Thread::Stop() { StopImpl(); }

bool Thread::IsRunning() const noexcept {
  return future_.valid() &&
         future_.wait_for(0s) == std::future_status::timeout;
}

void Thread::RequestInterruption() const noexcept {
  data_->SetInterruptionRequest();
}

bool Thread::IsInterruptionRequested() const noexcept {
  return data_->InterruptionRequested();
}

void Thread::Run() {
  current_thread_data->SetInterruptionRequest(false);
  current_thread_data->Queue().SetInterruptFlag(false);

  const auto tid = Thread::CurrentThreadId();

  //
  // emit 'Started' signal
  //
  {
    std::scoped_lock _{ObjectsRegistry::Instance()};
    Started();
  }

  LOG_TRACE(
    "the '{}' thread started, current queue '{}' contains '{}' "
    "pending messages",
    tid,
    (void*) &current_thread_data->Queue(),
    current_thread_data->Queue().Size());

  while (!current_thread_data->InterruptionRequested()) {
    std::vector<Message> messages;
    const auto error = current_thread_data->Queue().Poll(messages, 1s);

    LOG_TRACE("the '{}' thread is reading from '{}' queue", tid, (void*) &current_thread_data->Queue());

    if (error == std::errc::interrupted) {
      LOG_TRACE("the '{}' thread is interrupted", tid);
      break;
    }

    if (error == std::errc::timed_out) {
      LOG_TRACE("the '{}' thread has no messages", tid);
      continue;
    }

    if (!messages.empty()) {
      LOG_TRACE("the '{}' thread got the '{}' messages to process", tid, messages.size());
    }

    HandleMessages(messages);
  }

  //
  // TODO: what if some object still lives and sends to us a messages?
  // we need here block incoming messages but should continue handle that
  // already received
  //
  /*current_thread_data->Queue().SetInterruptFlag(false);
    std::error_code ec;

    do {
      std::deque<Message> messages;
      ec = current_thread_data->Queue().Poll(messages, 1s);
      HandleMessages(messages);
    } while (ec != std::errc::timed_out);*/

  //
  // emit 'Finished' signal
  //
  {
    std::scoped_lock _{ObjectsRegistry::Instance()};
    Finished();
  }

  LOG_TRACE("the '{}' thread finished", tid);
}

void Thread::HandleMessage(Message&& message) {
  const auto this_thread = current_thread_data->Thread();
  const auto receiver = std::visit(GetReceiver, message);
  const auto receiver_thread = receiver->Thread();

  if (this_thread == receiver_thread) {
    LOG_TRACE("the thread '{}' received and handling a message",
              this_thread->Name());

    //
    // block ability to destroy an Object class objects
    // to be sure that we can safely call
    // message->Receiver()->OnMessage(message);
    //
    // It ensures that receiver object is alive or already dead (doesn't exists) but not in destroy state!
    // Each object in dtor removes itself from ObjectsRegistry.
    //
    // WARN: this approach is invalid!
    // Because the Object type usually just a first subobject of the real object!
    // So when actual object start calling his dtor, that part of the overall object already dead.
    // Need to reconsider the approach to sending messages to dying objects!
    //
    std::scoped_lock _{ObjectsRegistry::Instance()};

    if (!ObjectsRegistry::Instance().HasObject(receiver)) {
      LOG_WARNING(
        "the object {} that lived in thread {} is dead so the "
        "message to it is skipped",
        static_cast<void*>(receiver),
        ToString(current_thread_data->Id()));

      return;
    }

    receiver->OnMessage(message);
  } else {
    LOG_TRACE(
      "the thread '{}' received a message for the '{}' thread, "
      "dispatching it further",
      this_thread->Name(),
      receiver_thread->Name());

    GetThreadData(receiver_thread)->Queue().Post(std::move(message));
  }
}

void Thread::HandleMessages(std::vector<Message>& messages) {
  for (auto& message : messages) {
    if (std::holds_alternative<SetThreadNameMessage>(message)) {
      const auto set_thread_name_message =
        std::get<SetThreadNameMessage>(message);
      SetCurrentThreadName(set_thread_name_message.Name());
      continue;
    }

    HandleMessage(std::move(message));
  }
}

std::string Thread::CurrentThreadId() {
  //
  // 1. const auto id = ToString(current_thread_data->Id()); - invalid line,
  // because we can start thread in any other thread
  // 2. each evaluation of current_thread_data should compare its value with
  // nullptr
  //
  const auto id = ToString(current_thread_data->Id());
  const auto& thread_name = current_thread_data->Thread()->Name();

  if (thread_name.empty()) {
    return ToString(id);
  }

  return thread_name;
}

Thread::Thread(std::shared_ptr<ThreadData> data)
    : Thread{{}, std::move(data)} {}

void Thread::StopImpl() {
  const auto is_adopted = data_->IsAdopted();

  if (!is_adopted && !future_.valid()) {
    return;
  }

  auto tid = ToString(data_->Id());

  if (!name_.empty()) {
    tid = name_ + "/" + tid;
  }

  if (IsInterruptionRequested()) {
    LOG_INFO("the '{}' thread is already in stopping process", tid);
    return;
  }

  RequestInterruption();
  data_->Queue().SetInterruptFlag(true);

  if (!is_adopted) {
    while (future_.wait_for(1s) == std::future_status::timeout) {
      LOG_TRACE("waiting for '{}' thread to stop", tid);
    }

    future_.get();
  } else {
    Wait();
  }

  LOG_TRACE("the '{}' thread has stopped", tid);
}

Thread::Thread(std::function<void()> alternative_entry_point,
               std::shared_ptr<ThreadData> data)
    : Finished{this},
      Started{this},
      data_{std::move(data)},
      alternative_entry_point_{std::move(alternative_entry_point)} {
  if (!data_) {
    data_ = std::make_shared<ThreadData>();
  }
}

void Thread::Initialize(Thread& thread) {
  thread.SetThread(&thread);// points to itself (Thread is also Object that points to Thread in which it alives)
  thread.data_->SetThread(&thread);
}

}// namespace mdo

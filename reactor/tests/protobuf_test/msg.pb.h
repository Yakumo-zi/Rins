// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_msg_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_msg_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021011 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_msg_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_msg_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_msg_2eproto;
namespace protobuf_test {
class hellworld;
struct hellworldDefaultTypeInternal;
extern hellworldDefaultTypeInternal _hellworld_default_instance_;
}  // namespace protobuf_test
PROTOBUF_NAMESPACE_OPEN
template<> ::protobuf_test::hellworld* Arena::CreateMaybeMessage<::protobuf_test::hellworld>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace protobuf_test {

// ===================================================================

class hellworld final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:protobuf_test.hellworld) */ {
 public:
  inline hellworld() : hellworld(nullptr) {}
  ~hellworld() override;
  explicit PROTOBUF_CONSTEXPR hellworld(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  hellworld(const hellworld& from);
  hellworld(hellworld&& from) noexcept
    : hellworld() {
    *this = ::std::move(from);
  }

  inline hellworld& operator=(const hellworld& from) {
    CopyFrom(from);
    return *this;
  }
  inline hellworld& operator=(hellworld&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const hellworld& default_instance() {
    return *internal_default_instance();
  }
  static inline const hellworld* internal_default_instance() {
    return reinterpret_cast<const hellworld*>(
               &_hellworld_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(hellworld& a, hellworld& b) {
    a.Swap(&b);
  }
  inline void Swap(hellworld* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(hellworld* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  hellworld* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<hellworld>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const hellworld& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const hellworld& from) {
    hellworld::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(hellworld* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "protobuf_test.hellworld";
  }
  protected:
  explicit hellworld(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kStrFieldNumber = 2,
    kIdFieldNumber = 1,
    kOptFieldNumber = 3,
  };
  // string str = 2;
  void clear_str();
  const std::string& str() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_str(ArgT0&& arg0, ArgT... args);
  std::string* mutable_str();
  PROTOBUF_NODISCARD std::string* release_str();
  void set_allocated_str(std::string* str);
  private:
  const std::string& _internal_str() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_str(const std::string& value);
  std::string* _internal_mutable_str();
  public:

  // int32 id = 1;
  void clear_id();
  int32_t id() const;
  void set_id(int32_t value);
  private:
  int32_t _internal_id() const;
  void _internal_set_id(int32_t value);
  public:

  // int32 opt = 3;
  void clear_opt();
  int32_t opt() const;
  void set_opt(int32_t value);
  private:
  int32_t _internal_opt() const;
  void _internal_set_opt(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:protobuf_test.hellworld)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr str_;
    int32_t id_;
    int32_t opt_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_msg_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// hellworld

// int32 id = 1;
inline void hellworld::clear_id() {
  _impl_.id_ = 0;
}
inline int32_t hellworld::_internal_id() const {
  return _impl_.id_;
}
inline int32_t hellworld::id() const {
  // @@protoc_insertion_point(field_get:protobuf_test.hellworld.id)
  return _internal_id();
}
inline void hellworld::_internal_set_id(int32_t value) {
  
  _impl_.id_ = value;
}
inline void hellworld::set_id(int32_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:protobuf_test.hellworld.id)
}

// string str = 2;
inline void hellworld::clear_str() {
  _impl_.str_.ClearToEmpty();
}
inline const std::string& hellworld::str() const {
  // @@protoc_insertion_point(field_get:protobuf_test.hellworld.str)
  return _internal_str();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void hellworld::set_str(ArgT0&& arg0, ArgT... args) {
 
 _impl_.str_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:protobuf_test.hellworld.str)
}
inline std::string* hellworld::mutable_str() {
  std::string* _s = _internal_mutable_str();
  // @@protoc_insertion_point(field_mutable:protobuf_test.hellworld.str)
  return _s;
}
inline const std::string& hellworld::_internal_str() const {
  return _impl_.str_.Get();
}
inline void hellworld::_internal_set_str(const std::string& value) {
  
  _impl_.str_.Set(value, GetArenaForAllocation());
}
inline std::string* hellworld::_internal_mutable_str() {
  
  return _impl_.str_.Mutable(GetArenaForAllocation());
}
inline std::string* hellworld::release_str() {
  // @@protoc_insertion_point(field_release:protobuf_test.hellworld.str)
  return _impl_.str_.Release();
}
inline void hellworld::set_allocated_str(std::string* str) {
  if (str != nullptr) {
    
  } else {
    
  }
  _impl_.str_.SetAllocated(str, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.str_.IsDefault()) {
    _impl_.str_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:protobuf_test.hellworld.str)
}

// int32 opt = 3;
inline void hellworld::clear_opt() {
  _impl_.opt_ = 0;
}
inline int32_t hellworld::_internal_opt() const {
  return _impl_.opt_;
}
inline int32_t hellworld::opt() const {
  // @@protoc_insertion_point(field_get:protobuf_test.hellworld.opt)
  return _internal_opt();
}
inline void hellworld::_internal_set_opt(int32_t value) {
  
  _impl_.opt_ = value;
}
inline void hellworld::set_opt(int32_t value) {
  _internal_set_opt(value);
  // @@protoc_insertion_point(field_set:protobuf_test.hellworld.opt)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf_test

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_msg_2eproto

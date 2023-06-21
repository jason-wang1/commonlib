// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: other/local_file.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_other_2flocal_5ffile_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_other_2flocal_5ffile_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3014000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3014000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_other_2flocal_5ffile_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_other_2flocal_5ffile_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_other_2flocal_5ffile_2eproto;
namespace RSP_LocalFileData {
class FileData;
class FileDataDefaultTypeInternal;
extern FileDataDefaultTypeInternal _FileData_default_instance_;
}  // namespace RSP_LocalFileData
PROTOBUF_NAMESPACE_OPEN
template<> ::RSP_LocalFileData::FileData* Arena::CreateMaybeMessage<::RSP_LocalFileData::FileData>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace RSP_LocalFileData {

// ===================================================================

class FileData PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:RSP_LocalFileData.FileData) */ {
 public:
  inline FileData() : FileData(nullptr) {}
  virtual ~FileData();

  FileData(const FileData& from);
  FileData(FileData&& from) noexcept
    : FileData() {
    *this = ::std::move(from);
  }

  inline FileData& operator=(const FileData& from) {
    CopyFrom(from);
    return *this;
  }
  inline FileData& operator=(FileData&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const FileData& default_instance();

  static inline const FileData* internal_default_instance() {
    return reinterpret_cast<const FileData*>(
               &_FileData_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(FileData& a, FileData& b) {
    a.Swap(&b);
  }
  inline void Swap(FileData* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(FileData* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline FileData* New() const final {
    return CreateMaybeMessage<FileData>(nullptr);
  }

  FileData* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<FileData>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const FileData& from);
  void MergeFrom(const FileData& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(FileData* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "RSP_LocalFileData.FileData";
  }
  protected:
  explicit FileData(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_other_2flocal_5ffile_2eproto);
    return ::descriptor_table_other_2flocal_5ffile_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kProtosFieldNumber = 1,
  };
  // repeated bytes protos = 1;
  int protos_size() const;
  private:
  int _internal_protos_size() const;
  public:
  void clear_protos();
  const std::string& protos(int index) const;
  std::string* mutable_protos(int index);
  void set_protos(int index, const std::string& value);
  void set_protos(int index, std::string&& value);
  void set_protos(int index, const char* value);
  void set_protos(int index, const void* value, size_t size);
  std::string* add_protos();
  void add_protos(const std::string& value);
  void add_protos(std::string&& value);
  void add_protos(const char* value);
  void add_protos(const void* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& protos() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_protos();
  private:
  const std::string& _internal_protos(int index) const;
  std::string* _internal_add_protos();
  public:

  // @@protoc_insertion_point(class_scope:RSP_LocalFileData.FileData)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> protos_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_other_2flocal_5ffile_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// FileData

// repeated bytes protos = 1;
inline int FileData::_internal_protos_size() const {
  return protos_.size();
}
inline int FileData::protos_size() const {
  return _internal_protos_size();
}
inline void FileData::clear_protos() {
  protos_.Clear();
}
inline std::string* FileData::add_protos() {
  // @@protoc_insertion_point(field_add_mutable:RSP_LocalFileData.FileData.protos)
  return _internal_add_protos();
}
inline const std::string& FileData::_internal_protos(int index) const {
  return protos_.Get(index);
}
inline const std::string& FileData::protos(int index) const {
  // @@protoc_insertion_point(field_get:RSP_LocalFileData.FileData.protos)
  return _internal_protos(index);
}
inline std::string* FileData::mutable_protos(int index) {
  // @@protoc_insertion_point(field_mutable:RSP_LocalFileData.FileData.protos)
  return protos_.Mutable(index);
}
inline void FileData::set_protos(int index, const std::string& value) {
  // @@protoc_insertion_point(field_set:RSP_LocalFileData.FileData.protos)
  protos_.Mutable(index)->assign(value);
}
inline void FileData::set_protos(int index, std::string&& value) {
  // @@protoc_insertion_point(field_set:RSP_LocalFileData.FileData.protos)
  protos_.Mutable(index)->assign(std::move(value));
}
inline void FileData::set_protos(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  protos_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:RSP_LocalFileData.FileData.protos)
}
inline void FileData::set_protos(int index, const void* value, size_t size) {
  protos_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:RSP_LocalFileData.FileData.protos)
}
inline std::string* FileData::_internal_add_protos() {
  return protos_.Add();
}
inline void FileData::add_protos(const std::string& value) {
  protos_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:RSP_LocalFileData.FileData.protos)
}
inline void FileData::add_protos(std::string&& value) {
  protos_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:RSP_LocalFileData.FileData.protos)
}
inline void FileData::add_protos(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  protos_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:RSP_LocalFileData.FileData.protos)
}
inline void FileData::add_protos(const void* value, size_t size) {
  protos_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:RSP_LocalFileData.FileData.protos)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
FileData::protos() const {
  // @@protoc_insertion_point(field_list:RSP_LocalFileData.FileData.protos)
  return protos_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
FileData::mutable_protos() {
  // @@protoc_insertion_point(field_mutable_list:RSP_LocalFileData.FileData.protos)
  return &protos_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace RSP_LocalFileData

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_other_2flocal_5ffile_2eproto
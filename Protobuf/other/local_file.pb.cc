// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: other/local_file.proto

#include "other/local_file.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
namespace RSP_LocalFileData {
class FileDataDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<FileData> _instance;
} _FileData_default_instance_;
}  // namespace RSP_LocalFileData
static void InitDefaultsscc_info_FileData_other_2flocal_5ffile_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::RSP_LocalFileData::_FileData_default_instance_;
    new (ptr) ::RSP_LocalFileData::FileData();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_FileData_other_2flocal_5ffile_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0, InitDefaultsscc_info_FileData_other_2flocal_5ffile_2eproto}, {}};

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_other_2flocal_5ffile_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_other_2flocal_5ffile_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_other_2flocal_5ffile_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_other_2flocal_5ffile_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::RSP_LocalFileData::FileData, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::RSP_LocalFileData::FileData, protos_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::RSP_LocalFileData::FileData)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::RSP_LocalFileData::_FileData_default_instance_),
};

const char descriptor_table_protodef_other_2flocal_5ffile_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\026other/local_file.proto\022\021RSP_LocalFileD"
  "ata\"\032\n\010FileData\022\016\n\006protos\030\001 \003(\014b\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_other_2flocal_5ffile_2eproto_deps[1] = {
};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase*const descriptor_table_other_2flocal_5ffile_2eproto_sccs[1] = {
  &scc_info_FileData_other_2flocal_5ffile_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_other_2flocal_5ffile_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_other_2flocal_5ffile_2eproto = {
  false, false, descriptor_table_protodef_other_2flocal_5ffile_2eproto, "other/local_file.proto", 79,
  &descriptor_table_other_2flocal_5ffile_2eproto_once, descriptor_table_other_2flocal_5ffile_2eproto_sccs, descriptor_table_other_2flocal_5ffile_2eproto_deps, 1, 0,
  schemas, file_default_instances, TableStruct_other_2flocal_5ffile_2eproto::offsets,
  file_level_metadata_other_2flocal_5ffile_2eproto, 1, file_level_enum_descriptors_other_2flocal_5ffile_2eproto, file_level_service_descriptors_other_2flocal_5ffile_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_other_2flocal_5ffile_2eproto = (static_cast<void>(::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_other_2flocal_5ffile_2eproto)), true);
namespace RSP_LocalFileData {

// ===================================================================

class FileData::_Internal {
 public:
};

FileData::FileData(::PROTOBUF_NAMESPACE_ID::Arena* arena)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena),
  protos_(arena) {
  SharedCtor();
  RegisterArenaDtor(arena);
  // @@protoc_insertion_point(arena_constructor:RSP_LocalFileData.FileData)
}
FileData::FileData(const FileData& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      protos_(from.protos_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:RSP_LocalFileData.FileData)
}

void FileData::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_FileData_other_2flocal_5ffile_2eproto.base);
}

FileData::~FileData() {
  // @@protoc_insertion_point(destructor:RSP_LocalFileData.FileData)
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void FileData::SharedDtor() {
  GOOGLE_DCHECK(GetArena() == nullptr);
}

void FileData::ArenaDtor(void* object) {
  FileData* _this = reinterpret_cast< FileData* >(object);
  (void)_this;
}
void FileData::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void FileData::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const FileData& FileData::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_FileData_other_2flocal_5ffile_2eproto.base);
  return *internal_default_instance();
}


void FileData::Clear() {
// @@protoc_insertion_point(message_clear_start:RSP_LocalFileData.FileData)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  protos_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* FileData::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // repeated bytes protos = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          ptr -= 1;
          do {
            ptr += 1;
            auto str = _internal_add_protos();
            ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<10>(ptr));
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* FileData::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:RSP_LocalFileData.FileData)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated bytes protos = 1;
  for (int i = 0, n = this->_internal_protos_size(); i < n; i++) {
    const auto& s = this->_internal_protos(i);
    target = stream->WriteBytes(1, s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:RSP_LocalFileData.FileData)
  return target;
}

size_t FileData::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:RSP_LocalFileData.FileData)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated bytes protos = 1;
  total_size += 1 *
      ::PROTOBUF_NAMESPACE_ID::internal::FromIntSize(protos_.size());
  for (int i = 0, n = protos_.size(); i < n; i++) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::BytesSize(
      protos_.Get(i));
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void FileData::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:RSP_LocalFileData.FileData)
  GOOGLE_DCHECK_NE(&from, this);
  const FileData* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<FileData>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:RSP_LocalFileData.FileData)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:RSP_LocalFileData.FileData)
    MergeFrom(*source);
  }
}

void FileData::MergeFrom(const FileData& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:RSP_LocalFileData.FileData)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  protos_.MergeFrom(from.protos_);
}

void FileData::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:RSP_LocalFileData.FileData)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void FileData::CopyFrom(const FileData& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:RSP_LocalFileData.FileData)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool FileData::IsInitialized() const {
  return true;
}

void FileData::InternalSwap(FileData* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
  protos_.InternalSwap(&other->protos_);
}

::PROTOBUF_NAMESPACE_ID::Metadata FileData::GetMetadata() const {
  return GetMetadataStatic();
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace RSP_LocalFileData
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::RSP_LocalFileData::FileData* Arena::CreateMaybeMessage< ::RSP_LocalFileData::FileData >(Arena* arena) {
  return Arena::CreateMessageInternal< ::RSP_LocalFileData::FileData >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
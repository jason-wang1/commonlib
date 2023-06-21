// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: other/annoy_file_data.proto

#include "other/annoy_file_data.pb.h"

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
namespace RSP_AnnoyFileData {
class AnnoyItemVectorDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<AnnoyItemVector> _instance;
} _AnnoyItemVector_default_instance_;
class AnnoyKeynameVectorDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<AnnoyKeynameVector> _instance;
} _AnnoyKeynameVector_default_instance_;
}  // namespace RSP_AnnoyFileData
static void InitDefaultsscc_info_AnnoyItemVector_other_2fannoy_5ffile_5fdata_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::RSP_AnnoyFileData::_AnnoyItemVector_default_instance_;
    new (ptr) ::RSP_AnnoyFileData::AnnoyItemVector();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_AnnoyItemVector_other_2fannoy_5ffile_5fdata_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0, InitDefaultsscc_info_AnnoyItemVector_other_2fannoy_5ffile_5fdata_2eproto}, {}};

static void InitDefaultsscc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::RSP_AnnoyFileData::_AnnoyKeynameVector_default_instance_;
    new (ptr) ::RSP_AnnoyFileData::AnnoyKeynameVector();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0, InitDefaultsscc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto}, {}};

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_other_2fannoy_5ffile_5fdata_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_other_2fannoy_5ffile_5fdata_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_other_2fannoy_5ffile_5fdata_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_other_2fannoy_5ffile_5fdata_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyItemVector, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyItemVector, item_id_),
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyItemVector, res_type_),
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyItemVector, item_pos_),
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyItemVector, vector_data_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyKeynameVector, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyKeynameVector, keyname_),
  PROTOBUF_FIELD_OFFSET(::RSP_AnnoyFileData::AnnoyKeynameVector, vector_data_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::RSP_AnnoyFileData::AnnoyItemVector)},
  { 9, -1, sizeof(::RSP_AnnoyFileData::AnnoyKeynameVector)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::RSP_AnnoyFileData::_AnnoyItemVector_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::RSP_AnnoyFileData::_AnnoyKeynameVector_default_instance_),
};

const char descriptor_table_protodef_other_2fannoy_5ffile_5fdata_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\033other/annoy_file_data.proto\022\021RSP_Annoy"
  "FileData\"[\n\017AnnoyItemVector\022\017\n\007item_id\030\001"
  " \001(\003\022\020\n\010res_type\030\002 \001(\005\022\020\n\010item_pos\030\003 \001(\005"
  "\022\023\n\013vector_data\030\004 \003(\002\":\n\022AnnoyKeynameVec"
  "tor\022\017\n\007keyname\030\001 \001(\t\022\023\n\013vector_data\030\002 \003("
  "\002b\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_deps[1] = {
};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase*const descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_sccs[2] = {
  &scc_info_AnnoyItemVector_other_2fannoy_5ffile_5fdata_2eproto.base,
  &scc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_other_2fannoy_5ffile_5fdata_2eproto = {
  false, false, descriptor_table_protodef_other_2fannoy_5ffile_5fdata_2eproto, "other/annoy_file_data.proto", 209,
  &descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_once, descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_sccs, descriptor_table_other_2fannoy_5ffile_5fdata_2eproto_deps, 2, 0,
  schemas, file_default_instances, TableStruct_other_2fannoy_5ffile_5fdata_2eproto::offsets,
  file_level_metadata_other_2fannoy_5ffile_5fdata_2eproto, 2, file_level_enum_descriptors_other_2fannoy_5ffile_5fdata_2eproto, file_level_service_descriptors_other_2fannoy_5ffile_5fdata_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_other_2fannoy_5ffile_5fdata_2eproto = (static_cast<void>(::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_other_2fannoy_5ffile_5fdata_2eproto)), true);
namespace RSP_AnnoyFileData {

// ===================================================================

class AnnoyItemVector::_Internal {
 public:
};

AnnoyItemVector::AnnoyItemVector(::PROTOBUF_NAMESPACE_ID::Arena* arena)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena),
  vector_data_(arena) {
  SharedCtor();
  RegisterArenaDtor(arena);
  // @@protoc_insertion_point(arena_constructor:RSP_AnnoyFileData.AnnoyItemVector)
}
AnnoyItemVector::AnnoyItemVector(const AnnoyItemVector& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      vector_data_(from.vector_data_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::memcpy(&item_id_, &from.item_id_,
    static_cast<size_t>(reinterpret_cast<char*>(&item_pos_) -
    reinterpret_cast<char*>(&item_id_)) + sizeof(item_pos_));
  // @@protoc_insertion_point(copy_constructor:RSP_AnnoyFileData.AnnoyItemVector)
}

void AnnoyItemVector::SharedCtor() {
  ::memset(reinterpret_cast<char*>(this) + static_cast<size_t>(
      reinterpret_cast<char*>(&item_id_) - reinterpret_cast<char*>(this)),
      0, static_cast<size_t>(reinterpret_cast<char*>(&item_pos_) -
      reinterpret_cast<char*>(&item_id_)) + sizeof(item_pos_));
}

AnnoyItemVector::~AnnoyItemVector() {
  // @@protoc_insertion_point(destructor:RSP_AnnoyFileData.AnnoyItemVector)
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void AnnoyItemVector::SharedDtor() {
  GOOGLE_DCHECK(GetArena() == nullptr);
}

void AnnoyItemVector::ArenaDtor(void* object) {
  AnnoyItemVector* _this = reinterpret_cast< AnnoyItemVector* >(object);
  (void)_this;
}
void AnnoyItemVector::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void AnnoyItemVector::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const AnnoyItemVector& AnnoyItemVector::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_AnnoyItemVector_other_2fannoy_5ffile_5fdata_2eproto.base);
  return *internal_default_instance();
}


void AnnoyItemVector::Clear() {
// @@protoc_insertion_point(message_clear_start:RSP_AnnoyFileData.AnnoyItemVector)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  vector_data_.Clear();
  ::memset(&item_id_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&item_pos_) -
      reinterpret_cast<char*>(&item_id_)) + sizeof(item_pos_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* AnnoyItemVector::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // int64 item_id = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          item_id_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // int32 res_type = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 16)) {
          res_type_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // int32 item_pos = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 24)) {
          item_pos_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // repeated float vector_data = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 34)) {
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::PackedFloatParser(_internal_mutable_vector_data(), ptr, ctx);
          CHK_(ptr);
        } else if (static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 37) {
          _internal_add_vector_data(::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<float>(ptr));
          ptr += sizeof(float);
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

::PROTOBUF_NAMESPACE_ID::uint8* AnnoyItemVector::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:RSP_AnnoyFileData.AnnoyItemVector)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // int64 item_id = 1;
  if (this->item_id() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt64ToArray(1, this->_internal_item_id(), target);
  }

  // int32 res_type = 2;
  if (this->res_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt32ToArray(2, this->_internal_res_type(), target);
  }

  // int32 item_pos = 3;
  if (this->item_pos() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt32ToArray(3, this->_internal_item_pos(), target);
  }

  // repeated float vector_data = 4;
  if (this->_internal_vector_data_size() > 0) {
    target = stream->WriteFixedPacked(4, _internal_vector_data(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:RSP_AnnoyFileData.AnnoyItemVector)
  return target;
}

size_t AnnoyItemVector::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:RSP_AnnoyFileData.AnnoyItemVector)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated float vector_data = 4;
  {
    unsigned int count = static_cast<unsigned int>(this->_internal_vector_data_size());
    size_t data_size = 4UL * count;
    if (data_size > 0) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32Size(
            static_cast<::PROTOBUF_NAMESPACE_ID::int32>(data_size));
    }
    int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(data_size);
    _vector_data_cached_byte_size_.store(cached_size,
                                    std::memory_order_relaxed);
    total_size += data_size;
  }

  // int64 item_id = 1;
  if (this->item_id() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int64Size(
        this->_internal_item_id());
  }

  // int32 res_type = 2;
  if (this->res_type() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32Size(
        this->_internal_res_type());
  }

  // int32 item_pos = 3;
  if (this->item_pos() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32Size(
        this->_internal_item_pos());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void AnnoyItemVector::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:RSP_AnnoyFileData.AnnoyItemVector)
  GOOGLE_DCHECK_NE(&from, this);
  const AnnoyItemVector* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<AnnoyItemVector>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:RSP_AnnoyFileData.AnnoyItemVector)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:RSP_AnnoyFileData.AnnoyItemVector)
    MergeFrom(*source);
  }
}

void AnnoyItemVector::MergeFrom(const AnnoyItemVector& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:RSP_AnnoyFileData.AnnoyItemVector)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  vector_data_.MergeFrom(from.vector_data_);
  if (from.item_id() != 0) {
    _internal_set_item_id(from._internal_item_id());
  }
  if (from.res_type() != 0) {
    _internal_set_res_type(from._internal_res_type());
  }
  if (from.item_pos() != 0) {
    _internal_set_item_pos(from._internal_item_pos());
  }
}

void AnnoyItemVector::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:RSP_AnnoyFileData.AnnoyItemVector)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void AnnoyItemVector::CopyFrom(const AnnoyItemVector& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:RSP_AnnoyFileData.AnnoyItemVector)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool AnnoyItemVector::IsInitialized() const {
  return true;
}

void AnnoyItemVector::InternalSwap(AnnoyItemVector* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
  vector_data_.InternalSwap(&other->vector_data_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(AnnoyItemVector, item_pos_)
      + sizeof(AnnoyItemVector::item_pos_)
      - PROTOBUF_FIELD_OFFSET(AnnoyItemVector, item_id_)>(
          reinterpret_cast<char*>(&item_id_),
          reinterpret_cast<char*>(&other->item_id_));
}

::PROTOBUF_NAMESPACE_ID::Metadata AnnoyItemVector::GetMetadata() const {
  return GetMetadataStatic();
}


// ===================================================================

class AnnoyKeynameVector::_Internal {
 public:
};

AnnoyKeynameVector::AnnoyKeynameVector(::PROTOBUF_NAMESPACE_ID::Arena* arena)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena),
  vector_data_(arena) {
  SharedCtor();
  RegisterArenaDtor(arena);
  // @@protoc_insertion_point(arena_constructor:RSP_AnnoyFileData.AnnoyKeynameVector)
}
AnnoyKeynameVector::AnnoyKeynameVector(const AnnoyKeynameVector& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      vector_data_(from.vector_data_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  keyname_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_keyname().empty()) {
    keyname_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_keyname(), 
      GetArena());
  }
  // @@protoc_insertion_point(copy_constructor:RSP_AnnoyFileData.AnnoyKeynameVector)
}

void AnnoyKeynameVector::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto.base);
  keyname_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

AnnoyKeynameVector::~AnnoyKeynameVector() {
  // @@protoc_insertion_point(destructor:RSP_AnnoyFileData.AnnoyKeynameVector)
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void AnnoyKeynameVector::SharedDtor() {
  GOOGLE_DCHECK(GetArena() == nullptr);
  keyname_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void AnnoyKeynameVector::ArenaDtor(void* object) {
  AnnoyKeynameVector* _this = reinterpret_cast< AnnoyKeynameVector* >(object);
  (void)_this;
}
void AnnoyKeynameVector::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void AnnoyKeynameVector::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const AnnoyKeynameVector& AnnoyKeynameVector::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_AnnoyKeynameVector_other_2fannoy_5ffile_5fdata_2eproto.base);
  return *internal_default_instance();
}


void AnnoyKeynameVector::Clear() {
// @@protoc_insertion_point(message_clear_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  vector_data_.Clear();
  keyname_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* AnnoyKeynameVector::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // string keyname = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_keyname();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "RSP_AnnoyFileData.AnnoyKeynameVector.keyname"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // repeated float vector_data = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::PackedFloatParser(_internal_mutable_vector_data(), ptr, ctx);
          CHK_(ptr);
        } else if (static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 21) {
          _internal_add_vector_data(::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<float>(ptr));
          ptr += sizeof(float);
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

::PROTOBUF_NAMESPACE_ID::uint8* AnnoyKeynameVector::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string keyname = 1;
  if (this->keyname().size() > 0) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_keyname().data(), static_cast<int>(this->_internal_keyname().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "RSP_AnnoyFileData.AnnoyKeynameVector.keyname");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_keyname(), target);
  }

  // repeated float vector_data = 2;
  if (this->_internal_vector_data_size() > 0) {
    target = stream->WriteFixedPacked(2, _internal_vector_data(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:RSP_AnnoyFileData.AnnoyKeynameVector)
  return target;
}

size_t AnnoyKeynameVector::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated float vector_data = 2;
  {
    unsigned int count = static_cast<unsigned int>(this->_internal_vector_data_size());
    size_t data_size = 4UL * count;
    if (data_size > 0) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32Size(
            static_cast<::PROTOBUF_NAMESPACE_ID::int32>(data_size));
    }
    int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(data_size);
    _vector_data_cached_byte_size_.store(cached_size,
                                    std::memory_order_relaxed);
    total_size += data_size;
  }

  // string keyname = 1;
  if (this->keyname().size() > 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_keyname());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void AnnoyKeynameVector::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  GOOGLE_DCHECK_NE(&from, this);
  const AnnoyKeynameVector* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<AnnoyKeynameVector>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:RSP_AnnoyFileData.AnnoyKeynameVector)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:RSP_AnnoyFileData.AnnoyKeynameVector)
    MergeFrom(*source);
  }
}

void AnnoyKeynameVector::MergeFrom(const AnnoyKeynameVector& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  vector_data_.MergeFrom(from.vector_data_);
  if (from.keyname().size() > 0) {
    _internal_set_keyname(from._internal_keyname());
  }
}

void AnnoyKeynameVector::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void AnnoyKeynameVector::CopyFrom(const AnnoyKeynameVector& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:RSP_AnnoyFileData.AnnoyKeynameVector)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool AnnoyKeynameVector::IsInitialized() const {
  return true;
}

void AnnoyKeynameVector::InternalSwap(AnnoyKeynameVector* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
  vector_data_.InternalSwap(&other->vector_data_);
  keyname_.Swap(&other->keyname_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}

::PROTOBUF_NAMESPACE_ID::Metadata AnnoyKeynameVector::GetMetadata() const {
  return GetMetadataStatic();
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace RSP_AnnoyFileData
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::RSP_AnnoyFileData::AnnoyItemVector* Arena::CreateMaybeMessage< ::RSP_AnnoyFileData::AnnoyItemVector >(Arena* arena) {
  return Arena::CreateMessageInternal< ::RSP_AnnoyFileData::AnnoyItemVector >(arena);
}
template<> PROTOBUF_NOINLINE ::RSP_AnnoyFileData::AnnoyKeynameVector* Arena::CreateMaybeMessage< ::RSP_AnnoyFileData::AnnoyKeynameVector >(Arena* arena) {
  return Arena::CreateMessageInternal< ::RSP_AnnoyFileData::AnnoyKeynameVector >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
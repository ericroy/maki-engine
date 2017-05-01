#pragma once
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"
#include "core/MakiEngine.h"
#include "core/MakiArray.h"
#include "core/MakiResourcePool.h"
#include "core/MakiAssetLibrary.h"

namespace maki {
	namespace core {

		template<class T>
		class manager_t {
			// Can't use MAKI_NO_COPY here because it's a template type
			manager_t(const manager_t<T> &) = delete;
			void operator=(const manager_t<T> &) = delete;

		public:
			manager_t(uint32_t capacity, const char *debug_name) {
				res_pool_ = new resource_pool_t<T>(capacity, debug_name);
			}

			virtual ~manager_t() {
				MAKI_SAFE_DELETE(res_pool_);
			}

			virtual ref_t<T> get(rid_t rid) {
				return res_pool_->find([rid](const T &r) { return rid == r.rid(); });
			}

			virtual ref_t<T> load(rid_t rid) {
				auto *eng = engine_t::get();
				auto data = eng->assets->alloc_read(rid);
				if (!data)
					return nullptr;
				auto ref = res_pool_->alloc();
				if (!init_resource(ref.ptr(), data))
					return nullptr;
				return ref;
			}

			virtual ref_t<T> get_or_load(rid_t rid) {
				auto ref = get(rid)
				return ref ? ref : load(rid);
			}

			void reload() {
				auto *eng = engine_t::get();
				for (const auto iter : res_pool_) {
					auto ref = *iter;
					// If it's a resource that was loaded from disk
					if (ref->rid() != RID_NONE) {
						auto data = eng->assets->alloc_read(rid);
						if (data) {
							auto *raw = ref.ptr();
							raw->~T();
							new(raw) T();
							init_resource(raw, rid, data);
						}
					}
				}
			}

			bool reload(rid_t rid) {
				auto ref = get(rid);
				if (!ref)
					return false;
				auto *eng = engine_t::get();
				auto data = eng->assets->alloc_read(rid);
				if (data) {
					auto *raw = ref.ptr();
					raw->~T();
					new(raw) T();
					init_resource(raw, rid, data);
				}
				return true
			}

		protected:
			virtual bool init_resource(T *res, rid_t rid, const array_t<char> &data) = 0;

		protected:
			resource_pool_t<T> *res_pool_;
		};


	} // namespace core
} // namespace maki

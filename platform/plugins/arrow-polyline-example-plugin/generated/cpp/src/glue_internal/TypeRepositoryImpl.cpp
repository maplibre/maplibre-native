// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2019 HERE Europe B.V.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
// License-Filename: LICENSE
//
// -------------------------------------------------------------------------------------------------

#include "glue_internal/TypeRepository.h"
#include <mutex>
#include <unordered_map>

namespace glue_internal {

struct TypeRepository::Impl
{
    std::unordered_map<const void*, TypeId> m_repo;
    mutable std::mutex m_mutex;
};

TypeRepository::TypeRepository()
    : pimpl(new TypeRepository::Impl())
{
}

TypeRepository::~TypeRepository()
{
    delete pimpl;
}

void
TypeRepository::add_type(const void* instance, const TypeId& id)
{
    std::lock_guard<std::mutex> lock(pimpl->m_mutex);
    pimpl->m_repo[instance] = id;
}

TypeId
TypeRepository::get_id(const void* instance) const
{
    std::lock_guard<std::mutex> lock(pimpl->m_mutex);
    const auto& found = pimpl->m_repo.find(instance);
    return found != pimpl->m_repo.end() ? found->second : "";
}

void
TypeRepository::remove_type(const void* instance)
{
    std::lock_guard<std::mutex> lock(pimpl->m_mutex);
    pimpl->m_repo.erase(instance);
}

TypeRepository& get_type_repository()
{
    static TypeRepository s_repo;
    return s_repo;
}

}

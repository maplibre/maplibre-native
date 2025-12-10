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

#include "cbridge_internal/include/TypeInitRepository.h"
#include <mutex>
#include <unordered_map>

struct TypeInitRepository::Impl
{
    ::std::unordered_map<TypeId, InitFunction*> m_init;
    std::mutex m_mutex;
};

TypeInitRepository::TypeInitRepository()
    : pimpl(new Impl())
{
}

TypeInitRepository::~TypeInitRepository()
{
    delete pimpl;
}

void
TypeInitRepository::add_init(const TypeId& id, TypeInitRepository::InitFunction* init) {
    std::lock_guard<std::mutex> lock(pimpl->m_mutex);
    pimpl->m_init[id] = init;
}

TypeInitRepository::InitFunction*
TypeInitRepository::get_init(const TypeId& id) const {
    std::lock_guard<std::mutex> lock(pimpl->m_mutex);
    const auto& found = pimpl->m_init.find(id);
    return found != pimpl->m_init.end() ? found->second : nullptr;
}

TypeInitRepository& get_init_repository()
{
    static TypeInitRepository s_repo;
    return s_repo;
}

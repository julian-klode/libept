/** -*- C++ -*-
 * @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * libpkg Debtags data provider
 *
 * Copyright (C) 2003-2007  Enrico Zini <enrico@debian.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef EPT_DEBTAGS_DEBTAGS_TCC
#define EPT_DEBTAGS_DEBTAGS_TCC

#include <ept/debtags/debtags.h>
#include <ept/debtags/maint/serializer.h>

#include <tagcoll/input/stdio.h>
#include <tagcoll/stream/patcher.h>
#include <tagcoll/TextFormat.h>

namespace ept {
namespace debtags {

template<typename OUT>
void Debtags::outputSystem(const OUT& cons)
{
	m_rocoll.output(intToPkg(m_pkgid, vocabulary(), cons));
}

template<typename OUT>
void Debtags::outputSystem(const std::string& filename, const OUT& out)
{
	if (filename == "-")
	{
		tagcoll::input::Stdio input(stdin, "<stdin>");
		tagcoll::textformat::parse(input, ept::debtags::stringToPkg(m_pkgid, m_voc, out));
	}
	else
	{
		tagcoll::input::Stdio input(filename);
		tagcoll::textformat::parse(input, ept::debtags::stringToPkg(m_pkgid, m_voc, out));
	}
}

template<typename OUT>
void Debtags::outputPatched(const OUT& cons)
{
	m_coll.output(intToPkg(m_pkgid, vocabulary(), cons));
}

template<typename OUT>
void Debtags::outputPatched(const std::string& filename, const OUT& out)
{
	const tagcoll::PatchList<string, Tag>& patch = m_coll.changes();
	if (filename == "-")
	{
		tagcoll::input::Stdio input(stdin, "<stdin>");
		tagcoll::textformat::parse(input, ept::debtags::stringToPkg(m_pkgid, m_voc, patcher(patch, out)));
	}
	else
	{
		tagcoll::input::Stdio input(filename);
		tagcoll::textformat::parse(input, ept::debtags::stringToPkg(m_pkgid, m_voc, patcher(patch, out)));
	}
}

}
}

#include <tagcoll/coll/patched.tcc>

#endif

// vim:set ts=4 sw=4:

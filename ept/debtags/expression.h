#ifndef EPT_DEBTAGS_EXPRESSION_H
#define EPT_DEBTAGS_EXPRESSION_H

/** \file
 * Match tag expressions against sets of Debtags Tags
 */

/*
 * Copyright (C) 2003,2004,2005,2006,2007  Enrico Zini <enrico@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ept/debtags/tag.h>
#include <tagcoll/expression.h>
#include <set>

namespace tagcoll
{

template<>
bool Expression::operator()(const std::set<ept::debtags::Tag>& tags) const;

template<>
inline bool Expression::operator()(const std::set<ept::debtags::Facet>& tags) const;

};

#endif
// vim:set ts=4 sw=4:

#ifndef EPT_TEXTSEARCH_EXTRAINDEXERS_H
#define EPT_TEXTSEARCH_EXTRAINDEXERS_H

/** @file
 * @author Enrico Zini <enrico@enricozini.org>
 * Fast full-text search
 */

/*
 * Copyright (C) 2007  Enrico Zini <enrico@debian.org>
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

#include <ept/textsearch/textsearch.h>

namespace ept {
namespace debtags {
class Debtags;
}
namespace textsearch {

struct AptTagsExtraIndexer : public TextSearch::ExtraIndexer
{
	virtual void operator()(Xapian::Document& doc, const apt::PackageRecord& rec) const;
};

struct DebtagsExtraIndexer : public TextSearch::ExtraIndexer
{
	const debtags::Debtags& debtags;
	DebtagsExtraIndexer(const debtags::Debtags& debtags) : debtags(debtags) {}
	virtual void operator()(Xapian::Document& doc, const apt::PackageRecord& rec) const;
};

}
}

// vim:set ts=4 sw=4:
#endif

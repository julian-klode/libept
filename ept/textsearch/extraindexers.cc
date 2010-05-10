
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

#include <ept/textsearch/extraindexers.h>
#include <ept/apt/packagerecord.h>
#include <ept/debtags/debtags.h>

using namespace std;
using namespace ept::debtags;

namespace ept {
namespace textsearch {

void AptTagsExtraIndexer::operator()(Xapian::Document& doc, const apt::PackageRecord& rec) const
{
	// Index tags as well
	set<string> tags = rec.tag();
	for (set<string>::const_iterator ti = tags.begin();
			ti != tags.end(); ++ti)
		doc.add_term("XT"+*ti);
}

void DebtagsExtraIndexer::operator()(Xapian::Document& doc, const apt::PackageRecord& rec) const
{
	// Index tags as well
	set<std::string> tags = debtags.getTagsOfItem(doc.get_data());
	for (set<std::string>::const_iterator ti = tags.begin();
			ti != tags.end(); ++ti)
		doc.add_term("XT"+*ti);
}

}
}

#include <ept/debtags/debtags.tcc>

// vim:set ts=4 sw=4:

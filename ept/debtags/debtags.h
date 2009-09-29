// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
/* @file
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

/*
 * libpkg Debtags data provider
 *
 * Copyright (C) 2003-2007  Enrico Zini <enrico@debian.org>
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

#ifndef EPT_DEBTAGS_DEBTAGS_H
#define EPT_DEBTAGS_DEBTAGS_H

#include <ept/debtags/tag.h>
#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/pkgid.h>

#include <tagcoll/coll/base.h>
#include <tagcoll/coll/intdiskindex.h>
#include <tagcoll/coll/patched.h>

namespace ept {
namespace debtags {
class Debtags;
}
}

namespace tagcoll {
template< typename _, typename _1 > class PatchList;

namespace coll {

template<>
struct coll_traits< ept::debtags::Debtags >
{
	typedef std::string item_type;
	typedef ept::debtags::Tag tag_type;
	typedef std::set< ept::debtags::Tag > tagset_type;
	typedef std::set< std::string > itemset_type;
};

}
}

namespace ept {
namespace debtags {

/**
 * Access the on-disk Debtags tag database.
 *
 * The database is normally found in /var/lib/debtags.
 *
 * Tags and Facets are returned as Tag and Facet objects.  The objects follow
 * the flyweight pattern and access the data contained in the Vocabulary
 * instantiated inside Debtags.
 *
 * It is possible to get a reference to the Vocabulary object using the
 * vocabulary() method.
 */
class Debtags : public tagcoll::coll::Collection<Debtags>
{
protected:
	// Master mmap index container
	tagcoll::diskindex::MasterMMap mastermmap;

	// Debtags database
	tagcoll::coll::IntDiskIndex m_rocoll;
	tagcoll::coll::Patched< tagcoll::coll::IntDiskIndex > m_coll;

	// Package name to ID mapping
	PkgId m_pkgid;

	// Tag vocabulary
	Vocabulary m_voc;

	// User rc directory to store patches
	std::string rcdir;

	// Last modification timestamp of the index
	time_t m_timestamp;

	std::string packageByID(int id) const
	{
		return m_pkgid.byID(id);
	}

	template<typename IDS>
	std::set<std::string> packagesById(const IDS& ids) const
	{
		std::set<std::string> pkgs;
		for (typename IDS::const_iterator i = ids.begin();
				i != ids.end(); ++i)
			pkgs.insert(packageByID(*i));
		return pkgs;
	}

	int idByPackage(const std::string& pkg) const
	{
		return m_pkgid.byName(pkg);
	}

	template<typename PKGS>
	std::set<int> idsByPackages(const PKGS& pkgs) const
	{
		std::set<int> ids;
		for (typename PKGS::const_iterator i = pkgs.begin();
				i != pkgs.end(); ++i)
			ids.insert(idByPackage(*i));
		return ids;
	}

public:
	typedef tagcoll::coll::Patched< tagcoll::coll::IntDiskIndex > coll_type;
	typedef std::pair< std::string, std::set<Tag> > value_type;

	class const_iterator
	{
		const Debtags& coll;
		Debtags::coll_type::const_iterator ci;
		mutable const Debtags::value_type* cached_val;

	protected:
		const_iterator(const Debtags& coll,
						const Debtags::coll_type::const_iterator& ci)
			: coll(coll), ci(ci), cached_val(0) {}

	public:
		~const_iterator()
		{
			if (cached_val)
				delete cached_val;
		}
		const Debtags::value_type operator*() const
		{
			if (cached_val)
				return *cached_val;

			return make_pair(coll.packageByID(ci->first), coll.vocabulary().tagsByID(ci->second));
		}
		const Debtags::value_type* operator->() const
		{
			if (cached_val)
				return cached_val;
			return cached_val = new Debtags::value_type(*(*this));
		}
		const_iterator& operator++()
		{
			++ci;
			if (cached_val)
			{
				delete cached_val;
				cached_val = 0;
			}
			return *this;
		}
		bool operator==(const const_iterator& iter) const
		{
			return ci == iter.ci;
		}
		bool operator!=(const const_iterator& iter) const
		{
			return ci != iter.ci;
		}

		friend class Debtags;
	};
	const_iterator begin() const { return const_iterator(*this, m_coll.begin()); }
	const_iterator end() const { return const_iterator(*this, m_coll.end()); }

	/**
	 * Create a new accessor for the on-disk Debtags database
	 *
	 * \param editable
	 * Specifies if recording of modifications should be enabled.  If editable
	 * is true, then the local state directory will be created when the object
	 * is instantiated.
	 */
    Debtags(bool editable = false);
    ~Debtags() {}

	/// Get the timestamp of when the index was last updated
	time_t timestamp() const { return m_timestamp; }

	/// Return true if this data source has data, false if it's empty
	bool hasData() const { return m_timestamp != 0; }

	coll_type& tagdb() { return m_coll; }
	const coll_type& tagdb() const { return m_coll; }
	tagcoll::PatchList<std::string, Tag> changes() const;

#if 0
	template<typename ITEMS, typename TAGS>
	void insert(const ITEMS& items, const TAGS& tags)
	{
		for (typename ITEMS::const_iterator i = items.begin();
				i != items.end(); ++i)
			m_changes.addPatch(Patch(*i, tags, TagSet()));
	}

	template<typename ITEMS>
	void insert(const ITEMS& items, const wibble::Empty<Tag>& tags)
	{
		// Nothing to do in this case
	}

	/**
	 * Get the changes that have been applied to this collection
	 */
	const Patches& changes() const { return m_changes; }

	/**
	 * Throw away all changes previously applied to this collection
	 */
	void resetChanges() { m_changes.clear(); }

	/**
	 * Set the changes list to a specific patch list
	 */
	void setChanges(const Patches& changes);

	/**
	 * Add a specific patch list to the changes list
	 */
	void addChanges(const Patches& changes);
#endif

    bool hasTag(const Tag& tag) const { return m_coll.hasTag(tag.id()); }

	std::set<Tag> getTagsOfItem(const std::string& item) const
	{
		int id = idByPackage(item);
		if (id == -1) return std::set<Tag>();
		return vocabulary().tagsByID(m_coll.getTagsOfItem(id));
	}

	template<typename ITEMS>
	std::set<Tag> getTagsOfItems(const ITEMS& items) const
	{
		return vocabulary().tagsByID(m_coll.getTagsOfItems(idsByPackages(items)));
	}

	std::set<std::string> getItemsHavingTag(const Tag& tag) const
	{
		return packagesById(m_coll.getItemsHavingTag(tag.id()));
	}
	template<typename TAGS>
	std::set<std::string> getItemsHavingTags(const TAGS& tags) const
	{
		std::set<int> itags;
		for (typename TAGS::const_iterator i = tags.begin();
				i != tags.end(); ++i)
			itags.insert(i->id());
		return packagesById(m_coll.getItemsHavingTags(itags));
	}

#if 0
	ItemSet getTaggedItems() const;
#endif
	std::set<Tag> getAllTags() const
	{
		return vocabulary().tagsByID(m_coll.getAllTags());
	}

	/// Access the vocabulary in use
    Vocabulary& vocabulary() { return m_voc; }
	/// Access the vocabulary in use
    const Vocabulary& vocabulary() const { return m_voc; }

	/**
	 * Access the PkgId in use.
	 *
	 * \note Future implementations may not rely on a PkgId
	 */
	PkgId& pkgid() { return m_pkgid; }
	/**
	 * Access the PkgId in use.
	 *
	 * \note Future implementations may not rely on a PkgId
	 */
	const PkgId& pkgid() const { return m_pkgid; }

	int getCardinality(const Tag& tag) const
	{
		return m_coll.getCardinality(tag.id());
	}

	void applyChange(const tagcoll::PatchList<std::string, Tag>& change)
	{
		using namespace tagcoll;
		PatchList<int, int> intp;
		for (PatchList<std::string, Tag>::const_iterator i = change.begin();
				i != change.end(); ++i)
		{
			Patch<int, int> p(idByPackage(i->first));
			for (std::set<Tag>::const_iterator j = i->second.added.begin();
					j != i->second.added.end(); ++j)
				p.add(j->id());
			for (std::set<Tag>::const_iterator j = i->second.removed.begin();
					j != i->second.removed.end(); ++j)
				p.remove(j->id());
			intp.addPatch(p);
		}
		m_coll.applyChange(intp);
	}

#if 0
	template<typename OUT>
	void output(OUT out) const
	{
		for (const_iterator i = begin(); i != end(); ++i)
		{
			*out = *i;
			++out;
		}
	}
#endif



	/**
	 * Check if the tag database has been created (i.e. if something
	 * equivalend to debtags update has been run)
	 */
	//static bool hasTagDatabase();


	/**
	 * Save in the state storage directory a patch that can be used to turn
	 * the system database into the collection given
	 */
	void savePatch();

	/**
	 * Save in the state storage directory a patch to turn the system database
	 * into the collection given
	 */
	void savePatch(const tagcoll::PatchList<std::string, std::string>& patch);

	/**
	 * Save in the state storage directory a patch to turn the system database
	 * into the collection given
	 */
	void savePatch(const tagcoll::PatchList<std::string, Tag>& patch);

	/**
	 * Send to the central archive a patch that can be used to turn
	 * the system database into the collection given
	 */
	void sendPatch();

	/**
	 * Send the given patch to the central archive
	 */
	void sendPatch(const tagcoll::PatchList<std::string, std::string>& patch);

	/**
	 * Send the given patch to the central archive
	 */
	void sendPatch(const tagcoll::PatchList<std::string, Tag>& patch);


	/**
	 * Output the current Debian tags database to a consumer of <std::string, Tag>
	 *
	 * \note The collection is sent to 'cons' without merging repeated items
	 */
	template<typename OUT>
	void outputSystem(const OUT& cons);

	/**
	 * Output the given tag file to a consumer of <std::string, Tag>
	 *
	 * \note The collection is sent to 'cons' without merging repeated items
	 */
	template<typename OUT>
	void outputSystem(const std::string& filename, const OUT& out);

	/**
	 * Output the current Debian tags database, patched with local patch,
	 * to a Consumer of <std::string, Tag>
	 *
	 * \note The collection is sent to 'cons' without merging repeated items
	 */
	template<typename OUT>
	void outputPatched(const OUT& cons);

	/**
	 * Output the given tag file, patched with local patch,
	 * to a Consumer of <std::string, Tag>
	 *
	 * \note The collection is sent to 'cons' without merging repeated items
	 */
	template<typename OUT>
	void outputPatched(const std::string& filename, const OUT& out);
};


}
}

// vim:set ts=4 sw=4:
#endif

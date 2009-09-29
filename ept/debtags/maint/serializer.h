// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-
/**
 * @file cache/component/debtags/serializer.h
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

#ifndef EPT_DEBTAGS_SERIALIZER_H
#define EPT_DEBTAGS_SERIALIZER_H

#include <ept/debtags/vocabulary.h>
#include <ept/debtags/maint/pkgid.h>
#include <tagcoll/patch.h>
#include <wibble/mixin.h>
#include <string>

namespace ept {
namespace debtags {

template<typename OUT>
class IntToPkg : public wibble::mixin::OutputIterator< IntToPkg<OUT> >
{
	PkgId& pkgid;
	Vocabulary& voc;
	OUT out;

public:
	IntToPkg(PkgId& pkgid, Vocabulary& voc, const OUT& out)
		: pkgid(pkgid), voc(voc), out(out) {}

	template<typename ITEMS, typename TAGS>
	IntToPkg<OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		std::set<std::string> ritems;
		std::set<Tag> rtags;

		for (typename ITEMS::const_iterator i = data.first.begin();
				i != data.first.end(); ++i)
		{
			std::string pkg = pkgid.byID(*i);
			if (!pkg.empty())
				ritems.insert(pkg);
		}

		for (typename TAGS::const_iterator i = data.second.begin();
				i != data.second.end(); ++i)
		{
			Tag t = voc.tagByID(*i);
			if (t.valid())
				rtags.insert(t);
		}

		if (!ritems.empty() && !rtags.empty())
		{
			*out = make_pair(ritems, rtags);
			++out;
		}
		return *this;
	}
};

template<typename OUT>
IntToPkg<OUT> intToPkg(PkgId& pkgid, Vocabulary& voc, const OUT& out)
{
	return IntToPkg<OUT>(pkgid, voc, out);
}

template<typename OUT>
class StringToInt : public wibble::mixin::OutputIterator< StringToInt<OUT> >
{
	PkgId& pkgid;
	Vocabulary& voc;
	OUT out;

public:
	StringToInt(PkgId& pkgid, Vocabulary& voc, const OUT& out)
		: pkgid(pkgid), voc(voc), out(out) {}

	template<typename ITEMS, typename TAGS>
	StringToInt<OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		std::set<int> ritems;
		std::set<int> rtags;

		for (typename ITEMS::const_iterator i = data.first.begin();
				i != data.first.end(); ++i)
		{
			int id = pkgid.byName(*i);
			if (id != -1)
				ritems.insert(id);
		}

		for (typename TAGS::const_iterator i = data.second.begin();
				i != data.second.end(); ++i)
		{
			Tag t = voc.tagByName(*i);
			if (t.valid())
				rtags.insert(t.id());
		}

		if (!ritems.empty() && !rtags.empty())
		{
			*out = make_pair(ritems, rtags);
			++out;
		}
		return *this;
	}

};

template<typename OUT>
StringToInt<OUT> stringToInt(PkgId& pkgid, Vocabulary& voc, const OUT& out)
{
	return StringToInt<OUT>(pkgid, voc, out);
}

template<typename OUT>
class StringToPkg : public wibble::mixin::OutputIterator< StringToPkg<OUT> >
{
	PkgId& pkgid;
	Vocabulary& voc;
	OUT out;

public:
	StringToPkg(PkgId& pkgid, Vocabulary& voc, const OUT& out)
		: pkgid(pkgid), voc(voc), out(out) {}

	template<typename ITEMS, typename TAGS>
	StringToPkg<OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		std::set<std::string> ritems;
		std::set<Tag> rtags;

		for (typename ITEMS::const_iterator i = data.first.begin();
				i != data.first.end(); ++i)
		{
			// Ensure that the package exists in the pkgid database
			if (pkgid.byName(*i) == -1)
				continue;
			ritems.insert(*i);
		}

		for (typename TAGS::const_iterator i = data.second.begin();
				i != data.second.end(); ++i)
		{
			Tag t = voc.tagByName(*i);
			if (t.valid())
				rtags.insert(t);
		}

		if (!ritems.empty() && !rtags.empty())
		{
			*out = make_pair(ritems, rtags);
			++out;
		}
		return *this;
	}

};

template<typename OUT>
StringToPkg<OUT> stringToPkg(PkgId& pkgid, Vocabulary& voc, const OUT& out)
{
	return StringToPkg<OUT>(pkgid, voc, out);
}

template<typename OUT>
class PkgToString : public wibble::mixin::OutputIterator< PkgToString<OUT> >
{
	OUT out;
public:
	PkgToString(const OUT& out) : out(out) {}

	template<typename ITEMS, typename TAGS>
	PkgToString<OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		std::set<std::string> stags;
		for (typename TAGS::const_iterator i = data.second.begin();
				i != data.second.end(); ++i)
			if (i->valid())
				stags.insert(i->fullname());
		*out = make_pair(data.first, stags);
		++out;
		return *this;
	}
};

template<typename OUT>
PkgToString<OUT> pkgToString(const OUT& out)
{
	return PkgToString<OUT>(out);
}

template<typename OUT>
class PatchStringToInt : public wibble::mixin::OutputIterator< PatchStringToInt<OUT> >
{
	PkgId& pkgid;
	Vocabulary& voc;
	OUT out;

public:
	PatchStringToInt(PkgId& pkgid, Vocabulary& voc, const OUT& out)
		: pkgid(pkgid), voc(voc), out(out) {}

	PatchStringToInt<OUT>& operator=(const tagcoll::Patch<std::string, std::string>& patch)
	{
		int id = pkgid.byName(patch.item);
		if (id == -1)
			return *this;

		tagcoll::Patch<int, int> res(id);
		for (std::set<std::string>::const_iterator i = patch.added.begin();
				i != patch.added.end(); ++i)
		{
			Tag tag = voc.tagByName(*i);
			if (tag.valid())
				res.add(tag.id());
		}
		for (std::set<std::string>::const_iterator i = patch.removed.begin();
				i != patch.removed.end(); ++i)
		{
			Tag tag = voc.tagByName(*i);
			if (tag.valid())
				res.remove(tag.id());
		}
		*out = res;
		++out;
		return *this;
	}
};

template<typename OUT>
PatchStringToInt<OUT> patchStringToInt(PkgId& pkgid, Vocabulary& voc, const OUT& out)
{
	return PatchStringToInt<OUT>(pkgid, voc, out);
}

template<typename OUT>
class PatchIntToString : public wibble::mixin::OutputIterator< PatchIntToString<OUT> >
{
	PkgId& pkgid;
	Vocabulary& voc;
	OUT out;

public:
	PatchIntToString(PkgId& pkgid, Vocabulary& voc, const OUT& out)
		: pkgid(pkgid), voc(voc), out(out) {}

	PatchIntToString<OUT>& operator=(const tagcoll::Patch<int, int>& patch)
	{
		std::string name = pkgid.byID(patch.item);
		if (name.empty())
			return *this;

		tagcoll::Patch<std::string, std::string> res(name);
		for (std::set<int>::const_iterator i = patch.added.begin();
				i != patch.added.end(); ++i)
		{
			Tag tag = voc.tagByID(*i);
			if (tag.valid())
				res.add(tag.fullname());
		}
		for (std::set<int>::const_iterator i = patch.removed.begin();
				i != patch.removed.end(); ++i)
		{
			Tag tag = voc.tagByID(*i);
			if (tag.valid())
				res.remove(tag.fullname());
		}
		*out = res;
		++out;
		return *this;
	}
};

template<typename OUT>
PatchIntToString<OUT> patchIntToString(PkgId& pkgid, Vocabulary& voc, const OUT& out)
{
	return PatchIntToString<OUT>(pkgid, voc, out);
}

#if 0
	GOOD STUFF

template<typename OUT>
class ToInt : public wibble::mixin::OutputIterator< ToInt<OUT> >
{
	OUT out;
public:
	ToInt(const OUT& out) : out(out) {}

	template<typename ITEMS, typename TAGS>
	ToInt<OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		std::set<int> iitems;
		std::set<int> itags;
		for (typename ITEMS::const_iterator i = data.first.begin();
				i != data.first.end(); ++i)
			if (i->valid())
				iitems.insert(i->ondiskId());
		for (typename TAGS::const_iterator i = data.second.begin();
				i != data.second.end(); ++i)
			if (i->valid())
				itags.insert(i->id());
		*out = make_pair(iitems, itags);
		++out;
		return *this;
	}
};

template<typename OUT>
ToInt<OUT> toInt(const OUT& out)
{
	return ToInt<OUT>(out);
}

template<typename ITEMCONV, typename TAGCONV, typename OUT>
class Converter : public wibble::mixin::OutputIterator< Converter<ITEMCONV, TAGCONV, OUT> >
{
	ITEMCONV itemconv;
	TAGCONV tagconv;
	OUT out;

public:
	Converter(const ITEMCONV& itemconv, const TAGCONV& tagconv, const OUT& out)
		: itemconv(itemconv), tagconv(tagconv), out(out) {}

	template<typename ITEMS, typename TAGS>
	Converter<ITEMCONV, TAGCONV, OUT>& operator=(const std::pair<ITEMS, TAGS>& data)
	{
		*out = make_pair(itemconv(data.first), tagconv(data.second));
		++out;
		return *this;
	}
};

template<typename ITEMCONV, typename TAGCONV, typename OUT>
Converter<ITEMCONV, TAGCONV, OUT> converter(const ITEMCONV& itemconv, const TAGCONV& tagconv, const OUT& out)
{
	return Converter<ITEMCONV, TAGCONV, OUT>(itemconv, tagconv, out);
}


template<typename OUT>
class PatchToString : public wibble::mixin::OutputIterator< PatchToString<OUT> >
{
	OUT out;

public:
	PatchToString(const OUT& out) : out(out) {}

	template<typename PKG, typename TAG>
	PatchToString<OUT>& operator=(const tagcoll::Patch<PKG, TAG>& patch)
	{
		if (!patch.item.valid())
			return *this;

		tagcoll::Patch<std::string, std::string> res(patch.item.name());
		for (typename std::set<TAG>::const_iterator i = patch.added.begin();
				i != patch.added.end(); ++i)
			if (i->valid())
				res.add(i->fullname());
		for (typename std::set<TAG>::const_iterator i = patch.removed.begin();
				i != patch.removed.end(); ++i)
			if (i->valid())
				res.remove(i->fullname());
		*out = res;
		++out;
		return *this;
	}
};

template<typename OUT>
PatchToString<OUT> patchToString(const OUT& out)
{
	return PatchToString<OUT>(out);
}

#endif

}
}

#if 0

namespace tagcoll {
namespace coll {

template<>
struct coll_traits< ept::cache::debtags::DebtagsIndex >
{
	typedef ept::cache::Package<> item_type;
	typedef ept::cache::debtags::Tag tag_type;
	typedef std::set< ept::cache::Package<> > itemset_type;
	typedef std::set<ept::cache::debtags::Tag> tagset_type;
};

}
}

namespace ept {
namespace cache {
namespace debtags {

#if 0
/**
 * Convert Facets to ints
 */
class FacetIntConverter : public Implementation<FacetIntConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Facet, int>,
	public Tagcoll::Converter<int, aptFront::cache::entity::Facet>
{
	typedef aptFront::cache::entity::Facet Facet;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Facet> FacetSet;
	typedef Tagcoll::OpSet<int> IntSet;
public:
	virtual int operator()(const aptFront::cache::entity::Facet& item) const;
	virtual aptFront::cache::entity::Facet operator()(const int& item) const;

	virtual IntSet operator()(const FacetSet& item) const
		{ return Tagcoll::Converter<Facet, int>::operator()(item); }
	virtual FacetSet operator()(const IntSet& item) const
		{ return Tagcoll::Converter<int, Facet>::operator()(item); }

	static std::string componentName();
};

/**
 * Convert Facets to strings
 */
class FacetStringConverter : public Implementation<FacetStringConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Facet, std::string>,
	public Tagcoll::Converter<std::string, aptFront::cache::entity::Facet>
{
	typedef aptFront::cache::entity::Facet Facet;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Facet> FacetSet;
	typedef Tagcoll::OpSet<std::string> StringSet;
public:
	virtual std::string operator()(const aptFront::cache::entity::Facet& item) const;
	virtual aptFront::cache::entity::Facet operator()(const std::string& item) const;

	virtual StringSet operator()(const FacetSet& item) const
		{ return Tagcoll::Converter<Facet, std::string>::operator()(item); }
	virtual FacetSet operator()(const StringSet& item) const
		{ return Tagcoll::Converter<std::string, Facet>::operator()(item); }

	static std::string componentName();
};

/**
 * Convert Vocabulary to ints
 */
class TagIntConverter : public Implementation<TagIntConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Tag, int>,
	public Tagcoll::Converter<int, aptFront::cache::entity::Tag>
{
	typedef aptFront::cache::entity::Tag Tag;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Tag> TagSet;
	typedef Tagcoll::OpSet<int> IntSet;
public:
	virtual int operator()(const aptFront::cache::entity::Tag& item) const;
	virtual aptFront::cache::entity::Tag operator()(const int& item) const;

	virtual IntSet operator()(const TagSet& item) const
		{ return Tagcoll::Converter<Tag, int>::operator()(item); }
	virtual TagSet operator()(const IntSet& item) const
		{ return Tagcoll::Converter<int, Tag>::operator()(item); }

	static std::string componentName();
};

/**
 * Convert Vocabulary to strings
 */
class TagStringConverter : public Implementation<TagStringConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Tag, std::string>,
	public Tagcoll::Converter<std::string, aptFront::cache::entity::Tag>
{
	typedef aptFront::cache::entity::Tag Tag;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Tag> TagSet;
	typedef Tagcoll::OpSet<std::string> StringSet;
public:
	virtual std::string operator()(const Tag& item) const;
	virtual Tag operator()(const std::string& item) const;

	virtual StringSet operator()(const TagSet& item) const
		{ return Tagcoll::Converter<Tag, std::string>::operator()(item); }
	virtual TagSet operator()(const StringSet& item) const
		{ return Tagcoll::Converter<std::string, Tag>::operator()(item); }

	TagSet parseTagList(const std::string& str) const;

	static std::string componentName();
};

/**
 * Convert Aggregator to ints
 */
class PackageIntConverter : public Implementation<PackageIntConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Package, int>,
	public Tagcoll::Converter<int, aptFront::cache::entity::Package>
{
	typedef aptFront::cache::entity::Package Package;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Package> PackageSet;
	typedef Tagcoll::OpSet<int> IntSet;
public:
	virtual int operator()(const Package& item) const;
	virtual Package operator()(const int& item) const;

	virtual IntSet operator()(const PackageSet& item) const
		{ return Tagcoll::Converter<Package, int>::operator()(item); }
	virtual PackageSet operator()(const IntSet& item) const
		{ return Tagcoll::Converter<int, Package>::operator()(item); }

	static std::string componentName();
};

/**
 * Convert Aggregator to strings
 */
class PackageStringConverter : public Implementation<PackageStringConverter>,
	public Tagcoll::Converter<aptFront::cache::entity::Package, std::string>,
	public Tagcoll::Converter<std::string, aptFront::cache::entity::Package>
{
	typedef aptFront::cache::entity::Package Package;
	typedef Tagcoll::OpSet<aptFront::cache::entity::Package> PackageSet;
	typedef Tagcoll::OpSet<std::string> StringSet;
public:
	virtual std::string operator()(const Package& item) const;
	virtual Package operator()(const std::string& item) const;

	virtual StringSet operator()(const PackageSet& item) const
		{ return Tagcoll::Converter<Package, std::string>::operator()(item); }
	virtual PackageSet operator()(const StringSet& item) const
		{ return Tagcoll::Converter<std::string, Package>::operator()(item); }

	static std::string componentName();
};

#endif

}
}
}

#endif

#endif
// -*- mode: c++; tab-width: 4; indent-tabs-mode: t -*-

#if 0
/**
 * @file cache/debtags/serializer.h
 * @author Enrico Zini (enrico) <enrico@enricozini.org>
 */

#ifndef EPT_CACHE_DEBTAGS_SERIALIZER_TCC
#define EPT_CACHE_DEBTAGS_SERIALIZER_TCC

#include <ept/cache/debtags/serializer.h>
#if 0
#include <ept/cache/debtags/pkgidx.h>
#include <ept/cache/debtags/vocabulary.h>
#include <ept/cache/package.h>
//#include <ept/cache/cache.h>
#endif

namespace ept {
namespace t {
namespace cache {
namespace debtags {



#if 0
string FacetIntConverter::componentName() { return "FacetIntConverter"; }

int FacetIntConverter::operator()(const aptFront::cache::entity::Facet& item) const
{
	if (!item.valid()) return -1;
	return item.id();
}
aptFront::cache::entity::Facet FacetIntConverter::operator()(const int& item) const
{
	return cache().tags().facetByID(item);
}

string FacetStringConverter::componentName() { return "FacetStringConverter"; }

std::string FacetStringConverter::operator()(const aptFront::cache::entity::Facet& item) const
{
	if (!item.valid()) return string();
	return item.name();
}
aptFront::cache::entity::Facet FacetStringConverter::operator()(const std::string& item) const
{
	return cache().tags().facetByName(item);
}

string TagIntConverter::componentName() { return "TagIntConverter"; }

int TagIntConverter::operator()(const aptFront::cache::entity::Tag& item) const
{
	if (!item.valid()) return -1;
	return item.id();
}
aptFront::cache::entity::Tag TagIntConverter::operator()(const int& item) const
{
	return cache().tags().tagByID(item);
}

string TagStringConverter::componentName() { return "TagStringConverter"; }

std::string TagStringConverter::operator()(const aptFront::cache::entity::Tag& item) const
{
	if (!item.valid()) return string();
	return item.fullname();
}
aptFront::cache::entity::Tag TagStringConverter::operator()(const std::string& item) const
{
	return cache().tags().tagByName(item);
}

Tagcoll::OpSet<entity::Tag> TagStringConverter::parseTagList(const std::string& str) const
{
	if (str.empty())
		return Tagcoll::OpSet<entity::Tag>();

	size_t i = str.find(", ");
	if (i == string::npos)
	{
		// Check if we need curly brace expansion
		if (str[str.size() - 1] == '}')
		{
			using namespace std;
			Tagcoll::OpSet<entity::Tag> res;
			size_t begin = str.find('{');
			if (begin == string::npos)
				return res;
			string prefix(str, 0, begin);
			++begin;
			size_t end;
			while ((end = str.find(',', begin)) != string::npos)
			{
				res += (*this)(prefix + str.substr(begin, end-begin));
				begin = end + 1;
			}
			res += (*this)(prefix + str.substr(begin, str.size() - 1 - begin));
			return res;
		} else {
			entity::Tag t = (*this)(str);
			if (t.valid())
				return Tagcoll::OpSet<entity::Tag>() + t;
			else
				return Tagcoll::OpSet<entity::Tag>();
		}
	} else {
		return parseTagList(string(str, 0, i)) + parseTagList(string(str, i+2));
	}
}

string PackageIntConverter::componentName() { return "PackageIntConverter"; }

int PackageIntConverter::operator()(const aptFront::cache::entity::Package& item) const
{
	if (!item.valid()) return -1;
	return item.id();
}
aptFront::cache::entity::Package PackageIntConverter::operator()(const int& item) const
{
	PkgIdx& p = cache().pkgidx();
	return cache().packages().packageByName(string(p.name(item), p.size(item)));
}

string PackageStringConverter::componentName() { return "PackageStringConverter"; }

std::string PackageStringConverter::operator()(const aptFront::cache::entity::Package& item) const
{
	if (!item.valid()) return string();
	return item.name();
}
aptFront::cache::entity::Package PackageStringConverter::operator()(const std::string& item) const
{
	return cache().packages().packageByName(item);
}
#endif

}
}

#endif

#if 0
#ifdef COMPILE_TESTSUITE
//#include <apt-front/cache/component/debtags/update.h>
#include <iostream>
#include "test-utils.h"

namespace tut {
using namespace aptFront::cache;
using namespace component;
using namespace debtags;
using namespace std;

struct cache_component_debtags_serializer_shar {
    cache_component_debtags_serializer_shar () {
        aptInit ();
        ok = true;
        debtags::fetchNewData();
        c.open( Cache::OpenDefault |
                Cache::OpenReadOnly | Cache::OpenDebtags );
    }
    void check() {
        if (ok) return;
        ok = true;
        throw warning( "debtags init failed, cancelling" );
    }
    ~cache_component_debtags_serializer_shar() {
        check();
    }
    Cache c;
    bool ok;
};

TESTGRP( cache_component_debtags_serializer );

using namespace Tagcoll;

template<> template<>
void to::test<1> ()
{
    check();

	PackageStringConverter& psc = c.packagestringconverter();

	ensure(psc("Slartibartsfart") == entity::Package());

    /* Get the 'debtags' package */
    entity::Package p = c.packages().packageByName( "debtags" );
    ensure(p.valid());

	/* Get the 'debtags' package using the serializer */
	entity::Package p1 = psc("debtags");
	ensure(p1.valid());

	/* They must be the same */
	ensure(p == p1);

	ensure_equals(psc(p), "debtags");
	ensure_equals(psc(p1), "debtags");
	ensure_equals(psc(p), psc(p1));

	/* If there is an invalid package to serialize, it should be discarded */
	{
		Tagcoll::OpSet<entity::Package> pkgs;
		pkgs += c.packages().packageByName( "debtags" );
		pkgs += c.packages().packageByName( "tagcoll" );
		pkgs += entity::Package();

		ensure_equals (pkgs.size(), 3u);
		ensure_equals (psc(pkgs).size(), 2u);
		ensure (psc(pkgs).contains("debtags"));
		ensure (psc(pkgs).contains("tagcoll"));
	}

	/* If there is an invalid package to serialize, it should be discarded */
	{
		Tagcoll::OpSet<std::string> pkgs;
		pkgs += "debtags";
		pkgs += "tagcoll";
		pkgs += "Slartibartsfart";

		ensure_equals (pkgs.size(), 3u);
		ensure_equals (psc(pkgs).size(), 2u);
		ensure (psc(pkgs).contains(psc("debtags")));
		ensure (psc(pkgs).contains(psc("tagcoll")));
		ensure (!psc(pkgs).contains(entity::Package()));
	}
}

ostream& operator<<(ostream& out, const entity::Package& pkg)
{
	if (pkg.valid())
		return out << pkg.name();
	else
		return out << "(invalid package)";
}

// Check that package conversions work two-way
template<> template<>
void to::test<2> ()
{
	PackageStringConverter& psc = c.packagestringconverter();
	for (component::Aggregator::iterator i = c.packages().packagesBegin();
			i != c.packages().packagesEnd(); ++i)
	{
		try {
			ensure_equals(*i, psc(psc(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[string] of package " << i->name(string("(invalid package)")) << endl;
			throw;
		}
	}

	PackageIntConverter& pic = c.packageintconverter();
	for (component::Aggregator::iterator i = c.packages().packagesBegin();
			i != c.packages().packagesEnd(); ++i)
	{
		try {
			ensure_equals(*i, pic(pic(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[int] of package " << i->name(string("(invalid package)")) << endl;
			throw;
		}
	}
}

// Check that facet conversions work two-way
template<> template<>
void to::test<3> ()
{
	typedef Tagcoll::OpSet<entity::Facet> FacetSet;

	FacetStringConverter& fsc = c.facetstringconverter();
	FacetSet allFacets(c.tags().facets());
	for (FacetSet::const_iterator i = allFacets.begin(); i != allFacets.end(); i++)
	{
		try {
			ensure_equals(*i, fsc(fsc(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[string] of facet " << i->name() << endl;
			throw;
		}
	}

	FacetIntConverter& fic = c.facetintconverter();
	for (FacetSet::const_iterator i = allFacets.begin(); i != allFacets.end(); i++)
	{
		try {
			ensure_equals(*i, fic(fic(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[int] of facet " << i->name() << endl;
			throw;
		}
	}
}

// Check that tag conversions work two-way
template<> template<>
void to::test<4> ()
{
	typedef Tagcoll::OpSet<entity::Tag> TagSet;

	TagStringConverter& tsc = c.tagstringconverter();
	TagSet allTags(c.tags().tags());
	for (TagSet::const_iterator i = allTags.begin(); i != allTags.end(); i++)
	{
		try {
			ensure_equals(*i, tsc(tsc(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[string] of tag " << i->fullname() << endl;
			throw;
		}
	}

	TagIntConverter& tic = c.tagintconverter();
	for (TagSet::const_iterator i = allTags.begin(); i != allTags.end(); i++)
	{
		try {
			ensure_equals(*i, tic(tic(*i)));
		} catch (...) {
			cerr << "Note: exception thrown during processing[int] of tag " << i->fullname() << endl;
			throw;
		}
	}
}

// Check TagStringConverter::parseTagList
template<> template<>
void to::test<5> ()
{
	TagStringConverter& tsc = c.tagstringconverter();
	OpSet<entity::Tag> ts;

	// First ensure that we're using existing tags as samples
	ensure(tsc("accessibility::TODO") != entity::Tag());
	ensure(tsc("role::sw:devel-lib") != entity::Tag());
	ensure(tsc("x11::xserver") != entity::Tag());
	ensure(tsc("antani") == entity::Tag());
	ensure(tsc("blinda") == entity::Tag());
	ensure(tsc("supercazzola") == entity::Tag());

	ts = tsc.parseTagList("role::sw:devel-lib");
	ensure_equals(ts.size(), 1u);
	ensure(ts.contains(tsc("role::sw:devel-lib")));

	ts = tsc.parseTagList("accessibility::TODO, x11::xserver, role::sw:devel-lib");
	ensure_equals(ts.size(), 3u);
	ensure(ts.contains(tsc("accessibility::TODO")));
	ensure(ts.contains(tsc("role::sw:devel-lib")));
	ensure(ts.contains(tsc("x11::xserver")));

	ts = tsc.parseTagList("antani");
	ensure_equals(ts.size(), 0u);

	ts = tsc.parseTagList("antani, blinda, supercazzola");
	ensure_equals(ts.size(), 0u);

	ts = tsc.parseTagList("antani, x11::xserver, blinda");
	ensure_equals(ts.size(), 1u);
	ensure(ts.contains(tsc("x11::xserver")));
}

// Check TagStringConverter::parseTagList's handling of curly brace expansion
template<> template<>
void to::test<6> ()
{
	TagStringConverter& tsc = c.tagstringconverter();
	OpSet<entity::Tag> ts;

	// First ensure that we're using existing tags as samples
	ensure(tsc("role::TODO") != entity::Tag());
	ensure(tsc("role::sw:server") != entity::Tag());
	ensure(tsc("role::aux:dummy") != entity::Tag());
	ensure(tsc("role::sw:amusement") != entity::Tag());
	ensure(tsc("role::sw:server{}") == entity::Tag());
	ensure(tsc("role::{}") == entity::Tag());
	ensure(tsc("role::{") == entity::Tag());
	ensure(tsc("role::}") == entity::Tag());

	ts = tsc.parseTagList("role::{TODO,sw:server,aux:dummy,sw:amusement}");
	ensure_equals(ts.size(), 4u);
	ensure(ts.contains(tsc("role::TODO")));
	ensure(ts.contains(tsc("role::sw:server")));
	ensure(ts.contains(tsc("role::aux:dummy")));
	ensure(ts.contains(tsc("role::sw:amusement")));

	ts = tsc.parseTagList("role::{TODO,aux:dummy}, role::sw:{server,amusement}");
	ensure_equals(ts.size(), 4u);
	ensure(ts.contains(tsc("role::TODO")));
	ensure(ts.contains(tsc("role::sw:server")));
	ensure(ts.contains(tsc("role::aux:dummy")));
	ensure(ts.contains(tsc("role::sw:amusement")));
}

}
#endif
#endif
#endif
// vim:set ts=4 sw=4:

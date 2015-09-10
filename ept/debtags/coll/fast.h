#ifndef EPT_DEBTAGS_COLL_FAST_H
#define EPT_DEBTAGS_COLL_FAST_H

/** \file
 * Fast index for tag data
 */

/*
 * Copyright (C) 2005--2015  Enrico Zini <enrico@debian.org>
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

#include <set>
#include <map>
#include <string>
#include <vector>

namespace ept {
namespace debtags {
namespace coll {

/**
 * In-memory collection with both item->tags and tag->items mappings.
 */
class Fast
{
protected:
    std::map<std::string, std::set<std::string>> items;
    std::map<std::string, std::set<std::string>> tags;

public:
    typedef std::map<std::string, std::set<std::string>>::const_iterator const_iterator;
    typedef std::map<std::string, std::set<std::string>>::iterator iterator;
    typedef std::map<std::string, std::set<std::string>>::value_type value_type;
    typedef std::map<std::string, std::set<std::string>>::const_iterator const_tag_iterator;
    typedef std::map<std::string, std::set<std::string>>::iterator tag_iterator;

    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }
    iterator begin() { return items.begin(); }
    iterator end() { return items.end(); }

    const_tag_iterator tagBegin() const { return tags.begin(); }
    const_tag_iterator tagEnd() const { return tags.end(); }
    tag_iterator tagBegin() { return tags.begin(); }
    tag_iterator tagEnd() { return tags.end(); }

    void insert(const std::string& item, const std::set<std::string>& tags);
    void insert(const std::set<std::string>& items, const std::string& tag);
    void insert(const std::set<std::string>& items, const std::set<std::string>& tags);

    void clear() { items.clear(); tags.clear(); }

    std::set<std::string> getTagsOfItem(const std::string& item) const;
    std::set<std::string> getItemsHavingTag(const std::string& tag) const;

    /**
     * Get the items which are tagged with at least the tags `tags'
     *
     * \return
     *   The items found, or an empty set if no items have that tag
     */
    std::set<std::string> getItemsHavingTags(const std::set<std::string>& tags) const;

    bool empty() const { return items.empty(); }

    bool hasItem(const std::string& item) const { return items.find(item) != items.end(); }
    bool hasTag(const std::string& tag) const { return tags.find(tag) != tags.end(); }
    std::set<std::string> getTaggedItems() const;
    std::set<std::string> getAllTags() const;
    std::vector<std::string> getAllTagsAsVector() const;

    unsigned int itemCount() const { return items.size(); }
    unsigned int tagCount() const { return tags.size(); }

    // tag1 implies tag2 if the itemset of tag1 is a subset of the itemset of
    // tag2
    std::set<std::string> getTagsImplying(const std::string& tag) const;

    // Return the items which have the exact tagset 'tags'
    std::set<std::string> getItemsExactMatch(const std::set<std::string>& tags) const;

    std::string findTagWithMaxCardinality(size_t& card) const;

    /**
     * Return the collection with only those items that have this tag, but with
     * the given tag removed
     */
    Fast getChildCollection(const std::string& tag) const;

    void removeTag(const std::string& tag);
    void removeTagsWithCardinalityLessThan(size_t card);
};

}
}
}
#endif

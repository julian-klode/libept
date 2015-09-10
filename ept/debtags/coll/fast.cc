/*
 * Fast index for tag data
 *
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

#include <ept/debtags/coll/fast.h>
#include <ept/debtags/coll/set.h>
#include <ept/debtags/coll/operators.h>

using namespace std;
using namespace ept::debtags::coll::operators;

namespace ept {
namespace debtags {
namespace coll {

void Fast::insert(const std::set<std::string>& items, const std::set<std::string>& tags)
{
    if (tags.empty())
        return;

    for (const auto& i: items)
        insert(i, tags);
}

void Fast::insert(const std::string& item, const std::set<std::string>& tags)
{
    if (tags.empty())
        return;

    auto iter = this->items.find(item);
    if (iter == this->items.end())
        this->items.insert(std::make_pair(item, tags));
    else
        iter->second |= tags;

    for (typename std::set<std::string>::const_iterator i = tags.begin();
            i != tags.end(); ++i)
    {
        typename std::map< std::string, std::set<std::string> >::iterator iter = this->tags.find(*i);
        if (iter == this->tags.end())
            this->tags.insert(std::make_pair(*i, std::set<std::string>() | item));
        else
            iter->second |= item;
    }
}

void Fast::insert(const std::set<std::string>& items, const std::string& tag)
{
    for (typename std::set<std::string>::const_iterator i = items.begin();
            i != items.end(); ++i)
    {
        typename std::map< std::string, std::set<std::string> >::iterator iter = this->items.find(*i);
        if (iter == this->items.end())
            this->items.insert(std::make_pair(*i, std::set<std::string>() | tag));
        else
            iter->second |= tag;
    }

    typename std::map< std::string, std::set<std::string> >::iterator iter = this->tags.find(tag);
    if (iter == this->tags.end())
        this->tags.insert(std::make_pair(tag, items));
    else
        iter->second |= items;
}

std::set<std::string> Fast::getItemsHavingTag(const std::string& tag) const
{
    typename map<std::string, std::set<std::string> >::const_iterator i = tags.find(tag);
    if (i != tags.end())
        return i->second;
    else
        return std::set<std::string>();
}

std::set<std::string> Fast::getItemsHavingTags(const std::set<std::string>& tags) const 
{
    if (tags.empty())
        return std::set<std::string>();

    auto i = tags.begin();
    auto res = getItemsHavingTag(*i);

    for (++i ; i != tags.end(); ++i)
        res &= getItemsHavingTag(*i);

    return res;
}


std::set<std::string> Fast::getTagsOfItem(const std::string& item) const
{
    typename map<std::string, std::set<std::string> >::const_iterator i = items.find(item);
    if (i != items.end())
        return i->second;
    else
        return std::set<std::string>();
}

std::set<std::string> Fast::getTaggedItems() const
{
    std::set<std::string> res;
    for (typename map<std::string, std::set<std::string> >::const_iterator i = items.begin();
            i != items.end(); i++)
        res |= i->first;
    return res;
}

std::set<std::string> Fast::getAllTags() const
{
    std::set<std::string> res;
    for (typename map<std::string, std::set<std::string> >::const_iterator i = tags.begin();
            i != tags.end(); i++)
        res |= i->first;
    return res;
}

std::vector<std::string> Fast::getAllTagsAsVector() const
{
    std::vector<std::string> res;
    for (typename map<std::string, std::set<std::string> >::const_iterator i = tags.begin();
            i != tags.end(); i++)
        res.push_back(i->first);
    return res;
}

std::set<std::string> Fast::getTagsImplying(const std::string& tag) const
{
    // tag1 implies tag2 if the itemset of tag1 is a subset of the itemset of tag2
    std::set<std::string> res;
    std::set<std::string> itemsToCheck = getItemsHavingTag(tag);
    // TODO: choose which one is the most efficient implementation
#if 0
    // Roughly:
    // O(n[pkgs per tag] * log(nitems) * log(n[items per pkg]) + n[tags per item] * n[items per tag])
    std::set<std::string> tagsToCheck;
    for (std::set<std::string>::const_iterator i = itemsToCheck.begin();
            i != itemsToCheck.end(); ++i)
        tagsToCheck |= getTags(*i);
    for (std::set<std::string>::const_iterator i = tagsToCheck.begin();
            i != tagsToCheck.end(); ++i)
        if (utils::set_contains(itemsToCheck, getItems(*i)))
            res |= *i;
#else
    // O(ntags * n[items per tag])
    for (typename std::map<std::string, std::set<std::string> >::const_iterator i = tags.begin();
            i != tags.end(); ++i)
        if (utils::set_contains(itemsToCheck, getItemsHavingTag(i->first)))
            res |= i->first;
#endif
    return res - tag;
}

std::set<std::string> Fast::getItemsExactMatch(const std::set<std::string>& tags) const
{
    std::set<std::string> res = this->getItemsHavingTags(tags);
    typename std::set<std::string>::iterator i = res.begin();
    while (i != res.end())
    {
        typename std::map<std::string, std::set<std::string> >::const_iterator t = items.find(*i);
        if (t != items.end() && t->second != tags)
        {
            typename std::set<std::string>::iterator j = i;
            ++i;
            res.erase(j);
        } else
            ++i;
    }
    return res;
}

std::string Fast::findTagWithMaxCardinality(size_t& card) const
{
    card = 0;
    std::string res = std::string();
    for (typename std::map<std::string, std::set<std::string> >::const_iterator i = tags.begin();
            i != tags.end(); ++i)
        if (i->second.size() > card)
        {
            card = i->second.size();
            res = i->first;
        }
    return res;
}

void Fast::removeTag(const std::string& tag)
{
    typename std::map<std::string, std::set<std::string> >::iterator itag = tags.find(tag);
    for (typename std::set<std::string>::const_iterator iitemset = itag->second.begin();
            iitemset != itag->second.end(); ++iitemset)
    {
        typename std::map<std::string, std::set<std::string> >::iterator iitem = items.find(*iitemset);
        iitem->second -= tag;
        if (iitem->second.empty())
            items.erase(iitem);
    }
    tags.erase(itag);
}

Fast Fast::getChildCollection(const std::string& tag) const
{
    Fast res;

    auto itag = tags.find(tag);
    for (const auto& i: itag->second)
    {
        auto iitem = items.find(i);
        res.insert(i, iitem->second);
    }

    res.removeTag(tag);
    return res;
}

void Fast::removeTagsWithCardinalityLessThan(size_t card)
{
    typename std::map<std::string, std::set<std::string> >::const_iterator i = tags.begin();
    while (i != tags.end())
    {
        if (i->second.size() < card)
        {
            typename std::map<std::string, std::set<std::string> >::const_iterator j = i;
            ++i;
            removeTag(j->first);
        } else
            ++i;
    }
}

}
}
}

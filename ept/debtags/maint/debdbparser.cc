/*
 * Parser for debian database files
 *
 * Copyright (C) 2003--2007  Enrico Zini <enrico@debian.org>
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

#include <ept/debtags/maint/debdbparser.h>
#include <map>
#include <cctype>
#include <system_error>

namespace ept {
namespace debtags {

// Eat spaces and empty lines
// Returns the number of '\n' encountered
int DebDBParser::eatSpacesAndEmptyLines()
{
    int res = 0;
    int c;
    while ((c = getc(in)) != EOF && (isblank(c) || c == '\n'))
		if (c == '\n')
		{
			isBOL = true;
			//line++;
			res++;
		} else
			isBOL = false;

    if (c == EOF)
    {
        if (ferror(in))
            throw std::system_error(errno, std::system_category(), "cannot read from " + pathname);
        isEOF = true;
    }
    else
        ungetc(c, in);

	return res;
}

// Get the ^([A-Za-z0-9]+) field name
std::string DebDBParser::getFieldName()
{
    if (! isBOL)
        throw std::runtime_error("field must start at the beginning of the line");

    std::string res;

    int c;
    while ((c = getc(in)) != EOF && (isalnum(c) || c == '-'))
        res += c;

    if (c == EOF)
    {
        if (ferror(in))
            throw std::system_error(errno, std::system_category(), "cannot read from " + pathname);
        isEOF = true;
        if (!res.empty())
            throw std::runtime_error("field is truncated at end of file.  Last line begins with: \"" + res + "\n");
    } else
        ungetc(c, in);

    return res;
}

// Eat the \s*: characters that divide the field name and the field
// data
void DebDBParser::eatFieldSep()
{
	int c;

	while ((c = getc(in)) != EOF && isblank(c))
		;

	if (c != ':')
	{
        if (c == EOF)
        {
            if (ferror(in))
                throw std::system_error(errno, std::system_category(), "cannot read from " + pathname);
            isEOF = true;
            throw std::runtime_error("field is truncated at end of file");
        } else {
            throw std::runtime_error(std::string("invalid character `") + (char)c + "' expecting `:'");
        }
	}
}

// Get the \s*(.+?)\s*\n of a body line
void DebDBParser::appendFieldBody(std::string& body)
{
	int c;

    // Skip leading spaces
    while ((c = getc(in)) != EOF && isblank(c))
        ;

    // Get the body part
    for ( ; c != EOF && c != '\n'; c = getc(in))
        body += c;

	// Delete trailing spaces
	size_t end = body.find_last_not_of(" \t");
	if (end != std::string::npos)
		body.resize(end + 1);

    if (c == EOF)
    {
        if (ferror(in))
            throw std::system_error(errno, std::system_category(), "cannot read from " + pathname);
        isEOF = true;
    }
    else
    {
        //line++;
        isBOL = true;
    }
}


DebDBParser::DebDBParser(FILE* input, const std::string& pathname)
    : in(input), pathname(pathname), isBOL(true), isEOF(false)
{
	// Go at the start of the next record
	eatSpacesAndEmptyLines();
}


// Read a record and positions itself at the start of the next one
// Returns false when there are no more records available
bool DebDBParser::nextRecord(Record& rec)
{
	if (isEOF)
		return false;

	rec.clear();

	int n;
	do {
		// Read the field name
        std::string field = getFieldName();
        std::string body;

		//fprintf(stderr, "Got field: %.*s\n", field.size(), field.data());

		// Read the colon
		eatFieldSep();

		// Read the first line of the field body
		appendFieldBody(body);
		//fprintf(stderr, "Got body: %.*s\n", body.size(), body.data());

		// Read the continuation lines of field body
		while ((n = eatSpacesAndEmptyLines()) == 0 && ! isBOL)
		{
			body += '\n';

			size_t start_size = body.size();

			appendFieldBody(body);

			// Check for dot-only lines to be changed to empty lines
			if (body.size() - start_size == 1 && body[body.size() - 1] == '.')
				body.resize(body.size() - 1);

			//fprintf(stderr, "Appended body: %.*s\n", body.size(), body.data());
		}
		//fprintf(stderr, "Trailing newlines: %d\n", n);


		rec.insert(std::pair<std::string,std::string>(field, body));
	} while (!isEOF && !n);

	return true;
}

}
}

// vim:set ts=4 sw=4:

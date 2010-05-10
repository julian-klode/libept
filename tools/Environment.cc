/*
 * Common environment for many program parts
 *
 * Copyright (C) 2003  Enrico Zini <enrico@debian.org>
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

#include "Environment.h"

#include <ept/apt/apt.h>
#include <ept/debtags/debtags.h>
#include <ept/debtags/vocabulary.h>
#include <ept/popcon/popcon.h>
#include <ept/popcon/local.h>
#include <ept/textsearch/textsearch.h>

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>	// isatty

using namespace std;

static Environment* instance = 0;

Environment& Environment::get() throw ()
{
	if (instance == 0)
		instance = new Environment;

	return *instance;
}

// Initialize the environment with default values
Environment::Environment() throw ()
	: m_apt(0), m_debtags(0), m_popcon(0), m_popconlocal(0), m_textsearch(0), _verbose(false), _debug(false) {}

void Environment::init(bool editable)
{
	m_apt = new ept::apt::Apt;
	m_debtags = new ept::debtags::Debtags(editable);
	m_vocabulary = new ept::debtags::Vocabulary;
	m_popcon = new ept::popcon::Popcon;
	m_popconlocal = new ept::popcon::Local;
	m_textsearch = new ept::textsearch::TextSearch;
}

void fatal_error(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	fprintf(stderr, "debtags: ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void warning(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void verbose(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	if (Environment::get().verbose())
	{
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

void debug(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	if (Environment::get().debug())
	{
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

void feedback(const char* fmt, ...) throw() ATTR_PRINTF(1, 2)
{
	if (isatty(1))
	{
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);
	}
}



// vim:set ts=4 sw=4:

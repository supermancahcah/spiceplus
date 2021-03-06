/*
 * SPICE+
 * Copyright (C) 2004 Andreas Unger
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <qdom.h>

#include <klocale.h>

#include "symboldirs.h"
#include "file.h"

using namespace Spiceplus;

bool SymbolDirs::load(const KURL &url)
{
    File file(url);

    if (!file.open(IO_ReadOnly))
    {
        m_errorString = file.errorString();
        return false;
    }

    QDomDocument doc;

    if (!doc.setContent(&file))
    {
        m_errorString = i18n("%1: Invalid document structure").arg(url.prettyURL());
        return false;
    }

    QDomElement root = doc.documentElement();

    if (root.tagName() != "symbol-dirs")
    {
        m_errorString = i18n("%1: Invalid symbol directory list file").arg(url.prettyURL());
        return false;
    }

    m_dirs.clear();

    for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement() || node.nodeName() != "symbol-dir")
            continue;

        QDomNode nameNode = node.firstChild();
        if (nameNode.isNull() || !nameNode.isElement() || nameNode.nodeName() != "name" || nameNode.firstChild().isNull() || !nameNode.firstChild().isText())
            continue;

        QDomNode pathNode = nameNode.nextSibling();
        if (!pathNode.isNull() && pathNode.isElement() && pathNode.nodeName() == "path" && !pathNode.firstChild().isNull() && pathNode.firstChild().isText())
        {
            Dir dir;
            dir.name = nameNode.firstChild().nodeValue();
            dir.path = pathNode.firstChild().nodeValue();
            m_dirs[node.toElement().attribute("id")] = dir;
        }
    }

    return true;
}

bool SymbolDirs::save(const KURL &url)
{
    File file(url);

    if (!file.open(IO_WriteOnly))
    {
        m_errorString = file.errorString();
        return false;
    }

    QDomDocument doc("symbol-dirs");
    QDomElement root = doc.createElement("symbol-dirs");
    doc.appendChild(root);

    for (QMap<QString, Dir>::Iterator it = m_dirs.begin(); it != m_dirs.end(); ++it)
    {
        QDomElement dirElem = doc.createElement("symbol-dir");
        dirElem.setAttribute("id", it.key());

        QDomElement nameElem = doc.createElement("name");
        nameElem.appendChild(doc.createTextNode(it.data().name));
        dirElem.appendChild(nameElem);

        QDomElement pathElem = doc.createElement("path");
        pathElem.appendChild(doc.createTextNode(it.data().path));
        dirElem.appendChild(pathElem);

        root.appendChild(dirElem);
    }

    QTextStream stream(&file);
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" << doc.toString();

    if (!file.closeWithStatus())
    {
        m_errorString = file.errorString();
        return false;
    }

    return true;
}

void SymbolDirs::setDir(const QString &id, const QString &name, const QString &path)
{
    Dir dir;
    dir.name = name;
    dir.path = path;
    m_dirs[id] = dir;
}

bool SymbolDirs::operator==(const SymbolDirs &dirs) const
{
    return m_dirs.keys() == dirs.m_dirs.keys() && m_dirs.values() == dirs.m_dirs.values();
}

bool SymbolDirs::operator!=(const SymbolDirs &dirs) const
{
    return m_dirs.keys() != dirs.m_dirs.keys() || m_dirs.values() != dirs.m_dirs.values();
}

// vim: ts=4 sw=4 et

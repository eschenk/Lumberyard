/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include <AzQtComponents/Components/StylesheetPreprocessor.h>
#include <QtCore/QObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace
{
    const char* cStylesheetVariablesKey = "StylesheetVariables";
}

namespace AzQtComponents
{
    StylesheetPreprocessor::StylesheetPreprocessor(QObject* pParent)
        : QObject(pParent)
    {
    }

    void StylesheetPreprocessor::ClearVariables()
    {
        m_namedVariables.clear();
        m_cachedColors.clear();
    }

    void StylesheetPreprocessor::ReadVariables(const QString& jsonString)
    {
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toLatin1());
        QJsonObject rootObject = doc.object();

        //load in the stylesheet variables
        if (rootObject.contains(cStylesheetVariablesKey))
        {
            QJsonObject variablesObject = rootObject.value(cStylesheetVariablesKey).toObject();
            for (const QString& key : variablesObject.keys())
            {
                m_namedVariables[key] = variablesObject[key].toString();

                // clear any cached colors of the same key, so that they get recached on next fetch
                m_cachedColors.remove(key);
            }
        }
    }

    QString StylesheetPreprocessor::ProcessStyleSheet(const QString& stylesheetData)
    {
        enum class ParseState
        {
            Normal, Variable, Done
        };

        ParseState state = ParseState::Normal;
        QString out;
        QString varName;

        auto i = stylesheetData.cbegin();
        while (state != ParseState::Done && i != stylesheetData.end())
        {
            while (state == ParseState::Normal && i != stylesheetData.end())
            {
                char c = i->toLatin1();
                switch (c)
                {
                case '@':
                    i++;
                    state = ParseState::Variable;
                    break;
                default:
                    out.append(*i);
                    i++;
                }
                ;
            }

            while (state == ParseState::Variable && i != stylesheetData.end())
            {
                char c = i->toLatin1();

                //All characters valid in identifier
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    varName.append(*i);
                    i++;
                }
                else
                {
                    //We are finished with reading the current varName
                    out.append(GetValueByName(varName));
                    varName.clear();
                    out.append(*i);
                    i++;
                    state = ParseState::Normal;
                    break;
                }
            }
        }

        return out;
    }


    QString StylesheetPreprocessor::GetValueByName(const QString& name)
    {
        if (m_namedVariables.contains(name))
        {
            return m_namedVariables.value(name);
        }
        else
        {
            return QString("");
        }
    }

    const QColor& StylesheetPreprocessor::GetColorByName(const QString& name)
    {
        if (m_cachedColors.contains(name))
        {
            return m_cachedColors[name];
        }

        if (m_namedVariables.contains(name))
        {
            QColor color;

            color.setNamedColor(m_namedVariables.value(name));

            m_cachedColors[name] = color;

            return m_cachedColors[name];
        }

        static QColor defaultColor;
        return defaultColor;
    }

#include <Components/StylesheetPreprocessor.moc>
} // namespace AzQtComponents
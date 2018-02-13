/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "highlighter.h"
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QResource>

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    QFile file(":/resources/files/keyWords.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "Error reading file KeyWords";

    QTextStream textStream(&file);
    while (!textStream.atEnd())
        keywordPatterns << "\\b" + textStream.readLine() + "\\b";
    file.close();

    keywordFormat.setForeground(QColor("#a626a4"));
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    quotationFormat.setForeground(QColor("#50a14f"));
    rule.pattern = QRegularExpression("(\".*\"|\'.*\'|\\<.*\\>)");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setForeground(QColor("#4078f2"));
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_.]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("#[A-Za-z]+");
    highlightingRules.append(rule);

    variableFormat.setForeground(QColor("#986801"));
    rule.format = variableFormat;
    rule.pattern = QRegularExpression("\\b(0[xX][0-9a-fA-F]+|(\\d+)(\\d+(\\.\\d{1,2})?)|0[b][0-1]+|[0-9]+.[0-9]+|[0-9]+)\\b");
    highlightingRules.append(rule);

    QFile fileSD(":/resources/files/SDWords.txt");
    if (!fileSD.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "Error reading file SDWords";

    keywordPatterns.clear();
    textStream.setDevice(&fileSD);
    while (!textStream.atEnd()){
        keywordPatterns << "\\b" + textStream.readLine() + "\\b";
    }
    fileSD.close();

    sdwordFormat.setForeground(Qt::darkYellow);

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = sdwordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::darkGray);
    singleLineCommentFormat.setFontItalic(true);

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGray);
    multiLineCommentFormat.setFontItalic(true);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

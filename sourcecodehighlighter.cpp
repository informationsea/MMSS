#include "sourcecodehighlighter.h"

#include <QBrush>
#include <QColor>
#include <QFont>

SourceCodeHighlighter::SourceCodeHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    m_keywordFormat.setForeground(QBrush(QColor("#0014AA")));
    m_keywordFormat.setFontWeight(QFont::Bold);
    m_quoteFormat.setForeground(QBrush(QColor("#00AA00")));
    m_macroFormat.setForeground(QBrush(QColor("#30006F")));
    m_commentFormat.setForeground(QBrush(QColor("#39A2A2")));

    m_keywordList << "alignas " << "alignof " << "and" << "and_eq" << "asm" << "auto"
                  << "bitand" << "bitor" << "bool" << "break" << "case" << "catch"
                  << "char" << "char16_t" << "char32_t" << "class" << "compl" << "const"
                  << "constexpr" << "const_cast" << "continue" << "decltype" << "default"
                  << "delete" << "do" << "double" << "dynamic_cast" << "else" << "enum"
                  << "explicit" << "export" << "extern" << "false" << "float" << "for"
                  << "friend" << "goto" << "if" << "inline" << "int" << "long" << "mutable"
                  << "namespace" << "new" << "noexcept" << "not" << "not_eq" << "nullptr"
                  << "operator" << "or" << "or_eq" << "private" << "protected" << "public"
                  << "register" << "reinterpret_cast" << "return" << "short" << "signed"
                  << "sizeof" << "static" << "static_assert" << "static_cast" << "struct"
                  << "switch" << "template" << "this" << "thread_local" << "throw" << "true"
                  << "try" << "typedef" << "typeid" << "typename" << "union" << "unsigned"
                  << "using" << "virtual" << "void" << "volatile" << "wchar_t" << "while"
                  << "xor" << "xor_eq" << "signals" << "slots" << "foreach";
}

void SourceCodeHighlighter::highlightBlock(const QString &text)
{
    highlightBlockHelper(text, m_keywordList, m_keywordFormat);

    highlightBlockHelper(text, QRegExp("#.*$"), m_macroFormat);
    highlightBlockHelper(text, QRegExp("\"[^\"]*\""), m_quoteFormat);
    highlightBlockHelper(text, QRegExp("//.*$"), m_commentFormat);
    //highlightBlockHelper(text, QRegExp("<[^>]*>"), m_quoteFormat);
}

static bool isPartOfName(QChar ch)
{
    if (ch.isLetterOrNumber())
        return true;
    if (ch == QChar('_'))
        return true;
    return false;
}

QStringList SourceCodeHighlighter::highlightBlockHelper(const QString &text, const QStringList &keys, const QTextCharFormat &format)
{
    QStringList found;
    foreach(QString one, keys) {
        int pos = -1;
        while ((pos = text.indexOf(one, pos+1, Qt::CaseSensitive)) >= 0) {
            if ((pos == 0 || !isPartOfName(text.at(pos-1))) &&
                    (pos+one.length() >= text.length() || !isPartOfName(text.at(pos+one.length())))) {
                setFormat(pos, one.length(), format);

                if (!found.contains(one))
                    found << one;
            }
        }
    }
    return found;
}

void SourceCodeHighlighter::highlightBlockHelper(const QString &text, QRegExp exp, const QTextCharFormat &format)
{
    int pos = -1;
    while ((pos = text.indexOf(exp, pos+1)) >= 0) {
        setFormat(pos, exp.matchedLength(), format);
        pos += exp.matchedLength();
    }
}

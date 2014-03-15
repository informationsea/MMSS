#ifndef SOURCECODEHIGHLIGHTER_H
#define SOURCECODEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QStringList>

class SourceCodeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SourceCodeHighlighter(QTextDocument *parent = 0);

signals:

protected:
    virtual void highlightBlock(const QString & text);

public slots:

private:
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_quoteFormat;
    QTextCharFormat m_macroFormat;
    QTextCharFormat m_commentFormat;

    QStringList m_keywordList;

    QStringList highlightBlockHelper (const QString & text, const QStringList & keys, const QTextCharFormat & format);
    void highlightBlockHelper(const QString &text, QRegExp exp, const QTextCharFormat &format);
};

#endif // SOURCECODEHIGHLIGHTER_H

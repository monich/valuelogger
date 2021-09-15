/*
Copyright (c) 2021 Slava Monich <slava@monich.com>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef COLORMODEL_H
#define COLORMODEL_H

#include <QList>
#include <QColor>
#include <QAbstractListModel>

class ColorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList colors READ getColors WRITE setColors NOTIFY colorsChanged)
    Q_PROPERTY(int dragPos READ getDragPos WRITE setDragPos NOTIFY dragPosChanged)
    Q_ENUMS(ItemType)

public:
    enum ItemType {
        ColorItem,
        TrashedItem,
        AddItem
    };

    ColorModel(QObject* parent = Q_NULLPTR);

    QStringList getColors() const;
    void setColors(QStringList colors);

    int getDragPos() const;
    void setDragPos(int pos);

    Q_INVOKABLE void addColor(QColor color);
    Q_INVOKABLE int indexOf(QColor color) const;

    // QAbstractListModel
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void colorsChanged();
    void dragPosChanged();

private:
    int m_dragPos;
    int m_dragStartPos;
    QList<QColor> m_colors;
    const QColor m_addColor;
};

#endif // COLORMODEL_H

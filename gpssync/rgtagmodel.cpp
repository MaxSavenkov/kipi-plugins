/* ============================================================
 *
 * Date        : 2010-03-21
 * Description : A model to hold information about image tags
 *
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rgtagmodel.moc"

//local includes
#include "rgtagmodel.h"
//KDE includes
#include "kdebug.h"
#include "klocale.h"

//Qt includes
#include <QtGui>

namespace KIPIGPSSyncPlugin
{

class RGTagModelPrivate
{
public:
    RGTagModelPrivate()
    : tagModel(0),
      rootTag(0),
      newTags(),
      auxTagList(),
      auxIndexList(),
      savedSpacerList()
    {
    }

    QAbstractItemModel*          tagModel;
    TreeBranch*                  rootTag;

    QModelIndex                  parent;
    int                          startInsert, endInsert;
    int                          startRemove, endRemove;

    QList<QList<TagData> >       newTags;

    QStringList                  auxTagList;
    QList<Type>                  auxTagTypeList;
    QList<QPersistentModelIndex> auxIndexList;

    QList<QList<TagData> >       savedSpacerList;
};

RGTagModel::RGTagModel(QAbstractItemModel* const externalTagModel, QObject* const parent)
          : QAbstractItemModel(parent), d(new RGTagModelPrivate)
{
    d->tagModel      = externalTagModel;
    d->rootTag       = new TreeBranch();
    d->rootTag->type = TypeChild;

    i18n("{Country}");
    i18n("{State}");
    i18n("{County}");
    i18n("{City}");
    i18n("{Town}");
    i18n("{Village}");
    i18n("{Hamlet}");
    i18n("{Street}");

    connect(d->tagModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotSourceDataChanged(const QModelIndex&, const QModelIndex&)));

    connect(d->tagModel, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
            this, SLOT(slotSourceHeaderDataChanged(Qt::Orientation, int, int)));
    connect(d->tagModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex&, int, int)),
            this, SLOT(slotColumnsAboutToBeInserted(const QModelIndex&, int, int)));
    connect(d->tagModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex&, int,int,const QModelIndex&, int)),
            this, SLOT(slotColumnsAboutToBeMoved(const QModelIndex&,int,int,const QModelIndex&, int)));
    connect(d->tagModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotColumnsAboutToBeRemoved(const QModelIndex&, int, int)));
    connect(d->tagModel, SIGNAL(columnsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotColumnsInserted()));
    connect(d->tagModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(slotColumnsMoved()));
    connect(d->tagModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
            this, SLOT(slotColumnsRemoved()));
    connect(d->tagModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotLayoutAboutToBeChanged()));
    connect(d->tagModel, SIGNAL(layoutChanged()),
            this, SLOT(slotLayoutChanged()));
    connect(d->tagModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotModelAboutToBeReset()));
    connect(d->tagModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsAboutToBeInserted(const QModelIndex&, int, int)));

    connect(d->tagModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex&, int,int,const QModelIndex&, int)),
            this, SLOT(slotRowsAboutToBeMoved(const QModelIndex&,int,int,const QModelIndex&, int)));
    connect(d->tagModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    connect(d->tagModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsInserted()));
    connect(d->tagModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(slotRowsMoved()));
    connect(d->tagModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(slotRowsRemoved()));
}

RGTagModel::~RGTagModel()
{
    delete d->rootTag;
    delete d;
}

TreeBranch* RGTagModel::branchFromIndex(const QModelIndex& index) const
{
    return index.isValid() ? static_cast<TreeBranch*>(index.internalPointer()) : d->rootTag;
}

void checkTree(TreeBranch* const checkBranch, int level)
{
    if(!checkBranch->sourceIndex.isValid())
        return;


    for(int j = 0; j < checkBranch->oldChildren.count(); ++j)
    {
        checkTree(checkBranch->oldChildren[j], level+1);
    }

    for(int j = 0; j < checkBranch->spacerChildren.count(); ++j)
    {
        checkTree(checkBranch->spacerChildren[j], level+1);
    }
}

QModelIndex RGTagModel::fromSourceIndex(const QModelIndex& externalTagModelIndex) const
{
    if(!externalTagModelIndex.isValid())
        return QModelIndex();

    Q_ASSERT(externalTagModelIndex.model()==d->tagModel);

    QList<QModelIndex> parents;
    QModelIndex myIndex = externalTagModelIndex;
    parents<<myIndex;
    while(myIndex.parent().isValid())
    {
        myIndex = myIndex.parent();
        parents.prepend(myIndex);
    }

    TreeBranch* subModelBranch = d->rootTag;

    int level = 0;
    while(level <= parents.count())
    {

        if(subModelBranch->sourceIndex == externalTagModelIndex)
        {
            return createIndex(subModelBranch->sourceIndex.row()+subModelBranch->parent->spacerChildren.count()+subModelBranch->parent->newChildren.count(), subModelBranch->sourceIndex.column(), subModelBranch);

        }

        int where = -1;
        for (int i=0; i < subModelBranch->oldChildren.count(); ++i)
        {
            if(subModelBranch->oldChildren[i]->sourceIndex == parents[level])
            {
                where = i;
                break;
            }
        }

        if(where >= 0)
        {
            subModelBranch = subModelBranch->oldChildren[where];
        }
        else
        {
            if (level>=parents.count())
                return QModelIndex();

            //TODO: check when rows are different
            TreeBranch* newTreeBranch = new TreeBranch();
            newTreeBranch->sourceIndex = parents[level];
            newTreeBranch->data = d->tagModel->data(externalTagModelIndex, Qt::DisplayRole).toString();
            newTreeBranch->parent = subModelBranch;
            newTreeBranch->type = TypeChild;

            subModelBranch->oldChildren.append(newTreeBranch);
            subModelBranch = newTreeBranch;
        }
        level++;
    }

    //no index is found
    return QModelIndex();
}

QModelIndex RGTagModel::toSourceIndex(const QModelIndex& tagModelIndex) const
{
    if(!tagModelIndex.isValid())
        return QModelIndex();

    Q_ASSERT(tagModelIndex.model()==this);

    TreeBranch* const treeBranch = branchFromIndex(tagModelIndex);  //static_cast<TreeBranch*>(tagModelIndex.internalPointer());
    if(!treeBranch)
        return QModelIndex();

    return treeBranch->sourceIndex;
}

void RGTagModel::addSpacerTag(const QModelIndex& parent, const QString& spacerName)
{
    //TreeBranch* const parentBranch = parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;
    TreeBranch* const parentBranch = branchFromIndex(parent);

    bool found = false;
    if(!parentBranch->spacerChildren.empty())
    {
        for( int i=0; i < parentBranch->spacerChildren.count(); ++i)
        {
            if(parentBranch->spacerChildren[i]->data == spacerName)
            {
                found = true;
                break;
            }
        }
    }

    if(!found)
    {
        TreeBranch* newSpacer = new TreeBranch();
        newSpacer->parent = parentBranch;
        newSpacer->data = spacerName;
        newSpacer->type = TypeSpacer;

        beginInsertRows(parent, parentBranch->spacerChildren.count(), parentBranch->spacerChildren.count());
        parentBranch->spacerChildren.append(newSpacer);
        endInsertRows();
    }
}

QPersistentModelIndex RGTagModel::addNewTag(const QModelIndex& parent, const QString& newTagName)
{

    TreeBranch* const parentBranch = branchFromIndex(parent);  //parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;

    bool found = false;
    if(!parentBranch->newChildren.empty())
    {
        for( int i=0; i<parentBranch->newChildren.count(); ++i)
        {
            if(parentBranch->newChildren[i]->data == newTagName)
            {
                found = true;
                return createIndex(parentBranch->spacerChildren.count()+i,0,parentBranch->newChildren[i]);
                break;
            }
        }
    }

    if(!found)
    {

        TreeBranch* newTagChild = new TreeBranch();
        newTagChild->parent = parentBranch;
        newTagChild->data = newTagName;
        newTagChild->type = TypeNewChild;

        beginInsertRows(parent, parentBranch->spacerChildren.count()+parentBranch->newChildren.count(), parentBranch->spacerChildren.count()+parentBranch->newChildren.count());
        parentBranch->newChildren.append(newTagChild);
        endInsertRows();

        return createIndex(parentBranch->spacerChildren.count()+parentBranch->newChildren.count()-1, 0, parentBranch->newChildren.last());
    }

    return QPersistentModelIndex();
}

QList<TagData> RGTagModel::getTagAddress()
{
    QList<TagData> tagAddress;
    for(int i=0; i<d->auxTagList.count(); i++)
    {
        TagData tagData;
        tagData.tagName = d->auxTagList[i];
        tagData.tagType = d->auxTagTypeList[i];
        tagAddress.append(tagData);
    }
    return tagAddress;
}

void RGTagModel::addDataInTree(TreeBranch* currentBranch, int currentRow,const QStringList& addressElements,const QStringList& elementsData)
{
    bool newDataAdded;

    for(int i=0; i<currentBranch->spacerChildren.count(); ++i)
    {
        newDataAdded = false;

        //this spacer is not an address element
        if(currentBranch->spacerChildren[i]->data.indexOf("{") != 0)
        {
            d->auxTagList.append(currentBranch->spacerChildren[i]->data);
            d->auxTagTypeList.append(TypeSpacer);
            addDataInTree(currentBranch->spacerChildren[i], i, addressElements, elementsData);
            d->auxTagList.removeLast();
            d->auxTagTypeList.removeLast();
        }

        else
        {

            for( int j=0; j<addressElements.count(); ++j)
            {
                if(currentBranch->spacerChildren[i]->data == addressElements[j])
                {
                    newDataAdded = true;

                    QModelIndex currentIndex;
                    if(currentBranch == d->rootTag)
                        currentIndex = QModelIndex();
                    else
                        currentIndex = createIndex(currentRow, 0, currentBranch);

                    //checks if adds the new tag as a sibling to a spacer, or as a child of a new tag
                    QPersistentModelIndex auxIndex;
                    if((currentBranch->type != TypeSpacer) || (((currentBranch->type == TypeSpacer) && (currentBranch->data.indexOf("{") != 0))) || (d->auxIndexList.isEmpty()))
                    {
                        auxIndex = addNewTag(currentIndex, elementsData[j]);
                    }
                    else
                    {
                        auxIndex = addNewTag(d->auxIndexList.last(), elementsData[j]);
                    }

                    d->auxTagList.append(elementsData[j]);
                    d->auxTagTypeList.append(TypeNewChild);
                    d->auxIndexList.append(auxIndex);
                    QList<TagData> newTag=getTagAddress();
                    d->newTags.append(newTag);
                }
            }

            if(currentBranch->spacerChildren[i])
                addDataInTree(currentBranch->spacerChildren[i],i, addressElements, elementsData);
            if(newDataAdded)
            {
                d->auxTagList.removeLast();
                d->auxTagTypeList.removeLast();
                d->auxIndexList.removeLast();
            }

        }
    }

    for(int i=0; i<currentBranch->newChildren.count(); ++i)
    {
        d->auxTagList.append(currentBranch->newChildren[i]->data);
        d->auxTagTypeList.append(TypeNewChild);
        addDataInTree(currentBranch->newChildren[i],i+currentBranch->spacerChildren.count(), addressElements, elementsData);
        d->auxTagList.removeLast();
        d->auxTagTypeList.removeLast();
    }

    for(int i=0; i<currentBranch->oldChildren.count(); ++i)
    {
        d->auxTagList.append(currentBranch->oldChildren[i]->data);
        d->auxTagTypeList.append(TypeChild);
        addDataInTree(currentBranch->oldChildren[i],i+currentBranch->spacerChildren.count()+currentBranch->newChildren.count(), addressElements, elementsData);
        d->auxTagList.removeLast();
        d->auxTagTypeList.removeLast();
    }
}

QList<QList<TagData> > RGTagModel::addNewData(QStringList& elements, QStringList& resultedData)
{
    d->newTags.clear();

    //elements contains address elements {Country}, {City}, ...
    //resultedData contains RG data (example Spain,Barcelona)
    addDataInTree(d->rootTag, 0, elements, resultedData);

    return d->newTags;
}

int RGTagModel::columnCount(const QModelIndex& parent) const
{
    TreeBranch* const parentBranch = branchFromIndex(parent); //static_cast<TreeBranch*>(parent.internalPointer());
    if(!parentBranch)
    {
        return 1;
    }

    if(parentBranch && parentBranch->type == TypeSpacer)
        return 1;
    else if(parentBranch && parentBranch->type == TypeNewChild)
        return 1;

    return d->tagModel->columnCount(toSourceIndex(parent));
}

bool RGTagModel::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

QVariant RGTagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeBranch* const treeBranch = branchFromIndex(index);  //static_cast<TreeBranch*>(index.internalPointer());
    if ((!treeBranch) || (treeBranch->type == TypeChild))
    {
        return d->tagModel->data(toSourceIndex(index), role);
    }
    else if((treeBranch->type == TypeSpacer) && (role == Qt::DisplayRole))
    {
        return i18n(treeBranch->data.toUtf8());
    }
    else if((treeBranch->type == TypeSpacer) && (role == Qt::ForegroundRole))
    {
        return QColor(Qt::red);
    }
    else if((treeBranch->type == TypeNewChild) && (role == Qt::DisplayRole))
    {
        return treeBranch->data;
    }
    else if((treeBranch->type == TypeNewChild) && (role == Qt::ForegroundRole))
    {
        return QColor(Qt::green);
    }

    return QVariant();
}

QModelIndex RGTagModel::index(int row, int column, const QModelIndex& parent) const
{
    if ( (column!=0) || (row<0) )
        return QModelIndex();

    TreeBranch* const parentBranch = branchFromIndex(parent);//d->rootTag;
  //  if (parent.isValid())
  //      parentBranch = static_cast<TreeBranch*>(parent.internalPointer());

    // this should not happen!
    if (!parentBranch)
        return QModelIndex();

    if (row < parentBranch->spacerChildren.count())
    {
        return createIndex(row, column, parentBranch->spacerChildren[row]);
    }
    else if(row >= parentBranch->spacerChildren.count() && row < (parentBranch->newChildren.count() + parentBranch->spacerChildren.count()))
    {
        return createIndex(row, column, parentBranch->newChildren[row-parentBranch->spacerChildren.count()]);
    }
    else
    {
        return fromSourceIndex(d->tagModel->index(row-parentBranch->spacerChildren.count()-parentBranch->newChildren.count(),column,toSourceIndex(parent)));
    }

    return QModelIndex();
}

QModelIndex RGTagModel::parent(const QModelIndex& index) const
{
    TreeBranch* const currentBranch = branchFromIndex(index);  // static_cast<TreeBranch*>(index.internalPointer());
    if (!currentBranch)
        return QModelIndex();

    if(currentBranch->type == TypeSpacer || currentBranch->type == TypeNewChild)
    {
        TreeBranch* const parentBranch = currentBranch->parent;

        if (!parentBranch)
            return QModelIndex();

        TreeBranch* const gParentBranch = parentBranch->parent;
        if(!gParentBranch)
            return QModelIndex();

        if (parentBranch->type==TypeSpacer)
        {
            for (int parentRow=0; parentRow<gParentBranch->spacerChildren.count(); ++parentRow)
            {
                if(gParentBranch->spacerChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow, 0, parentBranch);
                }
            }

            return QModelIndex();
        }
        else if (parentBranch->type == TypeNewChild)
        {
            for (int parentRow=0; parentRow<gParentBranch->newChildren.count(); ++parentRow)
            {
                if(gParentBranch->newChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow+gParentBranch->spacerChildren.count(), 0, parentBranch);
                }
            }
        }
        else if (parentBranch->type==TypeChild)
        {
            // TODO: don't we have a function for this?
            for (int parentRow=0; parentRow<gParentBranch->oldChildren.count(); ++parentRow)
            {
                if(gParentBranch->oldChildren[parentRow] == parentBranch)
                {
                    return createIndex(parentRow+gParentBranch->spacerChildren.count()+gParentBranch->newChildren.count(), 0, parentBranch);
                }
            }

            return QModelIndex();
        }
    }

    return fromSourceIndex(d->tagModel->parent(toSourceIndex(index)));
}

int RGTagModel::rowCount(const QModelIndex& parent) const
{
    TreeBranch* const parentBranch = branchFromIndex(parent);  //parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;

    int myRowCount = parentBranch->spacerChildren.count() + parentBranch->newChildren.count();

    // TODO: we don't know whether the oldChildren have been set up, therefore query the source model
    if (parentBranch->type==TypeChild)
    {
        const QModelIndex sourceIndex = toSourceIndex(parent);
        myRowCount+=d->tagModel->rowCount(sourceIndex);
    }

    return myRowCount;
}


bool RGTagModel::setHeaderData(int /*section*/, Qt::Orientation /*orientation*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

QVariant RGTagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->tagModel->headerData(section, orientation, role);
}

Qt::ItemFlags RGTagModel::flags(const QModelIndex& index) const
{
    TreeBranch* const currentBranch = branchFromIndex(index);  // static_cast<TreeBranch*>(index.internalPointer());
    if (currentBranch && ((currentBranch->type == TypeSpacer) || (currentBranch->type == TypeNewChild)) )
    {
        return QAbstractItemModel::flags(index);
    }

    return d->tagModel->flags(toSourceIndex(index));
}

void RGTagModel::slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    emit dataChanged(fromSourceIndex(topLeft),fromSourceIndex(bottomRight));

}

void RGTagModel::slotSourceHeaderDataChanged(const Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation, first, last);
}

void RGTagModel::slotColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    //TODO: Should we do something here?
    beginInsertColumns(fromSourceIndex(parent), start, end);
}

void RGTagModel::slotColumnsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationColumn)
{
    beginMoveColumns(fromSourceIndex(sourceParent), sourceStart, sourceEnd, fromSourceIndex(destinationParent), destinationColumn );
}

void RGTagModel::slotColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end )
{
    beginRemoveColumns(fromSourceIndex(parent), start, end);
}

void RGTagModel::slotColumnsInserted()
{
    endInsertColumns();
}

void RGTagModel::slotColumnsMoved()
{
    endMoveColumns();
}

void RGTagModel::slotColumnsRemoved()
{
    endRemoveColumns();
}

void RGTagModel::slotLayoutAboutToBeChanged()
{
    emit layoutAboutToBeChanged();
}

void RGTagModel::slotLayoutChanged()
{
    emit layoutChanged();
}

void RGTagModel::slotModelAboutToBeReset()
{
    beginResetModel();
}

void RGTagModel::slotModelReset()
{
    reset();
}

void RGTagModel::slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end )
{
    TreeBranch* const parentBranch = parent.isValid() ? static_cast<TreeBranch*>(fromSourceIndex(parent).internalPointer()) : d->rootTag;

    d->parent      = fromSourceIndex(parent);
    d->startInsert = start;
    d->endInsert   = end;

    beginInsertRows(d->parent, start+parentBranch->newChildren.count()+parentBranch->spacerChildren.count(), end+parentBranch->newChildren.count()+parentBranch->spacerChildren.count());
}

void RGTagModel::slotRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow)
{
    beginMoveRows(fromSourceIndex(sourceParent), sourceStart, sourceEnd, fromSourceIndex(destinationParent), destinationRow );
}

void RGTagModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void RGTagModel::slotRowsInserted()
{
   TreeBranch* const parentBranch = d->parent.isValid() ? static_cast<TreeBranch*>(d->parent.internalPointer()) : d->rootTag;

    for(int i=d->startInsert; i<d->endInsert; ++i)
    {
        TreeBranch* newBranch = new TreeBranch();
        newBranch->parent = parentBranch;
        newBranch->sourceIndex = d->tagModel->index(i, 0, d->parent);
        newBranch->type = TypeChild;

        parentBranch->oldChildren.insert(i, newBranch);
    }

    endInsertRows();

    d->parent = QModelIndex();
    d->startInsert = -1;
    d->endInsert = -1;
}

void RGTagModel::slotRowsMoved()
{
    endMoveRows();
}

void RGTagModel::slotRowsRemoved()
{
}

void RGTagModel::deleteTag(const QModelIndex& currentIndex)
{
    if(!currentIndex.isValid())
        return;

    QModelIndex parentIndex              = currentIndex.parent();
    int currentRow                       = currentIndex.row();
    TreeBranch* const parentBranch       =  branchFromIndex(parentIndex);  //parentIndex.isValid() ? static_cast<TreeBranch*>(parentIndex.internalPointer()) : d->rootTag;
    TreeBranch* const currentChildBranch = branchFromIndex(currentIndex);  //index.isValid() ? static_cast<TreeBranch*>(index.internalPointer()) : d->rootTag;

    if(currentChildBranch->type == TypeChild)
        return;

    if(currentChildBranch->spacerChildren.count() > 0 || currentChildBranch->newChildren.count() > 0)
    {
        beginMoveRows(currentIndex,0,currentChildBranch->spacerChildren.count()-1,parentIndex, parentBranch->spacerChildren.count());

        for(int j=0; j<currentChildBranch->spacerChildren.count(); ++j)
        {
            parentBranch->spacerChildren.append(currentChildBranch->spacerChildren[j]);
            parentBranch->spacerChildren.last()->parent = parentBranch;

            QModelIndex testIndex = createIndex(parentBranch->spacerChildren.count()-1,0, parentBranch->spacerChildren.last());

        }
        currentChildBranch->spacerChildren.clear();
        endMoveRows();

        beginMoveRows(currentIndex, currentChildBranch->spacerChildren.count(), currentChildBranch->spacerChildren.count()+currentChildBranch->newChildren.count()-1, parentIndex, parentBranch->spacerChildren.count()+parentBranch->newChildren.count());

        for(int j=currentChildBranch->spacerChildren.count(); j<currentChildBranch->spacerChildren.count()+currentChildBranch->newChildren.count(); ++j)
        {
            parentBranch->newChildren.append(currentChildBranch->newChildren[j-currentChildBranch->spacerChildren.count()]);
            parentBranch->newChildren.last()->parent = parentBranch;
        }
        currentChildBranch->newChildren.clear();
        endMoveRows();
    }

    beginRemoveRows(parentIndex, currentRow, currentRow);
    //TODO: is it good here?
    if(currentRow < parentBranch->spacerChildren.count())
        parentBranch->spacerChildren.removeAt(currentRow);
    else if(currentRow >= parentBranch->spacerChildren.count())
        parentBranch->newChildren.removeAt(currentRow - parentBranch->spacerChildren.count());

    endRemoveRows();
}

void RGTagModel::findAndDeleteSpacersOrNewTags( TreeBranch* currentBranch, int currentRow, Type whatShouldRemove)
{
    QModelIndex currentIndex = createIndex(currentRow, 0, currentBranch);

    for(int i=0; i<currentBranch->spacerChildren.count(); ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->spacerChildren[i], i, whatShouldRemove);
        if(whatShouldRemove == TypeSpacer)
        {
            QModelIndex spacerIndex = createIndex(i,0,currentBranch->spacerChildren[i]);
            deleteTag(spacerIndex);
            i--;
        }
    }
    for(int i=0; i<currentBranch->newChildren.count(); ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->newChildren[i], i+currentBranch->spacerChildren.count(), whatShouldRemove);
        if(whatShouldRemove == TypeNewChild)
        {
            QModelIndex newTagIndex = createIndex(i+currentBranch->spacerChildren.count(),0,currentBranch->newChildren[i]);
            deleteTag(newTagIndex);
            i--;
        }

    }
    for(int i=0; i<currentBranch->oldChildren.count(); ++i)
    {
        findAndDeleteSpacersOrNewTags(currentBranch->oldChildren[i], i+currentBranch->spacerChildren.count()+currentBranch->newChildren.count(), whatShouldRemove);
    }
}

void RGTagModel::deleteAllSpacersOrNewTags(const QModelIndex& currentIndex, Type whatShouldRemove)
{
    if(whatShouldRemove == TypeSpacer)
    {
        TreeBranch* currentBranch = branchFromIndex(currentIndex);  //currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;
        findAndDeleteSpacersOrNewTags(currentBranch, 0, whatShouldRemove);
    }
    else if(whatShouldRemove == TypeNewChild)
    {
        findAndDeleteSpacersOrNewTags(d->rootTag,0, whatShouldRemove);
    }
}

//tagAddressElements contains address tag: Places,Spain,Barcelona
//readdTag climbs the tree and checks on each level if tagAddressElements[level] is found.
//if the tag is found, it climbs up the next level
//else, it recreates the new tag and climbs up that tree.
void RGTagModel::readdTag(TreeBranch*& currentBranch, int currentRow,const QList<TagData> tagAddressElements, int currentAddressElementIndex)
{

    bool found=false;
    int foundIndex;

    if(currentAddressElementIndex >= tagAddressElements.count())
        return;

    if(tagAddressElements[currentAddressElementIndex].tagType == TypeSpacer)
    {

        for(int i=0; i<currentBranch->spacerChildren.count(); ++i)
        {
            if(currentBranch->spacerChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found = true;
                foundIndex = i;
                break;
            }
        }

        if(found)
        {

            readdTag(currentBranch->spacerChildren[foundIndex], foundIndex, tagAddressElements, currentAddressElementIndex+1);
            return;
        }
        else
        {
        //recreates the spacer
            QModelIndex currentIndex;
            if(currentBranch == d->rootTag)
                currentIndex = QModelIndex();
            else
                currentIndex = createIndex(currentRow, 0, currentBranch);

            addSpacerTag(currentIndex,tagAddressElements[currentAddressElementIndex].tagName);

            if( (tagAddressElements.count()-1) > currentAddressElementIndex)
                readdTag(currentBranch->spacerChildren[currentBranch->spacerChildren.count()-1], currentBranch->spacerChildren.count()-1, tagAddressElements, currentAddressElementIndex+1);

        }

    }
    else if(tagAddressElements[currentAddressElementIndex].tagType == TypeNewChild)
    {
        for(int i=0; i<currentBranch->newChildren.count(); ++i)
        {
            if(currentBranch->newChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found = true;
                foundIndex = i;
                break;
            }
        }

        if(found)
        {
            readdTag(currentBranch->newChildren[foundIndex], foundIndex+currentBranch->spacerChildren.count(),tagAddressElements, currentAddressElementIndex+1);
            return;
        }

        if(!found)
        {
            QModelIndex currentIndex;
    	    if(currentBranch == d->rootTag)
        	currentIndex = QModelIndex();
    	    else
        	currentIndex = createIndex(currentRow, 0, currentBranch);

            addNewTag(currentIndex,tagAddressElements[currentAddressElementIndex].tagName);

	    if( (tagAddressElements.count()-1) > currentAddressElementIndex)
    	        readdTag(currentBranch->newChildren[currentBranch->newChildren.count()-1], currentBranch->spacerChildren.count()+currentBranch->newChildren.count()-1, tagAddressElements, currentAddressElementIndex+1);
        }
    }
    else if(tagAddressElements[currentAddressElementIndex].tagType == TypeChild)
    {
        bool found = false;
        for(int i=0; i<currentBranch->oldChildren.count(); ++i)
        {
            if(currentBranch->oldChildren[i]->data == tagAddressElements[currentAddressElementIndex].tagName)
            {
                found = true;
                foundIndex = i;
                break;
            }
        }

        if(found)
        {
            readdTag(currentBranch->oldChildren[foundIndex], foundIndex+currentBranch->spacerChildren.count()+currentBranch->newChildren.count(), tagAddressElements, currentAddressElementIndex+1);
            return;
        }
        else
        {
            QModelIndex currentIndex;
            if(currentBranch == d->rootTag)
                currentIndex = QModelIndex();
            else
                currentIndex = createIndex(currentRow, 0, currentBranch);

            addSpacerTag(currentIndex,tagAddressElements[currentAddressElementIndex].tagName);

            if( (tagAddressElements.count()-1) > currentAddressElementIndex)
                readdTag(currentBranch->spacerChildren[currentBranch->spacerChildren.count()-1], currentBranch->spacerChildren.count()-1, tagAddressElements, currentAddressElementIndex+1);
        }
    }
}

void RGTagModel::readdNewTags(const QList<QList<TagData> >& tagAddressList)
{
    for(int i=0; i<tagAddressList.count(); ++i)
    {
        QList<TagData> currentAddressTag = tagAddressList[i];
        readdTag(d->rootTag, 0, currentAddressTag, 0);
    }
}

QList<TagData> RGTagModel::getSpacerAddress(TreeBranch* currentBranch)
{
    QList<TagData> spacerAddress;

    while(currentBranch->parent != NULL)
    {
        TagData currentTag;
        currentTag.tagName = currentBranch->data;
        currentTag.tagType = currentBranch->type;

        spacerAddress.prepend(currentTag);
        currentBranch = currentBranch->parent;
    }

    return spacerAddress;
}

void RGTagModel::climbTreeAndGetSpacers(const TreeBranch* currentBranch)
{
    for(int i=0; i<currentBranch->spacerChildren.count(); i++)
    {
        QList<TagData> currentSpacerAddress;
        currentSpacerAddress = getSpacerAddress(currentBranch->spacerChildren[i]);
        d->savedSpacerList.append(currentSpacerAddress);
        climbTreeAndGetSpacers(currentBranch->spacerChildren[i]);
    }

    for(int i=0; i<currentBranch->newChildren.count(); i++)
    {
        climbTreeAndGetSpacers(currentBranch->newChildren[i]);
    }

    for(int i=0; i<currentBranch->oldChildren.count(); i++)
    {
        climbTreeAndGetSpacers(currentBranch->oldChildren[i]);
    }
}

QList<QList<TagData> > RGTagModel::getSpacers()
{
    d->savedSpacerList.clear();
    climbTreeAndGetSpacers(d->rootTag);

    return d->savedSpacerList;
}

void RGTagModel::addExternalTags(TreeBranch* parentBranch, int currentRow)
{
    QModelIndex parentIndex = createIndex(currentRow, 0, parentBranch);
    int howManyRows = rowCount(parentIndex);

    for(int i = 0; i < howManyRows; ++i)
    {
        QModelIndex currentIndex = index(i,0,parentIndex);
        TreeBranch* const currentBranch = branchFromIndex(currentIndex);  //currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;
        if(currentBranch == d->rootTag)
            return;

        addExternalTags(currentBranch,i);
    }
}

void RGTagModel::addAllExternalTagsToTreeView()
{
    addExternalTags(d->rootTag,0);
}

void RGTagModel::addAllSpacersToTag(const QModelIndex currentIndex,const QStringList spacerList, int spacerListIndex)
{
    if(spacerListIndex >= spacerList.count())
        return;

    TreeBranch* const currentBranch = branchFromIndex(currentIndex);  //currentIndex.isValid() ? static_cast<TreeBranch*>(currentIndex.internalPointer()) : d->rootTag;

    for(int i=0; i<currentBranch->spacerChildren.count(); ++i)
    {
        if(currentBranch->data == spacerList[spacerListIndex])
        {
            QModelIndex foundIndex = createIndex(i,0, currentBranch->spacerChildren[i]);
            addAllSpacersToTag(foundIndex, spacerList, spacerListIndex+1);
            return;
        }
    }

    addSpacerTag(currentIndex, spacerList[spacerListIndex]);
    QModelIndex newIndex = createIndex(currentBranch->spacerChildren.count()-1, 0, currentBranch->spacerChildren[currentBranch->spacerChildren.count()-1]);
    addAllSpacersToTag(newIndex, spacerList, spacerListIndex+1);
}

Type RGTagModel::getTagType(const QModelIndex& index) const
{
    const TreeBranch* const treeBranch = branchFromIndex(index);//  index.isValid() ? static_cast<TreeBranch*>(index.internalPointer()) : d->rootTag;

    return treeBranch->type;
}

} // namespace KIPIGPSSyncPlugin

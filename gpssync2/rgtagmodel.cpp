
#include "rgtagmodel.moc"

#include "rgtagmodel.h"
#include "kdebug.h"


namespace KIPIGPSSyncPlugin
{

class TreeBranch {
public:
    TreeBranch()
    : sourceIndex(),
      parent(0),
      data(),
      type(),
      children(),
      spacerChildren()
    {
    }

    ~TreeBranch()
    {
        qDeleteAll(children);
    }

    QPersistentModelIndex sourceIndex;
    TreeBranch* parent;
    QString data;
    RGTagModel::Type type;
    QList<TreeBranch*> children;
    QList<TreeBranch*> spacerChildren;
};


class RGTagModelPrivate
{
public:
    RGTagModelPrivate()
    : tagModel(),
      rootTag(0)
    {
    }

    QAbstractItemModel* tagModel;
    TreeBranch* rootTag;

    QModelIndex parent;
    int startInsert, endInsert;   
 
};

RGTagModel::RGTagModel(QAbstractItemModel* const externalTagModel, QObject* const parent)
: QAbstractItemModel(parent), d(new RGTagModelPrivate)
{
    d->tagModel = externalTagModel;
    d->rootTag = new TreeBranch();
   
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

void checkTree(TreeBranch* const checkBranch, int level)
{
    if(!checkBranch->sourceIndex.isValid())
        return;


    for(int j = 0; j < checkBranch->children.count(); ++j)
    {
        kDebug()<<"Index children:"<<checkBranch->sourceIndex<<"LEVEL:"<<level+1;
        checkTree(checkBranch->children[j], level+1);  
    }

    for(int j = 0; j < checkBranch->spacerChildren.count(); ++j)
    {
        kDebug()<<"Index spacer:"<<checkBranch->sourceIndex<<"LEVEL:"<<level+1;
        checkTree(checkBranch->spacerChildren[j], level+1);
    }
}

QModelIndex RGTagModel::fromSourceIndex(const QModelIndex& externalTagModelIndex) const
{
    if(!externalTagModelIndex.isValid())
        return QModelIndex();

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
            return createIndex(subModelBranch->sourceIndex.row()+subModelBranch->parent->spacerChildren.count(), subModelBranch->sourceIndex.column(), subModelBranch);

        }

        int where = -1;
        for (int i=0; i < subModelBranch->children.count(); ++i)
        {
            if(subModelBranch->children[i]->sourceIndex == parents[level])
            {
                where = i;
                break;
            }
        }        


        if(where >= 0)
        {
            subModelBranch = subModelBranch->children[where];
        }
        else
        {
            if (level>=parents.count())
                return QModelIndex();

            //TODO: check when rows are different
            TreeBranch* newTreeBranch = new TreeBranch();
            newTreeBranch->sourceIndex = parents[level];
            newTreeBranch->parent = subModelBranch;

            subModelBranch->children.append(newTreeBranch); 
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

    TreeBranch* const treeBranch = static_cast<TreeBranch*>(tagModelIndex.internalPointer());
    if(!treeBranch)
        return QModelIndex();

    return treeBranch->sourceIndex;
}

void RGTagModel::addSpacerTag(const QModelIndex& parent, const QString& spacerName)
{
    TreeBranch* const parentBranch = static_cast<TreeBranch*>(parent.internalPointer());

    TreeBranch* newSpacer = new TreeBranch();
    newSpacer->parent = parentBranch;
    newSpacer->data = spacerName;
    newSpacer->type = TypeSpacer;

    beginInsertRows(parent, parentBranch->spacerChildren.count(), parentBranch->spacerChildren.count());
    parentBranch->spacerChildren.append(newSpacer);
    //endInsertRows();
}

int RGTagModel::columnCount(const QModelIndex& parent) const
{
    //Change something here?
    TreeBranch* const parentBranch = static_cast<TreeBranch*>(parent.internalPointer());
    if(!parentBranch)
    {
        return 1;
    }    

    if(parentBranch && parentBranch->type == TypeSpacer)
        return 1;
    
    
    return d->tagModel->columnCount(toSourceIndex(parent));

}

bool RGTagModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return false;
}

QVariant RGTagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeBranch* const treeBranch = static_cast<TreeBranch*>(index.internalPointer());
    if ((!treeBranch) || (treeBranch->type != TypeSpacer))
    {
        return d->tagModel->data(toSourceIndex(index), role);
    }
    else if(role == Qt::DisplayRole)
    {
        return treeBranch->data;
    }

    return QVariant();
}

QModelIndex RGTagModel::index(int row, int column, const QModelIndex& parent) const
{
    if ( (column!=0) || (row<0) )
        return QModelIndex();

    TreeBranch* parentBranch = d->rootTag;
    if (parent.isValid())
        parentBranch = static_cast<TreeBranch*>(parent.internalPointer());

    // this should not happen!
    if (!parentBranch)
        return QModelIndex();

    if (row < parentBranch->spacerChildren.count())
    {
        return createIndex(row, column, parentBranch->spacerChildren[row]);
    } else {
        return fromSourceIndex(d->tagModel->index(row-parentBranch->spacerChildren.count(),column,toSourceIndex(parent)));
    }

    return QModelIndex();
}

QModelIndex RGTagModel::parent(const QModelIndex& index) const
{
    TreeBranch* const currentBranch = static_cast<TreeBranch*>(index.internalPointer());
    if (!currentBranch)
        return QModelIndex();

    if(currentBranch->type == TypeSpacer)
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
        else if (parentBranch->type==TypeChild)
        {
            // TODO: don't we have a function for this?
            for (int parentRow=0; parentRow<gParentBranch->children.count(); ++parentRow)
            {
                if(gParentBranch->children[parentRow] == parentBranch)
                {
                    return createIndex(parentRow+gParentBranch->spacerChildren.count(), 0, parentBranch);
                }
            }

            return QModelIndex();
        }

        
    }

    return fromSourceIndex(d->tagModel->parent(toSourceIndex(index)));
}

int RGTagModel::rowCount(const QModelIndex& parent) const
{
    TreeBranch* const parentBranch = parent.isValid() ? static_cast<TreeBranch*>(parent.internalPointer()) : d->rootTag;

    int myRowCount = parentBranch->spacerChildren.count();
    
    // TODO: we don't know whether the children have been set up, therefore query the source model
    if (parentBranch->type==TypeChild)
    {
        const QModelIndex sourceIndex = toSourceIndex(parent);
        myRowCount+=d->tagModel->rowCount(sourceIndex);
    }

    return myRowCount;
}


bool RGTagModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    return false;
}

QVariant RGTagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->tagModel->headerData(section, orientation, role);
}

Qt::ItemFlags RGTagModel::flags(const QModelIndex& index) const
{
    TreeBranch* const currentBranch = static_cast<TreeBranch*>(index.internalPointer());
    if (currentBranch && (currentBranch->type == TypeSpacer) )
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
    kDebug()<<"Entered columnsAboutToBeInserted";
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

    d->parent = fromSourceIndex(parent);
    d->startInsert = start;
    d->endInsert = end;

    beginInsertRows(fromSourceIndex(parent), start+parentBranch->spacerChildren.count(), end+parentBranch->spacerChildren.count());
}

void RGTagModel::slotRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow)
{
    beginMoveRows(fromSourceIndex(sourceParent), sourceStart, sourceEnd, fromSourceIndex(destinationParent), destinationRow );
}

void RGTagModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{

    beginRemoveRows(parent, start, end);
}

void RGTagModel::slotRowsInserted()
{
    
    TreeBranch* const parentBranch = d->parent.isValid() ? static_cast<TreeBranch*>(fromSourceIndex(d->parent).internalPointer()) : d->rootTag;

    

    for(int i=d->startInsert; i<d->endInsert; ++i)
    {
    TreeBranch* newBranch = new TreeBranch();
    newBranch->parent = parentBranch;
    newBranch->sourceIndex = d->tagModel->index(i,0,d->parent);

    parentBranch->children.insert(i,newBranch);

    }
    kDebug()<<"Entered rowsInserted";
    endInsertRows();
}

void RGTagModel::slotRowsMoved()
{
    endMoveRows();
}

void RGTagModel::slotRowsRemoved()
{
    endRemoveRows();
}

}    //KIPIGPSSyncPlugin

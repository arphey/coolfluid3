// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHeaderView>
#include <QMainWindow>
#include <QModelIndex>
#include <QMouseEvent>
#include <QRegExp>
#include <QSortFilterProxyModel>

#include "common/CF.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/CommitDetails.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"

#include "UI/Graphics/FilteringModel.hpp"
#include "UI/Graphics/ConfirmCommitDialog.hpp"
#include "UI/Graphics/CentralPanel.hpp"
#include "UI/Graphics/SignalManager.hpp"

#include "UI/Graphics/TreeView.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;
using namespace cf3::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

TreeView::TreeView(CentralPanel * optionsPanel, QMainWindow * parent,
                   bool contextMenuAllowed)
: QTreeView(parent),
  m_contextMenuAllowed(contextMenuAllowed)
{
  if(m_contextMenuAllowed && optionsPanel == nullptr)
    throw BadValue(FromHere(), "Options panel is a nullptr pointer");

  // instantiate class attributes
  m_modelFilter = new FilteringModel(this);
  m_centralPanel = optionsPanel;
  m_signalManager = new SignalManager(parent);

  m_modelFilter->setSourceModel(NTree::global().get());
  m_modelFilter->setDynamicSortFilter(true);

  this->setModel(m_modelFilter);

  this->setReadOnly(false);

  // when right clicking on the Client,
  // a "Context menu event" must be generated
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->header()->setResizeMode(QHeaderView::ResizeToContents);
  this->header()->setStretchLastSection(true);

  if(m_contextMenuAllowed)
  {
    connect(NTree::global().get(),
            SIGNAL(current_index_changed(QModelIndex, QModelIndex)),
            this,
            SLOT(current_index_changed(QModelIndex, QModelIndex)));
  }
}

//////////////////////////////////////////////////////////////////////////

TreeView::~TreeView()
{
  delete m_modelFilter;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::setReadOnly(bool readOnly)
{
  m_readOnly = readOnly;
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::isReadOnly() const
{
  return m_readOnly;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::selectedPath() const
{
  QModelIndex currentPath = this->selectionModel()->currentIndex();
  URI path;

  if(currentPath.isValid())
  {
    QModelIndex indexInModel = m_modelFilter->mapToSource(currentPath);

    path = NTree::global()->pathFromIndex(indexInModel);
  }

  return path;
}

//////////////////////////////////////////////////////////////////////////

URI TreeView::pathFromIndex(const QModelIndex & index)
{
  return NTree::global()->pathFromIndex(m_modelFilter->mapToSource(index));
}

//////////////////////////////////////////////////////////////////////////

QIcon TreeView::iconFromIndex(const QModelIndex & index)
{
  QModelIndex indexInModel = m_modelFilter->mapToSource(index);
  return NTree::global()->data(indexInModel, Qt::DecorationRole).value<QIcon>();
}

//////////////////////////////////////////////////////////////////////////

void TreeView::selectItem(const URI & path)
{
  QModelIndex index = NTree::global()->index_from_path(path);

  if(index.isValid())
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_modelFilter->mapFromSource(index);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeView::setFilter(const QString & pattern)
{
  QRegExp regex(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
  m_modelFilter->setFilterRegExp(regex);
  this->expandAll();
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::tryToCommit()
{
  bool committed = true;
  QModelIndex currentIndex = NTree::global()->current_index();

  if(currentIndex.isValid())
  {
    committed = confirmChangeOptions(currentIndex, true);
  }

  return committed;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mousePressEvent(QMouseEvent * event)
{
  QTreeView::mousePressEvent(event);
  QPoint mousePosition = event->pos() + this->geometry().topLeft();

  QModelIndex index = this->indexAt(mousePosition);
  NTree::Ptr tree = NTree::global();

  QModelIndex indexInModel = m_modelFilter->mapToSource(this->currentIndex());

  Qt::MouseButton button = event->button();

  try
  {
    if(m_contextMenuAllowed && indexInModel.isValid())
    {
      if(button == Qt::RightButton && this->confirmChangeOptions(index))
      {
        QList<ActionInfo> actions;
        URI path;

        tree->set_current_index(indexInModel);
        tree->list_node_actions(indexInModel, actions);
        path =  tree->current_path();

        m_signalManager->showMenu(QCursor::pos(), tree->node_by_path(path), actions);
      }
      else if(!tree->are_from_same_node(indexInModel, tree->current_index()))
      {
        if(this->confirmChangeOptions(index))
          tree->set_current_index(indexInModel);
        else
          this->current_index_changed(tree->current_index(), tree->current_index());
      }
    }
  }
  catch(Exception & e)
  {
    NLog::global()->add_exception(e.what());
  }
}

//////////////////////////////////////////////////////////////////////////

void TreeView::mouseDoubleClickEvent(QMouseEvent * event)
{
  if(event->button() == Qt::LeftButton)
    emit itemDoubleClicked(this->currentIndex());
}

//////////////////////////////////////////////////////////////////////////

void TreeView::keyPressEvent(QKeyEvent * event)
{
  NTree::Ptr tree= NTree::global();
  QModelIndex currentIndex = m_modelFilter->mapFromSource(tree->current_index());

  if(m_contextMenuAllowed)
  {
    if(event->key() == Qt::Key_Up)
    {
      if(this->confirmChangeOptions(currentIndex, true))
      {
        QModelIndex above = this->indexAbove(currentIndex);

        if(above.isValid())
          tree->set_current_index(m_modelFilter->mapToSource(above));
      }
    }
    else if(event->key() == Qt::Key_Down)
    {
      if(this->confirmChangeOptions(currentIndex, true))
      {
        QModelIndex below = this->indexBelow(currentIndex);

        if(below.isValid())
          tree->set_current_index(m_modelFilter->mapToSource(below));
      }
    }
    else
      QTreeView::keyPressEvent(event);
  }
  else
  {
    QTreeView::keyPressEvent(event);
    emit clicked(this->selectionModel()->currentIndex());
  }
}

//////////////////////////////////////////////////////////////////////////

bool TreeView::confirmChangeOptions(const QModelIndex & index, bool okIfSameIndex)
{
  bool confirmed = true;
  NTree::Ptr tree = NTree::global();

  if(!okIfSameIndex &&  tree->are_from_same_node(tree->current_index(), index))
    return confirmed;

  if(m_centralPanel->isModified())
  {
    CommitDetails commitDetails;

    ConfirmCommitDialog dlg;

    m_centralPanel->modifiedOptions(commitDetails);

    ConfirmCommitDialog::CommitConfirmation answer = dlg.show(commitDetails);

    if(answer == ConfirmCommitDialog::COMMIT)
      m_centralPanel->btApplyClicked();

    confirmed = answer != ConfirmCommitDialog::CANCEL;
  }

  return confirmed;
}

//////////////////////////////////////////////////////////////////////////

void TreeView::current_index_changed(const QModelIndex & newIndex,
                                   const QModelIndex & oldIndex)
{
  if(m_contextMenuAllowed)
  {
    QItemSelectionModel::SelectionFlags flags;
    QModelIndex indexInFilter = m_modelFilter->mapFromSource(newIndex);

    flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    this->selectionModel()->clearSelection();
    this->selectionModel()->select(indexInFilter, flags);
    this->selectionModel()->setCurrentIndex(indexInFilter, flags);
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

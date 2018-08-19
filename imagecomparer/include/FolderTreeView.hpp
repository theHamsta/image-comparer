#ifndef FOLDERTREEVIEW_HPP
#define FOLDERTREEVIEW_HPP
#include <QTreeView>

class FolderTreeView : public QTreeView
{
		Q_OBJECT
	public:
		FolderTreeView();
		FolderTreeView( FolderTreeView&& ) = default;
		FolderTreeView( const FolderTreeView& ) = default;
		FolderTreeView& operator=( FolderTreeView&& ) = default;
		FolderTreeView& operator=( const FolderTreeView& ) = default;
		~FolderTreeView();

	private:
};

FolderTreeView::FolderTreeView()
{
}

FolderTreeView::~FolderTreeView()
{
}
#endif /* FOLDERTREEVIEW_HPP */

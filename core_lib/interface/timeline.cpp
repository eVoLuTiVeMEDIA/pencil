/*

Pencil - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2013-2014 Matt Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation;

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "timeline.h"

#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QMessageBox>
#include <QLabel>

#include "layer.h"
#include "editor.h"
#include "layermanager.h"
#include "toolbox.h"
#include "timecontrols.h"
#include "timelinecells.h"


TimeLine::TimeLine( QWidget* parent ) : BaseDockWidget( parent, Qt::Tool )
{
}

void TimeLine::initUI()
{
    Q_ASSERT( editor() != nullptr );

    setWindowTitle( "Timeline" );

    QWidget* timeLineContent = new QWidget( this );

    connect( editor(), &Editor::currentFrameChanged, this, &TimeLine::updateFrame );

    mLayerList = new TimeLineCells( this, editor(), TIMELINE_CELL_TYPE::Layers );
    mTracks = new TimeLineCells( this, editor(), TIMELINE_CELL_TYPE::Tracks );

    connect( mLayerList, &TimeLineCells::mouseMovedY, mLayerList, &TimeLineCells::setMouseMoveY );
    connect( mLayerList, &TimeLineCells::mouseMovedY, mTracks,    &TimeLineCells::setMouseMoveY );

    hScrollBar = new QScrollBar( Qt::Horizontal );
    vScrollBar = new QScrollBar( Qt::Vertical );
    vScrollBar->setMinimum( 0 );
    vScrollBar->setMaximum( 1 );
    vScrollBar->setPageStep( 1 );
    updateLength( getFrameLength() );

    QWidget* leftWidget = new QWidget();
    leftWidget->setMinimumWidth( 120 );
    QWidget* rightWidget = new QWidget();

    QWidget* leftToolBar = new QWidget();
    leftToolBar->setFixedHeight( 31 );
    QWidget* rightToolBar = new QWidget();
    rightToolBar->setFixedHeight( 31 );

    // --- left widget ---
    // --------- layer buttons ---------
    QToolBar* layerButtons = new QToolBar( this );
    QLabel* layerLabel = new QLabel( tr( "Layers:" ) );
    layerLabel->setIndent( 5 );
    layerLabel->setFont( QFont( "Helvetica", 10 ) );

    QToolButton* addLayerButton = new QToolButton( this );
    addLayerButton->setIcon( QIcon( ":icons/add.png" ) );
    addLayerButton->setToolTip( "Add Layer" );
    addLayerButton->setFixedSize( 24, 24 );

    QToolButton* removeLayerButton = new QToolButton( this );
    removeLayerButton->setIcon( QIcon( ":icons/remove.png" ) );
    removeLayerButton->setToolTip( "Remove Layer" );
    removeLayerButton->setFixedSize( 24, 24 );

    layerButtons->addWidget( layerLabel );
    layerButtons->addWidget( addLayerButton );
    layerButtons->addWidget( removeLayerButton );

    QHBoxLayout* leftToolBarLayout = new QHBoxLayout();
    leftToolBarLayout->setAlignment( Qt::AlignLeft );
    leftToolBarLayout->setMargin( 0 );
    leftToolBarLayout->addWidget( layerButtons );
    leftToolBar->setLayout( leftToolBarLayout );

    QAction* newBitmapLayerAct = new QAction( QIcon( ":icons/layer-bitmap.png" ), tr( "New Bitmap Layer" ), this );
    QAction* newVectorLayerAct = new QAction( QIcon( ":icons/layer-vector.png" ), tr( "New Vector Layer" ), this );
    QAction* newSoundLayerAct = new QAction( QIcon( ":icons/layer-sound.png" ), tr( "New Sound Layer" ), this );
    QAction* newCameraLayerAct = new QAction( QIcon( ":icons/layer-camera.png" ), tr( "New Camera Layer" ), this );

    QMenu* layerMenu = new QMenu( tr( "&Layer" ), this );
    layerMenu->addAction( newBitmapLayerAct );
    layerMenu->addAction( newVectorLayerAct );
    layerMenu->addAction( newSoundLayerAct );
    layerMenu->addAction( newCameraLayerAct );
    addLayerButton->setMenu( layerMenu );
    addLayerButton->setPopupMode( QToolButton::InstantPopup );

    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->addWidget( leftToolBar, 0, 0 );
    leftLayout->addWidget( mLayerList, 1, 0 );
    leftLayout->setMargin( 0 );
    leftLayout->setSpacing( 0 );
    leftWidget->setLayout( leftLayout );

    // --- right widget ---
    // --------- key buttons ---------
    QToolBar* keyButtons = new QToolBar( this );
    QLabel* keyLabel = new QLabel( tr( "Keys:" ) );
    keyLabel->setFont( QFont( "Helvetica", 10 ) );
    keyLabel->setIndent( 5 );

    QToolButton* addKeyButton = new QToolButton( this );
    addKeyButton->setIcon( QIcon( ":icons/add.png" ) );
    addKeyButton->setToolTip( "Add Frame" );
    addKeyButton->setFixedSize( 24, 24 );

    QToolButton* removeKeyButton = new QToolButton( this );
    removeKeyButton->setIcon( QIcon( ":icons/remove.png" ) );
    removeKeyButton->setToolTip( "Remove Frame" );
    removeKeyButton->setFixedSize( 24, 24 );

    QToolButton* duplicateKeyButton = new QToolButton( this );
    duplicateKeyButton->setIcon( QIcon( ":icons/controls/duplicate.png" ) );
    duplicateKeyButton->setToolTip( "Duplicate Frame" );
    duplicateKeyButton->setFixedSize( 24, 24 );
    keyButtons->addWidget( keyLabel );
    keyButtons->addWidget( addKeyButton );
    keyButtons->addWidget( removeKeyButton );
    keyButtons->addWidget( duplicateKeyButton );

    // --------- Time controls ---------
    mTimeControls = new TimeControls( this );

    QHBoxLayout* rightToolBarLayout = new QHBoxLayout();
    rightToolBarLayout->addWidget( keyButtons );
    rightToolBarLayout->addStretch( 1 );
    rightToolBarLayout->addWidget( mTimeControls );
    rightToolBarLayout->setMargin( 0 );
    rightToolBarLayout->setSpacing( 0 );
    rightToolBar->setLayout( rightToolBarLayout );

    QGridLayout* rightLayout = new QGridLayout();
    rightLayout->addWidget( rightToolBar, 0, 0 );
    rightLayout->addWidget( mTracks, 1, 0 );
    rightLayout->setMargin( 0 );
    rightLayout->setSpacing( 0 );
    rightWidget->setLayout( rightLayout );

    // --- Splitter ---
    QSplitter* splitter = new QSplitter( this );
    splitter->addWidget( leftWidget );
    splitter->addWidget( rightWidget );
    splitter->setSizes( QList<int>() << 100 << 600 );


    QGridLayout* lay = new QGridLayout();
    lay->addWidget( splitter, 0, 0 );
    lay->addWidget( vScrollBar, 0, 1 );
    lay->addWidget( hScrollBar, 1, 0 );
    lay->setMargin( 0 );
    lay->setSpacing( 0 );
    timeLineContent->setLayout( lay );
    setWidget( timeLineContent );

    setWindowFlags( Qt::WindowStaysOnTopHint );

    connect( this, &TimeLine::lengthChange, mTracks, &TimeLineCells::lengthChange );
    connect( this, &TimeLine::fontSizeChange, mTracks, &TimeLineCells::fontSizeChange );
    connect( this, &TimeLine::frameSizeChange, mTracks, &TimeLineCells::frameSizeChange );
    connect( this, &TimeLine::labelChange, mTracks, &TimeLineCells::labelChange );
    connect( this, &TimeLine::scrubChange, mTracks, &TimeLineCells::scrubChange );

    connect( hScrollBar, &QScrollBar::valueChanged, mTracks, &TimeLineCells::hScrollChange );
    connect( vScrollBar, &QScrollBar::valueChanged, mTracks, &TimeLineCells::vScrollChange );
    connect( vScrollBar, &QScrollBar::valueChanged, mLayerList, &TimeLineCells::vScrollChange );

    connect( addKeyButton,    &QToolButton::clicked, this, &TimeLine::addKeyClick );
    connect( removeKeyButton, &QToolButton::clicked, this, &TimeLine::removeKeyClick );
    connect( duplicateKeyButton, &QToolButton::clicked, this, &TimeLine::duplicateKeyClick );

    connect( mTimeControls, SIGNAL( loopStartClick( int ) ), this, SIGNAL( loopStartClick( int ) ) );
    connect( mTimeControls, SIGNAL( loopEndClick( int ) ), this, SIGNAL( loopEndClick( int ) ) );

    connect( mTimeControls, SIGNAL( soundClick( bool ) ), this, SIGNAL( soundClick( bool ) ) );
    connect( mTimeControls, SIGNAL( fpsClick( int ) ), this, SIGNAL( fpsClick( int ) ) );

    connect( newBitmapLayerAct, &QAction::triggered, this, &TimeLine::newBitmapLayer );
    connect( newVectorLayerAct, &QAction::triggered, this, &TimeLine::newVectorLayer );
    connect( newSoundLayerAct, &QAction::triggered, this, &TimeLine::newSoundLayer );
    connect( newCameraLayerAct, &QAction::triggered, this, &TimeLine::newCameraLayer );
    connect( removeLayerButton, &QPushButton::clicked, this, &TimeLine::deleteCurrentLayer );
    
    mTimeControls->setCore( editor() );

    scrubbing = false;
}

void TimeLine::updateUI()
{
    mTracks->update();
    mLayerList->update();


}

int TimeLine::getFrameLength()
{
    return mTracks->getFrameLength();
}

void TimeLine::resizeEvent(QResizeEvent*)
{
    updateLayerView();
}

void TimeLine::deleteCurrentLayer()
{
    QString strLayerName = editor()->layers()->currentLayer()->name();

    int ret = QMessageBox::warning( this,
                                    tr( "Warning" ),
                                    tr( "Are you sure you want to delete layer: " ) + strLayerName + " ?",
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Ok );
    if ( ret == QMessageBox::Ok )
    {
        editor()->layers()->deleteCurrentLayer();
    }
}

void TimeLine::updateFrame( int frameNumber )
{
    Q_ASSERT ( mTracks );

    mTracks->updateFrame( mLastUpdatedFrame );
    mTracks->updateFrame( frameNumber );

    mLastUpdatedFrame = frameNumber;
}

void TimeLine::updateLayerView()
{
    vScrollBar->setPageStep( (height()-mTracks->getOffsetY()-hScrollBar->height())/mTracks->getLayerHeight() -2 );
    vScrollBar->setMinimum( 0 );
    vScrollBar->setMaximum( qMax(0, mNumLayers - vScrollBar->pageStep()) );
    update();
    updateContent();
}

void TimeLine::updateLayerNumber(int numberOfLayers)
{
    this->mNumLayers = numberOfLayers;
    updateLayerView();
}

void TimeLine::updateLength(int frameLength)
{
    hScrollBar->setMaximum( frameLength );
}

void TimeLine::updateContent()
{
    mLayerList->updateContent();
    mTracks->updateContent();
    update();
}

void TimeLine::setFps ( int value )
{
    mTimeControls->setFps(value);
}

void TimeLine::forceUpdateLength(QString newLength)
{
    bool ok;
    int dec = newLength.toInt(&ok, 10);

    if ( dec > getFrameLength())
    {
        updateLength(dec);
        updateContent();
        QSettings settings("Pencil","Pencil");
        settings.setValue("length", dec);
    }
}

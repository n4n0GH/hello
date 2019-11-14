//////////////////////////////////////////////////////////////////////////////
// breezewayconfigurationui.cpp
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezewayconfigwidget.h"
#include "breezewayexceptionlist.h"
#include "breezewaysettings.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>

namespace Breezeway
{

    //_________________________________________________________
    ConfigWidget::ConfigWidget( QWidget* parent, const QVariantList &args ):
        KCModule(parent, args),
        m_configuration( KSharedConfig::openConfig( QStringLiteral( "breezewayrc" ) ) ),
        m_changed( false )
    {

        // configuration
        m_ui.setupUi( this );

        // track ui changes
        connect( m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        // connect( m_ui.outlineCloseButton, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawBorderOnMaximizedWindows, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawSizeGrip, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawBackgroundGradient, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawTitleBarSeparator, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.borderRadius, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSpacing, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonMargin, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.matchTitleBarColor, SIGNAL(currentIndexChanged(int)), SLOT( updateChanged()) );
        connect( m_ui.titleBarHeight, SIGNAL(currentIndexChanged(int)), SLOT(
            updateChanged()) );
        connect( m_ui.alwaysShowButtonIcons, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.customColorBox, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.customColorSelect, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.drawHighlight, SIGNAL(clicked()), SLOT(updateChanged()) );

        // track animations changes
        connect( m_ui.animationsEnabled, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.animationsDuration, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

        // track shadows changes
        connect( m_ui.shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.inactiveShadowBehaviour, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );

        // track exception changes
        connect( m_ui.exceptions, SIGNAL(changed(bool)), SLOT(updateChanged()) );

    }

    //_________________________________________________________
    void ConfigWidget::load()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonSize->setCurrentIndex( m_internalSettings->buttonSize() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        // m_ui.outlineCloseButton->setChecked( m_internalSettings->outlineCloseButton() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.borderRadius->setCurrentIndex( m_internalSettings->borderRadius() );
        m_ui.buttonSpacing->setCurrentIndex( m_internalSettings->buttonSpacing() );
        m_ui.buttonMargin->setCurrentIndex( m_internalSettings->buttonMargin() );
        m_ui.matchTitleBarColor->setCurrentIndex( m_internalSettings->matchTitleBarColor() );
        m_ui.titleBarHeight->setCurrentIndex( m_internalSettings->titleBarHeight() );
        m_ui.alwaysShowButtonIcons->setChecked( m_internalSettings->alwaysShowButtonIcons() );
        m_ui.customColorBox->setChecked( m_internalSettings->customColorBox() );
        m_ui.customColorSelect->setColor( m_internalSettings->customColorSelect() );
        m_ui.drawHighlight->setChecked( m_internalSettings->drawHighlight() );

        // load shadows
        if( m_internalSettings->shadowSize() <= InternalSettings::ShadowVeryLarge ) m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        else m_ui.shadowSize->setCurrentIndex( InternalSettings::ShadowLarge );

        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );

        m_ui.inactiveShadowBehaviour->setCurrentIndex( m_internalSettings->inactiveShadowBehaviour() );

        // load exceptions
        ExceptionList exceptions;
        exceptions.readConfig( m_configuration );
        m_ui.exceptions->setExceptions( exceptions.get() );
        setChanged( false );

    }

    //_________________________________________________________
    void ConfigWidget::save()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // apply modifications from ui
        m_internalSettings->setTitleAlignment( m_ui.titleAlignment->currentIndex() );
        m_internalSettings->setButtonSize( m_ui.buttonSize->currentIndex() );
        // m_internalSettings->setOutlineCloseButton( m_ui.outlineCloseButton->isChecked() );
        m_internalSettings->setDrawBorderOnMaximizedWindows( m_ui.drawBorderOnMaximizedWindows->isChecked() );
        m_internalSettings->setDrawSizeGrip( m_ui.drawSizeGrip->isChecked() );
        m_internalSettings->setDrawBackgroundGradient( m_ui.drawBackgroundGradient->isChecked() );
        m_internalSettings->setAnimationsEnabled( m_ui.animationsEnabled->isChecked() );
        m_internalSettings->setAnimationsDuration( m_ui.animationsDuration->value() );
        m_internalSettings->setDrawTitleBarSeparator(m_ui.drawTitleBarSeparator->isChecked() );
        m_internalSettings->setBorderRadius( m_ui.borderRadius->currentIndex() );
        m_internalSettings->setButtonSpacing( m_ui.buttonSpacing->currentIndex() );
        m_internalSettings->setButtonMargin( m_ui.buttonMargin->currentIndex() );
        m_internalSettings->setMatchTitleBarColor( m_ui.matchTitleBarColor->currentIndex() );
        m_internalSettings->setTitleBarHeight( m_ui.titleBarHeight->currentIndex() );
        m_internalSettings->setAlwaysShowButtonIcons( m_ui.alwaysShowButtonIcons->isChecked() );
        m_internalSettings->setCustomColorBox( m_ui.customColorBox->isChecked() );
        m_internalSettings->setCustomColorSelect( m_ui.customColorSelect->color() );
        m_internalSettings->setDrawHighlight( m_ui.drawHighlight->isChecked() );

        m_internalSettings->setShadowSize( m_ui.shadowSize->currentIndex() );
        m_internalSettings->setShadowStrength( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) );
        m_internalSettings->setShadowColor( m_ui.shadowColor->color() );

        m_internalSettings->setInactiveShadowBehaviour( m_ui.inactiveShadowBehaviour->currentIndex() );

        // save configuration
        m_internalSettings->save();

        // get list of exceptions and write
        InternalSettingsList exceptions( m_ui.exceptions->exceptions() );
        ExceptionList( exceptions ).writeConfig( m_configuration );

        // sync configuration
        m_configuration->sync();
        setChanged( false );

        // needed to tell kwin to reload when running from external kcmshell
        {
            QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
            QDBusConnection::sessionBus().send(message);
        }

        // needed for breezeway style to reload shadows
        {
            QDBusMessage message( QDBusMessage::createSignal("/BreezewayDecoration",  "org.kde.Breezeway.Style", "reparseConfiguration") );
            QDBusConnection::sessionBus().send(message);
        }

    }

    //_________________________________________________________
    void ConfigWidget::defaults()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->setDefaults();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonSize->setCurrentIndex( m_internalSettings->buttonSize() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.borderRadius->setCurrentIndex( m_internalSettings->borderRadius() );
        m_ui.buttonSpacing->setCurrentIndex( m_internalSettings->buttonSpacing() );
        m_ui.buttonMargin->setCurrentIndex( m_internalSettings->buttonMargin() );
        m_ui.matchTitleBarColor->setCurrentIndex( m_internalSettings->matchTitleBarColor() );
        m_ui.titleBarHeight->setCurrentIndex( m_internalSettings->titleBarHeight() );
        m_ui.alwaysShowButtonIcons->setChecked( m_internalSettings->alwaysShowButtonIcons() );
        m_ui.customColorBox->setChecked( m_internalSettings->customColorBox() );
        m_ui.customColorSelect->setColor( m_internalSettings->customColorSelect() );
        m_ui.drawHighlight->setChecked( m_internalSettings->drawHighlight() );

        m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );

        m_ui.inactiveShadowBehaviour->setCurrentIndex( m_internalSettings->inactiveShadowBehaviour() );

    }

    //_______________________________________________
    void ConfigWidget::updateChanged()
    {

        // check configuration
        if( !m_internalSettings ) return;

        // track modifications
        bool modified( false );

        if (m_ui.drawTitleBarSeparator->isChecked() != m_internalSettings->drawTitleBarSeparator()) modified = true;
        if( m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment() ) modified = true;
        else if( m_ui.buttonSize->currentIndex() != m_internalSettings->buttonSize() ) modified = true;
        // else if( m_ui.outlineCloseButton->isChecked() != m_internalSettings->outlineCloseButton() ) modified = true;
        else if( m_ui.drawBorderOnMaximizedWindows->isChecked() !=  m_internalSettings->drawBorderOnMaximizedWindows() ) modified = true;
        else if( m_ui.drawSizeGrip->isChecked() !=  m_internalSettings->drawSizeGrip() ) modified = true;
        else if( m_ui.drawBackgroundGradient->isChecked() !=  m_internalSettings->drawBackgroundGradient() ) modified = true;
        else if( m_ui.borderRadius->currentIndex() != m_internalSettings->borderRadius() ) modified = true;
        else if( m_ui.buttonSpacing->currentIndex() != m_internalSettings->buttonSpacing() ) modified = true;
        else if( m_ui.buttonMargin->currentIndex() != m_internalSettings->buttonMargin() ) modified = true;
        else if( m_ui.matchTitleBarColor->currentIndex() != m_internalSettings->matchTitleBarColor() ) modified = true;
        else if( m_ui.titleBarHeight->currentIndex() != m_internalSettings->titleBarHeight() ) modified = true;
        else if( m_ui.alwaysShowButtonIcons->isChecked() != m_internalSettings->alwaysShowButtonIcons() ) modified = true;
        else if( m_ui.customColorBox->isChecked() != m_internalSettings->customColorBox() ) modified = true;
        else if( m_ui.customColorSelect->color() != m_internalSettings->customColorSelect() ) modified = true;
        else if( m_ui.drawHighlight->isChecked() != m_internalSettings->drawHighlight() ) modified = true;

        // animations
        else if( m_ui.animationsEnabled->isChecked() !=  m_internalSettings->animationsEnabled() ) modified = true;
        else if( m_ui.animationsDuration->value() != m_internalSettings->animationsDuration() ) modified = true;

        // shadows
        else if( m_ui.shadowSize->currentIndex() !=  m_internalSettings->shadowSize() ) modified = true;
        else if( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) != m_internalSettings->shadowStrength() ) modified = true;
        else if( m_ui.shadowColor->color() != m_internalSettings->shadowColor() ) modified = true;
        else if( m_ui.inactiveShadowBehaviour->currentIndex() != m_internalSettings->inactiveShadowBehaviour() ) modified = true;

        // exceptions
        else if( m_ui.exceptions->isChanged() ) modified = true;

        setChanged( modified );

    }

    //_______________________________________________
    void ConfigWidget::setChanged( bool value )
    {
        emit changed( value );
    }

}
